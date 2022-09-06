#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <raplcap.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
  raplcap rc;
  raplcap_limit rl_short, rl_long;
  uint32_t i, j, n, d;

  // get the number of RAPL packages
  n = raplcap_get_num_packages(NULL);
  if (n == 0) {
    perror("raplcap_get_num_packages");
    return -1;
  }

  // initialize
  if (raplcap_init(&rc)) {
    perror("raplcap_init");
    return -1;
  }

  // assuming each package has the same number of die, only querying for package=0
  d = raplcap_get_num_die(&rc, 0);
  if (d == 0) {
    perror("raplcap_get_num_die");
    raplcap_destroy(&rc);
    return -1;
  }

  // for each package die, set a power cap of 100 Watts for short_term and 50 Watts for long_term constraints
  // a time window of 0 leaves the time window unchanged
  // rl_short.watts = 100.0;
  // rl_short.seconds = 0.0;
  // rl_long.watts = 50.0;
  // rl_long.seconds = 0.0;
  // for (i = 0; i < n; i++) {
  //   for (j = 0; j < d; j++) {
  //     if (raplcap_pd_set_limits(&rc, i, j, RAPLCAP_ZONE_PACKAGE, &rl_long, &rl_short)) {
  //       perror("raplcap_pd_set_limits");
  //     }
  //   }
  // }

  // // for each package die, enable the power caps
  // // this could be done before setting caps, at the risk of enabling unknown power cap values first
  // for (i = 0; i < n; i++) {
  //   for (j = 0; j < d; j++) {
  //     if (raplcap_pd_set_zone_enabled(&rc, i, j, RAPLCAP_ZONE_PACKAGE, 1)) {
  //       perror("raplcap_pd_set_zone_enabled");
  //     }
  //   }
  // }

  // double energy = raplcap_pd_get_energy_counter(&rc, 0, 0, RAPLCAP_ZONE_PACKAGE);
  // printf("energy:%f\n", energy);
  // raplcap_pd_get_limits(&rc, 0, 0, RAPLCAP_ZONE_PACKAGE, &rl_long, &rl_short);
  // printf("rl_long:(power:%f, time:%f)\n", rl_long.watts, rl_long.seconds);

  // int x = raplcap_pd_get_limit(&rc, 1, 0, RAPLCAP_ZONE_PACKAGE,
  //                          RAPLCAP_CONSTRAINT_PEAK_POWER, &rl_long);
  // printf("rl_long:(power:%f, time:%f)\n", rl_long.watts, rl_long.seconds);

  struct timeval start_time, end_time;
  struct timespec interval;
  interval.tv_sec = 0;
  interval.tv_nsec = 100000000; // 100ms

  double energy1_before = raplcap_pd_get_energy_counter(&rc, 0, 0, RAPLCAP_ZONE_PACKAGE);
  double energy2_before = raplcap_pd_get_energy_counter(&rc, 1, 0, RAPLCAP_ZONE_PACKAGE);
  gettimeofday(&start_time, NULL);

  nanosleep(&interval, NULL);

  double energy1_after = raplcap_pd_get_energy_counter(&rc, 0, 0, RAPLCAP_ZONE_PACKAGE);
  double energy2_after = raplcap_pd_get_energy_counter(&rc, 1, 0, RAPLCAP_ZONE_PACKAGE); 
  gettimeofday(&end_time, NULL);

  double diff_time = (end_time.tv_usec - start_time.tv_usec) + (end_time.tv_sec - start_time.tv_sec) * 1000000;
  double power1 = (energy1_after - energy1_before) / diff_time * 1000000;
  double power2 = (energy2_after - energy2_before) / diff_time * 1000000;
  printf("power1:%f, power2:%f\n", power1, power2);
              

  // cleanup
  if (raplcap_destroy(&rc)) {
    perror("raplcap_destroy");
  }
}