#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/time.h>
#include <libvirt/libvirt.h>
#include <raplcap.h>
#include "monitor.h"
#include "util.h"

/**
 * Monitor default constructor
 * 
 */
Monitor::Monitor() {
  // init energy unit
  int fd = open_msr(0);
  uint64_t rapl_power_unit = read_msr(fd, MSR_RAPL_POWER_UNIT);
  energy_unit = pow(0.5, (double)((rapl_power_unit >> 8) & 0x1F));

  vm_ptr = virConnectOpen(NULL);
  int cpu_cores_count = sysconf(_SC_NPROCESSORS_ONLN);
  int maplen = VIR_CPU_MAPLEN(cpu_cores_count);
  unsigned char *cpumap = (unsigned char *)calloc(maplen, 1);
  if (raplcap_init(&cpu_rc)) {
    perror("raplcap_init");
    return;
  }

  int *ids = (int *)calloc(VM_ID_LIMIT, sizeof(int)); 
  vm_count = virConnectListDomains(vm_ptr, ids, VM_ID_LIMIT);
  if (vm_count < 0) {
    perror("virConnectListDomains");
    return;
  }

  for (int i=0; i<vm_count; i++) {
    // init vm ids and domains
    vm_ids.push_back(ids[i]);
    vm_domains.push_back(virDomainLookupByID(vm_ptr, ids[i]));

    // init vm cores
    vm_cpus.push_back(std::vector<int>());
    memset(cpumap, 0, maplen);
    virDomainGetVcpuPinInfo(vm_domains[i], 1, cpumap, maplen, VIR_DOMAIN_AFFECT_CURRENT);
    for (int j=0; j<cpu_cores_count; j++) {
      if (VIR_CPU_USED(cpumap, j)) {
        vm_cpus[i].push_back(j);
      }
    }

    // init log files
    std::string log_str = "./log/log";
    log_str.append(std::to_string(i));
    log_str.append(".txt");
    log_files.push_back(log_str);
  }

  free(ids);
}

/******************************* CPU related **********************************/
/**
 * Get CPU utilization of a domain
 * 
 */
double Monitor::GetCPUUtilFromDomain(virDomainPtr domain, double intv) {
  timeval start_time, end_time;
  timespec interval;
  interval.tv_sec = (int)intv;
  interval.tv_nsec = (intv - (int)intv) * 1000000000;
  
  virDomainInfo info[2];
  virDomainGetInfo(domain, &info[0]);
  gettimeofday(&start_time, NULL);

  nanosleep(&interval, NULL);

  virDomainGetInfo(domain, &info[1]);
  gettimeofday(&end_time, NULL);

  double diff_time = (end_time.tv_usec - start_time.tv_usec) * 1000 + (end_time.tv_sec - start_time.tv_sec) * 1000000000;
  return (info[1].cpuTime - info[0].cpuTime) * 100. / diff_time; // virdomain return ns, percentage 100, difftime is ns
}

/**
 * Get IPC from cores
 * 
 */
double Monitor::GetIPCFromCores(std::vector<int> cores) {
  char tmp_buf[256];
  char ipc_cmd[256];
  double retv;
  sprintf(ipc_cmd, "sudo perf stat -e cycles,instructions --cpu %d-%d sleep 0.2 2>&1 | awk 'NR==5{print}'", cores[0], cores[cores.size()-1]);
  FILE *fp = popen(ipc_cmd, "r");
  if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
    char *token = strtok(tmp_buf, " ");
    for (int i=0; i<3; i++) {
      token = strtok(NULL, " ");
    }
    sscanf(token, "%lf", &retv);
  }
  pclose(fp);
  return retv;
}


/**
 * Get CPU power from cores
 * 
 */
double Monitor::GetCPUPower(double intv) {
  timeval start_time, end_time;
  timespec interval;
  interval.tv_sec = (int)intv;
  interval.tv_nsec = (intv - (int)intv) * 1000000000;

  double energy1_before = raplcap_pd_get_energy_counter(&cpu_rc, 0, 0, RAPLCAP_ZONE_PACKAGE);
  double energy2_before = raplcap_pd_get_energy_counter(&cpu_rc, 1, 0, RAPLCAP_ZONE_PACKAGE);
  gettimeofday(&start_time, NULL);

  nanosleep(&interval, NULL);

  double energy1_after = raplcap_pd_get_energy_counter(&cpu_rc, 0, 0, RAPLCAP_ZONE_PACKAGE);
  double energy2_after = raplcap_pd_get_energy_counter(&cpu_rc, 1, 0, RAPLCAP_ZONE_PACKAGE); 
  gettimeofday(&end_time, NULL);

  double diff_time = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;
  double power1 = (energy1_after - energy1_before) / diff_time * 1000000;
  double power2 = (energy2_after - energy2_before) / diff_time * 1000000;
  return power1+power2;
}


