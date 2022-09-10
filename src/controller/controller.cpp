#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <raplcap.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include "util.h"
#include "controller.h"
// #include <asm/msr.h>

template <typename T, int MaxLen, typename Container>
void FixedQueue<T, MaxLen, Container>::push(const T& value) {
 if (this->size() == MaxLen) {
    this->c.pop_front();
  }
  std::queue<T, Container>::push(value);
}

/**
 * Controller default Constructor
 * 
 * Defualt cpu governor is powersave.
 */
Controller::Controller() {
  cpu_cores_count = sysconf(_SC_NPROCESSORS_ONLN);
  std::string str;
  std::ifstream max_freq_file(MAX_FREQ_FILE);
  std::ifstream min_freq_file(MIN_FREQ_FILE);
  std::ifstream max_short_file(MAX_PWR_SHORT_FILE);
  std::ifstream max_long_file(MAX_PWR_LONG_FILE);

  /* data structure init */
  for (int i=0; i<cpu_cores_count; i++) {
    cpu_freqs.push_back(0.);
  }

  /* init lock */
  if (pthread_mutex_init(&work_lock, NULL) != 0) {
    perror("init mutex");
    exit(1);
  }

  /* cpu freq config */
  std::getline(max_freq_file, str);
  cpu_max_freq = std::stoi(str);
  str.clear();
  std::getline(min_freq_file, str);
  cpu_min_freq = std::stoi(str);
  str.clear();

  /* rapl config */
  cpu_pkgs = raplcap_get_num_packages(NULL);
  if (cpu_pkgs == 0) {
    perror("raplcap_get_num_packages");
    exit(1);
  }
  if (raplcap_init(&cpu_rc)) {
    perror("raplcap_init");
    exit(1);
  }
  cpu_dies = raplcap_get_num_die(&cpu_rc, 0);
  if (cpu_dies == 0) {
    perror("raplcap_get_num_die");
    raplcap_destroy(&cpu_rc);
    exit(1);
  }
  std::getline(max_short_file, str);
  cpu_max_short_pc = 1.0 * std::stoi(str) / 1000000;
  str.clear();
  std::getline(max_long_file, str);
  cpu_max_long_pc = 1.0 * std::stoi(str) / 1000000;
  str.clear();
  if (raplcap_pd_get_limits(&cpu_rc, 0, 0, RAPLCAP_ZONE_PACKAGE, &cpu_long_pc, &cpu_short_pc)) {
    perror("raplcap_pd_get_limits");
    exit(1);
  }
  cpu_total_long_pc = cpu_long_pc.watts * cpu_pkgs * cpu_dies;
  cpu_total_short_pc = cpu_short_pc.watts * cpu_pkgs * cpu_dies;

  /* file close */
  max_freq_file.close();
  min_freq_file.close();
  max_short_file.close();
  max_long_file.close();
}

/************************ CPU freq related ******************************/
int Controller::SetCPUFreq(int cpu_freq, int core) {
  if (cpu_freq < cpu_min_freq || cpu_freq > cpu_max_freq) {
    perror("set frequency invalid");
    return 1;
  }
  
  int status;
  std::ostringstream cpupower_cmd;
  cpupower_cmd << "sudo cpupower -c " << core << " frequency-set -u " 
                << cpu_freq << "khz > /dev/null";
  status = system(cpupower_cmd.str().c_str());
  if (status == -1) {
    perror("system cpupower");
    return 1;
  }
  cpupower_cmd.str("");
  cpupower_cmd.clear();
  cpupower_cmd << "sudo cpupower -c " << core << " frequency-set -d " 
                << cpu_freq << "khz > /dev/null";
  status = system(cpupower_cmd.str().c_str());
  if (status == -1) {
    perror("system cpupower");
    return 1;
  }
  cpupower_cmd.str("");
  cpupower_cmd.clear();
  cpu_freqs[core] = cpu_freq;
  return 0;
}

