#pragma once

#include <AP_Param/AP_Param.h>

/*
  Fork: airspeed-scheduled pitch trim.

  PTCH_TRIM_SPD1..3 (m/s) and PTCH_TRIM_OFS1..3 (deg) define a curve of extra
  pitch trim, added to PTCH_TRIM_DEG, blended linearly between the points and
  held flat outside them. PTCH_TRIM_SPD1 = 0 turns it off.

  RCx_OPTION 254 is a 3-position switch: LOW = adaptive trim off (plain
  PTCH_TRIM_DEG, the curve is kept), MID = on, HIGH = learn. While learning in
  FBWB or CRUISE, the pitch the autopilot holds during steady, level,
  wings-level flight is recorded per 1 m/s airspeed bin; leaving HIGH fits a
  curve through the bins and saves the three points. With no switch assigned
  the adaptive trim is on whenever a curve is set.
 */
class PitchTrimSched {
public:
    PitchTrimSched();

    static const struct AP_Param::GroupInfo var_info[];

    // degrees to add to PTCH_TRIM_DEG right now (smoothed; 0 when off)
    float get_offset_deg() const { return offset_deg; }

    // multiplier for KFF_THR2PTCH: 1 while the curve is off, fading to 0 as
    // it takes over, because KFF_THR2PTCH does the same job by throttle and
    // would apply the speed correction a second time
    float get_kff_scale() const { return 1.0f - curve_weight; }

    // call at 10 Hz. fw_flying: fixed-wing flight past takeoff, where the
    // offset applies. learn_ok: also in a mode that holds altitude by itself
    // (FBWB / CRUISE), so the pitch it settles on is the level-flight pitch.
    // airspeed_min: AIRSPEED_MIN, for the stall taper.
    // kff_set: KFF_THR2PTCH is non-zero (for the one-time "ignored" notice).
    void update(bool fw_flying, bool learn_ok, float airspeed_min, bool kff_set);

    // RCx_OPTION 254 switch: 0 = LOW (off), 1 = MID (on), 2 = HIGH (learn).
    // at_boot: set on/off from the position but never start learning.
    // boot_pos_known: false when RC wasn't up at boot, so the position given
    // is a placeholder; the first real position is then treated as the boot
    // position (a switch left HIGH doesn't start learning when RC appears).
    void set_switch(uint8_t pos, bool at_boot, float base_trim_deg, bool boot_pos_known = true);
    bool learning() const { return learn.running; }

private:
    static constexpr uint8_t NUM_BINS = 50;        // 1 m/s bins, 0-50 m/s
    static constexpr uint16_t MIN_BIN_SAMPLES = 30; // 3 s of steady flight

    AP_Float spd1;
    AP_Float spd2;
    AP_Float spd3;
    AP_Float ofs1;
    AP_Float ofs2;
    AP_Float ofs3;

    float offset_deg;
    float curve_weight;    // 0 = curve off, 1 = curve fully applied
    bool kff_msg_sent;
    bool switched_off;     // switch assigned and LOW
    bool boot_sync_pending;
    uint32_t rc_valid_since_ms;
    float aspd_filt;
    bool have_aspd_filt;

    struct {
        bool running;
        float sum[NUM_BINS];
        uint16_t count[NUM_BINS];
        uint32_t last_report_ms;
    } learn;

    bool enabled() const;
    float curve_deg(float aspd) const;
    float scheduled_deg(float aspd, float airspeed_min) const;
    void learn_sample(float aspd, float aspd_rate);
    uint8_t learned_bins() const;
    void learn_start();
    void learn_stop(float base_trim_deg);
};
