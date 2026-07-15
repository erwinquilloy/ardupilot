#pragma once

#include <AP_HAL/AP_HAL_Boards.h>
#include <AP_Mount/AP_Mount_config.h>  // for HAL_MOUNT_ENABLED (head tracker gate)

#ifndef HAL_MSP_ENABLED
#define HAL_MSP_ENABLED 1
#endif

// define for enabling MSP sensor drivers
#ifndef HAL_MSP_SENSORS_ENABLED
#define HAL_MSP_SENSORS_ENABLED HAL_MSP_ENABLED
#endif

// define for enabling MSP DisplayPort
#ifndef HAL_WITH_MSP_DISPLAYPORT
#define HAL_WITH_MSP_DISPLAYPORT HAL_MSP_ENABLED
#endif

// define for enabling iNav-style MSP head tracker -> gimbal control.
// Requires an inbound MSP link (e.g. DisplayPort UART) and a mount to drive.
#ifndef HAL_MSP_HEADTRACKER_ENABLED
#define HAL_MSP_HEADTRACKER_ENABLED (HAL_MSP_ENABLED && HAL_MOUNT_ENABLED)
#endif
