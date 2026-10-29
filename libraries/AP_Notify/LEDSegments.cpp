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

#include "LEDSegments.h"

#if AP_NOTIFY_LED_SEGMENTS_ENABLED

const AP_Param::GroupInfo NotifyLEDSegment::var_info[] = {

    // @Param: START
    // @DisplayName: LED segment first LED
    // @Description: First LED of this segment, counting from 1 at the data-input end of the strip. 0 turns the segment off. Segments are drawn in order, so where two overlap the higher-numbered segment wins. Only used when NTF_LS_ENABLE is 1.
    // @Range: 0 127
    // @User: Standard
    AP_GROUPINFO("START", 1, NotifyLEDSegment, start, 0),

    // @Param: LEN
    // @DisplayName: LED segment length
    // @Description: Number of LEDs in this segment. LEDs past NTF_LED_LEN are ignored.
    // @Range: 0 127
    // @User: Standard
    AP_GROUPINFO("LEN", 2, NotifyLEDSegment, len, 0),

    // @Param: FUNC
    // @DisplayName: LED segment function
    // @Description: What this segment shows. Fixed colour uses COLOR. Flight mode uses NTF_LS_MODECLR. Arm state is green when armed, blue when disarmed and ready, amber when pre-arm checks fail. Battery and Link quality fade from red through yellow to green. GPS is red without a 3D fix, yellow with 3D, green with DGPS or better. Throttle shows COLOR at a brightness that follows the throttle. Status copies the stock NeoPixel status pattern.
    // @Values: 0:Off,1:Fixed colour,2:Flight mode,3:Arm state,4:Battery,5:GPS,6:Link quality,7:Throttle,8:Status
    // @User: Standard
    AP_GROUPINFO("FUNC", 3, NotifyLEDSegment, func, 0),

    // @Param: COLOR
    // @DisplayName: LED segment colour
    // @Description: Palette colour for Fixed colour and Throttle segments, and the fallback for Battery when no remaining-capacity estimate is available.
    // @Values: 0:Off,1:White,2:Red,3:Orange,4:Yellow,5:Lime,6:Green,7:Mint,8:Cyan,9:Light blue,10:Blue,11:Violet,12:Magenta,13:Pink,14:Amber,15:Warm white
    // @User: Standard
    AP_GROUPINFO("COLOR", 4, NotifyLEDSegment, colour, 0),

    // @Param: OVL
    // @DisplayName: LED segment effects
    // @Description: Effects drawn on top of the segment function. Blink is 1 Hz. Strobe is a white double flash every 1.2 s (with COLOR Off between flashes this makes a white anti-collision strobe). Larson scanner sweeps one bright LED along the segment. Failsafe warning flashes red during any radio/battery/GCS/EKF failsafe. Bar lights a share of the segment proportional to Battery/GPS satellites/Link quality/Throttle. Reverse runs the segment from its last LED. Only when armed blanks the segment while disarmed.
    // @Bitmask: 0:Blink,1:Strobe,2:Larson scanner,3:Failsafe warning,4:Bar,5:Reverse,6:Only when armed
    // @User: Standard
    AP_GROUPINFO("OVL", 5, NotifyLEDSegment, overlays, 0),

    AP_GROUPEND
};

NotifyLEDSegment::NotifyLEDSegment()
{
    AP_Param::setup_object_defaults(this, var_info);
}

#endif  // AP_NOTIFY_LED_SEGMENTS_ENABLED
