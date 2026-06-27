#pragma once

#include <RC_Channel/RC_Channel.h>

class RC_Channel_Plane : public RC_Channel
{

public:

protected:

    void init_aux_function(AUX_FUNC ch_option,
                           AuxSwitchPos ch_flag) override;
    bool do_aux_function(AUX_FUNC ch_option, AuxSwitchPos) override;

    // called when the mode switch changes position:
    void mode_switch_changed(modeswitch_pos_t new_pos) override;

public:
    // Fork FLTMODE_EXT: 12-position PWM scan + dispatch. RC_Channels_Plane
    // calls this when FLTMODE_EXT = 1 instead of the upstream 6-position
    // path. Public so the container override can reach it (the base
    // RC_Channel::read_mode_switch is non-virtual and mode_switch_changed
    // is protected, so neither can be touched from outside the class).
    void read_12pos_mode_switch();

protected:
    // Fork FLTMODE_EXT: 12-position PWM scan, mirrors the shape of
    // RC_Channel::read_6pos_switch (debounce + position out-param).
    bool read_12pos_switch(int8_t &position) WARN_IF_UNUSED;

private:

    void do_aux_function_change_mode(Mode::Number number,
                                     AuxSwitchPos ch_flag);

#if HAL_QUADPLANE_ENABLED
    void do_aux_function_q_assist_state(AuxSwitchPos ch_flag);
#endif

    void do_aux_function_crow_mode(AuxSwitchPos ch_flag);

    void do_aux_function_soaring_3pos(AuxSwitchPos ch_flag);

    void do_aux_function_flare(AuxSwitchPos ch_flag);
};

class RC_Channels_Plane : public RC_Channels
{
public:

    RC_Channel_Plane obj_channels[NUM_RC_CHANNELS];
    RC_Channel_Plane *channel(const uint8_t chan) override {
        if (chan >= NUM_RC_CHANNELS) {
            return nullptr;
        }
        return &obj_channels[chan];
    }

    bool in_rc_failsafe() const override;
    bool has_valid_input() const override;

    bool auto_switch_to_fbwa_with_sticks() const {
        return option_is_enabled(Option::AUTO_SWITCH_TO_FBWA_WITH_STICKS);
    }

    RC_Channel *get_arming_channel(void) const override;

    void read_mode_switch() override;

protected:

    // note that these callbacks are not presently used on Plane:
    int8_t flight_mode_channel_number() const override;

};
