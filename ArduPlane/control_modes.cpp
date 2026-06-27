#include "Plane.h"

#include "quadplane.h"
#include "qautotune.h"

Mode *Plane::mode_from_mode_num(const enum Mode::Number num)
{
    Mode *ret = nullptr;
    switch (num) {
    case Mode::Number::MANUAL:
        ret = &mode_manual;
        break;
    case Mode::Number::CIRCLE:
        ret = &mode_circle;
        break;
    case Mode::Number::STABILIZE:
        ret = &mode_stabilize;
        break;
    case Mode::Number::TRAINING:
        ret = &mode_training;
        break;
    case Mode::Number::ACRO:
        ret = &mode_acro;
        break;
    case Mode::Number::FLY_BY_WIRE_A:
        ret = &mode_fbwa;
        break;
    case Mode::Number::FLY_BY_WIRE_B:
        ret = &mode_fbwb;
        break;
    case Mode::Number::COURSE_HOLD:
        ret = &mode_course_hold;
        break;
    case Mode::Number::AUTO_TRIM:
        ret = &mode_auto_trim;
        break;
    case Mode::Number::CRUISE:
        ret = &mode_cruise;
        break;
    case Mode::Number::AUTOTUNE:
        ret = &mode_autotune;
        break;
    case Mode::Number::AUTO:
        ret = &mode_auto;
        break;
    case Mode::Number::RTL:
        ret = &mode_rtl;
        break;
    case Mode::Number::LOITER:
        ret = &mode_loiter;
        break;
    case Mode::Number::AVOID_ADSB:
#if HAL_ADSB_ENABLED
        ret = &mode_avoidADSB;
        break;
#endif
    // if ADSB is not compiled in then fallthrough to guided
    case Mode::Number::GUIDED:
        ret = &mode_guided;
        break;
    case Mode::Number::INITIALISING:
        ret = &mode_initializing;
        break;
#if HAL_QUADPLANE_ENABLED
    case Mode::Number::QSTABILIZE:
        ret = &mode_qstabilize;
        break;
    case Mode::Number::QHOVER:
        ret = &mode_qhover;
        break;
    case Mode::Number::QLOITER:
        ret = &mode_qloiter;
        break;
    case Mode::Number::QLAND:
        ret = &mode_qland;
        break;
    case Mode::Number::QRTL:
        ret = &mode_qrtl;
        break;
    case Mode::Number::QACRO:
        ret = &mode_qacro;
        break;
#if QAUTOTUNE_ENABLED
    case Mode::Number::QAUTOTUNE:
        ret = &mode_qautotune;
        break;
#endif
#endif  // HAL_QUADPLANE_ENABLED
    case Mode::Number::TAKEOFF:
        ret = &mode_takeoff;
        break;
    case Mode::Number::THERMAL:
#if HAL_SOARING_ENABLED
        ret = &mode_thermal;
#endif
        break;
#if HAL_QUADPLANE_ENABLED
    case Mode::Number::LOITER_ALT_QLAND:
        ret = &mode_loiter_qland;
        break;
#endif  // HAL_QUADPLANE_ENABLED

    }
    return ret;
}

// Fork FLTMODE_EXT: number of FLTMODE switch positions in use. 12 when
// the extension is enabled, 6 otherwise (the upstream constant).
uint8_t Plane::num_flight_modes() const
{
    return g2.fltmode_ext == 0 ? 6 : 12;
}

void RC_Channels_Plane::read_mode_switch()
{
    if (millis() - plane.failsafe.last_valid_rc_ms > 100) {
        // only use signals that are less than 0.1s old.
        return;
    }
    if (plane.g2.fltmode_ext == 0) {
        // Upstream 6-position path -- no behaviour change with default param.
        RC_Channels::read_mode_switch();
        return;
    }
    // Fork FLTMODE_EXT: route the read through the per-channel 12-pos
    // helper.  flight_mode_channel() is private on RC_Channels so we
    // look the channel up the same way that base code does -- via the
    // FLTMODE_CH param and the public channel() accessor.
    const int8_t fltmode_ch = flight_mode_channel_number();
    if (fltmode_ch < 1) {
        return;
    }
    RC_Channel_Plane *c = channel(fltmode_ch - 1);
    if (c == nullptr) {
        return;
    }
    c->read_12pos_mode_switch();
}

