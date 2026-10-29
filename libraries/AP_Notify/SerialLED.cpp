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

#include "SerialLED.h"

#if AP_NOTIFY_SERIALLED_ENABLED

#if AP_NOTIFY_LED_SEGMENTS_ENABLED
#include "AP_Notify.h"
#include <AP_BattMonitor/AP_BattMonitor.h>
#include <AP_RSSI/AP_RSSI.h>
#include <AP_Vehicle/AP_Vehicle_Type.h>
#include <SRV_Channel/SRV_Channel.h>
#endif

extern const AP_HAL::HAL& hal;

SerialLED::SerialLED(uint8_t led_off, uint8_t led_bright, uint8_t led_medium, uint8_t led_dim) :
    RGBLed(led_off, led_bright, led_medium, led_dim)
{
}

bool SerialLED::init()
{
    enable_mask = init_ports();
    return true;
}

bool SerialLED::hw_set_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    if (enable_mask == 0) {
        // nothing is enabled, no pins set as LED output
        return true;
    }

#if AP_NOTIFY_LED_SEGMENTS_ENABLED
    if (segments_active()) {
        // the strip belongs to draw_segments(); RGBLed still records
        // the status colour in _red/_green/_blue_curr for FUNC Status
        return true;
    }
#endif

    AP_SerialLED *led = AP_SerialLED::get_singleton();
    if (led == nullptr) {
        return false;
    }

    for (uint16_t chan=0; chan<16; chan++) {
        if ((1U<<chan) & enable_mask) {
            led->set_RGB(chan+1, -1, red, green, blue);
        }
    }

    for (uint16_t chan=0; chan<16; chan++) {
        if ((1U<<chan) & enable_mask) {
            led->send(chan+1);
        }
    }

    return true;
}

#if AP_NOTIFY_LED_SEGMENTS_ENABLED
/*
  Fork-only: INAV-style LED segments (NTF_LS_ENABLE, NTF_LSn_*)
 */
namespace {

struct Colour {
    uint8_t r, g, b;
};

// palette for NTF_LSn_COLOR and NTF_LS_MODECLR, full brightness
const Colour palette[16] {
    {  0,   0,   0},    // 0 off
    {255, 255, 255},    // 1 white
    {255,   0,   0},    // 2 red
    {255,  60,   0},    // 3 orange
    {255, 180,   0},    // 4 yellow
    {128, 255,   0},    // 5 lime
    {  0, 255,   0},    // 6 green
    {  0, 255,  96},    // 7 mint
    {  0, 255, 255},    // 8 cyan
    {  0, 128, 255},    // 9 light blue
    {  0,   0, 255},    // 10 blue
    {128,   0, 255},    // 11 violet
    {255,   0, 255},    // 12 magenta
    {255,   0,  96},    // 13 pink
    {255, 110,   0},    // 14 amber
    {255, 180, 100},    // 15 warm white
};

const Colour &pal(int32_t idx)
{
    return palette[uint32_t(idx) & 0x0F];
}

Colour scale(const Colour &c, uint8_t s)
{
    return { uint8_t((c.r * s) / 255), uint8_t((c.g * s) / 255), uint8_t((c.b * s) / 255) };
}

// red at 0, yellow at 0.5, green at 1
Colour gradient(float f)
{
    f = constrain_float(f, 0, 1);
    if (f < 0.5f) {
        return { 255, uint8_t(f * 2 * 180), 0 };
    }
    return { uint8_t((1 - f) * 2 * 255), uint8_t(180 + (f - 0.5f) * 2 * 75), 0 };
}

// flight mode -> NTF_LS_MODECLR group (4 bits each)
uint8_t mode_group(uint8_t mode)
{
#if APM_BUILD_TYPE(APM_BUILD_ArduPlane)
    switch (mode) {
    case 0: case 2: case 3: case 4:             // Manual Stabilize Training Acro
        return 0;
    case 5: case 6: case 7: case 8: case 26: case 27:
        // FBWA FBWB Cruise Autotune, fork Course Hold and Auto Trim
        return 1;
    case 10: case 13: case 14: case 15: case 24: // Auto Takeoff AvoidADSB Guided Thermal
        return 2;
    case 11: case 20: case 21: case 25:          // RTL QLand QRTL Loiter to QLand
        return 3;
    case 1: case 12:                             // Circle Loiter
        return 4;
    case 17: case 18: case 19: case 22: case 23: // QStabilize QHover QLoiter QAutotune QAcro
        return 5;
    }
#endif
    return 0;
}

}  // namespace

bool SerialLED::segments_active() const
{
    return enable_mask != 0 && pNotify->_led_seg_enable != 0;
}

void SerialLED::update()
{
    if (!segments_active()) {
        if (_segments_were_active) {
            // segments just switched off: repaint the stock colour
            _segments_were_active = false;
            hw_set_rgb(_red_curr, _green_curr, _blue_curr);
        }
        RGBLed::update();
        return;
    }
    _segments_were_active = true;

    // keeps the stock status colour current for FUNC Status
    RGBLed::update();

    // update() runs at 50 Hz; 25 Hz is plenty for strobe and scanner
    if ((++_segment_tick & 1) == 0) {
        draw_segments();
    }
}

