#include "mode.h"
#include "Plane.h"

bool ModeRTL::_enter()
{
    plane.prev_WP_loc = plane.current_loc;
    plane.do_RTL(plane.get_RTL_altitude_cm());
    plane.rtl.done_climb = false;
    // remember whether this RTL was triggered by RC failsafe so the
    // RTL_CLIMB_FIRST_ONLY_IN_FS option can keep applying the climb-first
    // logic for the duration of this RTL even if the link recovers
    plane.rtl.triggered_by_rc_failsafe = plane.failsafe.rc_failsafe;
    // fork PR #150: reset the emergency-landing state on each fresh RTL entry
    plane.auto_state.reached_home_in_fs_ms = 0;
    plane.auto_state.emergency_landing = false;
    plane.auto_state.reached_emergency_landing_no_return_altitude = false;
#if HAL_QUADPLANE_ENABLED
    plane.vtol_approach_s.approach_stage = Plane::VTOLApproach::Stage::RTL;

    // Quadplane specific checks
    if (plane.quadplane.available()) {
        // treat RTL as QLAND if we are in guided wait takeoff state, to cope
        // with failsafes during GUIDED->AUTO takeoff sequence
        if (plane.quadplane.guided_wait_takeoff_on_mode_enter) {
            plane.set_mode(plane.mode_qland, ModeReason::QLAND_INSTEAD_OF_RTL);
            return true;
        }

        // if Q_RTL_MODE is QRTL always, immediately switch to QRTL mode
        if (plane.quadplane.rtl_mode == QuadPlane::RTL_MODE::QRTL_ALWAYS) {
            plane.set_mode(plane.mode_qrtl, ModeReason::QRTL_INSTEAD_OF_RTL);
            return true;
        }

        // if VTOL landing is expected and quadplane motors are active and within QRTL radius and under QRTL altitude then switch to QRTL
        const bool vtol_landing = (plane.quadplane.rtl_mode == QuadPlane::RTL_MODE::SWITCH_QRTL) || (plane.quadplane.rtl_mode == QuadPlane::RTL_MODE::VTOL_APPROACH_QRTL);
        if (vtol_landing && (quadplane.motors->get_desired_spool_state() == AP_Motors::DesiredSpoolState::THROTTLE_UNLIMITED)) {
            int32_t alt_cm;
            if ((plane.current_loc.get_distance(plane.next_WP_loc) < plane.mode_qrtl.get_VTOL_return_radius()) &&
                plane.current_loc.get_alt_cm(Location::AltFrame::ABOVE_HOME, alt_cm) && (alt_cm < plane.quadplane.qrtl_alt*100)) {
                plane.set_mode(plane.mode_qrtl, ModeReason::QRTL_INSTEAD_OF_RTL);
                return true;
            }
        }
    }
#endif

    return true;
}

void ModeRTL::_exit()
{
    // clear loitering flag so the OSD loiter-radius element hides when leaving RTL
    plane.rtl.loitering = false;
}

