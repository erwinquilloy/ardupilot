/*
  CADDX mount using serial protocol backend class
 */
#pragma once

#include "AP_Mount_config.h"

#if HAL_MOUNT_CADDX_ENABLED

#include "AP_Mount_Backend_Serial.h"
#include <AP_Math/quaternion.h>

class AP_Mount_CADDX : public AP_Mount_Backend_Serial
{

public:
    // Constructor
    using AP_Mount_Backend_Serial::AP_Mount_Backend_Serial;

#if AP_MOUNT_CADDX_HEADTRACKER_ENABLED
    // fork: falls back to the head tracker port's TX when there is no gimbal port
    void init() override;
#endif

    // update mount position - should be called periodically
    void update() override;

    // has_pan_control - returns true if this mount can control its pan (required for multicopters)
    bool has_pan_control() const override { return yaw_range_valid(); };

    // has_roll_control - returns true if this mount can control its roll
    bool has_roll_control() const override { return roll_range_valid(); };

    // has_pitch_control - returns true if this mount can control its tilt
    bool has_pitch_control() const override { return pitch_range_valid(); };

protected:

    // get attitude as a quaternion.  returns true on success
    bool get_attitude_quaternion(Quaternion& att_quat) override;

private:

    // gimbal mode number in byte 2's low 3 bits (fork: GM-series gimbals use a
    // mode number, not lock flags; matches the Caddx tool's M0/M1/M2)
    enum class GimbalMode : uint8_t {
        FPV        = 0,
        PITCH_STAB = 1,
        HORIZON    = 2,
    };

    // CADDX can only send angles
    uint8_t natively_supported_mount_target_types() const override {
        return NATIVE_ANGLES_ONLY;
    };

    // send_target_angles
    void send_target_angles(const MountAngleTarget& angle_target_rad) override;
};
#endif // HAL_MOUNT_CADDX_ENABLED
