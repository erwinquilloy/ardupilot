#include "AP_HeadTracker_CADDX.h"

#if AP_MOUNT_CADDX_HEADTRACKER_ENABLED

#include "AP_Mount.h"
#include <AP_Math/AP_Math.h>
#include <AP_Math/crc.h>
#include <AP_SerialManager/AP_SerialManager.h>
#include <GCS_MAVLink/GCS.h>

AP_HeadTracker_CADDX *AP_HeadTracker_CADDX::create()
{
    AP_HAL::UARTDriver *uart = AP::serialmanager().find_serial(AP_SerialManager::SerialProtocol_CADDX_HeadTracker, 0);
    if (uart == nullptr) {
        return nullptr;
    }
    return NEW_NOTHROW AP_HeadTracker_CADDX(uart);
}

void AP_HeadTracker_CADDX::update()
{
    if (_uart == nullptr) {
        return;
    }

    uint32_t numc = MIN(_uart->available(), 256U);

    while (numc-- > 0) {
        _buf[_len++] = _uart->read();

        // resynchronise on the two header bytes
        if (_len == 1 && _buf[0] != HEADER1) {
            _len = 0;
            continue;
        }
        if (_len == 2 && _buf[1] != HEADER2) {
            // the byte we just rejected may itself start a header
            _buf[0] = _buf[1];
            _len = (_buf[0] == HEADER1) ? 1 : 0;
            continue;
        }
        if (_len == FRAME_LEN) {
            handle_frame();
            _len = 0;
        }
    }
}

void AP_HeadTracker_CADDX::handle_frame()
{
    // CRC16-CCITT over everything but the trailing checksum, big-endian on the wire
    const uint16_t crc = crc16_ccitt(_buf, FRAME_LEN - 2, 0);
    if (HIGHBYTE(crc) != _buf[8] || LOWBYTE(crc) != _buf[9]) {
        return;
    }

    // 12-bit fields, packed exactly as AP_Mount_CADDX writes them. Bytes 2 and
    // 3's low nibble (lock mode, sensitivity) are ignored: the goggles' recentre
    // arrives as re-zeroed angles, not as a flag (bench-checked with an Ascent).
    const uint16_t roll_raw  = (_buf[3] >> 4) | ((uint16_t)_buf[4] << 4);
    const uint16_t pitch_raw = _buf[5] | ((uint16_t)(_buf[6] & 0x0F) << 8);
    const uint16_t yaw_raw   = (_buf[6] >> 4) | ((uint16_t)_buf[7] << 4);

    // 0..4096 spans a full turn; wrap_180 folds the upper half to negative angles.
    // Taken as sent: Walksnail VTX firmware before 18.21.14 mirrored pitch and yaw
    // on this port (fixed there, "yaw and pitch axes were reversed"), so negating
    // them here would reverse both axes on current firmware.
    const float scale = 360.0f / AXIS_MAX;
    const float roll_deg  = wrap_180(roll_raw * scale);
    const float pitch_deg = wrap_180(pitch_raw * scale);
    const float yaw_deg   = wrap_180(yaw_raw * scale);

    if (!_announced) {
        _announced = true;
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "CDXHT: head tracker stream up");
    }

    AP_Mount *mount = AP::mount();
    if (mount != nullptr) {
        mount->handle_caddx_headtracker(roll_deg, pitch_deg, yaw_deg);
    }
}

#endif  // AP_MOUNT_CADDX_HEADTRACKER_ENABLED
