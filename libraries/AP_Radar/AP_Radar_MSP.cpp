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

#include "AP_Radar_MSP.h"

#if HAL_MSP_RADAR_ENABLED

#include <AP_HAL/AP_HAL.h>
#include <AP_AHRS/AP_AHRS.h>
#include <AP_Common/Location.h>

extern const AP_HAL::HAL& hal;

using namespace MSP;

AP_Radar_MSP *AP_Radar_MSP::detect(AP_Radar &_frontend)
{
    // backend is fed by the AP_MSP parser via handle_msp(); no UART of its own.
    return NEW_NOTHROW AP_Radar_MSP(_frontend);
}

void AP_Radar_MSP::update(void)
{
    if (update_count == 0) {
        return;
    }

    struct AP_Radar::Radar_state state {};

    for (uint8_t i = 0; i < RADAR_MAX_PEERS; i++) {
        state.peers[i] = peers[i];
    }

    _update_frontend(state);

    update_count = 0;
}

void AP_Radar_MSP::handle_msp(const MSP::msp_radar_pos_message_t &pkt, const char *name)
{
    const uint8_t id = pkt.radar_no;
    if (id >= RADAR_MAX_PEERS) {
        return;
    }
    peers[id].radar_no = pkt.radar_no;
    peers[id].state    = pkt.state;
    peers[id].location = Location(pkt.lat, pkt.lon, pkt.alt, Location::AltFrame::ABSOLUTE);
    peers[id].heading  = pkt.heading;
    peers[id].speed    = pkt.speed;
    peers[id].lq       = pkt.lq;
    peers[id].last_update = AP_HAL::millis();
    // The callsign is reassembled a character at a time from the radio's
    // extra_value slots, so a peer that has only just been heard sends an empty
    // or partial name. Ignore it until the first character is printable, which
    // keeps the OSD on its slot-letter fallback rather than showing blanks, and
    // leaves the last good name in place if a peer stops supplying one.
    if (name != nullptr && name[0] >= 0x20 && name[0] <= 0x7E) {
        uint8_t i = 0;
        for (; i < MSP_RADAR_NAME_LEN - 1 && name[i] != '\0'; i++) {
            const char c = name[i];
            peers[id].name[i] = (c >= 0x20 && c <= 0x7E) ? c : ' ';
        }
        peers[id].name[i] = '\0';
    }
    update_count++;
}

#endif // HAL_MSP_RADAR_ENABLED
