#include <iostream>
#include <vector>
#include <libvirt/libvirt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  virConnectPtr c;
  virDomainPtr d;
  virTypedParameter p;
  virDomainInfo info;
  virVcpuInfo cinfo;  
  c = virConnectOpen(NULL);
  d = virDomainLookupByID(c, 3);


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