void ModeRTL::update()
{
    plane.calc_nav_roll();
    plane.calc_nav_pitch();
    plane.calc_throttle();

    // RTL_CLIMB_FIRST_ONLY_IN_FS: if set, skip the initial-climb / bank-limit
    // behaviour unless this RTL was triggered by RC failsafe or we are
    // currently in RC failsafe. Lets manually-commanded RTLs turn straight
    // back without first stair-stepping up to altitude.
    if (plane.flight_option_enabled(FlightOptions::RTL_CLIMB_FIRST_ONLY_IN_FS) &&
        !plane.failsafe.rc_failsafe && !plane.rtl.triggered_by_rc_failsafe) {
        return;
    }

    bool alt_threshold_reached = false;
    if (plane.flight_option_enabled(FlightOptions::CLIMB_BEFORE_TURN)) {
        // Climb to RTL_ALTITUDE before turning. This overrides RTL_CLIMB_MIN.
        alt_threshold_reached = plane.current_loc.alt > plane.next_WP_loc.alt;
    } else if (plane.g2.rtl_climb_min > 0) {
        /*
           when RTL first starts limit bank angle to LEVEL_ROLL_LIMIT
           until we have climbed by RTL_CLIMB_MIN meters
           */
        alt_threshold_reached = (plane.current_loc.alt - plane.prev_WP_loc.alt)*0.01 > plane.g2.rtl_climb_min;
    } else {
        return;
    }

    if (!plane.rtl.done_climb && alt_threshold_reached) {
        plane.prev_WP_loc = plane.current_loc;
        plane.setup_glide_slope();
        plane.rtl.done_climb = true;
    }
    if (!plane.rtl.done_climb) {
        // Constrain the roll limit as a failsafe; without this if something
        // went wrong the plane could keep going perfectly straight. Use the
        // dedicated RTL_LVL_RLL_LMT so the climb-out can be more relaxed than
        // the takeoff/landing leveling constraint set by LEVEL_ROLL_LIMIT.
        const int level_roll_limit_cd = MIN(plane.roll_limit_cd, plane.g.rtl_level_roll_limit * 100);
        plane.nav_roll_cd = constrain_int32(plane.nav_roll_cd,
                                            -level_roll_limit_cd, level_roll_limit_cd);
    }
}

