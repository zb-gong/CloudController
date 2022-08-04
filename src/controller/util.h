#ifndef UTIL_H
#define UTIL_H

#include <iostream>

#define UNCORE_RATIO_LIMIT 0x620
#define UNCORE_BASE_FREQ 100000

// constant file path
static const char MAX_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
static const char MIN_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq";
static const char CUR_FREQ_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq";
static const char CUR_GOV_FILE[] = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq";
static const char MAX_PWR_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\\:0/constraint_1_max_power_uw";
static const char MAX_PWR_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\\:0/constraint_0_max_power_uw";
// static const char CUR_PWR_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_1_power_limit_uw ";
// static const char CUR_PWR_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_0_power_limit_uw ";
// static const char CUR_WIN_SHORT_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_1_power_limit_uw ";
// static const char CUR_WIN_LONG_FILE[] = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl\:0/constraint_0_power_limit_uw ";

// common util
void parser(int argc, char *argv[], int &cpu_freq, int &uncore_freq, double &cpu_power, std::string &cid);

// msr related
int open_msr(int core);
uint64_t read_msr(int fd, uint64_t which);
void write_msr(int fd, uint64_t which, uint64_t data);
int32_t wrmsr(int fd, uint64_t msr_number, uint64_t value);
int32_t rdmsr(int fd, uint64_t msr_number, uint64_t *value);

#endif