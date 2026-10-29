/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
  Fork-only: INAV-style addressable LED segments (NTF_LSn_*).

  Each segment is a run of LEDs on the NeoPixel strip with its own job
  (fixed colour, flight mode, battery, ...) and effects on top.  Drawn
  by SerialLED when NTF_LS_ENABLE is 1; otherwise the strip shows the
  stock single-colour status pattern.
 */
#pragma once

#include "AP_Notify_config.h"

#if AP_NOTIFY_LED_SEGMENTS_ENABLED

#include <AP_Param/AP_Param.h>

#define AP_NOTIFY_LED_SEGMENTS 8

class NotifyLEDSegment {
public:
    NotifyLEDSegment();

    static const struct AP_Param::GroupInfo var_info[];

    enum class Func : uint8_t {
        OFF         = 0,
        COLOUR      = 1,
        FLIGHT_MODE = 2,
        ARM_STATE   = 3,
        BATTERY     = 4,
        GPS         = 5,
        LINK        = 6,
        THROTTLE    = 7,
        STATUS      = 8,
    };

    enum class Overlay : uint8_t {
        BLINK      = (1U<<0),
        STROBE     = (1U<<1),
        LARSON     = (1U<<2),
        WARNING    = (1U<<3),
        BAR        = (1U<<4),
        REVERSE    = (1U<<5),
        ONLY_ARMED = (1U<<6),
    };

    bool option(Overlay o) const { return (uint8_t(overlays.get()) & uint8_t(o)) != 0; }

    AP_Int8 start;
    AP_Int8 len;
    AP_Int8 func;
    AP_Int8 colour;
    AP_Int8 overlays;
};

#endif  // AP_NOTIFY_LED_SEGMENTS_ENABLED
