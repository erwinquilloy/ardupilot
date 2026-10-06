#include "pitch_trim_sched.h"

#include <AP_AHRS/AP_AHRS.h>
#include <AP_Math/AP_Math.h>
#include <GCS_MAVLink/GCS.h>
#include <RC_Channel/RC_Channel.h>
#include <string.h>

const AP_Param::GroupInfo PitchTrimSched::var_info[] = {
    // @Param: SPD1
    // @DisplayName: Pitch trim schedule speed 1
    // @Description: Lowest airspeed point of the airspeed-scheduled pitch trim. At or below this airspeed PTCH_TRIM_OFS1 is added to PTCH_TRIM_DEG. Set 0 to disable the schedule. The three speeds must increase. Can be learned in flight with the Pitch Trim Learn switch (RCx_OPTION 254). Near the stall the schedule is tapered: from 1.25 x AIRSPEED_MIN down to AIRSPEED_MIN the extra trim fades to the least nose-up of the three offsets, so it never raises the nose to hold altitude while slowing towards the stall. Set KFF_THR2PTCH to 0 when using this schedule: it adds pitch with throttle and would apply the same correction a second time.
    // @Units: m/s
    // @Range: 0 60
    // @Increment: 0.1
    // @User: Advanced
    AP_GROUPINFO("SPD1", 1, PitchTrimSched, spd1, 0),

    // @Param: SPD2
    // @DisplayName: Pitch trim schedule speed 2
    // @Description: Middle airspeed point of the airspeed-scheduled pitch trim. The extra trim is blended linearly between the points.
    // @Units: m/s
    // @Range: 0 60
    // @Increment: 0.1
    // @User: Advanced
    AP_GROUPINFO("SPD2", 2, PitchTrimSched, spd2, 0),

    // @Param: SPD3
    // @DisplayName: Pitch trim schedule speed 3
    // @Description: Highest airspeed point of the airspeed-scheduled pitch trim. At or above this airspeed PTCH_TRIM_OFS3 is added to PTCH_TRIM_DEG.
    // @Units: m/s
    // @Range: 0 60
    // @Increment: 0.1
    // @User: Advanced
    AP_GROUPINFO("SPD3", 3, PitchTrimSched, spd3, 0),

    // @Param: OFS1
    // @DisplayName: Pitch trim schedule offset 1
    // @Description: Pitch trim added to PTCH_TRIM_DEG at PTCH_TRIM_SPD1 and below. Positive is nose up.
    // @Units: deg
    // @Range: -10 10
    // @Increment: 0.1
    // @User: Advanced
    AP_GROUPINFO("OFS1", 4, PitchTrimSched, ofs1, 0),

    // @Param: OFS2
    // @DisplayName: Pitch trim schedule offset 2
    // @Description: Pitch trim added to PTCH_TRIM_DEG at PTCH_TRIM_SPD2. Positive is nose up.
    // @Units: deg
    // @Range: -10 10
    // @Increment: 0.1
    // @User: Advanced
    AP_GROUPINFO("OFS2", 5, PitchTrimSched, ofs2, 0),

    // @Param: OFS3
    // @DisplayName: Pitch trim schedule offset 3
    // @Description: Pitch trim added to PTCH_TRIM_DEG at PTCH_TRIM_SPD3 and above. Positive is nose up.
    // @Units: deg
    // @Range: -10 10
    // @Increment: 0.1
    // @User: Advanced
    AP_GROUPINFO("OFS3", 6, PitchTrimSched, ofs3, 0),

    AP_GROUPEND
};

PitchTrimSched::PitchTrimSched()
{
    AP_Param::setup_object_defaults(this, var_info);
}

bool PitchTrimSched::enabled() const
{
    return is_positive(spd1) && spd2 > spd1 && spd3 > spd2;
}

// extra trim (deg) for an airspeed: flat outside the points, linear between
float PitchTrimSched::curve_deg(float aspd) const
{
    if (aspd <= spd1) {
        return ofs1;
    }
    if (aspd <= spd2) {
        return linear_interpolate(ofs1, ofs2, aspd, spd1, spd2);
    }
    if (aspd <= spd3) {
        return linear_interpolate(ofs2, ofs3, aspd, spd2, spd3);
    }
    return ofs3;
}

