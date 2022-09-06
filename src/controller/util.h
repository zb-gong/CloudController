#ifndef UTIL_H
#define UTIL_H

#include <iostream>

#define MSR_RAPL_POWER_UNIT 0x606
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT 0x610
#define MSR_PKG_ENERGY_STATUS 0x611
#define MSR_PKG_PERF_STATUS 0x613
#define MSR_PKG_POWER_INFO 0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT 0x638
#define MSR_PP0_ENERGY_STATUS 0x639
#define MSR_PP0_POLICY 0x63A
#define MSR_PP0_PERF_STATUS 0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT 0x640
#define MSR_PP1_ENERGY_STATUS 0x641
#define MSR_PP1_POLICY 0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT 0x618
#define MSR_DRAM_ENERGY_STATUS 0x619
#define MSR_DRAM_PERF_STATUS 0x61B
#define MSR_DRAM_POWER_INFO 0x61C
#define UNCORE_BASE_FREQ 20000

/* Uncore Freq Domain */
#define UNCORE_RATIO_LIMIT 0x620
#define MSR_U_PMON_UCLK_FIXED_CTR 0x704

/* Size Constants */
#define VM_ID_LIMIT 50

/* constant file path */
static const char MAX_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq";
static const char MIN_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq";
static const char CUR_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
static const char CUR_GOV_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor";
static const char MAX_PWR_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl:0/constraint_1_max_power_uw";
static const char MAX_PWR_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl:0/constraint_0_max_power_uw";
// static const char CUR_PWR_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_1_power_limit_uw ";
// static const char CUR_PWR_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_0_power_limit_uw ";
// static const char CUR_WIN_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_1_power_limit_uw ";
// static const char CUR_WIN_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_0_power_limit_uw ";

/* common util */
void parser(int argc, char *argv[], int &cpu_freq, int &uncore_freq, double &cpu_power, std::string &cid);

struct Msrconfig {
public:
  /* rapl unit related */
  double power_unit;
  double energy_unit;
  double time_unit;
  /* pkg power related */
  double pkg_thermal_spec_power;
  double pkg_minimum_power;
  double pkg_maximum_power;
  double pkg_maximum_time_windows;
  /* dram power related */
  double dram_thermal_spec_power;
  double dram_minimum_power;
  double dram_maximum_power;
  double dram_maximum_time_windows;
  Msrconfig();
};

/* msr related */
int open_msr(int core);
uint64_t read_msr(int fd, uint64_t which);
void write_msr(int fd, uint64_t which, uint64_t data);
int32_t wrmsr(int fd, uint64_t msr_number, uint64_t value);
int32_t rdmsr(int fd, uint64_t msr_number, uint64_t *value);
int close_msr(int fd);

#endif