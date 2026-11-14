#include "AP_Frsky_MAVliteMsgHandler.h"

#include <AC_Fence/AC_Fence.h>
#include <AP_Vehicle/AP_Vehicle.h>
#include <GCS_MAVLink/GCS.h>
#include <AP_Mission/AP_Mission.h>

extern const AP_HAL::HAL& hal;

#if HAL_WITH_FRSKY_TELEM_BIDIRECTIONAL
/*
 * Handle the COMMAND_LONG mavlite message
 * for FrSky SPort Passthrough (OpenTX) protocol (X-receivers)
 */

void AP_Frsky_MAVliteMsgHandler::handle_command_long(const AP_Frsky_MAVlite_Message &rxmsg)
{
    mavlink_command_long_t mav_command_long {};

    uint8_t cmd_options;
    float params[7] {};

    if (!rxmsg.get_uint16(mav_command_long.command, 0)) {
        return;
    }
    if (!rxmsg.get_uint8(cmd_options, 2)) {
        return;
    }
    uint8_t param_count = AP_Frsky_MAVlite_Message::bit8_unpack(cmd_options, 3, 0);               // first 3 bits

    for (uint8_t cmd_idx=0; cmd_idx<param_count; cmd_idx++) {
        // base offset is 3, relative offset is 4*cmd_idx
        if (!rxmsg.get_float(params[cmd_idx], 3+(4*cmd_idx))) {
            return;
        }
    }

    mav_command_long.param1 = params[0];
    mav_command_long.param2 = params[1];
    mav_command_long.param3 = params[2];
    mav_command_long.param4 = params[3];
    mav_command_long.param5 = params[4];
    mav_command_long.param6 = params[5];
    mav_command_long.param7 = params[6];

    const MAV_RESULT mav_result = handle_command(mav_command_long);
    send_command_ack(mav_result, mav_command_long.command);
}

MAV_RESULT AP_Frsky_MAVliteMsgHandler::handle_command(const mavlink_command_long_t &mav_command_long)
{
    // filter allowed commands
    switch (mav_command_long.command) {
        //case MAV_CMD_ACCELCAL_VEHICLE_POS:
        case MAV_CMD_DO_SET_MODE:
            return handle_command_do_set_mode(mav_command_long);
        //case MAV_CMD_DO_SET_HOME:
#if AP_FENCE_ENABLED
        case MAV_CMD_DO_FENCE_ENABLE:
            return handle_command_do_fence_enable(mav_command_long);
#endif
        case MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN:
            return handle_command_preflight_reboot(mav_command_long);
        //case MAV_CMD_DO_START_MAG_CAL:
        //case MAV_CMD_DO_ACCEPT_MAG_CAL:
        //case MAV_CMD_DO_CANCEL_MAG_CAL:
        //case MAV_CMD_START_RX_PAIR:
        //case MAV_CMD_DO_DIGICAM_CONFIGURE:
        //case MAV_CMD_DO_DIGICAM_CONTROL:
        //case MAV_CMD_DO_SET_CAM_TRIGG_DIST:
        //case MAV_CMD_DO_GRIPPER:
        //case MAV_CMD_DO_MOUNT_CONFIGURE:
        //case MAV_CMD_DO_MOUNT_CONTROL:
        //case MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES:
        //case MAV_CMD_DO_SET_ROI_SYSID:
        //case MAV_CMD_DO_SET_ROI_LOCATION:
        //case MAV_CMD_DO_SET_ROI:
        case MAV_CMD_PREFLIGHT_CALIBRATION:
            return handle_command_preflight_calibration_baro(mav_command_long);
        //case MAV_CMD_BATTERY_RESET:
        //case MAV_CMD_PREFLIGHT_UAVCAN:
        //case MAV_CMD_FLASH_BOOTLOADER:
        //case MAV_CMD_PREFLIGHT_SET_SENSOR_OFFSETS:
        //case MAV_CMD_GET_HOME_POSITION:
        //case MAV_CMD_PREFLIGHT_STORAGE:
        //case MAV_CMD_SET_MESSAGE_INTERVAL:
        //case MAV_CMD_GET_MESSAGE_INTERVAL:
        //case MAV_CMD_REQUEST_MESSAGE:
        //case MAV_CMD_DO_SET_SERVO:
        //case MAV_CMD_DO_REPEAT_SERVO:
        //case MAV_CMD_DO_SET_RELAY:
        //case MAV_CMD_DO_REPEAT_RELAY:
        //case MAV_CMD_DO_FLIGHTTERMINATION:
        //case MAV_CMD_COMPONENT_ARM_DISARM:
        //case MAV_CMD_FIXED_MAG_CAL_YAW:
        default:
            return MAV_RESULT_UNSUPPORTED;
    }
    return MAV_RESULT_UNSUPPORTED;
}

