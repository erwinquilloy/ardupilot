#pragma once

#include <AP_BoardConfig/AP_BoardConfig.h>

#ifndef HAL_LANDING_DEEPSTALL_ENABLED
// Fork: default off (was: BOARD_FLASH_SIZE > 1024). Deepstall writes
// elevator PWM directly via set_output_pwm(), which bypasses the
// ELEVATOR_DIFF attenuation applied in set_output_scaled() (see fork
// PR #133). The two features are mechanically incompatible: if you
// run deepstall with a non-zero ELEVATOR_DIFF, the deepstall code
// path produces an unmodified elevator output and the standard path
// produces an attenuated one, depending on which fires last. The
// fork's choice is to drop deepstall by default since ELEVATOR_DIFF
// is the more general feature. Per-board hwdef can override.
#define HAL_LANDING_DEEPSTALL_ENABLED 0
#endif
