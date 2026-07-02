#include "mode.h"
#include "Plane.h"

bool ModeRTL::_enter()
{
    // Fork PR #158: clear the L1 controller's reached-loiter latch so the
    // RTL_ALT_HOME descent logic doesn't fire on a stale circle from a
    // prior loiter session.
    plane.nav_controller->reset_reached_loiter_target();
    plane.prev_WP_loc = plane.current_loc;
    plane.do_RTL(plane.get_RTL_altitude_cm());
    plane.rtl.done_climb = false;
    plane.rtl.reached_home_altitude = false;
    plane.rtl.manual_alt_control = false;
    // remember whether this RTL was triggered by RC failsafe so the
    // RTL_CLIMB_FIRST_ONLY_IN_FS option can keep applying the climb-first
    // logic for the duration of this RTL even if the link recovers
    plane.rtl.triggered_by_rc_failsafe = plane.failsafe.rc_failsafe;
    // fork PR #194: reset the emergency-landing state machine on each fresh RTL entry
    plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::INACTIVE;
    plane.rtl.emergency_landing_tstamp_ms = 0;

    // Fork PR #180 port: seed dynamic loiter radius / direction from
    // RTL_RADIUS (falling back to WP_LOITER_RAD if RTL_RADIUS = 0) so
    // update_loiter_radius_and_direction() has a starting point on the
    // first navigate() tick.  navigate_last_ms = 0 skips the dt
    // integration on that first tick.
    {
        const int16_t rtl_radius = plane.g.rtl_radius != 0
                                       ? plane.g.rtl_radius.get()
                                       : plane.aparm.loiter_radius.get();
        plane.loiter.radius = fabsf(rtl_radius);
        plane.loiter.direction = rtl_radius < 0 ? -1 : 1;
        plane.loiter.navigate_last_ms = 0;
    }

    // Fork PR #155: if RTL_MANUAL_ALT_CONTROL is on and we are not in RC
    // failsafe, hand altitude control to the pilot's pitch stick from the
    // very first tick. Seed target_altitude.amsl_cm from prev_WP_loc so the
    // FBWB altitude controller starts from where the plane already is rather
    // than snapping to the RTL altitude.
    if (plane.flight_option_enabled(FlightOptions::RTL_MANUAL_ALT_CONTROL) &&
        !plane.rtl.triggered_by_rc_failsafe) {
        plane.rtl.manual_alt_control = true;
        IGNORE_RETURN(plane.prev_WP_loc.get_alt_cm(Location::AltFrame::ABSOLUTE, plane.target_altitude.amsl_cm));
    }
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

    // Fork PR #155: RTL_MANUAL_ALT_CONTROL. While not in RC failsafe and the
    // option is set, the pilot drives altitude via the pitch stick. We skip
    // the usual nav-pitch / throttle calcs (update_fbwb_speed_height owns
    // target_altitude) and let the L1 controller bring the plane home using
    // whatever altitude the pilot picks. Re-asserted every tick so the flag
    // tracks the live FlightOptions setting.
    const bool not_in_fs = !plane.failsafe.rc_failsafe && !plane.rtl.triggered_by_rc_failsafe;
    if (not_in_fs && plane.flight_option_enabled(FlightOptions::RTL_MANUAL_ALT_CONTROL)) {
        plane.rtl.manual_alt_control = true;
        plane.update_fbwb_speed_height();
        return;
    }

    // Fork PR #155: transition from manual-alt control into a normal RTL --
    // either because RC just failsafed, or because the user cleared the
    // option mid-flight. Reset the climb baseline to the current altitude
    // and re-arm the do_RTL plumbing so the plane climbs/descends to the
    // configured RTL altitude cleanly (without snapping). reached_home_altitude
    // is also cleared so the FS eland timer is re-gated by the descent block
    // in navigate().
    if (plane.rtl.manual_alt_control) {
        plane.prev_WP_loc = plane.current_loc;
        plane.do_RTL(plane.get_RTL_altitude_cm());
        plane.rtl.manual_alt_control = false;
        plane.rtl.reached_home_altitude = false;
    }

    plane.calc_nav_pitch();
    plane.calc_throttle();

    // Fork PR #194 + 2d1ec0e331: hold wings level once we're in the GLIDING / GLIDING_NO_RETURN
    // phases of the emergency-landing state machine, either because FS_ELAND_UPWIND is set (the
    // plane glides straight into wind) or because we've descended below FS_ELAND_LVLALT.
    // Use >= GLIDING comparison so future intermediate states inherit the level-out behaviour.
    if (plane.rtl.emergency_landing_status >= Plane::FSEmergencyLandingStatus::GLIDING) {
        float altitude = plane.relative_altitude;
#if AP_TERRAIN_AVAILABLE
        if (!plane.terrain_disabled()) {
            plane.terrain.height_above_terrain(altitude, true);
        }
#endif
        const bool below_lvlalt = plane.g.fs_emergency_landing_leveling_altitude > -1 &&
                                  altitude < plane.g.fs_emergency_landing_leveling_altitude.get();
        if (plane.g.fs_emergency_landing_land_upwind || below_lvlalt) {
            plane.nav_roll_cd = 0;
            return;
        }
    }

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

    // Fork PR #194: emergency-landing state machine, replacing the boolean trigger from #150.
    // Flow: INACTIVE -> DELAY (timer starts at home) -> SINKING_TO_GLIDE_ALTITUDE (sink to
    //   FS_ELAND_GLDALT + 2 m) -> ALIGNMENT_INTO_WIND (heading into wind, or skipped if
    //   FS_ELAND_UPWIND=0) -> GLIDING (TECS gliding flag) -> GLIDING_NO_RETURN (below 10 m AGL).
    // Skips when an explicit landing sequence (DO_LAND_START) is configured.
    if (plane.failsafe.rc_failsafe &&
        !(plane.mission.contains_item(MAV_CMD_DO_LAND_START) &&
          (plane.g.rtl_autoland == RtlAutoland::RTL_THEN_DO_LAND_START ||
           plane.g.rtl_autoland == RtlAutoland::RTL_IMMEDIATE_DO_LAND_START)) &&
        plane.g.fs_emergency_landing_delay > -1) {

        // glide target altitude clamped above FS_ELAND_LVLALT so the level-out never starts mid-sink
        float emergency_landing_gliding_altitude_m = plane.g.fs_emergency_landing_gliding_altitude;
        if (plane.g.fs_emergency_landing_leveling_altitude > -1 &&
            plane.g.fs_emergency_landing_leveling_altitude > emergency_landing_gliding_altitude_m) {
            emergency_landing_gliding_altitude_m = plane.g.fs_emergency_landing_leveling_altitude;
        }

        switch (plane.rtl.emergency_landing_status) {
            case Plane::FSEmergencyLandingStatus::INACTIVE:
                // Fork PR #158 + #194: only start the FS emergency-landing
                // timer once the plane has reached the home loiter AND
                // settled at RTL_ALT_HOME (or RTL_ALTITUDE if RTL_ALT_HOME
                // is -1). The reached_home_altitude flag is set by the
                // RTL_ALT_HOME descent block below.
                if (plane.reached_loiter_target() && plane.rtl.reached_home_altitude) {
                    plane.rtl.emergency_landing_tstamp_ms = now;
                    plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::DELAY;
                } else {
                    break;
                }
                FALLTHROUGH;

            case Plane::FSEmergencyLandingStatus::DELAY:
                if (now - plane.rtl.emergency_landing_tstamp_ms > uint32_t(MAX(0, plane.g.fs_emergency_landing_delay.get())) * 1000) {
                    // start the controlled sink to gliding altitude
                    plane.next_WP_loc.set_alt_cm(emergency_landing_gliding_altitude_m * 100, Location::AltFrame::ABOVE_HOME);
                    plane.setup_terrain_target_alt(plane.next_WP_loc);
                    plane.set_target_altitude_location(plane.next_WP_loc);
                    plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::SINKING_TO_GLIDE_ALTITUDE;
                    GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Emergency landing started");
                } else {
                    break;
                }
                FALLTHROUGH;

            case Plane::FSEmergencyLandingStatus::SINKING_TO_GLIDE_ALTITUDE: {
                float altitude = plane.relative_altitude;
#if AP_TERRAIN_AVAILABLE
                if (!plane.terrain_disabled()) {
                    plane.terrain.height_above_terrain(altitude, true);
                }
#endif
                if (altitude < emergency_landing_gliding_altitude_m + 2.0f) {
                    plane.rtl.emergency_landing_tstamp_ms = now;
                    plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::ALIGNMENT_INTO_WIND;
                } else {
                    break;
                }
                FALLTHROUGH;
            }

            case Plane::FSEmergencyLandingStatus::ALIGNMENT_INTO_WIND:
                if (plane.g.fs_emergency_landing_land_upwind) {
                    float yaw_rad;
                    Vector3f wind_v;
                    {
                        WITH_SEMAPHORE(ahrs.get_semaphore());
                        wind_v = ahrs.wind_estimate();
                        yaw_rad = ahrs.get_yaw();
                    }
                    const float wind_speed_mps = wind_v.length();
                    float wind_angle = 0;
                    if (wind_speed_mps > 1.0f) {
                        wind_angle = degrees(wrap_2PI(atan2f(wind_v.y, wind_v.x) - yaw_rad));
                    }
                    // target = 180 deg + small bias on loiter direction so the heading converges
                    // from the side the plane is already circling toward
                    const float wind_target_angle = 180 + plane.loiter.direction * 2;

                    if (now - plane.rtl.emergency_landing_tstamp_ms > 120000 ||
                        wind_speed_mps <= 1.0f ||
                        (wind_angle > wind_target_angle - 2 && wind_angle < wind_target_angle + 2)) {
                        plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::GLIDING;
                    } else {
                        break;
                    }
                } else {
                    plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::GLIDING;
                }
                FALLTHROUGH;

            case Plane::FSEmergencyLandingStatus::GLIDING: {
                plane.TECS_controller.set_gliding_requested_flag(true);
                float altitude = plane.relative_altitude;
#if AP_TERRAIN_AVAILABLE
                if (!plane.terrain_disabled()) {
                    plane.terrain.height_above_terrain(altitude, true);
                }
#endif
                if (altitude < 10) {
                    // committed below the no-return altitude
                    plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::GLIDING_NO_RETURN;
                }
                FALLTHROUGH;
            }

            case Plane::FSEmergencyLandingStatus::GLIDING_NO_RETURN:
                plane.TECS_controller.set_gliding_requested_flag(true);
                plane.disarm_if_autoland_complete();
                break;
        }
    } else if (plane.rtl.emergency_landing_status != Plane::FSEmergencyLandingStatus::INACTIVE &&
               plane.rtl.emergency_landing_status != Plane::FSEmergencyLandingStatus::GLIDING_NO_RETURN) {
        // FS cleared (or param disabled) before the no-return altitude -- abort the eland and
        // restore the normal RTL loiter radius (fork 2d1ec0e331) so the plane returns to a
        // standard RTL circle instead of staying on the smaller eland radius.
        const int16_t radius = plane.g.rtl_radius != 0 ? plane.g.rtl_radius.get() : plane.aparm.loiter_radius.get();
        plane.loiter.radius = abs(radius);
        plane.loiter.direction = radius < 0 ? -1 : 1;
        plane.rtl.emergency_landing_status = Plane::FSEmergencyLandingStatus::INACTIVE;
        plane.rtl.emergency_landing_tstamp_ms = 0;
    }
    // (auto-disarm on touchdown is now driven from inside the GLIDING_NO_RETURN case)

    // Fork PR #158: once the plane has reached the home loiter target,
    // descend/climb to RTL_ALT_HOME (if configured). PR #155 added the
    // !manual_alt_control gate so this block stops fighting the pilot's
    // pitch stick when RTL_MANUAL_ALT_CONTROL is active.
    if (plane.reached_loiter_target() && !plane.rtl.manual_alt_control) {
        int32_t home_altitude_cm;
        if (plane.g.RTL_home_altitude > -1) {
            home_altitude_cm = plane.get_home_RTL_altitude_cm();
            plane.next_WP_loc.set_alt_cm(home_altitude_cm, Location::AltFrame::ABSOLUTE);
            plane.setup_terrain_target_alt(plane.next_WP_loc);
            plane.set_target_altitude_location(plane.next_WP_loc);
        } else {
            home_altitude_cm = plane.get_RTL_altitude_cm();
        }
        plane.rtl.done_climb = true;
        // 5 m tolerance, like the fork
        if (abs(home_altitude_cm - plane.current_loc.alt) < 500) {
            plane.rtl.reached_home_altitude = true;
        }
    }

    // Fork PR #180 port: pilot rudder-stick flips loiter direction,
    // roll-stick integrates loiter radius.  Bails when in RC failsafe
    // so the FS_ELAND override block below still owns the geometry
    // during emergency landing.
    plane.update_loiter_radius_and_direction();

    // OSD loiter-radius element wants to know when RTL has actually settled into its loiter
    if (plane.reached_loiter_target()) {
        plane.rtl.loitering = true;
    }

    // Fork 2d1ec0e331: during RC failsafe, override loiter radius to FS_ELAND_LOTRAD once we're
    // past the SINKING_TO_GLIDE_ALTITUDE state (i.e. actively descending/gliding/aligned). A
    // smaller radius descends faster but may overshoot in strong wind -- the param lets the
    // user pick the trade-off. FS_ELAND_LOTRAD = 0 falls back to RTL_RADIUS / WP_LOITER_RAD.
    if (plane.failsafe.rc_failsafe) {
        int16_t override_radius = plane.g.rtl_radius != 0 ? plane.g.rtl_radius.get() : plane.aparm.loiter_radius.get();
        if (plane.rtl.emergency_landing_status >= Plane::FSEmergencyLandingStatus::SINKING_TO_GLIDE_ALTITUDE &&
            plane.g.fs_emergency_landing_loiter_radius != 0) {
            override_radius = plane.g.fs_emergency_landing_loiter_radius;
        }
        plane.loiter.radius = fabsf(override_radius);
        plane.loiter.direction = override_radius < 0 ? -1 : 1;
    }

    plane.update_loiter(lrintf(plane.loiter.radius));

    if (!plane.auto_state.checked_for_autoland) {
        if ((plane.g.rtl_autoland == RtlAutoland::RTL_IMMEDIATE_DO_LAND_START) ||
            (plane.g.rtl_autoland == RtlAutoland::RTL_THEN_DO_LAND_START &&
            plane.reached_loiter_target() && 
            labs(plane.calc_altitude_error_cm()) < 1000)) {
                // we've reached the RTL point, see if we have a landing sequence
                if (plane.have_position && plane.try_upwind_jump_to_landing_sequence(plane.current_loc)) {
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

// Fork PR #155: while the pilot is driving altitude with the pitch stick
// (RTL_MANUAL_ALT_CONTROL active), suppress the base-class target-altitude
// pipeline so set_target_altitude_location() doesn't fight FBWB control.
void ModeRTL::update_target_altitude()
{
    if (plane.rtl.manual_alt_control) {
        return;
    }
    Mode::update_target_altitude();
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