/*
  the curve with a stall taper: from 1.25 x AIRSPEED_MIN down to AIRSPEED_MIN
  the extra trim fades to the least nose-up value on the curve (normally the
  fast end), so the schedule never pitches the nose up to hold altitude while
  the plane is slowing towards the stall. It only ever removes nose-up: the
  target is the curve's minimum, not zero, because the offsets are relative to
  PTCH_TRIM_DEG and can all be negative.
 */
float PitchTrimSched::scheduled_deg(float aspd, float airspeed_min) const
{
    const float ofs_curve = curve_deg(aspd);
    if (!is_positive(airspeed_min)) {
        return ofs_curve;
    }
    const float taper_top = 1.25f * airspeed_min;
    if (aspd >= taper_top) {
        return ofs_curve;
    }
    const float least_nose_up = MIN(MIN(ofs1.get(), ofs2.get()), ofs3.get());
    const float k = constrain_float((aspd - airspeed_min) / (taper_top - airspeed_min), 0, 1);
    return least_nose_up + k * (ofs_curve - least_nose_up);
}

void PitchTrimSched::update(bool fw_flying, bool learn_ok, float airspeed_min)
{
    // a boot without RC left the switch position unknown. Once RC has been
    // valid for 1 s, any real position has already arrived through the normal
    // switch debounce; if none came the switch is still LOW, and the next
    // change is a deliberate one again
    if (boot_sync_pending) {
        const uint32_t now = AP_HAL::millis();
        if (!rc().has_valid_input()) {
            rc_valid_since_ms = 0;
        } else if (rc_valid_since_ms == 0) {
            rc_valid_since_ms = now;
        } else if (now - rc_valid_since_ms > 1000) {
            boot_sync_pending = false;
        }
    }

    float aspd;
    const bool have_aspd = AP::ahrs().airspeed_EAS(aspd);
    float aspd_rate = 0;
    if (have_aspd) {
        if (!have_aspd_filt) {
            aspd_filt = aspd;
            have_aspd_filt = true;
        }
        const float prev = aspd_filt;
        // ~0.3 s smoothing at 10 Hz: airspeed estimates are noisy, and the
        // trim must not follow every gust
        aspd_filt += (aspd - aspd_filt) * 0.3f;
        aspd_rate = (aspd_filt - prev) * 10;
    }

    float target = 0;
    if (fw_flying && have_aspd && enabled() && !switched_off) {
        target = scheduled_deg(aspd_filt, airspeed_min);
    }
    // ~1 s time constant, so entering or leaving fixed-wing flight, or a
    // parameter change, never steps the pitch demand
    offset_deg += (target - offset_deg) * 0.1f;

    if (learn.running) {
        if (learn_ok && have_aspd) {
            learn_sample(aspd_filt, aspd_rate);
        }
        const uint32_t now = AP_HAL::millis();
        if (now - learn.last_report_ms > 10000) {
            learn.last_report_ms = now;
            GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Adaptive trim learn: %u speeds", unsigned(learned_bins()));
        }
    }
}

void PitchTrimSched::learn_sample(float aspd, float aspd_rate)
{
    const AP_AHRS &ahrs = AP::ahrs();
    // steady, wings-level, level flight only: the pitch then is exactly what
    // this speed needs to hold altitude
    if (fabsf(degrees(ahrs.get_roll_rad())) > 5) {
        return;
    }
    Vector3f vel;
    if (!ahrs.get_velocity_NED(vel) || fabsf(vel.z) > 0.5f) {
        return;
    }
    if (fabsf(aspd_rate) > 0.5f) {
        return;
    }
    const int bin = int(aspd);
    if (bin < 0 || bin >= NUM_BINS) {
        return;
    }
    if (learn.count[bin] < UINT16_MAX) {
        learn.sum[bin] += degrees(ahrs.get_pitch_rad());
        learn.count[bin]++;
    }
}

uint8_t PitchTrimSched::learned_bins() const
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < NUM_BINS; i++) {
        n += learn.count[i] >= MIN_BIN_SAMPLES;
    }
    return n;
}

void PitchTrimSched::set_switch(uint8_t pos, bool at_boot, float base_trim_deg, bool boot_pos_known)
{
    if (at_boot) {
        boot_sync_pending = !boot_pos_known;
    } else if (boot_sync_pending) {
        boot_sync_pending = false;
        at_boot = true;
    }
    // leaving HIGH ends a learn run and saves what it got
    if (pos != 2 && learn.running) {
        learn_stop(base_trim_deg);
    }
    const bool was_off = switched_off;
    switched_off = (pos == 0);
    if (pos == 2 && !at_boot) {
        learn_start();
        return;
    }
    if (!at_boot && was_off != switched_off) {
        GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Adaptive trim %s", switched_off ? "off" : "on");
    }
}