int Controller::GetRealCPUFreq(int core) {
  std::string str;
  std::string freq_file = CUR_FREQ_FILE;
  freq_file = str.substr(0, 28) + std::to_string(core) + str.substr(29);
  std::ifstream cur_freq_file(freq_file);
  std::getline(cur_freq_file, str);
  double cpu_freq = std::stoi(str);
  str.clear();
  cur_freq_file.close();
  return cpu_freq;
}


/****************************  cpu powercap related ******************************/
int Controller::SetCPUPowercap(double cpu_total_long_pc, double cpu_total_short_pc = 0) {
  double cpu_long_pc_each =  cpu_total_long_pc / cpu_pkgs / cpu_dies;
  double cpu_short_pc_each =  cpu_total_short_pc / cpu_pkgs / cpu_dies;
  if (cpu_long_pc_each < 0 || cpu_long_pc_each > cpu_max_long_pc || 
      cpu_short_pc_each < 0 || cpu_short_pc_each > cpu_max_short_pc) {
    perror("invalid powercap");
    return 1;
  }

  this->cpu_total_long_pc = cpu_total_long_pc;
  cpu_long_pc.watts = cpu_long_pc_each;
  if (cpu_total_short_pc) {
    this->cpu_total_short_pc = cpu_total_short_pc;
    cpu_short_pc.watts = cpu_short_pc_each;
  }
  for (int i=0; i<cpu_pkgs; i++) {
    for (int j=0; j<cpu_dies; j++) {
      if (raplcap_pd_set_limits(&cpu_rc, i, j, RAPLCAP_ZONE_PACKAGE, &cpu_long_pc, &cpu_short_pc)) {
        perror("raplcap_pd_set_limits");
        return 1;
      }
    }
  }
  for (int i=0; i<cpu_pkgs; i++) {
    for (int j=0; j<cpu_dies; j++) {
      if (raplcap_pd_set_zone_enabled(&cpu_rc, i, j, RAPLCAP_ZONE_PACKAGE, 1)) {
        perror("raplcap_pd_set_zone_enabled");
        return 1;
      }
    }
  }
  return 0;
}

double Controller::GetCPULongPowerCap() {
  return cpu_long_pc.watts;
}

double Controller::GetCPUShortPowerCap() {
  return cpu_short_pc.watts;
}

double Controller::GetCPULongWindow() {
  return cpu_long_pc.seconds;
}

double Controller::GetCPUShortWindow() {
  return cpu_short_pc.seconds;
}

/**************************** CPU utilization ****************************/
double Controller::GetCPUMeanUtil(std::vector<double> utils) {
  int len_util = utils.size();
  double sum_util = 0.;
  double mean_util = 0.;
  double tmp_acc = 0.;
  double variance = 0.;
  
  sum_util = std::accumulate(utils.begin(), utils.end(), 0);
  mean_util = sum_util / len_util;
  std::for_each (utils.begin(), utils.end(), [&](const double d) {
    tmp_acc += (d-mean_util) * (d-mean_util);
  });
  variance = sqrt(tmp_acc / (len_util - 1));

  for (auto x:utils) {
    if (x > mean_util + 3*variance || x < mean_util - 3 * variance) {
      sum_util -= x;
      len_util--;
    }
  }
  return sum_util / len_util;
}

/******************************* uncore freq related ***********************/
int Controller::SetUncoreFreq(int uncore_freq) {
  int fd = open_msr(0);
  uint64_t results = read_msr(fd, UNCORE_RATIO_LIMIT);
  uint64_t reserved_bit = results >> 7 & 0x1;
  int ratio = uncore_freq / UNCORE_BASE_FREQ;
  uint64_t reg = ratio << 8 | reserved_bit << 7 | ratio;
  write_msr(fd, UNCORE_RATIO_LIMIT, reg);
  close_msr(fd);

  fd = open_msr(1);
  write_msr(fd, UNCORE_RATIO_LIMIT, reg);
  close_msr(fd);
  this->uncore_freq = uncore_freq;
  return 0;
}

