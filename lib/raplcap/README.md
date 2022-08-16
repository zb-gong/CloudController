# RAPLCap

This project provides a C interface for getting/setting power caps with Intel Running Average Power Limit (RAPL).
It supports multiple implementations with different backends, primarily:

* `libraplcap-msr` ([README](msr/README.md)): Uses [Model-Specific Register](https://en.wikipedia.org/wiki/Model-specific_register) files in the `/dev` filesystem (Linux).
* `libraplcap-powercap` ([README](powercap/README.md)): Uses the [Linux Power Capping Framework](https://www.kernel.org/doc/Documentation/power/powercap/powercap.txt) abstractions in the `/sys` filesystem (Linux).

It also provides binaries for getting/setting RAPL configurations from the command line.
Each provides the same command line interface, but use different RAPLCap library backends.

* `rapl-configure-msr`
* `rapl-configure-powercap`

Experimental backends, which are not documented here in any further detail and may be removed at any time, include:

* `libraplcap-ipg` ([README](ipg/README.md)): Uses Intel&reg; Power Gadget (OSX and Windows).

If using this project for other scientific works or publications, please reference:

* Connor Imes, Huazhe Zhang, Kevin Zhao, Henry Hoffmann. "CoPPer: Soft Real-time Application Performance Using Hardware Power Capping". In: IEEE International Conference on Autonomic Computing (ICAC). 2019. DOI: https://doi.org/10.1109/ICAC.2019.00015
  <details>
  <summary>BibTex</summary>

  ```BibTex
  @inproceedings{imes2019copper,
    author={Imes, Connor and Zhang, Huazhe and Zhao, Kevin and Hoffmann, Henry},
    booktitle={2019 IEEE International Conference on Autonomic Computing (ICAC)},
    title={{CoPPer}: Soft Real-Time Application Performance Using Hardware Power Capping},
    year={2019},
    pages={31-41},
    doi={10.1109/ICAC.2019.00015}
  }
  ```

  </details>


## Prerequisites

First, you must be using an Intel&reg; processor that supports RAPL - Sandy Bridge (2nd generation Intel&reg; Core) or newer.

Currently only Linux systems are supported by the primary implementations.

This project depends on:

* [powercap](https://github.com/powercap/powercap) - backend required to compile and run the `powercap` implementation (or install the [libpowercap-dev](apt:libpowercap-dev) package on recent Debian-based Linux distributions).

If dependencies with sufficient versions are not found, backends that require them will not be compiled.

Users are expected to be familiar with basic RAPL capabilities and terminology, like zones (domains) and long/short term power constraints.
Refer to Intel RAPL documentation for more technical information, especially the *Intel&reg; 64 and IA-32 Architectures Software Developer Manual, Volume 3: System Programming Guide.*

Due to lack of portability in backends and data availability on some systems, the interface does not support discovering processor min/max power caps or thermal design power.
Users should reference their hardware documentation or other system utilities to discover this information as needed.


## Running Average Power Limit

Intel RAPL allows software to configure power caps on hardware components, like processors or main memory.
Components manage themselves to respect the power cap while attempting to optimize performance.
Note that power caps are *NOT* the same as power consumption, they only specify an upper bound on power consumption over a time window.

For example, processors use Dynamic Voltage and Frequency Scaling (DVFS) to trade performance and power consumption, where power `P` is proportional to capacitance `C`, the square of the voltage `V`, and clock frequency `f`: `P ~ C * V^2 * f`.
An increase in frequency usually necessitates an increase in voltage, and vice versa, resulting in a non-linear tradeoff between performance (frequency) and power consumption.
With RAPL, hardware manages voltage and frequency at finer-grained time intervals and with lower overhead than software-based DVFS controllers.


## Building

This project uses CMake.

To build all libraries, run:

``` sh
mkdir _build
cd _build
cmake ..
make
```


## Installing

To install all libraries and headers, run with proper privileges:

``` sh
make install
```

On Linux, installation typically places libraries in `/usr/local/lib` and header files in `/usr/local/include`.


## Uninstalling

Install must be run before uninstalling in order to have a manifest.

To remove libraries and headers installed to the system, run with proper privileges:

``` sh
make uninstall
```


## Linking

### CMake

If your project uses CMake, import targets from the `RAPLCap` package by specifying the `MSR` and/or `Powercap` components as needed.
For example:

``` cmake
find_package(RAPLCap REQUIRED COMPONENTS MSR Powercap)
target_link_libraries(foo PRIVATE RAPLCap::raplcap-msr)
target_link_libraries(bar PRIVATE RAPLCap::raplcap-powercap)
```

### Pkg-config

If not using CMake, get linker information (including transitive dependencies) with `pkg-config`, e.g., one of:

``` sh
pkg-config --libs --static raplcap-msr
pkg-config --libs --static raplcap-powercap
```

Or in your Makefile, add to your linker flags one of:

``` Makefile
$(shell pkg-config --libs --static raplcap-msr)
$(shell pkg-config --libs --static raplcap-powercap)
```

You may leave off the `--static` option if you built shared object libraries.

Depending on your install location, you may also need to augment your compiler flags with one of:

``` sh
pkg-config --cflags raplcap-msr
pkg-config --cflags raplcap-powercap
```


## Usage

See the man pages for the `rapl-configure` binaries, or run them with the `-h` or `--help` option for instructions.

The [raplcap.h](inc/raplcap.h) header provides the C interface along with detailed function documentation for using the libraries.

For backend-specific runtime dependencies, see the README files in their implementation subdirectories (links above).

The following is a simple example of setting power caps that assumes a homogeneous architecture.

``` C
  // Note: more robust error handling may be desirable for a real application
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
  d = raplcap_get_num_die(rc, 0);
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
```


## Project Source

Find this and related project sources at the [powercap organization on GitHub](https://github.com/powercap).  
This project originates at: https://github.com/powercap/raplcap

Bug reports and pull requests for new implementations, bug fixes, and enhancements are welcome.
