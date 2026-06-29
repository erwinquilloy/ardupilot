#pragma once

#include "AP_Radar.h"

#if HAL_MSP_RADAR_ENABLED

#include <AP_HAL/utility/OwnPtr.h>

class AP_Radar_MSP : public Radar_backend
{
public:
    using Radar_backend::Radar_backend;

    void init() override {}
    void update(void) override;

    void handle_msp(const MSP::msp_radar_pos_message_t &pkt) override;

    static AP_Radar_MSP *detect(AP_Radar &_frontend);

private:
    radar_peer_t peers[RADAR_MAX_PEERS];
    uint8_t update_count;
};

#endif // HAL_MSP_RADAR_ENABLED
