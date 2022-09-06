#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <omp.h>
#include <iostream>
#include "controller.h"
#include "util.h"

int main(int argc, char *argv[]) {
  if (argc > 1) {
    int cpu_freq = 0;
    int uncore_freq = 0;
    double cpu_power = 0;
    std::string container_id;

    parser(argc, argv, cpu_freq, uncore_freq, cpu_power, container_id);
    Controller c(governor::MANUAL, cpu_power, 0., cpu_freq, uncore_freq);
    c.Schedule();
    auto s = c.GetCPUCurPower();
    std::cout << "cur power:" << s << std::endl;
    auto t = c.GetCPUFreq();
    std::cout << "cur freq:" << t << std::endl;
  } else {
    Controller c;
    // c.Schedule();
    auto s = c.GetCPUCurPower();
    std::cout << "cur power:" << s << std::endl;
    auto t = c.GetCPUFreq();
    std::cout << "cur freq:" << t << std::endl;
  }
}