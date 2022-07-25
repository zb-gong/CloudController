#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <raplcap.h>

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
  rl_short.watts = 100.0;
  rl_short.seconds = 0.0;
  rl_long.watts = 50.0;
  rl_long.seconds = 0.0;
  for (i = 0; i < n; i++) {
    for (j = 0; j < d; j++) {
      if (raplcap_pd_set_limits(&rc, i, j, RAPLCAP_ZONE_PACKAGE, &rl_long, &rl_short)) {
        perror("raplcap_pd_set_limits");
      }
    }
  }

  // for each package die, enable the power caps
  // this could be done before setting caps, at the risk of enabling unknown power cap values first
  for (i = 0; i < n; i++) {
    for (j = 0; j < d; j++) {
      if (raplcap_pd_set_zone_enabled(&rc, i, j, RAPLCAP_ZONE_PACKAGE, 1)) {
        perror("raplcap_pd_set_zone_enabled");
      }
    }
  }

  // cleanup
  if (raplcap_destroy(&rc)) {
    perror("raplcap_destroy");
  }
}