void SerialLED::draw_segments()
{
    AP_SerialLED *led = AP_SerialLED::get_singleton();
    if (led == nullptr) {
        return;
    }

    const auto &nflags = AP_Notify::flags;
    const uint32_t now_ms = AP_HAL::millis();
    const uint8_t num_leds = pNotify->get_led_len();
    const uint8_t brightness = get_brightness();
    const bool failsafe = nflags.failsafe_radio || nflags.failsafe_battery ||
                          nflags.failsafe_gcs || nflags.failsafe_ekf;

    // LEDs outside every segment stay off
    for (uint16_t chan=0; chan<16; chan++) {
        if ((1U<<chan) & enable_mask) {
            led->set_RGB(chan+1, -1, 0, 0, 0);
        }
    }

    using Func = NotifyLEDSegment::Func;
    using Overlay = NotifyLEDSegment::Overlay;

    for (const NotifyLEDSegment &seg : pNotify->_led_seg) {
        if (seg.start <= 0 || seg.len <= 0) {
            continue;
        }
        if (seg.option(Overlay::ONLY_ARMED) && !nflags.armed) {
            continue;
        }
        const uint8_t first = seg.start - 1;
        const uint8_t len = seg.len;

        Colour c = pal(seg.colour);
        float level = -1;   // 0..1 for functions with a value, used by Bar

        switch (Func(seg.func.get())) {
        case Func::COLOUR:
            break;
        case Func::FLIGHT_MODE:
            c = pal(pNotify->_led_seg_mode_colours >> (4 * mode_group(nflags.flight_mode)));
            break;
        case Func::ARM_STATE:
            c = nflags.armed ? palette[6] : (nflags.pre_arm_check ? palette[10] : palette[14]);
            break;
        case Func::BATTERY: {
#if AP_BATTERY_ENABLED
            uint8_t pct;
            if (AP::battery().capacity_remaining_pct(pct, AP_BATT_PRIMARY_INSTANCE)) {
                level = pct * 0.01f;
                c = gradient(level);
            }
#endif
            break;
        }
        case Func::GPS:
            c = nflags.gps_status < 3 ? palette[2] : (nflags.gps_status == 3 ? palette[4] : palette[6]);
            level = nflags.gps_num_sats * (1.0f / 20);
            break;
        case Func::LINK: {
#if AP_RSSI_ENABLED
            AP_RSSI *rssi = AP::rssi();
            if (rssi != nullptr && rssi->enabled()) {
                level = rssi->read_receiver_rssi();
                c = gradient(level);
            }
#endif
            break;
        }
        case Func::THROTTLE:
            level = constrain_float(fabsf(SRV_Channels::get_output_scaled(SRV_Channel::k_throttle)) * 0.01f, 0, 1);
            if (!seg.option(Overlay::BAR)) {
                c = scale(c, uint8_t(level * 255));
            }
            break;
        case Func::STATUS:
            c = { uint8_t(_red_curr ? 255 : 0), uint8_t(_green_curr ? 255 : 0), uint8_t(_blue_curr ? 255 : 0) };
            break;
        default:
            continue;
        }

        if (seg.option(Overlay::WARNING) && failsafe) {
            c = ((now_ms / 200) & 1) ? palette[2] : palette[0];
        }
        if (seg.option(Overlay::BLINK) && ((now_ms / 500) & 1)) {
            c = palette[0];
        }
        if (seg.option(Overlay::STROBE)) {
            const uint16_t phase = now_ms % 1200;
            if (phase < 50 || (phase >= 150 && phase < 200)) {
                c = palette[1];
            }
        }

        uint8_t lit = len;
        if (seg.option(Overlay::BAR) && level >= 0) {
            lit = uint8_t(constrain_float(level, 0, 1) * len + 0.5f);
        }

        // scanner moves one LED every 60 ms, end to end and back
        int16_t scan = -1;
        if (seg.option(Overlay::LARSON) && len > 1) {
            const uint16_t period = 2 * (len - 1);
            const uint16_t step = (now_ms / 60) % period;
            scan = step < len ? step : period - step;
        }

        for (uint8_t i=0; i<len; i++) {
            const uint16_t led_no = first + (seg.option(Overlay::REVERSE) ? len - 1 - i : i);
            if (led_no >= num_leds) {
                continue;
            }
            Colour out = i < lit ? c : palette[0];
            if (scan >= 0) {
                const int16_t d = abs(int16_t(i) - scan);
                if (d == 1) {
                    out = scale(out, 64);
                } else if (d > 1) {
                    out = scale(out, 16);
                }
            }
            out = scale(out, brightness);
            for (uint16_t chan=0; chan<16; chan++) {
                if ((1U<<chan) & enable_mask) {
                    led->set_RGB(chan+1, led_no, out.r, out.g, out.b);
                }
            }
        }
    }

    for (uint16_t chan=0; chan<16; chan++) {
        if ((1U<<chan) & enable_mask) {
            led->send(chan+1);
        }
    }
}
#endif  // AP_NOTIFY_LED_SEGMENTS_ENABLED

#endif  // AP_NOTIFY_SERIALLED_ENABLED
