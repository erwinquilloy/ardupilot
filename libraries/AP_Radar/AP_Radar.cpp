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

    // @Param: _OSD_MODE
    // @DisplayName: Radar OSD peer selection
    // @Description: Which peer the radar OSD element shows. Cycle steps through every peer heard in the last 3 seconds and shows each one for 2 seconds. Nearest always shows the closest peer that reports a position; another peer only takes its place once it is at least 10 percent closer so two aircraft at similar range do not flicker. Peers without a position cannot be ranked so Nearest falls back to cycling until one has a fix.
    // @Values: 0:Cycle through all peers,1:Nearest peer
    // @User: Standard
    AP_GROUPINFO("_OSD_MODE", 1, AP_Radar, _osd_mode, (int8_t)OsdMode::CYCLE),

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
void AP_Radar::handle_msp(const MSP::msp_radar_pos_message_t &pkt, const char *name)
{
    if (!enabled()) {
        return;
    }
    if (backend != nullptr) {
        backend->handle_msp(pkt, name);
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

bool AP_Radar::get_peer_fresh(uint8_t id)
{
    // Subtract in this direction: last_update > millis() - FRESH_TIME underflows
    // for the first few seconds after boot and reports every slot as fresh.
    return _state.peers[id].last_update != 0 &&
           (AP_HAL::millis() - _state.peers[id].last_update) < RADAR_PEER_FRESH_TIME_MS;
}

bool AP_Radar::get_peer_healthy(uint8_t id)
{
    // A peer without a GPS fix forwards lat/lon 0/0, but its altitude need
    // not be 0 (a node that lost its fix keeps the last value), so
    // Location::is_zero() would call it healthy and draw a distance to the
    // Gulf of Guinea. 0/0 is never a real position here.
    const Location &loc = _state.peers[id].location;
    return get_peer_fresh(id) && (loc.lat != 0 || loc.lng != 0);
}

uint8_t AP_Radar::get_next_healthy_peer(uint8_t current_id)
{
    // Select on freshness, not health. A peer that is transmitting but has no
    // GPS fix yet reports 0/0, which fails the health check — selecting only
    // healthy peers made such a peer invisible even though we know its
    // callsign and link quality. The draw path still gates the bearing arrow
    // and distance on get_peer_healthy(), so a position-less peer shows its
    // name alone until a fix arrives.
    for (uint8_t i = 1; i < RADAR_MAX_PEERS; i++) {
        uint8_t next_peer = (current_id + i) % RADAR_MAX_PEERS;
        if (get_peer_fresh(next_peer)) {
            return next_peer;
        }
    }
    return current_id;
}

// Closest peer with a position, measured from loc. Keeps current_id unless
// another peer is at least 10% closer, so two aircraft at similar range do
// not swap labels every frame. Returns false when no peer has a position;
// the caller then falls back to cycling so position-less peers still show.
bool AP_Radar::get_nearest_healthy_peer(const Location &loc, uint8_t current_id, uint8_t &nearest_id)
{
    bool found = false;
    float best_dist = 0;
    for (uint8_t i = 0; i < RADAR_MAX_PEERS; i++) {
        if (!get_peer_healthy(i)) {
            continue;
        }
        const float dist = loc.get_distance(_state.peers[i].location);
        if (!found || dist < best_dist) {
            found = true;
            best_dist = dist;
            nearest_id = i;
        }
    }
    if (!found) {
        return false;
    }
    if (nearest_id != current_id && current_id < RADAR_MAX_PEERS && get_peer_healthy(current_id)) {
        const float current_dist = loc.get_distance(_state.peers[current_id].location);
        if (best_dist > current_dist * 0.9f) {
            nearest_id = current_id;
        }
    }
    return true;
}

AP_Radar *AP_Radar::_singleton;

namespace AP {

AP_Radar *radar()
{
    return AP_Radar::get_singleton();
}

}

#endif // AP_RADAR_ENABLED