int Controller::GetRealUncoreFreq(double intv) {
  timeval start_time, end_time;
  timespec interval;
  interval.tv_sec = (int)intv;
  interval.tv_nsec = (intv - (int)intv) * 1000000000;  

  int fd = open_msr(0);
  uint64_t results1 = read_msr(fd, MSR_U_PMON_UCLK_FIXED_CTR);
  gettimeofday(&start_time, NULL);

  nanosleep(&interval, NULL);

  uint64_t results2 = read_msr(fd, MSR_U_PMON_UCLK_FIXED_CTR);
  close_msr(fd);
  gettimeofday(&end_time, NULL);

  double diff_time = (end_time.tv_usec - start_time.tv_usec) / 1000000 + (end_time.tv_sec - start_time.tv_sec);
  return (int)(results2 - results1) / diff_time; // difftime is ns
}

int Controller::GetUncoreFreq() {
  if (uncore_freq)
    return uncore_freq;
  int fd = open_msr(0);
  uint64_t results = read_msr(fd, UNCORE_RATIO_LIMIT);
  close_msr(fd);
  int ratio = results & 0x7F;
  return UNCORE_BASE_FREQ * ratio;
}

/* dram power related */
int Controller::SetDRAMPowercap(double dram_pc) {
  int fd = open_msr(0);
  uint64_t result = read_msr(fd, MSR_DRAM_POWER_LIMIT);
  // double power_limit = (result & 0x7FFF) * msr_config.power_unit * 1000000;
  uint64_t set_point = dram_pc / msr_config.power_unit;
  uint64_t reg = (result & ~0x7FFF) | set_point;
  write_msr(fd, MSR_DRAM_POWER_LIMIT, reg);
  close_msr(fd);

  fd = open_msr(1);
  write_msr(fd, MSR_DRAM_POWER_LIMIT, reg);
  close_msr(fd);
  return 0;
}

