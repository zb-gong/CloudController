#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include "util.h"

/************************* common **********************************/
void parser(int argc, char *argv[], int &cpu_freq, int &uncore_freq, double &cpu_power, std::string &cid) {
  int o; 
  
  static struct option long_option[] = {
    {"cfreq", required_argument, NULL, 'c'},
    {"ufreq", required_argument, NULL, 'u'},
    {"cpower", required_argument, NULL, 'C'},
    {"cid", required_argument, NULL, 0},
    {"help", no_argument, NULL, 'h'},
    {0,0,0,0}
  };

  while ((o = getopt_long(argc, argv, "hc:f:C:", long_option, NULL)) != -1) {
    switch(o) {
      case 0:
        if (optarg) {
          cid = optarg;
        } else {
          perror("--cid should include at least one arg");
          exit(1);
        }
        break;
      case 'c': {
        if (optarg) {
          int tmp_freq = atoi(optarg);
          if (tmp_freq < 10) {
            tmp_freq *= 1000000; // input is GHz
          } else if (tmp_freq < 10000) {
            tmp_freq *= 1000; // input is MHz
          } else if (tmp_freq >= 10000000) {
            perror("input cpu freq is invalid, GHz, MHz, kHz are supported");
            exit(1);
          }
          cpu_freq = tmp_freq;
        } else {
          perror("--cfreq should include at least one arg");
          exit(1);
        }
        break;
      }
      case 'u': {
        if (optarg) {
          int tmp_freq = atoi(optarg);
          if (tmp_freq < 10) {
            tmp_freq *= 1000000; // input is GHz
          } else if (tmp_freq < 10000) {
            tmp_freq *= 1000; // input is MHz
          } else if (tmp_freq >= 10000000) {
            perror("input uncore freq is invalid, GHz, MHz, kHz are supported");
            exit(1);
          }
          uncore_freq = tmp_freq;
        } else {
          perror("--ufreq should include at least one arg");
          exit(1);
        }
        break;
      }
      case 'C': {
        if (optarg) {
          cpu_power = atof(optarg);
        } else {
          perror("--cpower should include at least one arg");
          exit(1);
        }
        break;
      }
      default: {
        fprintf(stdout, "usage: controller -[-c|--cfreq] 1000000 [-u|--ufreq] 2000000 [-C|--cpower 70\n"
        "-c|--cfreq: specify cpu frequency (support unit: GHz MHz KHz)\n"
        "-u|--ufreq: specify uncore frequency (support unit: GHz MHz KHz)\n"
        "-C|--cpower: specify cpu powercap (total long term)\n");
        exit(0);
      }
    }
  }
}

Msrconfig::Msrconfig() {
  int fd = open_msr(0);
  uint64_t rapl_power_unit = read_msr(fd, MSR_RAPL_POWER_UNIT);
  uint64_t pkg_power_info = read_msr(fd, MSR_PKG_POWER_INFO);
  uint64_t dram_power_info = read_msr(fd, MSR_DRAM_POWER_INFO);

  power_unit = pow(0.5, (double)(rapl_power_unit & 0xF));
  energy_unit = pow(0.5, (double)((rapl_power_unit >> 8) & 0x1F));
  time_unit = pow(0.5, (double)((rapl_power_unit >> 16) & 0xF));

  pkg_thermal_spec_power = (pkg_power_info & 0x7FFF) * power_unit;
  pkg_minimum_power = ((pkg_power_info >> 16) & 0x7FFF) * power_unit;
  pkg_maximum_power = ((pkg_power_info >> 32) & 0x7FFF) * power_unit;
  pkg_maximum_time_windows = ((pkg_power_info >> 48) & 0x3F) * time_unit;

  dram_thermal_spec_power = (dram_power_info & 0x7FFF) * power_unit;
  dram_minimum_power = ((dram_power_info >> 16) & 0x7FFF) * power_unit;
  dram_maximum_power = ((dram_power_info >> 32) & 0x7FFF) * power_unit;
  dram_maximum_time_windows = ((dram_power_info >> 48) & 0x3F) * time_unit;
}

/************************* msr related *****************************/
int open_msr(int core) {

  char msr_filename[BUFSIZ];
  int fd;

  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDWR);
  if (fd < 0) {
    if (errno == ENXIO) {
      fprintf(stderr, "rdmsr: No CPU %d\n", core);
      exit(2);
    } else if (errno == EIO) {
      fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
      exit(3);
    } else {
      perror("rdmsr:open");
      fprintf(stderr, "Trying to open %s\n", msr_filename);
      exit(127);
    }
  }

  return fd;
}

uint64_t read_msr(int fd, uint64_t which) {
  uint64_t data;

  if (pread(fd, &data, sizeof(uint64_t), which) != sizeof data) {
    perror("rdmsr:pread");
    exit(127);
  }

  return data;
}

void write_msr(int fd, uint64_t which, uint64_t data) {
  if (pwrite(fd, &data, sizeof(uint64_t), which) != sizeof data) {
    perror("wrmsr:pwrite");
    exit(127);
  }
}

int32_t wrmsr(int fd, uint64_t msr_number, uint64_t value) {
  return pwrite(fd, (const void *)&value, sizeof(uint64_t), msr_number);
}

int32_t rdmsr(int fd, uint64_t msr_number, uint64_t *value) {
  return pread(fd, (void *)value, sizeof(uint64_t), msr_number);
}

int close_msr(int fd) {
  return close(fd);
}