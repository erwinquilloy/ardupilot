#pragma once

#include <AP_HAL/AP_HAL_Boards.h>
#include <AP_Frsky_Telem/AP_Frsky_config.h>
#include <GCS_MAVLink/GCS_config.h>
#include <AP_Radio/AP_Radio_config.h>

#ifndef AP_RCPROTOCOL_ENABLED
#define AP_RCPROTOCOL_ENABLED 1
#endif

#ifndef AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
// Light variant: flip the global RC-protocol backend default to OFF, then
// re-enable CRSF + SBUS + FPORT + FPORT2 + IBUS below — the dominant
// receivers for ELRS / TBS Crossfire / FrSky / FlySky hardware. DroneCAN
// and the rest (DSM, PPMSUM, SRXL/SRXL2, ST24, SUMD, GHST) stay off.
// Mirrors mf0o's "Disable useless RCIN protocols" commit; note we
// additionally keep FPORT2 (mf0o's 2022 strip dropped it).
#define AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED 0
#endif

#ifndef AP_RCPROTOCOL_CRSF_ENABLED
// Light variant: keep CRSF (covers ELRS + TBS Crossfire — by far the most
// common modern receivers).
#define AP_RCPROTOCOL_CRSF_ENABLED AP_RCPROTOCOL_ENABLED
#endif

#ifndef AP_RCPROTOCOL_DRONECAN_ENABLED
// Light variant: CAN disabled in strict 2022 definition.
#define AP_RCPROTOCOL_DRONECAN_ENABLED 0
#endif

#ifndef AP_RCPROTOCOL_DSM_ENABLED
#define AP_RCPROTOCOL_DSM_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_RCPROTOCOL_FPORT_ENABLED
// Light variant: keep FPORT (one of the four allowed RC protocols
// in the strict 2022 definition: SBUS, CRSF, IBUS, FPORT).
#define AP_RCPROTOCOL_FPORT_ENABLED AP_RCPROTOCOL_ENABLED
#endif
#ifndef AP_RCPROTOCOL_FPORT2_ENABLED
// Light variant: keep FPORT2 as a sibling of FPORT, gated on the
// FrSky SPort telem path that the protocol layers on top of.
#define AP_RCPROTOCOL_FPORT2_ENABLED AP_RCPROTOCOL_ENABLED && AP_FRSKY_SPORT_TELEM_ENABLED
#endif

#ifndef AP_RCPROTOCOL_IBUS_ENABLED
// Light variant: keep IBUS (FlySky receivers; allowed by the strict
// 2022 list).
#define AP_RCPROTOCOL_IBUS_ENABLED AP_RCPROTOCOL_ENABLED
#endif

#ifndef AP_RCPROTOCOL_JOYSTICK_SFML_ENABLED
#define AP_RCPROTOCOL_JOYSTICK_SFML_ENABLED defined(SFML_JOYSTICK)
#endif

#ifndef AP_RCPROTOCOL_PPMSUM_ENABLED
#define AP_RCPROTOCOL_PPMSUM_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_RCPROTOCOL_RADIO_ENABLED
#define AP_RCPROTOCOL_RADIO_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED && AP_RADIO_ENABLED
#endif

#ifndef AP_RCPROTOCOL_SBUS_ENABLED
// Light variant: keep SBUS (FrSky / Futaba / older Crossfire mode).
#define AP_RCPROTOCOL_SBUS_ENABLED AP_RCPROTOCOL_ENABLED
#endif

#ifndef AP_RCPROTOCOL_SRXL_ENABLED
#define AP_RCPROTOCOL_SRXL_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_RCPROTOCOL_SRXL2_ENABLED
#define AP_RCPROTOCOL_SRXL2_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_RCPROTOCOL_ST24_ENABLED
#define AP_RCPROTOCOL_ST24_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_RCPROTOCOL_SUMD_ENABLED
#define AP_RCPROTOCOL_SUMD_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_RCPROTOCOL_SBUS_NI_ENABLED
#define AP_RCPROTOCOL_SBUS_NI_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED && AP_RCPROTOCOL_SBUS_ENABLED
#endif

#ifndef AP_RCPROTOCOL_FASTSBUS_ENABLED
#define AP_RCPROTOCOL_FASTSBUS_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED && AP_RCPROTOCOL_SBUS_ENABLED
#endif

#ifndef AP_RCPROTOCOL_GHST_ENABLED
#define AP_RCPROTOCOL_GHST_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED
#endif

#ifndef AP_RCPROTOCOL_MAVLINK_RADIO_ENABLED
#define AP_RCPROTOCOL_MAVLINK_RADIO_ENABLED AP_RCPROTOCOL_BACKEND_DEFAULT_ENABLED && BOARD_FLASH_SIZE > 1024 && HAL_GCS_ENABLED
#endif

#ifndef AP_RCPROTOCOL_UDP_ENABLED
// Light variant: the UDP/FDM backends are the SITL RC-input path. They must
// stay enabled on SITL even though real-hardware RX protocols are stripped
// (BACKEND_DEFAULT_ENABLED = 0), otherwise a SITL build has no RC at all and
// autotest hangs at "PreArm: Waiting for RC". They are SITL-only, so this
// costs zero flash on the F4/H7 targets the light variant actually ships.
#define AP_RCPROTOCOL_UDP_ENABLED (CONFIG_HAL_BOARD == HAL_BOARD_SITL)
#endif

#ifndef AP_RCPROTOCOL_FDM_ENABLED
// Light variant: SITL-only RC-input backend — see AP_RCPROTOCOL_UDP_ENABLED
// above. Kept on for SITL so autotest has RC; zero flash cost on real boards.
#define AP_RCPROTOCOL_FDM_ENABLED (CONFIG_HAL_BOARD == HAL_BOARD_SITL)
#endif