
#pragma once

#include "AP_RCProtocol_config.h"

#if AP_RCPROTOCOL_MAVLINK_RADIO_ENABLED

#include "AP_RCProtocol.h"


class AP_RCProtocol_MAVLinkRadio : public AP_RCProtocol_Backend {
public:

    using AP_RCProtocol_Backend::AP_RCProtocol_Backend;

    // update from mavlink messages
    void update_radio_rc_channels(const mavlink_radio_rc_channels_t* packet) override;

private:
    // Fork: mLRS 1.4.04+ in 32-channel mode sends mostly 16-channel frames
    // with a 32-channel frame at 5 Hz. Channels beyond a short frame keep
    // their last value from the most recent wide frame, until wide frames
    // stop for WIDE_HOLD_MS.
    static const uint32_t WIDE_HOLD_MS = 1000;
    uint16_t _chan[MAX_RCIN_CHANNELS];
    uint8_t _wide_count;
    uint32_t _last_wide_ms;
};

#endif // AP_RCPROTOCOL_MAVLINK_RADIO_ENABLED

