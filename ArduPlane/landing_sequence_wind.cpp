/*
   Fork-only: wind-biased DO_LAND_START selection.

   Upstream AP_Mission::get_landing_sequence_start() picks the
   DO_LAND_START with the smallest straight-line distance from the
   current position.  In wind, that frequently picks a downwind
   approach.

   This helper walks the same DO_LAND_START list but scores each
   candidate by both distance and how well its approach aligns with
   the wind.  When LAND_WIND_BIAS is 0 (default), the upstream
   nearest-only behaviour is used unchanged so saved missions land
   identically.  When LAND_WIND_BIAS > 0, tailwind candidates are
   penalised proportional to alignment * distance.

   The approach heading for a DO_LAND_START is derived as the bearing
   from the start of approach to the next NAV cmd in the sequence
   (typically the descending leg toward NAV_LAND).  If no following
   NAV cmd can be found, the candidate is scored by distance only.
*/

#include "Plane.h"

bool Plane::try_upwind_jump_to_landing_sequence(const Location &from_loc)
{
    const float bias = g2.rtl_land_wind_bias.get();
    if (!is_positive(bias)) {
        // Disabled: upstream behaviour, including its "Landing sequence
        // start" / "Unable to start landing sequence" GCS messages.
        return mission.jump_to_landing_sequence(from_loc);
    }

    const Vector3f wind = ahrs.wind_estimate();
    const float wind_speed = wind.xy().length();
    if (wind_speed < 1.0f) {
        // Wind estimate too weak to trust; don't risk a wrong pick.
        return mission.jump_to_landing_sequence(from_loc);
    }
    // atan2 of the wind vector gives the "to" direction.  Flip for the
    // "from" direction that approach alignment is measured against.
    const float wind_from_rad = atan2f(-wind.y, -wind.x);

    // LAND_WIND_DIST cap: DO_LAND_STARTs whose start-of-approach sits
    // farther than dist_limit from the caller's from_loc (the plane's
    // position at decision time) are excluded from selection entirely.
    // The wind bias then picks among the survivors.
    //   - RTL_AUTOLAND=1: caller passes plane.current_loc after the
    //     plane has reached the RTL loiter point, so from_loc is
    //     effectively home.  Cap is measured from home.
    //   - RTL_AUTOLAND=2 / batt FS / GCS DO_LAND_START: caller passes
    //     the plane's actual current position, so the cap means
    //     "landings within dist_limit of where I am right now" --
    //     avoids adding significant travel just for wind alignment.
    // 0 means no cap.
    const float dist_limit = g2.rtl_land_wind_bias_dist.get();
    const bool have_dist_cap = is_positive(dist_limit);

    uint16_t best_idx = 0;
    float best_score = -1.0f;
    uint16_t nearest_idx = 0;
    float min_distance = -1.0f;

    const uint16_t count = mission.num_commands();
    for (uint16_t i = 1; i < count; i++) {
        // get_command_id() is private on AP_Mission, so read the full
        // command and check ::id here.  Same as the inner half of
        // upstream's AP_Mission::get_landing_sequence_start().
        AP_Mission::Mission_Command land_start;
        if (!mission.read_cmd_from_storage(i, land_start)) {
            continue;
        }
        if (land_start.id != MAV_CMD_DO_LAND_START) {
            continue;
        }

        // Mirror upstream's fallback: if the DO_LAND_START itself has
        // no location, use the next nav cmd's location as the IP.
        Location ip_loc;
        AP_Mission::Mission_Command first_nav;
        const bool has_own_loc = land_start.content.location.initialised();
        const bool have_first_nav = mission.get_next_nav_cmd(i + 1, first_nav);
        if (has_own_loc) {
            ip_loc = land_start.content.location;
        } else if (have_first_nav && first_nav.content.location.initialised()) {
            ip_loc = first_nav.content.location;
        } else {
            // No usable location for this candidate.
            continue;
        }

        // LAND_WIND_DIST hard cap: candidates farther than dist_limit
        // from from_loc are dropped entirely.  If every candidate is
        // out of range, best_idx stays 0 and we fall through to
        // upstream nearest-only selection below (safer than picking
        // nothing).
        if (have_dist_cap && from_loc.get_distance(ip_loc) > dist_limit) {
            continue;
        }

        const float distance = ip_loc.get_distance_NED_alt_frame(from_loc).length();
        if (min_distance < 0 || distance < min_distance) {
            min_distance = distance;
            nearest_idx = i;
        }

        // Approach heading: bearing from the IP toward the next nav cmd
        // after the DO_LAND_START.  When DO_LAND_START has no own
        // location, IP already IS that first nav cmd, so the bearing
        // baseline becomes IP -> the nav cmd after it.
        float score = distance;
        Location approach_loc;
        bool have_approach = false;
        if (has_own_loc) {
            if (have_first_nav && first_nav.content.location.initialised()) {
                approach_loc = first_nav.content.location;
                have_approach = true;
            }
        } else {
            AP_Mission::Mission_Command second_nav;
            if (have_first_nav &&
                mission.get_next_nav_cmd(first_nav.index + 1, second_nav) &&
                second_nav.content.location.initialised()) {
                approach_loc = second_nav.content.location;
                have_approach = true;
            }
        }
        if (have_approach && ip_loc.get_distance(approach_loc) >= 1.0f) {
            // bearing returned in centidegrees -> radians
            const float approach_rad = ip_loc.get_bearing_to(approach_loc) * 1.0e-2f * DEG_TO_RAD;
            // alignment > 0 = approach heading aligns with wind-from
            // (i.e. we're flying INTO wind during touchdown -- good).
            // alignment < 0 = tailwind landing (bad).
            const float alignment = cosf(approach_rad - wind_from_rad);
            // Multiplier ranges from (1 - bias) for full headwind to
            // (1 + bias) for full tailwind.  Bias = 1 doubles the
            // effective distance of a strict tailwind candidate.
            score = distance * (1.0f - bias * alignment);
        }
        if (best_score < 0 || score < best_score) {
            best_score = score;
            best_idx = i;
        }
    }

    if (best_idx == 0) {
        // No candidate scored; let upstream report the failure.
        return mission.jump_to_landing_sequence(from_loc);
    }

    if (!mission.set_current_cmd(best_idx)) {
        // set_current_cmd failed; fall through to upstream's recovery.
        return mission.jump_to_landing_sequence(from_loc);
    }

    if (mission.state() == AP_Mission::MISSION_STOPPED) {
        mission.resume();
    }

    if (best_idx != nearest_idx) {
        gcs().send_text(MAV_SEVERITY_INFO,
                        "DO_LAND_START %u (upwind, nearest was %u)",
                        (unsigned)best_idx, (unsigned)nearest_idx);
    }
    gcs().send_text(MAV_SEVERITY_INFO, "Landing sequence start");
    return true;
}
