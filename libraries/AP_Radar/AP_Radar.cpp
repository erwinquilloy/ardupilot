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

#include "AP_Radar.h"

#if AP_RADAR_ENABLED

#include "AP_Radar_MSP.h"
#include <AP_BoardConfig/AP_BoardConfig.h>
#include <AP_Logger/AP_Logger.h>
#include <GCS_MAVLink/GCS.h>

#define RADAR_TYPE_DEFAULT 1

extern const AP_HAL::HAL& hal;

const AP_Param::GroupInfo AP_Radar::var_info[] = {
    // @Param: _TYPE
    // @DisplayName: Radar sensor type
    // @Description: Selects the peer-aircraft radar backend. MSP listens for iNav Radar / FormationFlight position frames (MSP2_SET_RADAR_POS 0x100B) on any serial port configured as MSP (SERIALn_PROTOCOL=33).
    // @Values: 0:None,1:MSP
    // @User: Standard
    // @RebootRequired: True
    AP_GROUPINFO_FLAGS("_TYPE", 0, AP_Radar, _type, (float)RADAR_TYPE_DEFAULT, AP_PARAM_FLAG_ENABLE),

    AP_GROUPEND
};

AP_Radar::AP_Radar()
{
    _singleton = this;
    AP_Param::setup_object_defaults(this, var_info);
}

void AP_Radar::init(uint32_t log_bit)
{
    _log_bit = log_bit;

    if ((_type == Type::NONE) || (backend != nullptr)) {
        return;
    }

    switch ((Type)_type) {
    case Type::NONE:
        break;
    case Type::MSP:
#if HAL_MSP_RADAR_ENABLED
        backend = AP_Radar_MSP::detect(*this);
#endif
        break;
    }

    if (backend != nullptr) {
        backend->init();
    }
}

void AP_Radar::update(void)
{
    if (!enabled()) {
        return;
    }
    if (backend != nullptr) {
        backend->update();
    }

    // healthy if data is less than RADAR_PEER_FRESH_TIME_MS old
    _flags.healthy = (AP_HAL::millis() - _last_update_ms < RADAR_PEER_FRESH_TIME_MS);
}

void AP_Radar::handle_msg(const mavlink_message_t &msg)
{
    if (!enabled()) {
        return;
    }
    if (backend != nullptr) {
        backend->handle_msg(msg);
    }
}

#if HAL_MSP_RADAR_ENABLED
void AP_Radar::handle_msp(const MSP::msp_radar_pos_message_t &pkt)
{
    if (!enabled()) {
        return;
    }
    if (backend != nullptr) {
        backend->handle_msp(pkt);
    }
}
#endif

void AP_Radar::update_state(const Radar_state &state)
{
    _state = state;
    _last_update_ms = AP_HAL::millis();
}

radar_peer_t AP_Radar::get_peer(uint8_t id)
{
    return _state.peers[id];
}

bool AP_Radar::get_peer_healthy(uint8_t id)
{
    return _state.peers[id].last_update > (AP_HAL::millis() - RADAR_PEER_FRESH_TIME_MS) &&
           !_state.peers[id].location.is_zero();
}

uint8_t AP_Radar::get_next_healthy_peer(uint8_t current_id)
{
    for (uint8_t i = 1; i < RADAR_MAX_PEERS; i++) {
        uint8_t next_peer = (current_id + i) % RADAR_MAX_PEERS;
        if (get_peer_healthy(next_peer)) {
            return next_peer;
        }
    }
    return current_id;
}

AP_Radar *AP_Radar::_singleton;

namespace AP {

AP_Radar *radar()
{
    return AP_Radar::get_singleton();
}

}

#endif // AP_RADAR_ENABLED
