#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL_Boards.h>
#include <AP_MSP/AP_MSP_config.h>
#include <AP_ExternalAHRS/AP_ExternalAHRS_config.h>

#ifndef AP_AIRSPEED_ENABLED
#define AP_AIRSPEED_ENABLED 1
#endif

#ifndef AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
// Light variant: flip the global airspeed-backend default to OFF. Most
// FPV/wing builds run without a pitot or use the most common one
// (MS4525/ASP5033). Re-enable the common ones below; rarer chips
// (DLVR, MS5525, NMEA, SDP3X) compile out. Mirrors mf0o's
// "Disable detection of useless airspeed sensors" pattern.
#define AP_AIRSPEED_BACKEND_DEFAULT_ENABLED 0
#endif

// backends
#ifndef AP_AIRSPEED_ANALOG_ENABLED
// Light variant: keep analog airspeed (simple voltage-divider pitot).
#define AP_AIRSPEED_ANALOG_ENABLED AP_AIRSPEED_ENABLED
#endif

#ifndef AP_AIRSPEED_ASP5033_ENABLED
#define AP_AIRSPEED_ASP5033_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_AIRSPEED_DLVR_ENABLED
// Light variant: keep DLVR — the chip behind Matek's popular ASPD-DLVR
// and ASPD-7002 I2C digital airspeed sensors. mf0o stripped this; we
// keep it because our user base flies Matek wings.
#define AP_AIRSPEED_DLVR_ENABLED AP_AIRSPEED_ENABLED
#endif

#ifndef AP_AIRSPEED_DRONECAN_ENABLED
// Light variant: keep DroneCAN airspeed (Matek / CAN-attached pitots).
#define AP_AIRSPEED_DRONECAN_ENABLED AP_AIRSPEED_ENABLED && HAL_ENABLE_DRONECAN_DRIVERS
#endif

#ifndef AP_AIRSPEED_MS4525_ENABLED
// Light variant: keep MS4525 (the most common I2C pitot chip).
#define AP_AIRSPEED_MS4525_ENABLED AP_AIRSPEED_ENABLED
#endif

#ifndef AP_AIRSPEED_MS5525_ENABLED
// Light variant: keep MS5525 — the chip behind Matek's ASPD-MS5525 I2C
// airspeed sensor. mf0o stripped this; we keep it because our user
// base flies Matek wings.
#define AP_AIRSPEED_MS5525_ENABLED AP_AIRSPEED_ENABLED
#endif

#ifndef AP_AIRSPEED_MSP_ENABLED
// Light variant: keep MSP-airspeed (DJI / goggles passthrough).
#define AP_AIRSPEED_MSP_ENABLED (AP_AIRSPEED_ENABLED && HAL_MSP_SENSORS_ENABLED)
#endif

// note additional vehicle restrictions are made in the .cpp file!
#ifndef AP_AIRSPEED_NMEA_ENABLED
#define AP_AIRSPEED_NMEA_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_AIRSPEED_SDP3X_ENABLED
#define AP_AIRSPEED_SDP3X_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_AIRSPEED_SITL_ENABLED
#define AP_AIRSPEED_SITL_ENABLED AP_AIRSPEED_BACKEND_DEFAULT_ENABLED && AP_SIM_ENABLED
#endif

// other AP_Airspeed options:
#ifndef AIRSPEED_MAX_SENSORS
#define AIRSPEED_MAX_SENSORS 2
#endif

#ifndef AP_AIRSPEED_AUTOCAL_ENABLE
#define AP_AIRSPEED_AUTOCAL_ENABLE AP_AIRSPEED_ENABLED
#endif

#ifndef AP_AIRSPEED_HYGROMETER_ENABLE
#define AP_AIRSPEED_HYGROMETER_ENABLE (AP_AIRSPEED_ENABLED && BOARD_FLASH_SIZE > 1024)
#endif

#ifndef AP_AIRSPEED_EXTERNAL_ENABLED
#define AP_AIRSPEED_EXTERNAL_ENABLED AP_AIRSPEED_ENABLED && HAL_EXTERNAL_AHRS_ENABLED
#endif
