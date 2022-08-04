#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "controller.h"
#include <raplcap.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include "util.h"
// #include <asm/msr.h>


/**
 * perf event wrapper
 */
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
			    int cpu, int group_fd, unsigned long flags) {
	int ret;
	ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
	               group_fd, flags);
	return ret;
}

/**
 * Controller default Constructor
 * 
 * Defualt cpu governor is powersave.
 */
Controller::Controller() {
  cpu_total_cores = sysconf(_SC_NPROCESSORS_ONLN);

  std::string str;
  std::ifstream max_freq_file(MAX_FREQ_FILE);
  std::ifstream min_freq_file(MIN_FREQ_FILE);
  std::ifstream max_short_file(MAX_PWR_SHORT_FILE);
  std::ifstream max_long_file(MAX_PWR_LONG_FILE);
  // std::ifstream cur_short_file(CUR_PWR_SHORT_FILE);
  // std::ifstream cur_long_file(CUR_PWR_LONG_FILE);

  // cpu freq config
  std::getline(max_freq_file, str);
  cpu_max_freq = std::stoi(str);
  str.clear();
  std::getline(min_freq_file, str);
  cpu_min_freq = std::stoi(str);
  str.clear();
  GetCPUFreq();

  // set defualt governor to be powersave
  SetCPUGovernor("powersave");
  cpu_governor = governor::POWERSAVE;

  // rapl config
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

  // file close and gb
  max_freq_file.close();
  min_freq_file.close();
  max_short_file.close();
  max_long_file.close();
  // cur_short_file.close();
  // cur_long_file.close();
}

/**
 * Controller constructor
 * 
 * DVFS governor settings: manual or intel_pstate(performance or powersave)
 * Set cpu freq and cpu powercap (long/short term)
 */
Controller::Controller(governor cpu_governor, double cpu_total_long_pc = 0, double cpu_total_short_pc = 0, int cpu_freq = 0) {
  Controller();
  
  // cpu governor config
  switch (cpu_governor) {
  case governor::POWERSAVE: {
    break;
  }
  case governor::PERFORMANCE: {
    SetCPUGovernor("performance");
    this->cpu_governor = governor::PERFORMANCE;
    break;
  }
  case governor::MANUAL: {
    SetCPUGovernor("performance");
    this->cpu_governor = governor::MANUAL;
    if (SetCPUFreq(cpu_freq))
      exit(1);
    break;
  }
  default: {
    perror("input governor invalid");
    exit(1);
  }
  }

  // set powercap
  if (SetCPUPowercap(cpu_total_long_pc, cpu_total_short_pc))
    exit(1);
}

/************************ container related *****************************/
void Controller::BindContainer(std::string container_id = "96edd256c25b") {
  cid = container_id;
  GetContainerPID(cid);
  GetCurCPUCores();
}

/************************ CPU cores related *****************************/
int Controller::BindCPUCores() {
  std::ostringstream docker_cores_cmd;
  docker_cores_cmd << "sudo docker update --cpuset-cpus 0-" << cpu_cur_cores-1 << " " << cid << " > /dev/null";
  if (system(docker_cores_cmd.str().c_str()) == -1) {
    perror("system docker update");
    return 1;
  }
  return 0;
}

int Controller::GetCurCPUCores() {
  if (cpu_cur_cores)
    return cpu_cur_cores;
  char tmp_buf[64];
  std::ostringstream docker_cores_cmd;
  docker_cores_cmd << "sudo docker inspect " << cid << " | grep CpusetCpus";
  FILE *fp = popen(docker_cores_cmd, "r");
  if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
    std::string str = tmp_buf;
    std::size_t found = str.find('-');
    std::string core_str = str.substr(found+1, str.length()-found-3);
    cpu_cur_cores = std::stoi(core_str);
  }
  pclose(fp);
  return cpu_cur_cores;
}

/************************ CPU freq related ******************************/
int Controller::SetCPUFreq(int cpu_freq) {
  if (cpu_freq == 0)
    cpu_freq =  cpu_min_freq;
  if (cpu_freq < cpu_min_freq || cpu_freq > cpu_max_freq) {
    perror("input frequency invalid");
    return 1;
  }
  
  int status;
  std::ostringstream cpupower_cmd;
  #pragma omp parallel for
  for (int i=0; i<cpu_cur_cores; i++) {
    cpupower_cmd << "sudo cpupower -c " << i << " frequency-set -u " 
                 << cpu_freq << "khz > /dev/null";
    status = system(cpupower_cmd.str().c_str());
    if (status == -1) {
      perror("system cpupower");
    }
    cpupower_cmd.clear();
    cpupower_cmd << "sudo cpupower -c " << i << " frequency-set -d " 
                 << cpu_freq << "khz > /dev/null";
    status = system(cpupower_cmd.str().c_str());
    if (status == -1) {
      perror("system cpupower");
    }
    cpupower_cmd.clear();
  }
  this->cpu_freq = cpu_freq;
  return 0;
}

int Controller::GetMaxCPUFreq() {
  return cpu_max_freq;
}

int Controller::GetMinCPUFreq() {
  return cpu_min_freq;
}

int Controller::GetCPUFreq() {
  std::string str;
  std::ifstream cur_freq_file(CUR_FREQ_FILE);
  std::getline(cur_freq_file, str);
  cpu_freq = std::stoi(str);
  str.clear();
  cur_freq_file.close();
  return cpu_freq;
}

