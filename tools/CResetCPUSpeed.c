#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <omp.h>

void resetCPUSpeed(int core) {
  int cpuinfo_min_freq, cpuinfo_max_freq, status;
  char buf[100], governor[20];
  sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", core);
  FILE *f = fopen(buf, "r");
  status = fscanf(f, "%d", &cpuinfo_min_freq);
  fclose(f);
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", core);
  f = fopen(buf, "r");
  status = fscanf(f, "%d", &cpuinfo_max_freq);
  fclose(f);

  // reset freq bound in [cpuinfo_min_freq, cpuinfo_max_freq]
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "sudo cpupower -c %d frequency-set -u %dmhz", core, cpuinfo_max_freq/1000);
  status = system(buf);
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "sudo cpupower -c %d frequency-set -d %dmhz", core, cpuinfo_min_freq/1000);
  status = system(buf);

  // reset scaling governor to be powersave
  memset(buf, 0, sizeof(buf));
  sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", core);
  f = fopen(buf, "r");
  status = fscanf(f, "%s", governor);
  fclose(f);
  if (strcmp(governor, "powersave") != 0) {
    f = fopen(buf, "w");
    fprintf(f, "powersave");
    fclose(f);
  }
}

int main(int argc, char *argv[]) {
  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

  #pragma omp parallel for num_threads(num_cores)
  for (int i=0; i<num_cores; i++) {
    resetCPUSpeed(i);
  }
}