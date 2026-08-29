#include "mode.h"
#include "Plane.h"

bool ModeFBWA::_enter()
{
#if HAL_QUADPLANE_ENABLED
    // Seed the completion edge detector from the state we are entering with.
    // Entering FBWA from a VTOL mode leaves the transition in AIRSPEED_WAIT,
    // so this normally seeds false and the first completion is a real edge;
    // entering from another fixed-wing mode seeds true, so a transition that
    // finished before FBWA was selected cannot fire the handover.
    transition_was_complete = plane.quadplane.transition_complete();
#endif
    return true;
}

#if HAL_QUADPLANE_ENABLED
/*
  FLIGHT_OPTIONS bit 17: when a VTOL->fixed-wing transition completes while
  the pilot is flying FBWA, hand the aircraft over in FBWB instead of leaving
  it in FBWA.

  FBWB rather than FBWA for the same reason as the bit 21 takeoff handover:
  ModeFBWB::_enter() latches the current altitude as the target, so
  sticks-centred is straight and level. In FBWA sticks-centred is only
  wings-level at zero pitch demand and the throttle sits wherever the stick
  happens to be - which, right after a transition, is usually still up where
  it was driving the climb-out. Transition completion is another moment the
  pilot is not yet settled, so make doing nothing the safe option.

  Fires on the rising edge of transition completion only, and only when the
  mode we came from was a VTOL mode. Without that second guard a Q_ASSIST
  episode later in the same FBWA flight - assistance engages when slow, then
  clears - would read as a completion and silently move the pilot to FBWB
  when no transition took place.

  Held off while a long failsafe is pending or the RC link is down: FBWB is
  still pilot-flown, so with no transmitter there is nothing to be gained by
  changing mode out from under the failsafe handler. Default off - bit clear
  leaves the existing behaviour of staying in FBWA.
 */
void ModeFBWA::check_transition_handover()
{
    if (!plane.quadplane.available()) {
        return;
    }

    const bool complete = plane.quadplane.transition_complete();
    const bool just_completed = complete && !transition_was_complete;
    transition_was_complete = complete;

    if (!just_completed ||
        !plane.flight_option_enabled(FlightOptions::VTOL_TRANS_END_IN_FBWB)) {
        return;
    }
    if (!plane.arming.is_armed_and_safety_off()) {
        return;
    }
    if (!plane.previous_mode->is_vtol_mode()) {
        // not a transition out of a VTOL mode, so not our edge
        return;
    }
    if (plane.long_failsafe_pending || plane.failsafe.rc_failsafe) {
        return;
    }

    // sent immediately before the mode change so it is the last statustext
    // written - the transition state machine sends "Transition done" on the
    // same loop and the OSD only keeps the newest.
    gcs().send_text(MAV_SEVERITY_INFO, "Transition done, switching to FBWB");
    plane.set_mode(plane.mode_fbwb, ModeReason::VTOL_TRANSITION_COMPLETE);
}
#endif // HAL_QUADPLANE_ENABLED

void ModeFBWA::update()
{
#if HAL_QUADPLANE_ENABLED
    check_transition_handover();
    if (plane.control_mode != &plane.mode_fbwa) {
        // handover took us to FBWB; do not keep driving FBWA targets
        return;
    }
#endif

    // set nav_roll and nav_pitch using sticks
    plane.nav_roll_cd  = plane.channel_roll->norm_input() * plane.roll_limit_cd;
    plane.update_load_factor();
    float pitch_input = plane.channel_pitch->norm_input();
    if (pitch_input > 0) {
        plane.nav_pitch_cd = pitch_input * plane.aparm.pitch_limit_max*100;
    } else {
        plane.nav_pitch_cd = -(pitch_input * plane.pitch_limit_min*100);
    }
    plane.adjust_nav_pitch_throttle();
    plane.nav_pitch_cd = constrain_int32(plane.nav_pitch_cd, plane.pitch_limit_min*100, plane.aparm.pitch_limit_max.get()*100);
    if (plane.fly_inverted()) {
        plane.nav_pitch_cd = -plane.nav_pitch_cd;
    }
    if (plane.failsafe.rc_failsafe && plane.g.fs_action_short == FS_ACTION_SHORT_FBWA) {
        // FBWA failsafe glide
        plane.nav_roll_cd = 0;
        plane.nav_pitch_cd = 0;
        SRV_Channels::set_output_limit(SRV_Channel::k_throttle, SRV_Channel::Limit::MIN);
    }
    RC_Channel *chan = rc().find_channel_for_option(RC_Channel::AUX_FUNC::FBWA_TAILDRAGGER);
    if (chan != nullptr) {
        // check for the user enabling FBWA taildrag takeoff mode
        bool tdrag_mode = chan->get_aux_switch_pos() == RC_Channel::AuxSwitchPos::HIGH;
        if (tdrag_mode && !plane.auto_state.fbwa_tdrag_takeoff_mode) {
            if (plane.auto_state.highest_airspeed < plane.g.takeoff_tdrag_speed1) {
                plane.auto_state.fbwa_tdrag_takeoff_mode = true;
                plane.gcs().send_text(MAV_SEVERITY_WARNING, "FBWA tdrag mode");
            }
        }
    }
}

void ModeFBWA::run()
{
    // Run base class function and then output throttle
    Mode::run();

    output_pilot_throttle();
}