/******************************* Memory related **********************************/
/**
 * Get cache miss rate from cores
 * 
 */
double Monitor::GetCacheMissRateFromCores(std::vector<int> cores) {
  char tmp_buf[256];
  char cache_cmd[256];
  double retv;
  sprintf(cache_cmd, "sudo perf stat -e cache-misses,cache-references --cpu %d-%d sleep 0.6 2>&1 | awk 'NR==4{print}'", cores[0], cores[cores.size()-1]);
  FILE *fp = popen(cache_cmd, "r");
  if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
    char *token = strtok(tmp_buf, " ");
    for (int i=0; i<3; i++) {
      token = strtok(NULL, " ");
    }
    sscanf(token, "%lf", &retv);
  }
  pclose(fp);
  return retv;
}

/**
 * Get DRAM power
 * 
 */
double Monitor::GetDRAMPower(double intv) {
  timeval start_time, end_time;
  timespec interval;
  interval.tv_sec = (int)intv;
  interval.tv_nsec = (intv - (int)intv) * 1000000000;

  int fd1 = open_msr(0);
  int fd2 = open_msr(1);
  double energy1_before = (read_msr(fd1, MSR_DRAM_ENERGY_STATUS) & 0xFFFFFFFF) * energy_unit;
  double energy2_before = (read_msr(fd2, MSR_DRAM_ENERGY_STATUS) & 0xFFFFFFFF) * energy_unit;
  gettimeofday(&start_time, NULL);

  nanosleep(&interval, NULL);

  double energy1_after = (read_msr(fd1, MSR_DRAM_ENERGY_STATUS) & 0xFFFFFFFF) * energy_unit;
  double energy2_after = (read_msr(fd2, MSR_DRAM_ENERGY_STATUS) & 0xFFFFFFFF) * energy_unit;
  gettimeofday(&end_time, NULL);

  double diff_time = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;
  double power1 = (energy1_after - energy1_before) / diff_time * 1000000;
  double power2 = (energy2_after - energy2_before) / diff_time * 1000000;

  close_msr(fd1);
  close_msr(fd2);
  return power1+power2;
}


/**
 * Monitor for loop routine
 * 
 */
void Monitor::Run() {
  #pragma omp parallel for num_threads(vm_count)
  for (int i=0; i<vm_count; i++) {
    std::ofstream log_f;
    log_f.open(log_files[i], std::ios::out | std::ios::trunc);
    log_f << std::left << std::setw(10) << "vm_id" 
          << std::setw(15) << "cpu_cores"
          << std::setw(15) << "cpu_util"
          << std::setw(15) << "cpu_ipcs"
          << std::setw(15) << "cpu_power"
          << std::setw(20) << "cache_miss_rate"
          << std::setw(15) << "dram_power" << std::endl;
    #ifdef DEBUG
    for (int j=0; j<10; j++) {
    #else
    while (true) {
    #endif
      double tmp_util = GetCPUUtilFromDomain(vm_domains[i]);
      double tmp_ipcs = GetIPCFromCores(vm_cpus[i]);
      double tmp_cpu_power = GetCPUPower();
      double tmp_cache_miss_rate = GetCacheMissRateFromCores(vm_cpus[i]);
      double tmp_dram_power = GetDRAMPower();
      std::string tmp_cores = std::to_string(vm_cpus[i][0]) + "-" + std::to_string(vm_cpus[i][vm_cpus[i].size()-1]);
      log_f << std::left << std::setw(10) << vm_ids[i] 
            << std::setw(15) << tmp_cores
            << std::setw(15) << tmp_util 
            << std::setw(15) << tmp_ipcs 
            << std::setw(15) << tmp_cpu_power 
            << std::setw(20) << tmp_cache_miss_rate 
            << std::setw(15) << tmp_dram_power << std::endl;
    }
    log_f.close();
  }
}

/**
 * Monitor deconstructor
 * 
 */
Monitor::~Monitor() {
  free(vm_ptr);
  for (auto domain:vm_domains) {
    free(domain);
  }
  raplcap_destroy(&cpu_rc);
}

#ifdef DEBUG
/**
 * Get CPU utilization of all VMs
 * 
 * @return CPU utilization 
 */
std::vector<double> Monitor::GetAllCPUUtil(double interval) {
  std::vector<double> retv;
  for (int i=0; i<vm_count; i++) {
    retv.push_back(GetCPUUtilFromDomain(vm_domains[i], interval));
  }
  return retv;
}
#endif