void RC_Channel_Plane::mode_switch_changed(modeswitch_pos_t new_pos)
{
    if (new_pos < 0 || (uint8_t)new_pos >= plane.num_flight_modes()) {
        // should not have been called
        return;
    }

    // Fork FLTMODE_EXT: positions 0..5 map to upstream g.flight_mode1..6,
    // positions 6..11 map to g2.flight_mode7..12 via plane.flight_modes2.
    const AP_Int8 *modes = (new_pos < 6 || plane.flight_modes2 == nullptr)
                               ? &plane.flight_modes[new_pos]
                               : &plane.flight_modes2[(uint8_t)new_pos - 6];
    plane.set_mode_by_number((Mode::Number)modes->get(), ModeReason::RC_COMMAND);
}

// Fork FLTMODE_EXT: 12-position PWM scan. Uses the same debounce path as
// the upstream 6-position helper so behaviour is symmetric.
bool RC_Channel_Plane::read_12pos_switch(int8_t &position)
{
    const uint16_t pulsewidth = get_radio_in();
    if (pulsewidth <= RC_MIN_LIMIT_PWM || pulsewidth >= RC_MAX_LIMIT_PWM) {
        return false;  // signal lost / out of range
    }

    // 11 thresholds -> 12 positions, ~75 us per bin starting at 1126 us.
    if      (pulsewidth < 1126) position = 0;
    else if (pulsewidth < 1201) position = 1;
    else if (pulsewidth < 1276) position = 2;
    else if (pulsewidth < 1351) position = 3;
    else if (pulsewidth < 1426) position = 4;
    else if (pulsewidth < 1501) position = 5;
    else if (pulsewidth < 1576) position = 6;
    else if (pulsewidth < 1651) position = 7;
    else if (pulsewidth < 1726) position = 8;
    else if (pulsewidth < 1801) position = 9;
    else if (pulsewidth < 1876) position = 10;
    else                        position = 11;

    return debounce_completed(position);
}

void RC_Channel_Plane::read_12pos_mode_switch()
{
    int8_t position;
    if (read_12pos_switch(position)) {
        mode_switch_changed(modeswitch_pos_t(position));
    }
}

/*
  called when entering autotune
 */
void Plane::autotune_start(void)
{
    const bool tune_roll = g2.axis_bitmask.get() & int8_t(AutoTuneAxis::ROLL);
    const bool tune_pitch = g2.axis_bitmask.get() & int8_t(AutoTuneAxis::PITCH);
    const bool tune_yaw = g2.axis_bitmask.get() & int8_t(AutoTuneAxis::YAW);
    if (tune_roll || tune_pitch || tune_yaw) {
        gcs().send_text(MAV_SEVERITY_INFO, "Started autotune");
        if (tune_roll) { 
            rollController.autotune_start();
        }
        if (tune_pitch) { 
            pitchController.autotune_start();
        }
        if (tune_yaw) { 
            yawController.autotune_start();
        }
        autotuning = true;
        gcs().send_text(MAV_SEVERITY_INFO, "Autotuning %s%s%s", tune_roll?"roll ":"", tune_pitch?"pitch ":"", tune_yaw?"yaw":"");
    } else {
        gcs().send_text(MAV_SEVERITY_INFO, "No axis selected for tuning!");
    }        
}

/*
  called when exiting autotune
 */
void Plane::autotune_restore(void)
{
    rollController.autotune_restore();
    pitchController.autotune_restore();
    yawController.autotune_restore();
    if (autotuning) {
        autotuning = false;
        gcs().send_text(MAV_SEVERITY_INFO, "Stopped autotune");
    }
}

/*
  enable/disable autotune for AUTO modes
 */
void Plane::autotune_enable(bool enable)
{
    if (enable) {
        autotune_start();
    } else {
        autotune_restore();
    }
}

/*
  are we flying inverted?
 */
bool Plane::fly_inverted(void)
{
    if (control_mode == &plane.mode_manual) {
        return false;
    }
    if (inverted_flight) {
        // controlled with aux switch
        return true;
    }
    if (control_mode == &mode_auto && auto_state.inverted_flight) {
        return true;
    }
    return false;
}
