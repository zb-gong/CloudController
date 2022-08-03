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
#include "util.h"

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