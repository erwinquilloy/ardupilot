#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL_Boards.h>
#include <AP_MSP/AP_MSP_config.h>
#include <AP_ExternalAHRS/AP_ExternalAHRS_config.h>

#ifndef AP_AIRSPEED_ENABLED
#define AP_AIRSPEED_ENABLED 1
#endif

#ifndef AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: flip the global airspeed-backend default to OFF. The
// strict 2022 definition is Analog or MS4525 only, so just those two are
// re-enabled below; the digital chips (ASP5033, DLVR, MS5525, MSP, NMEA,
// SDP3X) and DroneCAN all compile out. Mirrors mf0o's "Disable detection
// of useless airspeed sensors" pattern.
#define AP_AIRSPEED_BACKEND_DEFAULT_ENABLED 0
#else
#define AP_AIRSPEED_BACKEND_DEFAULT_ENABLED AP_AIRSPEED_ENABLED
#endif
#endif

// backends
#ifndef AP_AIRSPEED_ANALOG_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: keep analog airspeed (simple voltage-divider pitot).
#define AP_AIRSPEED_ANALOG_ENABLED AP_AIRSPEED_ENABLED
#else
#define AP_AIRSPEED_ANALOG_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif
#endif

#ifndef AP_AIRSPEED_ASP5033_ENABLED
#define AP_AIRSPEED_ASP5033_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_AIRSPEED_DLVR_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: strict 2022 definition is Analog or MS4525 only.
#define AP_AIRSPEED_DLVR_ENABLED 0
#else
#define AP_AIRSPEED_DLVR_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif
#endif

#ifndef AP_AIRSPEED_DRONECAN_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: CAN disabled in strict 2022 definition.
#define AP_AIRSPEED_DRONECAN_ENABLED 0
#else
#define AP_AIRSPEED_DRONECAN_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED && HAL_ENABLE_DRONECAN_DRIVERS
#endif
#endif

#ifndef AP_AIRSPEED_MS4525_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: keep MS4525 (the most common I2C pitot chip; one of two
// allowed pressure sensors in the strict 2022 definition).
#define AP_AIRSPEED_MS4525_ENABLED AP_AIRSPEED_ENABLED
#else
#define AP_AIRSPEED_MS4525_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif
#endif

#ifndef AP_AIRSPEED_MS5525_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: strict 2022 definition is Analog or MS4525 only.
#define AP_AIRSPEED_MS5525_ENABLED 0
#else
#define AP_AIRSPEED_MS5525_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif
#endif

#ifndef AP_AIRSPEED_MSP_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: strict 2022 definition is Analog or MS4525 only.
#define AP_AIRSPEED_MSP_ENABLED 0
#else
#define AP_AIRSPEED_MSP_ENABLED (AP_AIRSPEED_BACKEND_DEFAULT_ENABLED && HAL_MSP_SENSORS_ENABLED)
#endif
#endif

// note additional vehicle restrictions are made in the .cpp file!
#ifndef AP_AIRSPEED_NMEA_ENABLED
#define AP_AIRSPEED_NMEA_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_AIRSPEED_SDP3X_ENABLED
#define AP_AIRSPEED_SDP3X_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_AIRSPEED_SITL_ENABLED
#if AP_CUSTOM_LIGHT
// Light variant: the backend trim above targets flash on real boards, where
// AP_SIM_ENABLED is 0 and this costs nothing either way. Gating the SITL
// backend on that trim left sensor[i] permanently null for ARSPD_TYPE=100,
// so airspeed could never become healthy and SITL could never arm with
// ARSPD_USE=1 - i.e. no airspeed-dependent behaviour was testable at all.
#define AP_AIRSPEED_SITL_ENABLED AP_SIM_ENABLED
#else
#define AP_AIRSPEED_SITL_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED && AP_SIM_ENABLED
#endif
#endif

#ifndef AP_AIRSPEED_AUAV_ENABLED
#define AP_AIRSPEED_AUAV_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

// other AP_Airspeed options:
#ifndef AIRSPEED_MAX_SENSORS
#define AIRSPEED_MAX_SENSORS 2
#endif

#ifndef AP_AIRSPEED_AUTOCAL_ENABLE
#define AP_AIRSPEED_AUTOCAL_ENABLE AP_AIRSPEED_ENABLED
#endif

#ifndef AP_AIRSPEED_HYGROMETER_ENABLE
#define AP_AIRSPEED_HYGROMETER_ENABLE (AP_AIRSPEED_ENABLED && HAL_PROGRAM_SIZE_LIMIT_KB > 1024)
#endif

#ifndef AP_AIRSPEED_EXTERNAL_ENABLED
#define AP_AIRSPEED_EXTERNAL_ENABLED AP_AIRSPEED_ENABLED && AP_EXTERNAL_AHRS_ENABLED
#endif
