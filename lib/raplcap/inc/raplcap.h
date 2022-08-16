/**
 * A power capping interface for Intel Running Average Power Limit (RAPL).
 *
 * If a NULL value is passed to functions for the raplcap (rc) parameter, a global default context is used.
 * This global (NULL) context must be initialized/destroyed the same as an application-managed (non-NULL) context.
 *
 * It is the developer's responsibility to synchronize as needed when a context is accessed by multiple threads.
 *
 * RAPL "clamping" may be managed automatically as part of enabling, disabling, or setting power caps.
 * It is implementation-specific if clamping is considered when getting or setting a zone's "enabled" status.
 *
 * The term "socket" is now deprecated in favor of "package".
 * Historically, sockets always contained a single package, but some Intel architectures may now contain more than one.
 *
 * @author Connor Imes
 * @date 2016-05-13
 */
#ifndef _RAPLCAP_H_
#define _RAPLCAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

#if !defined(RAPLCAP_DEPRECATED)
#if defined(RAPLCAP_ALLOW_DEPRECATED)
#define RAPLCAP_DEPRECATED(x)
#elif defined(__GNUC__) || defined(__clang__)
#define RAPLCAP_DEPRECATED(x) __attribute__((deprecated(x)))
#elif defined(_MSC_VER)
#define RAPLCAP_DEPRECATED(x) __declspec(deprecated(x))
#else
#define RAPLCAP_DEPRECATED(x)
#endif
#endif

/**
 * A RAPLCap context
 *
 * With the nsockets field deprecated, all that remains in the struct is a void pointer.
 * For better forward compatibility, we chose not to expose new fields for packages and die, so that the ABI won't need
 * to change as processor architectures continue to evolve in ways that impact RAPL management structure.
 * It's easier to make functions backward compatible than it is to make structs backward compatible.
 * In future releases, it might therefore be beneficial to just make the raplcap context struct opaque.
 * If such a change is made, raplcap_init will also be changed to something like: `raplcap* raplcap_init(void)`
 */
typedef struct raplcap {
  /**
   * The nsockets field is now deprecated since it doesn't reflect the RAPL control structure anymore.
   * Do not access the field directly - use functions 'raplcap_get_num_packages' and 'raplcap_get_num_die' instead.
   *
   * @deprecated
   */
  RAPLCAP_DEPRECATED("Call functions raplcap_get_num_packages() and raplcap_get_num_die() instead")
  uint32_t nsockets;
  void* state;
} raplcap;

/**
 * A RAPL power capping limit
 */
typedef struct raplcap_limit {
  double seconds;
  double watts;
} raplcap_limit;

/**
 * Available RAPL zones (domains)
 */
typedef enum raplcap_zone {
  RAPLCAP_ZONE_PACKAGE = 0,
  RAPLCAP_ZONE_CORE,
  RAPLCAP_ZONE_UNCORE,
  RAPLCAP_ZONE_DRAM,
  RAPLCAP_ZONE_PSYS,
} raplcap_zone;

/**
 * Available RAPL constraints
 */
typedef enum raplcap_constraint {
  RAPLCAP_CONSTRAINT_LONG_TERM,
  RAPLCAP_CONSTRAINT_SHORT_TERM,
  RAPLCAP_CONSTRAINT_PEAK_POWER,
} raplcap_constraint;

/**
 * Initialize a RAPLCap context.
 *
 * @param rc
 * @return 0 on success, a negative value on error
 */
int raplcap_init(raplcap* rc);

/**
 * Destroy a RAPLCap context.
 *
 * @param rc
 * @return 0 on success, a negative value on error
 */
int raplcap_destroy(raplcap* rc);

/**
 * Get the number of available packages.
 * If the raplcap context is not initialized, the function will attempt to discover the number of available packages.
 *
 * @param rc
 * @return the number of packages, 0 on error
 */
uint32_t raplcap_get_num_packages(const raplcap* rc);

/**
 * Deprecated, superseded by raplcap_get_num_packages.
 *
 * @param rc
 * @return the number of packages, 0 on error
 * @deprecated
 */
RAPLCAP_DEPRECATED("Call raplcap_get_num_packages() instead")
uint32_t raplcap_get_num_sockets(const raplcap* rc);

/**
 * Get the number of available die in a package.
 * If the raplcap context is not initialized, the function will attempt to discover the number of available die.
 *
 * @param rc
 * @param pkg
 * @return the number of die, 0 on error
 */
uint32_t raplcap_get_num_die(const raplcap* rc, uint32_t pkg);

/**
 * Check if a zone is supported.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @return 0 if unsupported, 1 if supported, a negative value on error
 */
int raplcap_pd_is_zone_supported(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone);

/**
 * Check if a zone constraint is supported.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @param constraint
 * @return 0 if unsupported, 1 if supported, a negative value on error
 */
