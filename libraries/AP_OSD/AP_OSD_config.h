#pragma once

#include <AP_HAL/AP_HAL_Boards.h>
#include <AP_Filesystem/AP_Filesystem_config.h>

#ifndef OSD_ENABLED
#define OSD_ENABLED 1
#endif

#ifndef HAL_WITH_OSD_BITMAP
#define HAL_WITH_OSD_BITMAP OSD_ENABLED && (defined(HAL_WITH_SPI_OSD) || defined(WITH_SITL_OSD))
#endif

#ifndef OSD_PARAM_ENABLED
#define OSD_PARAM_ENABLED 1
#endif

#ifndef HAL_OSD_SIDEBAR_ENABLE
#define HAL_OSD_SIDEBAR_ENABLE 1
#endif

#ifndef AP_OSD_CALLSIGN_FROM_SD_ENABLED
#define AP_OSD_CALLSIGN_FROM_SD_ENABLED (AP_FILESYSTEM_POSIX_ENABLED || AP_FILESYSTEM_FATFS_ENABLED)
#endif

#ifndef AP_OSD_LINK_STATS_EXTENSIONS_ENABLED
// Fork default: ON. Surfaces OSDx_LINK_Q, RC_PWR, RSSIDBM, RC_SNR, RC_ANT, RC_LQ
// extended CRSF telemetry elements. Upstream defaults to 0 to save ~3-5 KB flash;
// our fork users typically fly ELRS/CRSF and want these readings on the OSD.
// Per-board hwdef can still override by defining to 0 before including this header.
#define AP_OSD_LINK_STATS_EXTENSIONS_ENABLED 1
#endif
