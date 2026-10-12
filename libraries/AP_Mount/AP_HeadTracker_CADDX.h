#pragma once

#include "AP_Mount_config.h"

#if AP_MOUNT_CADDX_HEADTRACKER_ENABLED

#include <AP_HAL/AP_HAL.h>

/*
  Fork feature: head tracker input from a Walksnail/CADDX VTX gimbal port.

  The VTX drives a CADDX gimbal (e.g. GM3 v2) directly from its second UART
  using the same 10-byte set-attitude frame that AP_Mount_CADDX sends. Most
  Walksnail goggles also push MSP2_SENSOR_HEADTRACKER to the autopilot, but some
  only ever put head tracking on that port, so listening to it here is the only
  way to get head tracking from them.

  Wire the VTX gimbal port's TX to a free autopilot RX pad and set that port's
  SERIALn_PROTOCOL to 51. The decoded angles feed the same mount head-tracking
  path as the MSP source, so the RCx_OPTION enable/center-lock switches, the
  POI lock hand-off, the yaw unwrap and the stream-loss auto-lock all behave
  identically.
 */
class AP_HeadTracker_CADDX
{
public:
    AP_HeadTracker_CADDX(AP_HAL::UARTDriver *uart) : _uart(uart) {}

    CLASS_NO_COPY(AP_HeadTracker_CADDX);

    // returns an instance if a serial port is configured for the protocol, else nullptr
    static AP_HeadTracker_CADDX *create();

    // read and decode pending bytes. Call at the mount update rate.
    void update();

private:
    // frame layout, mirroring AP_Mount_CADDX::send_target_angles()
    static const uint8_t HEADER1 = 0xA5;
    static const uint8_t HEADER2 = 0x5A;
    static const uint8_t FRAME_LEN = 10;
    static const uint16_t AXIS_MAX = 4096;   // one full turn

    void handle_frame();

    AP_HAL::UARTDriver *_uart;
    uint8_t _buf[FRAME_LEN];
    uint8_t _len;
    bool _announced;    // "stream up" sent once, on the first valid frame
};

#endif  // AP_MOUNT_CADDX_HEADTRACKER_ENABLED