int raplcap_pd_is_constraint_supported(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone,
                                       raplcap_constraint constraint);

/**
 * Check if a zone is enabled.
 * Constraints can technically be enabled/disabled separately, but for simplicity and compatibility with lower-level
 * RAPL interfaces, we define a zone to be enabled only if all of its constraints are enabled (disabled otherwise).
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @return 0 if disabled, 1 if enabled, a negative value on error
 */
int raplcap_pd_is_zone_enabled(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone);

/**
 * Enable/disable a zone by enabling/disabling all of its constraints.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @param enabled
 * @return 0 on success, a negative value on error
 */
int raplcap_pd_set_zone_enabled(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone, int enabled);

/**
 * Get the long and/or short term constraint limits for a zone, if it is supported.
 * Not all zones use limit_short.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @param limit_long
 * @param limit_short
 * @return 0 on success, a negative value on error
 */
int raplcap_pd_get_limits(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone,
                          raplcap_limit* limit_long, raplcap_limit* limit_short);

/**
 * Set the long and/or short term constraint limits for a zone, if it is supported.
 * Not all zones use limit_short.
 * If the power or time window value is 0, it will not be written or the current value may be used.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @param limit_long
 * @param limit_short
 * @return 0 on success, a negative value on error
 */
int raplcap_pd_set_limits(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone,
                          const raplcap_limit* limit_long, const raplcap_limit* limit_short);

/**
 * Get a zone constraint's limit, if it is supported.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @param constraint
 * @param limit
 * @return 0 on success, a negative value on error
 */
int raplcap_pd_get_limit(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone,
                         raplcap_constraint constraint, raplcap_limit* limit);

/**
 * Set a zone constraint's limit, if it is supported.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @param constraint
 * @param limit
 * @return 0 on success, a negative value on error
 */
int raplcap_pd_set_limit(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone,
                         raplcap_constraint constraint, const raplcap_limit* limit);

/**
 * Get the current energy counter value for a zone in Joules.
 * Note that the counter rolls over - check the max value.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @return Joules on success, a negative value on error
 */
double raplcap_pd_get_energy_counter(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone);

/**
 * Get the maximum energy counter value for a zone in Joules.
 *
 * @param rc
 * @param pkg
 * @param die
 * @param zone
 * @return Joules on success, a negative value on error
 */
double raplcap_pd_get_energy_counter_max(const raplcap* rc, uint32_t pkg, uint32_t die, raplcap_zone zone);

/**
 * Assumes die=0.
 *
 * @deprecated
 * @see raplcap_pd_is_zone_supported
 */
RAPLCAP_DEPRECATED("Call raplcap_pd_is_zone_supported() instead")
int raplcap_is_zone_supported(const raplcap* rc, uint32_t pkg, raplcap_zone zone);

/**
 * Assumes die=0.
 *
 * @deprecated
 * @see raplcap_pd_is_zone_enabled
 */
RAPLCAP_DEPRECATED("Call raplcap_pd_is_zone_enabled() instead")
int raplcap_is_zone_enabled(const raplcap* rc, uint32_t pkg, raplcap_zone zone);

/**
 * Assumes die=0.
 *
 * @deprecated
 * @see raplcap_pd_set_zone_enabled
 */
RAPLCAP_DEPRECATED("Call raplcap_pd_set_zone_enabled() instead")
int raplcap_set_zone_enabled(const raplcap* rc, uint32_t pkg, raplcap_zone zone, int enabled);

/**
 * Assumes die=0.
 *
 * @deprecated
 * @see raplcap_pd_get_limits
 */
RAPLCAP_DEPRECATED("Call raplcap_pd_get_limits() instead")
int raplcap_get_limits(const raplcap* rc, uint32_t pkg, raplcap_zone zone,
                       raplcap_limit* limit_long, raplcap_limit* limit_short);

/**
 * Assumes die=0.
 *
 * @deprecated
 * @see raplcap_pd_set_limits
 */
RAPLCAP_DEPRECATED("Call raplcap_pd_set_limits() instead")
int raplcap_set_limits(const raplcap* rc, uint32_t pkg, raplcap_zone zone,
                       const raplcap_limit* limit_long, const raplcap_limit* limit_short);

/**
 * Assumes die=0.
 *
 * @deprecated
 * @see raplcap_pd_get_energy_counter
 */
RAPLCAP_DEPRECATED("Call raplcap_pd_get_energy_counter() instead")
double raplcap_get_energy_counter(const raplcap* rc, uint32_t pkg, raplcap_zone zone);

/**
 * Assumes die=0.
 *
 * @deprecated
 * @see raplcap_pd_get_energy_counter_max
 */
RAPLCAP_DEPRECATED("Call raplcap_pd_get_energy_counter_max() instead")
double raplcap_get_energy_counter_max(const raplcap* rc, uint32_t pkg, raplcap_zone zone);

#ifdef __cplusplus
}
#endif

#endif