void PitchTrimSched::learn_start()
{
    if (learn.running) {
        return;
    }
    memset(&learn, 0, sizeof(learn));
    learn.running = true;
    learn.last_report_ms = AP_HAL::millis();
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Adaptive trim learn: fly level FBWB/CRUISE");
}

void PitchTrimSched::learn_stop(float base_trim_deg)
{
    if (!learn.running) {
        return;
    }
    learn.running = false;

    // weighted least-squares fit of pitch against airspeed over the bins with
    // enough samples; quadratic with 4+ speeds (level-flight pitch curves
    // flatten out with speed), linear with 3. Speeds are centred on the middle
    // of the range to keep the normal equations well conditioned.
    float vmin = 1e6, vmax = -1;
    uint8_t n = 0;
    for (uint8_t i = 0; i < NUM_BINS; i++) {
        if (learn.count[i] >= MIN_BIN_SAMPLES) {
            const float v = i + 0.5f;
            vmin = MIN(vmin, v);
            vmax = MAX(vmax, v);
            n++;
        }
    }
    if (n < 3 || vmax - vmin < 3) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Adaptive trim: %u speeds (need 3 over 3m/s)", unsigned(n));
        return;
    }
    const float vc = 0.5f * (vmin + vmax);
    float S[5] {};   // sum w * x^k
    float T[3] {};   // sum w * y * x^k
    for (uint8_t i = 0; i < NUM_BINS; i++) {
        if (learn.count[i] < MIN_BIN_SAMPLES) {
            continue;
        }
        const float x = (i + 0.5f) - vc;
        const float y = learn.sum[i] / learn.count[i];
        // cap the weight so one long straight line can't swamp the others
        const float w = MIN(learn.count[i], 300);
        float xk = 1;
        for (uint8_t k = 0; k < 5; k++) {
            S[k] += w * xk;
            if (k < 3) {
                T[k] += w * y * xk;
            }
            xk *= x;
        }
    }
    float a, b, c = 0;
    if (n >= 4) {
        // solve [S0 S1 S2; S1 S2 S3; S2 S3 S4] [a b c]' = T by Cramer's rule
        const float det = S[0]*(S[2]*S[4] - S[3]*S[3]) - S[1]*(S[1]*S[4] - S[3]*S[2]) + S[2]*(S[1]*S[3] - S[2]*S[2]);
        if (is_zero(det)) {
            GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Adaptive trim: fit failed, not saved");
            return;
        }
        a = (T[0]*(S[2]*S[4] - S[3]*S[3]) - S[1]*(T[1]*S[4] - S[3]*T[2]) + S[2]*(T[1]*S[3] - S[2]*T[2])) / det;
        b = (S[0]*(T[1]*S[4] - S[3]*T[2]) - T[0]*(S[1]*S[4] - S[3]*S[2]) + S[2]*(S[1]*T[2] - T[1]*S[2])) / det;
        c = (S[0]*(S[2]*T[2] - T[1]*S[3]) - S[1]*(S[1]*T[2] - T[1]*S[2]) + T[0]*(S[1]*S[3] - S[2]*S[2])) / det;
    } else {
        const float det = S[0]*S[2] - S[1]*S[1];
        if (is_zero(det)) {
            GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Adaptive trim: fit failed, not saved");
            return;
        }
        b = (S[0]*T[1] - S[1]*T[0]) / det;
        a = (T[0] - b*S[1]) / S[0];
    }

    const float v[3] { vmin, vc, vmax };
    AP_Float *spd[3] { &spd1, &spd2, &spd3 };
    AP_Float *ofs[3] { &ofs1, &ofs2, &ofs3 };
    float o[3];
    for (uint8_t i = 0; i < 3; i++) {
        const float x = v[i] - vc;
        const float pitch = a + b*x + c*x*x;
        o[i] = constrain_float(pitch - base_trim_deg, -10, 10);
        spd[i]->set_and_save(v[i]);
        ofs[i]->set_and_save(o[i]);
    }
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Adaptive trim: %.1f %+.1f %.1f %+.1f %.1f %+.1f",
                  (double)v[0], (double)o[0], (double)v[1], (double)o[1], (double)v[2], (double)o[2]);
}
