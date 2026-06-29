#pragma once

#include <AP_HAL/AP_HAL_Boards.h>
#include <AP_MSP/AP_MSP_config.h>

#ifndef AP_RADAR_ENABLED
#define AP_RADAR_ENABLED HAL_MSP_ENABLED
#endif

#ifndef HAL_MSP_RADAR_ENABLED
#define HAL_MSP_RADAR_ENABLED (AP_RADAR_ENABLED && HAL_MSP_ENABLED)
#endif
