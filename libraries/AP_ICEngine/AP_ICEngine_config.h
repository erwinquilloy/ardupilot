#pragma once

#include <AP_HAL/AP_HAL_Boards.h>

#ifndef AP_ICENGINE_ENABLED
// Light variant: most FPV/wing airframes are electric. ICE adds substantial
// flash (governor, RPM, choke/throttle/ignition handling); disabled by
// default. Per-board hwdef can re-enable for an ICE airframe.
#define AP_ICENGINE_ENABLED 0
#endif

/*
  optional TCA9554 I2C for starter control
 */
#ifndef AP_ICENGINE_TCA9554_STARTER_ENABLED
// enable on SITL by default to ensure code is built
#define AP_ICENGINE_TCA9554_STARTER_ENABLED AP_ICENGINE_ENABLED && (CONFIG_HAL_BOARD == HAL_BOARD_SITL)
#endif