/****************************  cpu powercap related ******************************/
int Controller::SetCPUPowercap(double cpu_total_long_pc, double cpu_total_short_pc = 0) {
  if (cpu_total_long_pc == 0)
    return 0;

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

double Controller::GetCPUCurPower() {
  timeval start_time, end_time;
  timespec interval;
  interval.tv_sec = 0;
  interval.tv_nsec = 100000000; // 100ms

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
double Controller::GetCPUUtil() {
  char tmp_buf[10];
  char util_cmd[128];
  sprintf(util_cmd, "top -b -n 2 -d 0.2 -p %d | tail -1 | awk '{print $9}'", pid);
  FILE *fp = popen(util_cmd, "r");
  if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
    sscanf(tmp_buf, "%lf", &cpu_util);
  }
  pclose(fp);
  return cpu_util;
}

double Controller::GetCPUMeanUtil() {
  std::vector<double> tmp_total_utils(5, 0.);
  int len_total_util = tmp_total_utils.size();
  double sum_total_util = 0.;
  double mean_total_util = 0.;
  double tmp_acc = 0.;
  double variance = 0.;
  
  for (auto &x:tmp_total_utils) {
    x = GetCPUUtil();
    sum_total_util += x;
  }
  mean_total_util = sum_total_util / len_total_util;
  std::for_each (tmp_total_utils.begin(), tmp_total_utils.end(), [&](const double d) {
    tmp_acc += (d-mean_total_util) * (d-mean_total_util);
  });
  variance = sqrt(tmp_acc / (len_total_util - 1));

  for (auto &x:tmp_total_utils) {
    if (x > mean_total_util + 3*variance || x < mean_total_util - 3 * variance) {
      sum_total_util -= x;
      len_total_util--;
    }
  }
  return sum_total_util / len_total_util;
}

/*************************** CPU counter related **************************/
double Controller::GetCPUIPC() {
  char tmp_buf[256];
  char ipc_cmd[256];
  sprintf(ipc_cmd, "sudo perf stat -e cycles,instructions -p %d sleep 0.2 2>&1 | awk 'NR==5{print}'", pid);
  FILE *fp = popen(ipc_cmd, "r");
  if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
    char *token = strtok(tmp_buf, " ");
    for (int i=0; i<3; i++) {
      token = strtok(NULL, " ");
    }
    sscanf(token, "%lf", &cpu_ipc);
  }
  pclose(fp);
  return cpu_ipc;
}

double Controller::GetCPUCacheMissRate() {
  char tmp_buf[256];
  char cache_cmd[256];
  sprintf(cache_cmd, "sudo perf stat -e cache-misses,cache-references -p %d sleep 0.6 2>&1 | awk 'NR==4{print}'", pid);
  FILE *fp = popen(cache_cmd, "r");
  if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
    char *token = strtok(tmp_buf, " ");
    for (int i=0; i<3; i++) {
      token = strtok(NULL, " ");
    }
    sscanf(token, "%lf", &cpu_miss_rate);
  }
  pclose(fp);
  return cpu_miss_rate;
}

/******************************* uncore freq related ***********************/
int Controller::SetUncoreFreq(int uncore_freq) {
  int fd = open_msr(0);
  uint64_t results = read_msr(fd, UNCORE_RATIO_LIMIT);
  uint64_t other_bits = results >> 7;
  int ratio = uncore_freq / UNCORE_BASE_FREQ;
  uint64_t reg = other_bits << 7 | ratio;
  write_msr(fd, UNCORE_RATIO_LIMIT, reg);
  return 0;
}

int Controller::GetUncoreFreq() {
  int fd = open_msr(0);
  uint64_t results = read_msr(fd, UNCORE_RATIO_LIMIT);
  int ratio = results & 0x7F;
  return UNCORE_BASE_FREQ * ratio;
}

/********************************** misc **************************************/
void Controller::SetCPUGovernor(const char *gov) {
  std::string str;
  std::fstream cur_gov_file;
  char *gov_file = (char *)malloc(strlen(CUR_GOV_FILE) + 1);

  #pragma omp parallel for
  for (int i=0; i<cpu_cur_cores; i++) {
    sprintf(gov_file, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", i);
    cur_gov_file.open(gov_file);
    std::getline(cur_gov_file, str);
    if (str != gov)
      cur_gov_file << gov;
    str.clear();
    cur_gov_file.close();
  }
  free(gov_file);
}

int Controller::GetContainerPID(std::string container_id) {
  char tmp_buf[32];
  std::ostringstream docker_top_ps;
  docker_top_ps << "sudo docker top " << container_id << " | tail -1";
  FILE *fp = popen(docker_top_ps, "r");
  if (fgets(tmp_buf, sizeof(tmp_buf), fp)) {
    sscanf(tmp_buf, "%d", &pid);
  }
  pclose(fp);
  return pid;
}

void Controller::Schedule() {
  if (cpu_governor != governor::MANUAL)
    return;

  double cpu_cur_power;
  while (true) {
    cpu_cur_power = GetCPUCurPower();
    if (cpu_cur_power < cpu_total_long_pc) {
      if (cpu_freq == cpu_max_freq)
        break;
      
      cpu_freq += 100000;
      if (SetCPUFreq(cpu_freq))
        exit(1);
      sleep(0.5);
    } else {
      if (cpu_freq == cpu_min_freq)
        break;
      cpu_freq -= 100000;
      if (SetCPUFreq(cpu_freq))
        exit(1);
      break;
    }
  }
}

Controller::~Controller() {
  if (raplcap_destroy(&cpu_rc)) {
    perror("raplcap_destroy");
  }
}