MAV_RESULT AP_Frsky_MAVliteMsgHandler::handle_command_do_set_mode(const mavlink_command_long_t &mav_command_long)
{
    if (AP::vehicle()->set_mode(mav_command_long.param1, ModeReason::FRSKY_COMMAND)) {
        return MAV_RESULT_ACCEPTED;
    }
    return MAV_RESULT_FAILED;
}

void AP_Frsky_MAVliteMsgHandler::send_command_ack(const MAV_RESULT mav_result, const uint16_t cmdid)
{
    AP_Frsky_MAVlite_Message txmsg;
    txmsg.msgid = MAVLINK_MSG_ID_COMMAND_ACK;
    if (!txmsg.set_uint16(cmdid, 0)) {
        return;
    }
    if (!txmsg.set_uint8((uint8_t)mav_result, 2)) {
        return;
    }
    send_message(txmsg);
}

MAV_RESULT AP_Frsky_MAVliteMsgHandler::handle_command_preflight_calibration_baro(const mavlink_command_long_t &mav_command_long)
{
    if (!(is_equal(mav_command_long.param1,0.0f) && is_equal(mav_command_long.param2,0.0f) && is_equal(mav_command_long.param3,1.0f))) {
        return MAV_RESULT_FAILED;
    }

    if (hal.util->get_soft_armed()) {
        return MAV_RESULT_DENIED;
    }
    // fast barometer calibration
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Updating barometer calibration");
    AP::baro().update_calibration();
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "Barometer calibration complete");

#if AP_AIRSPEED_ENABLED
    AP_Airspeed *airspeed = AP_Airspeed::get_singleton();
    if (airspeed != nullptr) {
        airspeed->calibrate(false);
    }
#endif

    return MAV_RESULT_ACCEPTED;
}

#if AP_FENCE_ENABLED
MAV_RESULT AP_Frsky_MAVliteMsgHandler::handle_command_do_fence_enable(const mavlink_command_long_t &mav_command_long)
{
    AC_Fence *fence = AP::fence();
    if (fence == nullptr) {
        return MAV_RESULT_UNSUPPORTED;
    }

    switch ((uint16_t)mav_command_long.param1) {
        case 0:
            fence->enable_configured(false);
            return MAV_RESULT_ACCEPTED;
        case 1:
            fence->enable_configured(true);
            return MAV_RESULT_ACCEPTED;
        default:
            return MAV_RESULT_FAILED;
    }
}
#endif // AP_FENCE_ENABLED

/*
 * Handle the PARAM_REQUEST_READ mavlite message
 * for FrSky SPort Passthrough (OpenTX) protocol (X-receivers)
 */
void AP_Frsky_MAVliteMsgHandler::handle_param_request_read(const AP_Frsky_MAVlite_Message &rxmsg)
{
    float param_value;
    char param_name[AP_MAX_NAME_SIZE+1];
    if (!rxmsg.get_string(param_name, 0)) {
        return;
    }
    // find existing param
    if (!AP_Param::get(param_name,param_value)) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Param read failed (%s)", param_name);
        return;
    }
    AP_Frsky_MAVlite_Message txmsg;
    txmsg.msgid = MAVLINK_MSG_ID_PARAM_VALUE;
    if (!txmsg.set_float(param_value, 0)) {
        return;
    }
    if (!txmsg.set_string(param_name, 4)) {
        return;
    }
    send_message(txmsg);
}

/*
 * Handle the PARAM_SET mavlite message
 * for FrSky SPort Passthrough (OpenTX) protocol (X-receivers)
 */
void AP_Frsky_MAVliteMsgHandler::handle_param_set(const AP_Frsky_MAVlite_Message &rxmsg)
{
    float param_value;
    char param_name[AP_MAX_NAME_SIZE+1];
    // populate packet with mavlite payload
    if (!rxmsg.get_float(param_value, 0)) {
        return;
    }
    if (!rxmsg.get_string(param_name, 4)) {
        return;
    }
    // find existing param so we can get the old value
    enum ap_var_type var_type;
    // set parameter
    AP_Param *vp;
    uint16_t parameter_flags = 0;
    vp = AP_Param::find(param_name, &var_type, &parameter_flags);
    if (vp == nullptr || isnan(param_value) || isinf(param_value)) {
        return;
    }
    if (parameter_flags & AP_PARAM_FLAG_INTERNAL_USE_ONLY) {
        // the user can set BRD_OPTIONS to enable set of internal
        // parameters, for developer testing or unusual use cases
        if (AP_BoardConfig::allow_set_internal_parameters()) {
            parameter_flags &= ~AP_PARAM_FLAG_INTERNAL_USE_ONLY;
        }
    }
    if ((parameter_flags & AP_PARAM_FLAG_INTERNAL_USE_ONLY) || vp->is_read_only()) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Param write denied (%s)", param_name);
    } else if (!AP_Param::set_and_save_by_name(param_name, param_value)) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Param write failed (%s)", param_name);
    }
    // let's read back the last value, either the readonly one or the updated one
    if (!AP_Param::get(param_name,param_value)) {
        GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "Param read failed (%s)", param_name);
        return;
    }
    AP_Frsky_MAVlite_Message txmsg;
    txmsg.msgid = MAVLINK_MSG_ID_PARAM_VALUE;
    if (!txmsg.set_float(param_value, 0)) {
        return;
    }
    if (!txmsg.set_string(param_name, 4)) {
        return;
    }
    send_message(txmsg);
}

