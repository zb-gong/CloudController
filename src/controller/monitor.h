#ifndef CONTROLLER_H
#define CONTROLLER_H

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
  int cpu_cores_count;
  std::vector<std::vector<int> > vm_cpus;
  raplcap cpu_rc;

  Monitor();
  double GetCPUUtilFromDomain(virDomainPtr domain, double interval);
  double GetIPCFromCores(std::vector<int> cores);
  double GetCPUPowerFromCores(std::vector<int> cores, double interval);
  double GetCacheMissRateFromCores(std::vector<int> cores);
  double GetDRAMPower(double interval);
  void Run();
  #ifdef DEBUG
  std::vector<double> GetAllCPUUtil(double interval);
  #endif
  ~Monitor();
};

#endif