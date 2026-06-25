#pragma once

#include <AP_HAL/AP_HAL_Boards.h>
#include <GCS_MAVLink/GCS_config.h>

#ifndef AP_GPS_ENABLED
#define AP_GPS_ENABLED 1
#endif

#if AP_GPS_ENABLED
/**
   maximum number of GPS instances available on this platform. If more
   than 1 then redundant sensors may be available
 */
#ifndef GPS_MAX_RECEIVERS
#define GPS_MAX_RECEIVERS 2 // maximum number of physical GPS sensors allowed - does not include virtual GPS created by blending receiver data
#endif

#if !defined(GPS_MAX_INSTANCES)
#if GPS_MAX_RECEIVERS > 1
#define GPS_MAX_INSTANCES  (GPS_MAX_RECEIVERS + 1) // maximum number of GPS instances including the 'virtual' GPS created by blending receiver data
#else
#define GPS_MAX_INSTANCES 1
#endif // GPS_MAX_RECEIVERS > 1
#endif // GPS_MAX_INSTANCES

#if GPS_MAX_RECEIVERS <= 1 && GPS_MAX_INSTANCES > 1
#error "GPS_MAX_INSTANCES should be 1 for GPS_MAX_RECEIVERS <= 1"
#endif
#endif

#ifndef AP_GPS_BACKEND_DEFAULT_ENABLED
// Light variant: flip the global GPS-backend default to OFF, then
// explicitly re-enable just the three backends our typical user actually
// uses (UBLOX serial GPS, DroneCAN GPS, MSP GPS from a goggles passthrough).
// Mirrors mf0o's minimize_common.inc pattern.
#define AP_GPS_BACKEND_DEFAULT_ENABLED 0
#endif

#if !defined(AP_GPS_BLENDED_ENABLED) && defined(GPS_MAX_INSTANCES)
#define AP_GPS_BLENDED_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED && GPS_MAX_INSTANCES > GPS_MAX_RECEIVERS
#endif

#ifndef AP_GPS_BLENDED_ENABLED
#define AP_GPS_BLENDED_ENABLED 0
#endif

#if AP_GPS_BLENDED_ENABLED
#define GPS_BLENDED_INSTANCE GPS_MAX_RECEIVERS  // the virtual blended GPS is always the highest instance (2)
#endif

#ifndef AP_GPS_DRONECAN_ENABLED
// Light variant: keep DroneCAN GPS regardless of the global default flip,
// since CAN-attached GPS modules are common on H7 wing builds.
#define AP_GPS_DRONECAN_ENABLED HAL_ENABLE_DRONECAN_DRIVERS
#endif

#ifndef AP_GPS_ERB_ENABLED
  #define AP_GPS_ERB_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_GSOF_ENABLED
  #define AP_GPS_GSOF_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_MAV_ENABLED
  #define AP_GPS_MAV_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED && HAL_GCS_ENABLED
#endif

#ifndef HAL_MSP_GPS_ENABLED
// Light variant: keep MSP GPS regardless of the global default flip,
// since OSD goggles often pass through GPS data.
#define HAL_MSP_GPS_ENABLED HAL_MSP_SENSORS_ENABLED
#endif

#ifndef AP_GPS_NMEA_ENABLED
  #define AP_GPS_NMEA_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_NMEA_UNICORE_ENABLED
  #define AP_GPS_NMEA_UNICORE_ENABLED AP_GPS_NMEA_ENABLED
#endif

#ifndef AP_GPS_NOVA_ENABLED
  #define AP_GPS_NOVA_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_SBF_ENABLED
  #define AP_GPS_SBF_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_SBP_ENABLED
  #define AP_GPS_SBP_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_SBP2_ENABLED
   #define AP_GPS_SBP2_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_SIRF_ENABLED
  #define AP_GPS_SIRF_ENABLED AP_GPS_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_GPS_UBLOX_ENABLED
  // Light variant: keep UBLOX serial GPS unconditionally; it's the most
  // common GPS module for the fork's target boards.
  #define AP_GPS_UBLOX_ENABLED AP_GPS_ENABLED
#endif

#ifndef AP_GPS_RTCM_DECODE_ENABLED
  #define AP_GPS_RTCM_DECODE_ENABLED BOARD_FLASH_SIZE > 1024
#endif

#ifndef HAL_GPS_COM_PORT_DEFAULT
#define HAL_GPS_COM_PORT_DEFAULT 1
#endif
