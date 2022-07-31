#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "controller.h"
#include <raplcap.h>

/**
 * Defualt cpu governor is powersave.
 */
Controller::Controller() {
  cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);

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
  if (raplcap_init(&rc)) {
    perror("raplcap_init");
    exit(1);
  }
  cpu_dies = raplcap_get_num_die(&rc, 0);
  if (cpu_dies == 0) {
    perror("raplcap_get_num_die");
    raplcap_destroy(&rc);
    exit(1);
  }
  std::getline(max_short_file, str);
  cpu_max_short_pc = 1.0 * std::stoi(str) / 1000000;
  str.clear();
  std::getline(max_long_file, str);
  cpu_max_long_pc = 1.0 * std::stoi(str) / 1000000;
  str.clear();
  if (raplcap_pd_get_limits(&rc, 0, 0, RAPLCAP_ZONE_PACKAGE, &rl_long, &rl_short)) {
    perror("raplcap_pd_get_limits");
    exit(1);
  }

  // file close and gb
  max_freq_file.close();
  min_freq_file.close();
  max_short_file.close();
  max_long_file.close();
  // cur_short_file.close();
  // cur_long_file.close();
}

Controller::Controller(governor cpu_governor, double cpu_long_pc = 0, double cpu_short_pc = 0, int cpu_freq = 0) {
  cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);

  std::string str;
  std::ifstream max_freq_file(MAX_FREQ_FILE);
  std::ifstream min_freq_file(MIN_FREQ_FILE);
  std::ifstream max_short_file(MAX_PWR_SHORT_FILE);
  std::ifstream max_long_file(MAX_PWR_LONG_FILE);

  // cpu freq config
  std::getline(max_freq_file, str);
  cpu_max_freq = std::stoi(str);
  str.clear();
  std::getline(min_freq_file, str);
  cpu_min_freq = std::stoi(str);
  str.clear();
  GetCPUFreq();

  // rapl config
  cpu_pkgs = raplcap_get_num_packages(NULL);
  if (cpu_pkgs == 0) {
    perror("raplcap_get_num_packages");
    exit(1);
  }
  if (raplcap_init(&rc)) {
    perror("raplcap_init");
    exit(1);
  }
  cpu_dies = raplcap_get_num_die(&rc, 0);
  if (cpu_dies == 0) {
    perror("raplcap_get_num_die");
    raplcap_destroy(&rc);
    exit(1);
  }
  std::getline(max_short_file, str);
  cpu_max_short_pc = 1.0 * std::stoi(str) / 1000000;
  str.clear();
  std::getline(max_long_file, str);
  cpu_max_long_pc = 1.0 * std::stoi(str) / 1000000;
  str.clear();
  if (raplcap_pd_get_limits(&rc, 0, 0, RAPLCAP_ZONE_PACKAGE, &rl_long, &rl_short)) {
    perror("raplcap_pd_get_limits");
    exit(1);
  }

  // cpu governor config
  switch (cpu_governor) {
  case governor::POWERSAVE: {
    SetCPUGovernor("powersave");
    this->cpu_governor = governor::POWERSAVE;
  }
  case governor::PERFORMANCE: {
    SetCPUGovernor("performance");
    this->cpu_governor = governor::PERFORMANCE;
  }
  case governor::MANUAL: {
    SetCPUGovernor("performance");
    this->cpu_governor = governor::MANUAL;
    if (SetCPUFreq(cpu_freq))
      exit(1);
  }
  default: {
    perror("input governor invalid");
    exit(1);
  }
  }

  // set powercap
  if (SetCPUPowercap(cpu_long_pc, cpu_short_pc))
    exit(1);
  
  max_freq_file.close();
  min_freq_file.close();
  max_short_file.close();
  max_long_file.close();
}

int Controller::SetCPUFreq(int cpu_freq) {
  if (cpu_freq == 0)
    cpu_freq = cpu_min_freq;
  if (cpu_freq < cpu_min_freq || cpu_freq > cpu_max_freq) {
    perror("input frequency invalid");
    return 1;
  }
  
  int status;
  std::ostringstream cpupower_cmd;
  #pragma omp parallel for
  for (int i=0; i<cpu_cores; i++) {
    cpupower_cmd << "sudo cpupower -c " << i << " frequency-set -u " 
                 << cpu_freq << "hz > /dev/null";
    status = system(cpupower_cmd.str().c_str());
    cpupower_cmd.clear();
    cpupower_cmd << "sudo cpupower -c " << i << " frequency-set -d " 
                 << cpu_freq << "hz > /dev/null";
    status = system(cpupower_cmd.str().c_str());
    cpupower_cmd.clear();
  }
  this->cpu_freq = cpu_freq;
  return 0;
}

void Controller::SetCPUGovernor(const char *gov) {
  std::string str;
  std::fstream cur_gov_file;
  char *gov_file = (char *)malloc(strlen(CUR_GOV_FILE) + 1);

  #pragma omp parallel for
  for (int i=0; i<cpu_cores; i++) {
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

int Controller::SetCPUPowercap(double cpu_long_pc, double cpu_short_pc = 0) {
  if (cpu_long_pc == 0)
    return 0;
  if (cpu_long_pc < 0 || cpu_long_pc > cpu_max_long_pc || 
      cpu_short_pc < 0 || cpu_short_pc > cpu_max_short_pc) {
    perror("invalid powercap");
    return 1;
  }

  rl_long.watts = cpu_long_pc;
  for (int i=0; i<cpu_pkgs; i++) {
    for (int j=0; j<cpu_dies; j++) {
      if (raplcap_pd_set_limits(&rc, i, j, RAPLCAP_ZONE_PACKAGE, &rl_long, &rl_short)) {
        perror("raplcap_pd_set_limits");
        return 1;
      }
    }
  }
  for (int i=0; i<cpu_pkgs; i++) {
    for (int j=0; j<cpu_dies; j++) {
      if (raplcap_pd_set_zone_enabled(&rc, i, j, RAPLCAP_ZONE_PACKAGE, 1)) {
        perror("raplcap_pd_set_zone_enabled");
        return 1;
      }
    }
  }
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

double Controller::GetCPULongPowerCap() {
  return rl_long.watts;
}

double Controller::GetCPUShortPowerCap() {
  return rl_short.watts;
}

double Controller::GetCPULongWindow() {
  return rl_long.seconds;
}

double Controller::GetCPUShortWindow() {
  return rl_short.seconds;
}

Controller::~Controller() {
  if (raplcap_destroy(&rc)) {
    perror("raplcap_destroy");
  }
}
