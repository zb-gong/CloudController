#ifndef MONITOR_H
#define MONITOR_H

#include <iostream>
#include <vector>
#include <string>
#include <libvirt/libvirt.h>
#include <raplcap.h>
#include "util.h"

#define DEBUG

/**
 * resource monitor
 * 
 * monitor:
 * cpu -> cpu utilizaiton, ipc, cpu power
 * dram -> dram power, cache miss rate
 */
class Monitor {
public:
  /* general */
  std::vector<std::string> log_files;
  double energy_unit;

  /* vm related */
  int vm_count = 0;
  virConnectPtr vm_ptr;
  std::vector<int> vm_ids;
  std::vector<virDomainPtr> vm_domains;

  /* cpu related */
  std::vector<std::vector<int> > vm_cpus;
  raplcap cpu_rc;

  Monitor();
  double GetCPUUtilFromDomain(virDomainPtr domain, double interval = 0.5);
  double GetIPCFromCores(std::vector<int> cores);
  double GetCPUPower(double interval = 0.1);
  double GetCacheMissRateFromCores(std::vector<int> cores);
  double GetDRAMPower(double interval = 0.1);
  void Run();
  #ifdef DEBUG
  std::vector<double> GetAllCPUUtil(double interval = 0.05);
  #endif
  ~Monitor();
};

#endif