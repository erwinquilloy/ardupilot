#pragma once

#include <AP_Arming/AP_Arming.h>
#include "mode.h"

#ifndef AP_PLANE_BLACKBOX_LOGGING
#define AP_PLANE_BLACKBOX_LOGGING 0
#endif

/*
  a plane specific arming class
 */
class AP_Arming_Plane : public AP_Arming
{
public:
    AP_Arming_Plane()
        : AP_Arming()
    {
        AP_Param::setup_object_defaults(this, var_info);
    }

    /* Do not allow copies */
    CLASS_NO_COPY(AP_Arming_Plane);

    bool pre_arm_checks(bool report) override;
    bool arm_checks(AP_Arming::Method method) override;

    // var_info for holding Parameter information
    static const struct AP_Param::GroupInfo var_info[];

    // Fork PR #30: "arming switch safety" — when a disarm is requested
    // by an aux switch while the plane is still flying, we do NOT disarm.
    // We cut the throttle instead and (for auto-throttle modes) switch to
    // FBWA so the pilot can fly the plane down. Once the plane has actually
    // landed, the next loop iteration calls disarm_if_requested() which
    // completes the deferred disarm.
    void disarm_if_requested();
    bool disarm(AP_Arming::Method method, bool do_disarm_checks=true) override;
    bool arm(AP_Arming::Method method, bool do_arming_checks=true) override;

    void update_soft_armed();
    bool get_delay_arming() const { return delay_arming; };

    void set_throttle_cut(bool status);
    bool get_throttle_cut() const { return throttle_cut; };

    // mandatory checks that cannot be bypassed.  This function will only be called if ARMING_CHECK is zero or arming forced
    bool mandatory_checks(bool display_failure) override;

protected:
    bool ins_checks(bool report) override;
    bool terrain_database_required() const override;

    bool quadplane_checks(bool display_failure);
    bool mission_checks(bool report) override;

    // Checks rc has been received if it is configured to be used
    bool rc_received_if_enabled_check(bool display_failure);

private:
    void change_arm_state(void);

    // Fork PR #30: throttle cut state. True while the user has requested
    // disarm via aux switch but the plane is still flying.
    bool throttle_cut = false;

    // mode the plane was in when throttle cut was enabled (so we can
    // restore it on rearm). Null if throttle cut was activated outside
    // of an auto-throttle mode (no restore needed).
    Mode *throttle_cut_prev_mode;

    // oneshot with duration AP_ARMING_DELAY_MS used by quadplane to delay spoolup after arming:
    // ignored unless OPTION_DELAY_ARMING or OPTION_TILT_DISARMED is set
    bool delay_arming;

#if AP_PLANE_BLACKBOX_LOGGING
    AP_Float blackbox_speed;
    uint32_t last_over_3dspeed_ms;
#endif
};
