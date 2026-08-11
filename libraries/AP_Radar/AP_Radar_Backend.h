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

#include "AP_Radar.h"

#if AP_RADAR_ENABLED

class Radar_backend
{
    friend class AP_Radar;

public:
    Radar_backend(AP_Radar &_frontend);
    virtual ~Radar_backend(void);

    CLASS_NO_COPY(Radar_backend);

    virtual void init() {}
    virtual void update() = 0;

    virtual void handle_msg(const mavlink_message_t &msg) {}

#if HAL_MSP_RADAR_ENABLED
    virtual void handle_msp(const MSP::msp_radar_pos_message_t &pkt, const char *name) {}
#endif

protected:
    AP_Radar &frontend;

    void _update_frontend(const struct AP_Radar::Radar_state &state);

    HAL_Semaphore _sem;
};

#endif // AP_RADAR_ENABLED
