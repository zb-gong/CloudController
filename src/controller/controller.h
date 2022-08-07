#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <raplcap.h>
#include <iostream>

#define DEBUG

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
  int cpu_max_freq = 0;
  int cpu_min_freq = 0;
  double cpu_max_short_pc = 0.;
  double cpu_max_long_pc = 0.;

  // app config TODO: multiple App
  int pid = 0;
  std::string cid = "";

  // hardware config
  int cpu_total_cores = 0;
  int cpu_pkgs = 0;
  int cpu_dies = 0;
  governor cpu_governor = governor::POWERSAVE;
  raplcap cpu_rc;
  raplcap_limit cpu_short_pc, cpu_long_pc; // per package powercap

  // knobs TODO: specify cpu cores index (index array)
  int cpu_cur_cores = 0;
  int cpu_freq = 0;
  double cpu_total_long_pc = 0.;
  double cpu_total_short_pc = 0.;
  int uncore_freq = 0;
  // uint gpu_freq;
  // double gpu_powercap;
  // uint dram_pstate;
  // double dram_powercap;
  // double total_powercap;

  // counters
  double cpu_util = 0.;
  double cpu_ipc = 0.;
  double cpu_miss_rate = 0.;

public:
  // constructors
  Controller();
  Controller(governor cpu_governor, double cpu_total_long_pc, double cpu_total_short_pc, int cpu_freq, int uncore_freq);
  // container related
  void BindContainer();
  void BindContainer(std::string container_id);
  int GetPIDFromCID(std::string container_id);
  std::string GetContainerID();
  //CPU cores related
  int BindCPUCores();
  int GetCurCPUCores();
  // CPU freq related
  int SetCPUFreq(int cpu_freq);
  int GetMaxCPUFreq();
  int GetMinCPUFreq();
  int GetRealCPUFreq();
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
  int GetRealUncoreFreq();
  int GetUncoreFreq();
  // misc
  void SetCPUGovernor(const char *cpu_governor);
  int GetPID();
  void Schedule();
  ~Controller();
};

#endif