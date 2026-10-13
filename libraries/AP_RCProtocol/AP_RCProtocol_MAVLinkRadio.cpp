
#include "AP_RCProtocol_config.h"

#if AP_RCPROTOCOL_MAVLINK_RADIO_ENABLED

#include "AP_RCProtocol_MAVLinkRadio.h"

void AP_RCProtocol_MAVLinkRadio::update_radio_rc_channels(const mavlink_radio_rc_channels_t* packet)
{
    const uint8_t count = MIN(packet->count, MAX_RCIN_CHANNELS);

    for (uint8_t i = 0; i < count; i++) {
        // The channel values are in centered 13 bit format. Range is [-4096,4096], center is 0.
        // According to specification, the conversion to PWM is x * 5/32 + 1500.
        _chan[i] = ((int32_t)packet->channels[i] * 5) / 32 + 1500;
    }

    // hold the upper channels of the last wide frame across short frames
    const uint32_t now_ms = AP_HAL::millis();
    uint8_t out_count = count;
    if (count >= _wide_count) {
        _wide_count = count;
        _last_wide_ms = now_ms;
    } else if (now_ms - _last_wide_ms < WIDE_HOLD_MS) {
        out_count = _wide_count;
    } else {
        _wide_count = count;
    }

    bool failsafe = (packet->flags & RADIO_RC_CHANNELS_FLAGS_FAILSAFE);

    add_input(out_count, _chan, failsafe);
}

#endif // AP_RCPROTOCOL_MAVLINK_RADIO_ENABLED

