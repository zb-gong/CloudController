#include <iostream>
#include <vector>
#include <queue>
#include <deque>
#include <libvirt/libvirt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <raplcap.h>
#include <pthread.h>

#define QUEUE_LEN 100

template <typename T, int MaxLen, typename Container=std::deque<T>>
class FixedQueue : public std::queue<T, Container> {
public:
  void push(const T& value);
};

template <typename T, int MaxLen, typename Container>
void FixedQueue<T, MaxLen, Container>::push(const T& value) {
 if (this->size() == MaxLen) {
    this->c.pop_front();
  }
  std::queue<T, Container>::push(value);
}

struct WorkloadInfo {
  FixedQueue<std::vector<double>, QUEUE_LEN> cpu_utils;
  FixedQueue<std::vector<double>, QUEUE_LEN> ipcs;
  FixedQueue<std::vector<double>, QUEUE_LEN> cpu_powers;
  FixedQueue<std::vector<double>, QUEUE_LEN> cpu_cache_miss_rates;
  FixedQueue<std::vector<double>, QUEUE_LEN> cpu_dram_powers;
};

class Ship {
public:
  inline static int b = 1;
  WorkloadInfo w;
  static void *hello(void *a) {
    std::cout << "Hello, world!" << Ship::b << std::endl;
    return NULL;
  }
  void ship() {
    pthread_t t;
    pthread_create(&t, NULL, hello, NULL);
  }
  void hehe() {
    std::vector<double> tmp_utils(2);
    w.cpu_utils.push(tmp_utils);
  }
};

int main(int argc, char **argv) {
  virConnectPtr c;
  virDomainPtr d;
  virTypedParameter p;
  virDomainInfo info;
  virVcpuInfo cinfo;  
  c = virConnectOpen(NULL);
  d = virDomainLookupByID(c, 3);

  raplcap cpu_rc;
  if (raplcap_init(&cpu_rc)) {
    perror("raplcap_init");
    exit(1);
  }
  double energy1_before = raplcap_pd_get_energy_counter(&cpu_rc, 0, 0, RAPLCAP_ZONE_PACKAGE);
  printf("cpu power:%lf\n", energy1_before);

  Ship s;
  s.hehe();

  // virDomainGetInfo(d, &info);
  // printf("State:%d    ",info.state);  
  // printf("Maxmem:%ld    ",(info.maxMem)/1024);  
  // printf("Mem:%ld   ",info.memory/1024);  
  // printf("vCPU:%d   ",info.nrVirtCpu);  
  // printf("CPU time:%lld   ",info.cpuTime);  
  // printf("\n");

  // virDomainGetVcpus(d, &cinfo, 4, NULL, 0);
  // printf("cpu time:%lld\tcpu number:%d\n", cinfo.cpuTime, cinfo.cpu);

  // int numberOfPCPUS;
  // unsigned char * cpumap;
  // numberOfPCPUS = virNodeGetCPUMap(c, &cpumap, NULL, 0);
  // int num = VIR_CPU_USED(cpumap, 0);
  // int ncpumaps = 1;
  // int maplen = VIR_CPU_MAPLEN(numberOfPCPUS);
  // memset(cpumap, 0, maplen);
  // int vcpuPinInfo_ret = virDomainGetVcpuPinInfo(d, ncpumaps, cpumap, maplen, VIR_DOMAIN_AFFECT_CURRENT);
  // for (int i=0; i<numberOfPCPUS; i++) {
  //   num = VIR_CPU_USED(cpumap, i);
  //   printf("num:%d\n", num);
  // }
  // printf("Number of PCPUs available: %d, ", numberOfPCPUS);
  // printf("cpumap: %d\n", cpumap[1]);

  // int ids[10] = {};
  // int retv;
  // retv = virConnectListDomains(c, ids, 100);
  // for (int i=0; i<10; i++) {
  //   printf("id:%d\n", ids[i]);
  // }
  // printf("retv:%d\n", retv);

  // virDomainPtr d;
  // d = virDomainLookupByID(c, 3);
  // printf("name of domain %d is %s\n", 2, virDomainGetName(d));
  return 0;
}