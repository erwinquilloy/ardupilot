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
#pragma once

#include "AP_Radar_config.h"

#if AP_RADAR_ENABLED

/*
 * AP_Radar.h - peer-aircraft radar frontend (iNav Radar / FormationFlight wire-compatible).
 * Forward-ported from MUSTARDTIGERFPV/ArduPilot commit 35f5f0ef4f.
 */

#include <AP_MSP/msp.h>
#include <AP_Math/AP_Math.h>
#include <GCS_MAVLink/GCS_MAVLink.h>
#include <AP_Common/Location.h>
#include <AP_Param/AP_Param.h>

#define RADAR_MAX_PEERS 6
#define RADAR_PEER_FRESH_TIME_MS 3000

typedef struct radar_peer_t {
    uint8_t radar_no;
    uint8_t state;
    Location location;
    uint16_t heading;
    uint16_t speed;
    uint8_t lq;
    uint32_t last_update;
    // Peer callsign, empty when the sender does not supply one (stock iNav
    // Radar / FormationFlight). Sanitised to printable ASCII on receipt.
    char name[MSP_RADAR_NAME_LEN];
} radar_peer_t;

class Radar_backend;

class AP_Radar
{
    friend class Radar_backend;

public:
    AP_Radar();

    CLASS_NO_COPY(AP_Radar);

    static AP_Radar *get_singleton() {
        return _singleton;
    }

    enum class Type {
        NONE = 0,
        MSP  = 1,
    };

    void init(uint32_t log_bit);
    bool enabled() const { return _type != Type::NONE; }
    bool healthy() const { return backend != nullptr && _flags.healthy; }
    void update(void);

    void handle_msg(const mavlink_message_t &msg);

#if HAL_MSP_RADAR_ENABLED
    // name is nullptr when the sender did not append a peer callsign
    void handle_msp(const MSP::msp_radar_pos_message_t &pkt, const char *name);
#endif

    struct Radar_state {
        radar_peer_t peers[RADAR_MAX_PEERS];
    };

    radar_peer_t get_peer(uint8_t id);
    // fresh = heard recently. healthy = fresh AND has a usable position.
    bool get_peer_fresh(uint8_t id);
    bool get_peer_healthy(uint8_t id);
    uint8_t get_next_healthy_peer(uint8_t current_id);

    static const struct AP_Param::GroupInfo var_info[];

private:
    static AP_Radar *_singleton;

    Radar_backend *backend;

    struct AP_Radar_Flags {
        uint8_t healthy : 1;
    } _flags;

    AP_Enum<Type> _type;

    void update_state(const Radar_state &state);

    struct Radar_state _state;

    uint32_t _last_update_ms;
    uint32_t _log_bit = -1;
};

namespace AP {
    AP_Radar *radar();
}

#include "AP_Radar_Backend.h"

#endif // AP_RADAR_ENABLED