void ModeRTL::navigate()
{
    const uint32_t now = AP_HAL::millis();

#if HAL_QUADPLANE_ENABLED
    if (plane.quadplane.available()) {
        if (plane.quadplane.rtl_mode == QuadPlane::RTL_MODE::VTOL_APPROACH_QRTL) {
            // VTOL approach landing
            AP_Mission::Mission_Command cmd;
            cmd.content.location = plane.next_WP_loc;
            plane.verify_landing_vtol_approach(cmd);
            if (plane.vtol_approach_s.approach_stage == Plane::VTOLApproach::Stage::VTOL_LANDING) {
                plane.set_mode(plane.mode_qrtl, ModeReason::RTL_COMPLETE_SWITCHING_TO_VTOL_LAND_RTL);
            }
            return;
        }

        if ((now - plane.last_mode_change_ms > 1000) && switch_QRTL()) {
            return;
        }
    }
#endif

    // Fork PR #150 + #182: emergency-land if RC failsafe persists after reaching the home
    // loiter target for FS_ELAND_DELAY seconds. FS_ELAND_DELAY == -1 disables. Skips when an
    // explicit landing sequence (DO_LAND_START) is configured -- the mission's autoland
    // takes priority in that case.
    if (plane.failsafe.rc_failsafe &&
        !(plane.mission.contains_item(MAV_CMD_DO_LAND_START) &&
          (plane.g.rtl_autoland == RtlAutoland::RTL_THEN_DO_LAND_START ||
           plane.g.rtl_autoland == RtlAutoland::RTL_IMMEDIATE_DO_LAND_START)) &&
        plane.g.fs_emergency_landing_delay > -1 &&
        plane.reached_loiter_target()) {
        if (plane.auto_state.reached_home_in_fs_ms) {
            const uint32_t delay_ms = uint32_t(MAX(0, plane.g.fs_emergency_landing_delay.get())) * 1000;
            if (now - plane.auto_state.reached_home_in_fs_ms > delay_ms) {
                // delay elapsed -- request TECS gliding (throttle 0, hold AIRSPEED_MIN)
                plane.TECS_controller.set_gliding_requested_flag(true);
                if (!plane.auto_state.emergency_landing) {
                    plane.auto_state.emergency_landing = true;
                    GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Emergency landing started");
                }
            }
        } else {
            plane.auto_state.reached_home_in_fs_ms = now;
        }

        if (plane.auto_state.emergency_landing && plane.relative_altitude < 10) {
            // committed below the no-return altitude -- don't recover even if FS clears
            plane.auto_state.reached_emergency_landing_no_return_altitude = true;
        }
    } else if (!plane.auto_state.reached_emergency_landing_no_return_altitude) {
        // FS cleared (or param disabled) before the no-return altitude -- abort the eland.
        // Don't blanket-clear gliding here; bit-23 ALLOW_GLIDING_IN_AUTO_THR_MODES manages
        // the flag independently in update_flight_mode().
        plane.auto_state.emergency_landing = false;
        plane.auto_state.reached_home_in_fs_ms = 0;
    }

    if (plane.auto_state.reached_emergency_landing_no_return_altitude && !plane.is_flying()) {
        // post-flare detection -- auto-disarm using the same hook autoland uses
        plane.disarm_if_autoland_complete();
    }

    uint16_t radius = abs(plane.g.rtl_radius);
    if (radius > 0) {
        plane.loiter.direction = (plane.g.rtl_radius < 0) ? -1 : 1;
    }

    // OSD loiter-radius element wants to know when RTL has actually settled into its loiter
    if (plane.reached_loiter_target()) {
        plane.rtl.loitering = true;
    }

    plane.update_loiter(radius);

    if (!plane.auto_state.checked_for_autoland) {
        if ((plane.g.rtl_autoland == RtlAutoland::RTL_IMMEDIATE_DO_LAND_START) ||
            (plane.g.rtl_autoland == RtlAutoland::RTL_THEN_DO_LAND_START &&
            plane.reached_loiter_target() && 
            labs(plane.calc_altitude_error_cm()) < 1000)) {
                // we've reached the RTL point, see if we have a landing sequence
                if (plane.have_position && plane.mission.jump_to_landing_sequence(plane.current_loc)) {
                    // switch from RTL -> AUTO
                    plane.mission.set_force_resume(true);
                    if (plane.set_mode(plane.mode_auto, ModeReason::RTL_COMPLETE_SWITCHING_TO_FIXEDWING_AUTOLAND)) {
                        // return here so we don't change the radius and don't run the rtl update_loiter()
                        return;
                    }
                    // mode change failed, revert force resume flag
                    plane.mission.set_force_resume(false);
                }

                // prevent running the expensive jump_to_landing_sequence
                // on every loop
                plane.auto_state.checked_for_autoland = true;

        } else if (plane.g.rtl_autoland == RtlAutoland::DO_RETURN_PATH_START) {
            if (plane.have_position && plane.mission.jump_to_closest_mission_leg(plane.current_loc)) {
                plane.mission.set_force_resume(true);
                if (plane.set_mode(plane.mode_auto, ModeReason::RTL_COMPLETE_SWITCHING_TO_FIXEDWING_AUTOLAND)) {
                    // return here so we don't change the radius and don't run the rtl update_loiter()
                    return;
                }
                // mode change failed, revert force resume flag
                plane.mission.set_force_resume(false);
            }
            plane.auto_state.checked_for_autoland = true;
        }
    }
}

#if HAL_QUADPLANE_ENABLED
// Switch to QRTL if enabled and within radius
bool ModeRTL::switch_QRTL()
{
    if (plane.quadplane.rtl_mode != QuadPlane::RTL_MODE::SWITCH_QRTL) {
        return false;
    }

    uint16_t qrtl_radius = abs(plane.g.rtl_radius);
    if (qrtl_radius == 0) {
        qrtl_radius = abs(plane.aparm.loiter_radius);
    }

    if (plane.nav_controller->reached_loiter_target() ||
         plane.current_loc.past_interval_finish_line(plane.prev_WP_loc, plane.next_WP_loc) ||
         plane.auto_state.wp_distance < MAX(qrtl_radius, plane.quadplane.stopping_distance())) {
        /*
          for a quadplane in RTL mode we switch to QRTL when we
          are within the maximum of the stopping distance and the
          RTL_RADIUS
         */
        plane.set_mode(plane.mode_qrtl, ModeReason::RTL_COMPLETE_SWITCHING_TO_VTOL_LAND_RTL);
        return true;
    }

    return false;
}

#endif  // HAL_QUADPLANE_ENABLED
