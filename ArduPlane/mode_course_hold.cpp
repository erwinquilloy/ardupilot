#include "mode.h"
#include "Plane.h"

bool ModeCourseHold::_enter()
{
    plane.course_hold.locked_heading = false;
    plane.course_hold.lock_timer_ms = 0;
    plane.course_hold.heading_update_tstamp = 0;

    return true;
}

void ModeCourseHold::update()
{
    const float pitch_input = plane.channel_pitch->norm_input();
    if (pitch_input > 0) {
        plane.nav_pitch_cd = pitch_input * plane.aparm.pitch_limit_max * 100;
    } else {
        plane.nav_pitch_cd = -(pitch_input * plane.pitch_limit_min * 100);
    }
    plane.nav_pitch_cd = constrain_int32(plane.nav_pitch_cd,
                                         plane.pitch_limit_min * 100,
                                         plane.aparm.pitch_limit_max.get() * 100);

    /*
      Heading becomes unlocked on any aileron, or rudder input unless
      COURSE_HOLD_HEADING_CONTROL_WITH_YAW_STICK is set (in which case the
      rudder steers the locked heading rather than unlocking it).
    */
    const bool yaw_steers_heading = (plane.g2.flight_options & FlightOptions::COURSE_HOLD_HEADING_CONTROL_WITH_YAW_STICK) != 0;
    if (plane.channel_roll->get_control_in() != 0 ||
        (!yaw_steers_heading && plane.channel_rudder->get_control_in() != 0)) {
        plane.course_hold.locked_heading = false;
        plane.course_hold.lock_timer_ms = 0;
    }

    if (!plane.course_hold.locked_heading) {
        plane.nav_roll_cd = plane.channel_roll->norm_input() * plane.roll_limit_cd;
        plane.update_load_factor();
    } else {
        plane.calc_nav_roll();
    }
}

/*
  Lock heading to GPS course when we have sufficient ground speed and no
  aileron or rudder input.
 */
void ModeCourseHold::navigate()
{
    const uint32_t now = millis();
    if (!plane.course_hold.locked_heading &&
        plane.channel_roll->get_control_in() == 0 &&
        plane.rudder_input() == 0 &&
        plane.gps.status() >= AP_GPS::GPS_OK_FIX_2D &&
        plane.gps.ground_speed() >= 3 &&
        plane.course_hold.lock_timer_ms == 0) {
        // user wants to lock the heading — start the timer
        plane.course_hold.lock_timer_ms = now;
    }
    if (plane.course_hold.lock_timer_ms != 0 &&
        (now - plane.course_hold.lock_timer_ms) > 500) {
        // lock the heading after 0.5 s of zero heading input from user
        plane.course_hold.locked_heading = true;
        plane.course_hold.lock_timer_ms = 0;
        plane.course_hold.locked_heading_cd = plane.gps.ground_course_cd();
        plane.prev_WP_loc = plane.current_loc;
    }
    if (plane.course_hold.locked_heading) {
        if ((plane.g2.flight_options & FlightOptions::COURSE_HOLD_HEADING_CONTROL_WITH_YAW_STICK) != 0) {
            const float rudder_input = plane.channel_rudder->get_control_in() * (1.0f / 45);
            if (!is_zero(rudder_input) && plane.course_hold.heading_update_tstamp) {
                plane.prev_WP_loc = plane.current_loc;
                const float dt = (now - plane.course_hold.heading_update_tstamp) * 0.001f;
                plane.course_hold.locked_heading_cd += rudder_input * plane.g2.cruise_yaw_rate * dt;
                plane.course_hold.locked_heading_cd = wrap_360_cd(plane.course_hold.locked_heading_cd);
            }
            plane.course_hold.heading_update_tstamp = now;
        }

        plane.next_WP_loc = plane.prev_WP_loc;
        // always look 1 km ahead
        plane.next_WP_loc.offset_bearing(plane.course_hold.locked_heading_cd * 0.01f,
                                         plane.prev_WP_loc.get_distance(plane.current_loc) + 1000);
        plane.nav_controller->update_waypoint(plane.prev_WP_loc, plane.next_WP_loc);
    } else {
        plane.course_hold.heading_update_tstamp = 0;
    }
}

bool ModeCourseHold::get_target_heading_cd(int32_t &target_heading) const
{
    target_heading = plane.course_hold.locked_heading_cd;
    return plane.course_hold.locked_heading;
}