/********************************** misc **************************************/
void Controller::SetCPUGovernor(const char *gov) {
  #pragma omp parallel for
  for (int i=0; i<cpu_cores_count; i++) {
    std::string str;
    std::fstream cur_gov_file;
    char gov_file[128];

    sprintf(gov_file, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", i);
    cur_gov_file.open(gov_file);
    std::getline(cur_gov_file, str);
    if (str != gov)
      cur_gov_file << gov;
    str.clear();
    cur_gov_file.close();
  }
}

void Controller::Run() {
  while (true) {
    std::vector<double> tmp_utils(monitor.vm_count);
    std::vector<double> tmp_ipcs(monitor.vm_count);
    std::vector<double> tmp_cpu_powers(monitor.vm_count);
    std::vector<double> tmp_cache_miss_rates(monitor.vm_count);
    std::vector<double> tmp_dram_powers(monitor.vm_count);
    #pragma omp parallel for num_threads(monitor.vm_count)
    for (int i=0; i<monitor.vm_count; i++) {
      tmp_utils[i] = monitor.GetCPUUtilFromDomain(monitor.vm_domains[i]);
      tmp_ipcs[i] = monitor.GetIPCFromCores(monitor.vm_cpus[i]);
      tmp_cpu_powers[i] = monitor.GetCPUPower();
      tmp_cache_miss_rates[i] = monitor.GetCacheMissRateFromCores(monitor.vm_cpus[i]);
      tmp_dram_powers[i] = monitor.GetDRAMPower();
    }
    FixedQueue<std::vector<double>, QUEUE_LEN> cpu_utils;
    cpu_utils.push(std::vector<double>(1,2));
    // pthread_mutex_lock(&work_lock);
    work_info.cpu_utils.size();
    // work_info.ipcs.push(tmp_ipcs);
    // work_info.cpu_powers.push(tmp_cpu_powers);
    // work_info.cpu_cache_miss_rates.push(tmp_cache_miss_rates);
    // work_info.cpu_dram_powers.push(tmp_dram_powers);
    // pthread_mutex_unlock(&work_lock);

    pthread_t s_thread;
    pthread_create(&s_thread, NULL, Schedule, &work_info);
  }
}

void *Controller::Schedule(void *workload) {
  pthread_mutex_lock(&work_lock);
  WorkloadInfo tmp_workload = *((WorkloadInfo *)workload);
  pthread_mutex_unlock(&work_lock);
  return NULL;
}

Controller::~Controller() {
  if (raplcap_destroy(&cpu_rc)) {
    perror("raplcap_destroy");
  }
  pthread_mutex_destroy(&work_lock);
}



/*** deprecated ***/
// void* Controller::Schedule(void *workload) {
//   if (cpu_governor != governor::MANUAL)
//     return;

//   double cpu_cur_power;
//   while (true) {
//     cpu_cur_power = GetCPUCurPower();
//     if (cpu_cur_power < cpu_total_long_pc) {
//       if (cpu_freq == cpu_max_freq)
//         break;
      
//       cpu_freq += 100000;
//       if (SetCPUFreq(cpu_freq))
//         exit(1);
//       sleep(0.5);
//     } else {
//       if (cpu_freq == cpu_min_freq)
//         break;
//       cpu_freq -= 100000;
//       if (SetCPUFreq(cpu_freq))
//         exit(1);
//       break;
//     }
//   }
// }

// double Controller::GetCPUUtil() {
//   char tmp_buf[10];
//   char util_cmd[128];
//   sprintf(util_cmd, "top -b -n 2 -d 0.2 -p %d | tail -1 | awk '{print $9}'", pid);
//   FILE *fp = popen(util_cmd, "r");
//   if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
//     sscanf(tmp_buf, "%lf", &cpu_util);
//   }
//   pclose(fp);
//   return cpu_util;
// }
// /************************ CPU cores related *****************************/
// int Controller::GetCurCPUCores() {
//   if (cpu_cur_cores > 0 && cpu_cur_cores < cpu_total_cores)
//     return cpu_cur_cores;
//   char tmp_buf[64];
//   std::ostringstream docker_cores_cmd;
//   docker_cores_cmd << "sudo docker inspect " << cid << " | grep CpusetCpus";
//   FILE *fp = popen(docker_cores_cmd.str().c_str(), "r");
//   if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
//     std::string str = tmp_buf;
//     std::size_t found = str.find('-');
//     std::string core_str = str.substr(found+1, str.length()-found-3);
//     cpu_cur_cores = std::stoi(core_str) + 1;
//   }
//   pclose(fp);
//   return cpu_cur_cores;
// }

// int Controller::BindCPUCores() {
//   std::ostringstream docker_cores_cmd;
//   docker_cores_cmd << "sudo docker update --cpuset-cpus 0-" << cpu_cur_cores-1 << " " << cid << " > /dev/null";
//   if (system(docker_cores_cmd.str().c_str()) == -1) {
//     perror("system docker update");
//     return 1;
//   }
//   return 0;
// }
// /************************ container related *****************************/
// void Controller::BindContainer() {
//   cid = "3e874384b986";
//   GetPIDFromCID(cid);
//   GetCurCPUCores();
// }

// void Controller::BindContainer(std::string container_id) {
//   cid = container_id;
//   GetPIDFromCID(cid);
//   GetCurCPUCores();
// }

// int Controller::GetPIDFromCID(std::string container_id) {
//   char tmp_buf[32];
//   std::ostringstream docker_top_ps;
//   docker_top_ps << "sudo docker top " << container_id << " | tail -1";
//   FILE *fp = popen(docker_top_ps.str().c_str(), "r");
//   if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
//     char *token = strtok(tmp_buf, " ");
//     token = strtok(NULL, " ");
//     sscanf(token, "%d", &pid);
//   }
//   pclose(fp);
//   return pid;
// }

// std::string Controller::GetContainerID() {
//   return cid;
// }

// /**
//  * perf event wrapper
//  */
// static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
// 			    int cpu, int group_fd, unsigned long flags) {
// 	int ret;
// 	ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
// 	               group_fd, flags);
// 	return ret;
// }