/*
  Handle a MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN command 
  for FrSky SPort Passthrough (OpenTX) protocol (X-receivers)
  Optionally disable PX4IO overrides. This is done for quadplanes to
  prevent the mixer running while rebooting which can start the VTOL
  motors. That can be dangerous when a preflight reboot is done with
  the pilot close to the aircraft and can also damage the aircraft
 */
MAV_RESULT AP_Frsky_MAVliteMsgHandler::handle_command_preflight_reboot(const mavlink_command_long_t &mav_command_long)
{
    if (!(is_equal(mav_command_long.param1,1.0f) && is_zero(mav_command_long.param2))) {
        return MAV_RESULT_FAILED;
    }
    if (hal.util->get_soft_armed()) {
        // refuse reboot when armed
        return MAV_RESULT_FAILED;
    }

    AP::vehicle()->reboot(false);

    return MAV_RESULT_FAILED;
}

/*
 * Process an incoming mavlite message
 * for FrSky SPort Passthrough (OpenTX) protocol (X-receivers)
 */
void AP_Frsky_MAVliteMsgHandler::process_message(const AP_Frsky_MAVlite_Message &rxmsg)
{
    switch (rxmsg.msgid) {
        case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
            handle_param_request_read(rxmsg);
            break;
        case MAVLINK_MSG_ID_PARAM_SET:
            handle_param_set(rxmsg);
            break;
        case MAVLINK_MSG_ID_COMMAND_LONG:
            handle_command_long(rxmsg);
            break;
#if AP_MISSION_ENABLED
        case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
            handle_mission_request_list(rxmsg);
            break;
        case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
            handle_mission_request_int(rxmsg);
            break;
        case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
            handle_mission_clear_all(rxmsg);
            break;
        case MAVLINK_MSG_ID_MISSION_ITEM_INT:
            handle_mission_item_int(rxmsg);
            break;
#endif
    }
}

#if AP_MISSION_ENABLED
/*
  fork: mission download for radio scripts (OmniDeck), over FrSky passthrough and CRSF.
  MAVlite carries at most MAVLITE_MAX_PAYLOAD_LEN (31) bytes and a MISSION_ITEM_INT is 37, so
  the replies are compact (little endian):
    MISSION_REQUEST_LIST (no payload) -> MISSION_COUNT: count u16, last change ms u32
                                         (the change time tells the radio to fetch it again)
    MISSION_REQUEST_INT: seq u16      -> MISSION_ITEM_INT: seq u16, command u16, frame u8,
                                         x i32, y i32, z float, param1 float, param3 float,
                                         param2 float (29 bytes; param2 carries e.g. the speed
                                         of DO_CHANGE_SPEED)
                                         (seq 0 = the stored home slot, may be empty: use
                                         the live home from telemetry)
                                      or MISSION_ACK: seq u16, MAV_MISSION_INVALID_SEQUENCE
  Read only: nothing here changes the mission.
 */
void AP_Frsky_MAVliteMsgHandler::handle_mission_request_list(const AP_Frsky_MAVlite_Message &rxmsg)
{
    const AP_Mission *mission = AP::mission();
    if (mission == nullptr) {
        return;
    }
    AP_Frsky_MAVlite_Message txmsg;
    txmsg.msgid = MAVLINK_MSG_ID_MISSION_COUNT;
    if (!txmsg.set_uint16(mission->num_commands(), 0) ||
        !txmsg.set_uint32(mission->last_change_time_ms(), 2)) {
        return;
    }
    send_message(txmsg);
}

