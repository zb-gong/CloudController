#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <omp.h>

void setCPUSpeed(int core, int freq) {
  int cpuinfo_min_freq, status;
  char buf[100], governor[20];
  sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", core);
  FILE *f = fopen(buf, "r");
  status = fscanf(f, "%d", &cpuinfo_min_freq);
  fclose(f);

  memset(buf, 0, sizeof(buf));
  sprintf(buf, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", core);
  f = fopen(buf, "r");
  status = fscanf(f, "%s", governor);
  fclose(f);
  if (strcmp(governor, "performance") != 0) {
    f = fopen(buf, "w");
    fprintf(f, "performance");
    fclose(f);
  }

  if (freq >= cpuinfo_min_freq/1000) {
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "sudo cpupower -c %d frequency-set -u %dmhz > /dev/null", core, freq);
    status = system(buf);
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "sudo cpupower -c %d frequency-set -d %dmhz > /dev/null", core, freq);
    status = system(buf);    
  }
}

void gc(int *a, int *b, char *c, char *d) {
  free(a);
  free(b);
  free(c);
  free(d);
}

int main(int argc, char *argv[]) {
  if (argc <=1) {
    fprintf(stderr, "usage: setspeed -a -c [c1,c2,...] -f f1[,f2,...]\n"
    "-a: set all cores\n"
    "-c: set centain cores\n"
    "-f set consistent freq to core list if f has one arg\n");
    exit(1);
  }

  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  int o;
  int *core = (int *)malloc(num_cores * sizeof(int));
  int *freq = (int *)malloc(num_cores * sizeof(int));
  for (int i=0; i<num_cores; i++) {
    core[i] = -1;
    freq[i] = 0;
  }
  bool all_core = false;
  char *core_list = (char *)malloc(2 * num_cores * sizeof(char));
  char *freq_list = (char *)malloc(2 * num_cores * sizeof(char));

  static struct option long_option[] = {
    {"core", optional_argument, NULL, 'c'},
    {"freq", required_argument, NULL, 'f'},
    {"help", no_argument, NULL, 'h'},
    {"all", no_argument, NULL, 'a'},
    {0,0,0,0}
  };

  while ((o = getopt_long(argc, argv, "hac:f:", long_option, NULL)) != -1) {
    switch(o) {
      case 'c': {
        if (optarg) {
          strncpy(core_list, optarg, 2 * num_cores);        
        } else {
          fprintf(stderr, "--core should include at least one arg");
          exit(1);
        }
        break;
      }
      case 'f': {
        if (optarg) {
          strncpy(freq_list, optarg, 2 * num_cores);        
        } else {
          fprintf(stderr, "--freq should include at least one arg");
          exit(1);
        }
        break;
      }
      case 'a': {
        all_core = true;
        break;
      }
      default: {
        fprintf(stderr, "usage: setspeed -c [c1,c2,...] -f f1[,f2,...]\n"
        "set all cores frequency if c has no args\n"
        "set consistent freq to core list if f has one arg\n");
        gc(core, freq, core_list, freq_list);
        exit(1);
      }
    }
  }

  if (all_core) {
    int all_freq = atoi(freq_list); 
    #pragma omp parallel for num_threads(num_cores)
    for (int i=0; i<num_cores; i++) {
      setCPUSpeed(i, all_freq);
    }
  } else {
    int count = 0;
    char *tmp_core = strtok(core_list, ",");
    core[count++] = atoi(tmp_core);
    while ((tmp_core = strtok(NULL, ",")) != NULL) {
      core[count++] = atoi(tmp_core);
    }

    char *tmp_freq = strtok(freq_list, ",");
    freq[0] = atoi(tmp_freq);
    for (int i=1; i<count; i++) {
      tmp_freq = strtok(NULL, ",");
      if (tmp_freq)
        freq[i] = atoi(tmp_freq);
      else 
        freq[i] = freq[0];
    }

    #pragma omp parallel for num_threads(count)
    for (int i=0; i<count; i++) {
      setCPUSpeed(core[i], freq[i]);
    }
  }

  gc(core, freq, core_list, freq_list);
}