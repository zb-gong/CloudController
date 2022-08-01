#ifndef CONTROLLER_H
#define CONTROLLER_H

// constant file path
static const char MAX_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
static const char MIN_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq";
static const char CUR_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
static const char CUR_GOV_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
static const char MAX_PWR_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_1_max_power_uw";
static const char MAX_PWR_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_0_max_power_uw";
// static const char CUR_PWR_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_1_power_limit_uw ";
// static const char CUR_PWR_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_0_power_limit_uw ";
// static const char CUR_WIN_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_1_power_limit_uw ";
// static const char CUR_WIN_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_0_power_limit_uw ";

enum class governor{
  POWERSAVE = 0,
  PERFORMANCE = 1,
  MANUAL = 2
};

/**
 * A resource controller
 * 
 * knobs include: cpu frequency (unit is Hz) cpu powercap (long/short term.)
 * feedback counter include: cpu util, cache reference, ipc
 */
class Controller {
private:
  // constants
  static int cpu_max_freq;
  static int cpu_min_freq;
  static double cpu_max_short_pc;
  static double cpu_max_long_pc;

  // app config TODO: docker multiple app multiple names
  int pid;

  // hardware config
  int cpu_cores;
  int cpu_pkgs;
  int cpu_dies;
  governor cpu_governor;
  raplcap rc;
  raplcap_limit rl_short, rl_long;

  // knobs
  int cpu_freq;
  double cpu_powercap;
  double cpu_cur_power;
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
  Controller();
  Controller(governor cpu_governor, double cpu_long_pc, double cpu_short_pc, int cpu_freq);
  int SetCPUFreq(int cpu_freq);
  int SetCPUPowercap(double cpu_long_pc, double cpu_short_pc);
  void SetCPUGovernor(const char *cpu_governor);
  int GetMaxCPUFreq();
  int GetMinCPUFreq();
  int GetCPUFreq();
  double GetCPUCurPower();
  double GetCPULongPowerCap();
  double GetCPUShortPowerCap();
  double GetCPULongWindow();
  double GetCPUShortWindow();
  double GetCPUUtil();
  double GetCPUMeanUtil();
  double GetCPUIPC();
  double GetCPUCacheMissRate();
  void Schedule();
  ~Controller();
};

#endif