void AP_Frsky_MAVliteMsgHandler::handle_mission_request_int(const AP_Frsky_MAVlite_Message &rxmsg)
{
    uint16_t seq;
    if (!rxmsg.get_uint16(seq, 0)) {
        return;
    }
    const AP_Mission *mission = AP::mission();
    if (mission == nullptr) {
        return;
    }
    AP_Frsky_MAVlite_Message txmsg;
    mavlink_mission_item_int_t item {};
    if (seq >= mission->num_commands() || !mission->get_item(seq, item)) {
        txmsg.msgid = MAVLINK_MSG_ID_MISSION_ACK;
        if (txmsg.set_uint16(seq, 0) && txmsg.set_uint8(MAV_MISSION_INVALID_SEQUENCE, 2)) {
            send_message(txmsg);
        }
        return;
    }
    txmsg.msgid = MAVLINK_MSG_ID_MISSION_ITEM_INT;
    if (!txmsg.set_uint16(seq, 0) ||
        !txmsg.set_uint16(item.command, 2) ||
        !txmsg.set_uint8(item.frame, 4) ||
        !txmsg.set_int32(item.x, 5) ||
        !txmsg.set_int32(item.y, 9) ||
        !txmsg.set_float(item.z, 13) ||
        !txmsg.set_float(item.param1, 17) ||
        !txmsg.set_float(item.param3, 21) ||
        !txmsg.set_float(item.param2, 25)) {
        return;
    }
    send_message(txmsg);
}

/*
  fork: mission write from a radio (OmniDeck's mission editor). Refused while armed. Every
  request gets a MISSION_ACK: seq u16 (0xFFFF for the clear), result u8 (MAV_MISSION_RESULT).
    MISSION_CLEAR_ALL (no payload): AP_Mission::clear()
    MISSION_ITEM_INT (the compact item of the download, 29 bytes; 25 = no param2): item seq >= 1 replaces the
      item at seq, or is added when seq is the next free slot (home goes into slot 0 by
      itself). Checked by AP_Mission::mavlink_int_to_mission_cmd like a GCS upload.
  The radio reads the mission back afterwards to verify it.
 */
void AP_Frsky_MAVliteMsgHandler::send_mission_ack(uint16_t seq, MAV_MISSION_RESULT result)
{
    AP_Frsky_MAVlite_Message txmsg;
    txmsg.msgid = MAVLINK_MSG_ID_MISSION_ACK;
    if (txmsg.set_uint16(seq, 0) && txmsg.set_uint8(result, 2)) {
        send_message(txmsg);
    }
}

void AP_Frsky_MAVliteMsgHandler::handle_mission_clear_all(const AP_Frsky_MAVlite_Message &rxmsg)
{
    AP_Mission *mission = AP::mission();
    if (mission == nullptr) {
        return;
    }
    if (hal.util->get_soft_armed()) {
        send_mission_ack(0xFFFF, MAV_MISSION_DENIED);
        return;
    }
    send_mission_ack(0xFFFF, mission->clear() ? MAV_MISSION_ACCEPTED : MAV_MISSION_ERROR);
}

void AP_Frsky_MAVliteMsgHandler::handle_mission_item_int(const AP_Frsky_MAVlite_Message &rxmsg)
{
    AP_Mission *mission = AP::mission();
    if (mission == nullptr) {
        return;
    }
    mavlink_mission_item_int_t packet {};
    uint8_t frame;
    if (!rxmsg.get_uint16(packet.seq, 0) ||
        !rxmsg.get_uint16(packet.command, 2) ||
        !rxmsg.get_uint8(frame, 4) ||
        !rxmsg.get_int32(packet.x, 5) ||
        !rxmsg.get_int32(packet.y, 9) ||
        !rxmsg.get_float(packet.z, 13) ||
        !rxmsg.get_float(packet.param1, 17) ||
        !rxmsg.get_float(packet.param3, 21)) {
        return;
    }
    // param2 is optional: only read it when the radio sent it (bytes past len are not cleared)
    if (rxmsg.len >= 29 && !rxmsg.get_float(packet.param2, 25)) {
        return;
    }
    packet.frame = frame;
    packet.autocontinue = 1;
    if (hal.util->get_soft_armed()) {
        send_mission_ack(packet.seq, MAV_MISSION_DENIED);
        return;
    }
    const uint16_t total = mission->num_commands();
    if (packet.seq == 0 || packet.seq > MAX(total, (uint16_t)1)) {
        send_mission_ack(packet.seq, MAV_MISSION_INVALID_SEQUENCE);
        return;
    }
    AP_Mission::Mission_Command cmd {};
    const MAV_MISSION_RESULT res = AP_Mission::mavlink_int_to_mission_cmd(packet, cmd);
    if (res != MAV_MISSION_ACCEPTED) {
        send_mission_ack(packet.seq, res);
        return;
    }
    bool ok;
    if (packet.seq < total) {
        ok = mission->replace_cmd(packet.seq, cmd);
    } else {
        ok = mission->add_cmd(cmd);
    }
    send_mission_ack(packet.seq, ok ? MAV_MISSION_ACCEPTED : MAV_MISSION_NO_SPACE);
}
#endif // AP_MISSION_ENABLED

/*
 * Send a mavlite message
 */
bool AP_Frsky_MAVliteMsgHandler::send_message(AP_Frsky_MAVlite_Message &txmsg)
{
    return _send_fn(txmsg);
}
#endif
