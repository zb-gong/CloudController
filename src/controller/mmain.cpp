#include <iostream>
#include "monitor.h"
#include "util.h"

int main(int argc, char *argv[]) {
  Monitor m;
  for (int i=0; i<m.vm_count; i++) {
    std::cout << m.log_files[i] << std::endl;
    std::cout << m.vm_ids[i] << std::endl;
    for (auto x:m.vm_cpus[i])
      std::cout << x << " ";
    std::cout << std::endl;
  }
}