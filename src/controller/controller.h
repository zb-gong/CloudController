#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <raplcap.h>
#include <iostream>

enum class governor{
  POWERSAVE = 0,
  PERFORMANCE = 1,
  MANUAL = 2
};

/**
 * A resource controller
 * 
 * knobs include: cpu frequency (unit is kHz) cpu powercap (long/short term.)
 * feedback counter include: cpu util, cache reference, ipc
 */
class Controller {
private:
  // constants
  int cpu_max_freq;
  int cpu_min_freq;
  double cpu_max_short_pc;
  double cpu_max_long_pc;

  // app config TODO: docker multiple app multiple names
  int pid;
  std::string cid;

  // hardware config
  int cpu_total_cores;
  int cpu_pkgs;
  int cpu_dies;
  governor cpu_governor;
  raplcap cpu_rc;
  raplcap_limit cpu_short_pc, cpu_long_pc;

  // knobs
  int cpu_cur_cores;
  int cpu_freq;
  double cpu_total_long_pc;
  double cpu_total_short_pc;
  double cpu_cur_power;
  int uncore_freq;
  // uint gpu_freq;
  // double gpu_powercap;
  // uint dram_pstate;
  // double dram_powercap;
  // double total_powercap;

  // counters
  double cpu_util;
  double cpu_ipc;
  double cpu_miss_rate;

public:
  // constructors
  Controller();
  Controller(governor cpu_governor, double cpu_total_long_pc, double cpu_total_short_pc, int cpu_cores, int cpu_freq);
  // container related
  void BindContainer(std::string container_id);
  int GetContainerPID(std::string container_id);
  //CPU cores related
  int BindCPUCores();
  int GetCurCPUCores();
  // CPU freq related
  int SetCPUFreq(int cpu_freq);
  int GetMaxCPUFreq();
  int GetMinCPUFreq();
  int GetCPUFreq();
  // CPU powercap related
  int SetCPUPowercap(double cpu_total_long_pc, double cpu_total_short_pc);
  double GetCPUCurPower();
  double GetCPULongPowerCap();
  double GetCPUShortPowerCap();
  double GetCPULongWindow();
  double GetCPUShortWindow();
  // CPU utuilization related
  double GetCPUUtil();
  double GetCPUMeanUtil();
  // CPU counter related
  double GetCPUIPC();
  double GetCPUCacheMissRate();
  // uncore freq related
  int SetUncoreFreq(int uncore_freq);
  int GetUncoreFreq();
  // misc
  void SetCPUGovernor(const char *cpu_governor);
  void Schedule();
  ~Controller();
};

#endif