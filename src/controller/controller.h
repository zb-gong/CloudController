#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <raplcap.h>
#include <iostream>
#include <vector>
#include "util.h"
#include "monitor.h"

#define DEBUG

enum class governor{
  POWERSAVE = 0,
  PERFORMANCE = 1,
  MANUAL = 2
};

/**
 * A resource controller
 * 
 * knobs include: cpu frequency (unit is kHz) cpu powercap (long/short term.) uncore frequency (unit is kHz) dram powercap
 * feedback counter include: cpu util, cache reference, ipc
 */
class Controller {
public:
  /* monitor */
  Monitor monitor;

  /* constants */
  int cpu_max_freq = 0;
  int cpu_min_freq = 0;
  double cpu_max_short_pc = 0.;
  double cpu_max_long_pc = 0.;

  /* hardware config */
  int cpu_cores_count = 0;
  int cpu_pkgs = 0;
  int cpu_dies = 0;
  governor cpu_governor = governor::POWERSAVE;
  raplcap cpu_rc;
  raplcap_limit cpu_short_pc, cpu_long_pc; // per package powercap
  Msrconfig msr_config;

  /* knobs */
  std::vector<int> cpu_freqs; // per core frequency
  double cpu_total_long_pc = 0.;
  double cpu_total_short_pc = 0.;
  int uncore_freq = 0;

  /* history info */
  WorkloadInfo work_info;
  static inline pthread_mutex_t work_lock;

  /* constructors */
  Controller();
  
  /* CPU freq related */
  int SetCPUFreq(int cpu_freq, int core);
  int GetRealCPUFreq(int core);
  int GetCPUFreq();
  /* CPU powercap related */
  int SetCPUPowercap(double cpu_total_long_pc, double cpu_total_short_pc);
  double GetCPULongPowerCap();
  double GetCPUShortPowerCap();
  double GetCPULongWindow();
  double GetCPUShortWindow();
  /* CPU utuilization related */
  double GetCPUMeanUtil(std::vector<double> utils);
  /* uncore freq related */
  int SetUncoreFreq(int uncore_freq);
  int GetRealUncoreFreq(double intv = 0.5);
  int GetUncoreFreq();
  /* dram power related */
  int SetDRAMPowercap(double dram_pc);
  /* misc */
  void SetCPUGovernor(const char *cpu_governor);
  void Run();
  static void *Schedule(void *workload);
  ~Controller();

  // /* container related */
  // void BindContainer();
  // void BindContainer(std::string container_id);
  // int GetPIDFromCID(std::string container_id);
  // std::string GetContainerID();
  // /* CPU cores related */
  // int BindCPUCores();
  // int GetCurCPUCores();
};

#endif