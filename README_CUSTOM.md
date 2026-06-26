# ArduPlane Custom Build — **LIGHT VARIANT**

> 🪶 **This is the light variant of the fork.** A stripped-down build
> that fits more easily on F4 boards. For the full-features build, see
> the [`master_custom_4.6.3`](../../tree/master_custom_4.6.3) branch.
>
> Inspired by mf0o's [`master_custom_light`](https://github.com/mf0o/ardupilot/tree/master_custom_light)
> fork of ArduCustom. Same intent: distribute a smaller-binary variant
> for board operators who don't need every protocol backend.

## What's stripped vs. the full variant

Aligned to the **strict 2022 light-variant definition** mf0o released:
- Only **1 GPS** and **1 magnetometer**
- **No CAN** anywhere (no DroneCAN GPS / RC / battery / airspeed / rangefinder)
- RC: **SBUS, CRSF, IBUS, FPORT** only
- GPS: **UBLOX** only
- Airspeed: **Analog** or **MS4525** pressure sensors only
- Rangefinder: **Benewake LIDARs** only

A per-board hwdef can re-enable any of the stripped items via the
matching `define AP_<X>_ENABLED 1` if a specific airframe needs it.

| Family | Kept (in light) | Stripped (not in light) |
|---|---|---|
| **GPS instances** | 1 | (no redundant / blended GPS) |
| **Magnetometers** | 1 | (no second compass averaging) |
| **CAN entirely** | — | all DroneCAN backends: GPS, RC, BattMon, Rangefinder, Airspeed |
| **Servo drivers** | PWM, DShot, S.BUS-out | Volz, Robotis |
| **Engine** | Electric | ICE (governor, RPM, choke/ignition/throttle channels) |
| **GPS backends** | UBLOX | DroneCAN, MAV, MSP, NMEA, NMEA-Unicore, ERB, GSOF, NOVA, SBF, SBP, SBP2, SIRF |
| **RC input protocols** | CRSF (ELRS/Crossfire), SBUS, IBUS, FPORT, FPORT2 | DroneCAN, DSM, PPMSUM, SRXL, SRXL2, SUMD, ST24, GHST, MAVRADIO |
| **Battery monitor backends** | Analog, ESC telem | DroneCAN, BEBOP, EFI, SMBUS, FuelFlow/Level, Generator, INA2xx, INA3221, Sum, SynDev |
| **Rangefinder backends** | Benewake (TF02, TF03, TFmini, TFmini Plus) | Analog, MAVLink, LightWare, MaxBotix, VL53L0X/L1X, BLPing, BenewakeCAN, NoopLoop, USD1, Wasp, Ainstein, NRA24, GYUS42, HC-SR04, others |
| **Airspeed backends** | Analog, MS4525 | DroneCAN, MSP, ASP5033, DLVR, MS5525, NMEA, SDP3X |
| **EKF** | EKF3 (same as full variant) | EKF2 (off in full variant too, per upstream 4.6.3 default) |

All fork features described below are unchanged — Course Hold, idle
throttle family, AUTO→FBWA stick takeover, OSD additions, stats grid,
emergency landing state machine, etc. — the strips above are purely
about which hardware backends compile in.

> ⚠️ **Matek I2C digital airspeed sensors (ASPD-DLVR / ASPD-7002 /
> ASPD-MS5525) WILL NOT WORK on the light variant.** Use the full
> [`master_custom_4.6.3`](../../tree/master_custom_4.6.3) build for
> those airframes — those chips are DLVR/MS5525, which the strict
> 2022 definition excludes.

> ⚠️ **CAN hardware won't work on light** — no DroneCAN GPS, RC,
> battery, rangefinder, or airspeed. CAN-attached anything needs the
> full variant.

**Binary size:** SITL build is meaningfully smaller than the full
variant — actual per-board delta depends on which backends that
board's hwdef would have pulled in.

---

A curated fork of ArduPlane, rebased onto upstream **ArduPlane 4.6.3**
(tag `Plane-4.6.3`, commit `3fc7011a7d`). This branch carries a set of
additions and behaviour changes on top of stock 4.6.3, mostly ported
from [shellixyz's classic 2022 fork](https://github.com/shellixyz/ardupilot)
and the maintained successor at [ArduCustom/ardupilot](https://github.com/ArduCustom/ardupilot),
plus a few additions originating in this fork.

> If you fly stock ArduPlane and don't recognise the param names in this
> document, you are on the wrong firmware — go to [ardupilot.org](https://ardupilot.org)
> instead.

---

## Building

Identical to upstream. From a configured WSL / Linux toolchain:

```bash
./waf configure --board <your-board>
./waf plane
```

The branch builds clean for every board listed under "Supported boards"
below; SITL builds and runs cleanly.

## Supported boards

Fork-specific board work covers:

- **MatekF405-Wing** — LED-pad-as-relay output, U3 DMA fix, bootloader
  serial wiring
- **revo-mini-sd** *(custom target)*
- **SkystarsF405DJI** *(custom target)*
- **qUark mini wing v4** *(custom target)*
- **NeutronRC_H7_BT** *(custom target, H743, ported from ArduCustom)*
- **OMNIBUSF7V2** — quadplane disabled to fit the firmware
- All stock-upstream boards build and run as-is, including: SpeedyBee
  F405 / F405 Wing / F405 V3 / F405 V4 / F405 AIO / F405 Mini,
  CoreWing F405 Wing, Lefei Longbow F405 Wing, FlyingRC F405 mini,
  Holybro Kakute H743 Wing

Other boards in the upstream tree still build; only the boards above
have been actively validated.

---

# What this fork adds

## Flight modes

### Course Hold (mode 26)
A heading-hold mode similar to CRUISE but the locked target is your
**ground track** instead of the magnetic heading. Useful when you want
the plane to fly a straight line over the ground in wind. Roll stick
input still rolls the plane; releasing returns to the locked course.

- **Enable:** set any `FLTMODE_n = 26`
- **Tunable:** `CRUISE_YAW_RATE` (deg/s — how fast the locked course
  rotates when you hold roll input)

> ⚠️ **Mode-number collision** — Upstream ArduPlane post-4.6.3 uses 26
> for the new AUTOLAND mode. Mission Planner and MAVProxy will display
> Course Hold as "AUTOLAND" in their UI. The firmware runs Course Hold
> correctly; only the GCS label is wrong. Edit
> `modules/mavlink/pymavlink/mavutil.py` `mode_mapping_apm` to teach
> MAVProxy the right name, or change `FLTMODE_n` to 27+ locally.

## Arming & takeoff

### Arm-switch safety — in-flight disarm cuts throttle, doesn't disarm

If the pilot moves the arming aux switch to "disarm" *while the plane
is flying*, the firmware will **not** disarm. Instead it:

1. Cuts the throttle output (every mode, including MANUAL).
2. If the current mode is auto-throttle (RTL / LOITER / AUTO / CRUISE
   / FBWB / etc.), switches to **FBWA** so the pilot can fly the
   plane down by stick.
3. Updates the OSD flight-mode chip to show the disarmed symbol so
   the pilot has a visual confirmation.
4. Latches `emergency_landing` on for the duration of the cut, so if
   an RC failsafe fires before the plane is on the ground the failsafe
   path stays in FBWA rather than reverting to RTL into terrain. The
   flag is restored to its pre-cut value on rearm.
5. Defers the real disarm until `is_flying()` reports false. Once
   the plane has actually landed (`update_is_flying_5Hz` sees it
   below the speed/airspeed thresholds), the deferred disarm
   completes automatically.

While the throttle-cut state is active, **moving the arming switch back
to "arm" rearms instantly with no checks** — it just clears the
throttle-cut and restores the pre-cut mode (if the auto→FBWA bump
happened). This is the V10.0 ArduCustom safety pattern: the throttle
cut is a softer-than-disarm action and you can recover from it.

GCS messages:
- `Throttle cut by arm switch` — at the moment the cut fires (fixed-wing
  or VTOL with throttle stick at zero)
- `Disarm prevented` — VTOL with throttle stick still raised; the motors
  are still spinning, drop the stick to let them stop
- `Rearmed` — when the arm switch goes back to "arm" mid-cut

**VTOL / quadplane behaviour:** in a VTOL mode (QHover, QLoiter, QRTL,
QLand, etc.) the firmware **does not** switch the plane to FBWA on cut.
The motors only stop once the throttle stick reaches zero, so the
pilot can land vertically by holding throttle down. Raising the stick
again before the arming switch flips back keeps the motors live so the
pilot can fly out. The throttle output servo channels (`k_throttle`,
`k_throttleLeft`, `k_throttleRight`) are also forced to zero on the cut
side as belt-and-braces in case a twin-motor path bypasses the standard
throttle suppression.

Caveats:
- **SITL retains the upstream behaviour** so autotest still passes —
  the in-flight disarm guard is gated by `CONFIG_HAL_BOARD != HAL_BOARD_SITL`.
- The deferred disarm completes silently via `disarm_if_requested()`
  in `update_is_flying_5Hz`. It does the standard `disarm()` path so
  log close + AP_Notify state + everything else still runs.
- MAVLink and rudder-stick disarm requests in-flight are still rejected
  outright (no throttle-cut path).

### `ARMING_MODE_SW` — auto mode switch after arming
After arming, wait ~3 seconds, then automatically switch to the
selected mode. Lets a single aux-switch arm and launch a TKOFF/AUTO
mission with no second mode-switch action.

- `ARMING_MODE_SW = 0` — disabled (default)
- `ARMING_MODE_SW = 1` — switch to TAKEOFF after arming
- `ARMING_MODE_SW = 2` — switch to AUTO after arming

### `TKOFF_THR_IDLE` — pre-launch idle throttle
Hold a small throttle the moment the pilot raises the stick while the
plane is sitting in pre-launch suppressed-throttle state (AUTO with
current nav cmd = NAV_TAKEOFF, or TAKEOFF mode). Lets you idle an ICE
engine before launch or spin an electric motor at low RPM ready to fly.

- `TKOFF_THR_IDLE = 0` (%) — disabled (default)
- `TKOFF_THR_IDLE = 25` — hold 25 % throttle when stick raised
- `TKOFF_IDL_SRATE = 50` (%/s) — slew rate while ramping up to the
  idle value (0 = instant snap up)

Snap-to-zero when the stick is dropped is instant regardless of slew
rate. `THR_SUPP_MAN = 1` still takes precedence (continuous manual
passthrough) — set `THR_SUPP_MAN = 0` to let idle throttle take over.

### `TKOFF_IDL_DELAY` — pause before idle ramp

Optional delay (seconds) between the pilot first raising the stick
and the throttle starting to ramp toward `TKOFF_THR_IDLE`. Mainly
useful for ICE engines that want a fixed "warm at idle" pause
before being commanded up.

- `TKOFF_IDL_DELAY = 0` (default) — no delay, idle ramp starts
  immediately on stick-up
- `TKOFF_IDL_DELAY = 5` — hold throttle at 0 for 5 s after first
  stick-up, then start the idle ramp normally

The timer resets whenever the stick returns to zero or the plane
leaves the pre-launch suppressed-throttle state. A GCS text
`TKOFF idle THR timer started` fires when the timer begins.

### Pre-launch audio cues

The buzzer/ToneAlarm plays distinct cont tones during the pre-launch
sequence so the pilot doesn't have to watch the GCS:

| State | Tone | Meaning |
|-------|------|---------|
| `TKOFS_WAITING_TO_RAISE_THROTTLE` | Rapid triple-beep loop | Stick is at zero in pre-launch — raise it to start the idle ramp |
| `TKOFS_WAITING_FOR_IDLE_THROTTLE` | Rising scale (`cdefgab`) | Stick is up, idle throttle is ramping (or `TKOFF_IDL_DELAY` timer is still running) |
| `TKOFS_WAITING_FOR_LAUNCH` | Slow steady beep | At idle throttle, waiting for launch detection to fire |
| `TKOFS_IDLE` | (silent) | Default; takeoff sequence not active |

Tones loop until the state changes — works the same way pre-arm and
EKF-failsafe cont tones do. No parameter to enable; it's automatic
whenever `TKOFF_THR_IDLE` is configured.

### Pilot altitude control during TAKEOFF loiter

After the initial climb in `TAKEOFF` mode, the plane enters a loiter
circle around the target waypoint. The throttle stick now adjusts
altitude during this loiter phase the same way FBW-B does it — push
the stick up to climb, pull down to descend — so the pilot can settle
the plane at the right altitude before flipping to `AUTO` or `RTL`.

No parameter to enable; behaviour is on whenever TAKEOFF is the active
mode and `flight_stage` is past the initial climb.

> **Not (yet) ported:** manual radius and rotation-direction control
> during the same loiter phase. Roll-stick input still rolls the plane
> against the navigation controller, not the loiter geometry. Needs
> the [pilot-loiter-control prerequisite](https://github.com/ArduCustom/ardupilot/commit/dd65f3275f)
> from ArduCustom PR #180 first.

## Pitch trim & tuning knob

### `PTCH_TRIM_DEG` is now knob-tunable
`PTCH_TRIM_DEG` can be assigned to the in-flight tuning knob via
`TUNE_PARAM = 59` (TUNING_PITCH_TRIM). The OSD `OSDx_TUNED_PN` /
`OSDx_TUNED_PV` elements (see OSD section) display the live value
during the tune.

- `TUNE_PARAM = 59`, `TUNE_CHAN = <RC channel>`, `TUNE_RANGE = <max-multiplier>`
- **Note:** AP_Tuning scales multiplicatively, so seed `PTCH_TRIM_DEG`
  non-zero. To search the negative side, seed negative.

### Three switchable tuning sets — `TUNE_PARAM` / `PARAM2` / `PARAM3`

Two extra tuning-set slots and a new RC aux function to switch
between them in flight, so you can run three different tuning
workflows from a single 3-position switch without landing to
change `TUNE_PARAM`.

- `TUNE_PARAM` (existing) — active when the selector aux switch is **low**
- `TUNE_PARAM2` (new) — active in the **middle** position
- `TUNE_PARAM3` (new) — active in the **high** position
- Assign `RCx_OPTION = 250` (`TUNE_PARAM_SELECT`) to a 3-position switch
- All three default to `0` (disabled); set any/all to a non-zero
  tuning ID to enable

When the switch moves, the new set re-centres the knob and the GCS
announces the active parameter — no reboot needed.

### Faster tuning selector timing

The tuning *selector* channel (`TUNE_SELECTOR`) used to need full
seconds of hold time per action. Now:

| Action            | Old        | New        |
|-------------------|-----------:|-----------:|
| Save tune         | > 5000 ms  | > 3000 ms  |
| Debounce window   | < 200 ms   | < 50 ms    |
| Re-centre         | 200–2000   | 50–500     |
| Next parameter    | 2000–5000  | 500–3000   |

Interactions feel responsive in fractions of a second instead of
multi-second holds.

### ATT log uses trim-corrected pitch target
The logged `ATT.DesPitch` channel now has `PTCH_TRIM_DEG` subtracted
out, so log replay matches the value the controller actually targets
in horizon-relative terms.

## RTL & failsafe behaviour

### `RTL_LVL_RLL_LMT` — separate roll limit for the initial climb
During RTL's initial climb to `RTL_CLIMB_MIN`, the roll limit is
reduced to this value (deg) to keep the plane stable as it gains
altitude. After the climb phase the normal `LIM_ROLL_CD` applies.

### `FLIGHT_OPTIONS` bit 22 — `RTL_CLIMB_FIRST_ONLY_IN_FS`
Without this bit, RTL always climbs to `RTL_CLIMB_MIN` first before
heading home. With the bit set, the climb-first behaviour applies
**only during RC failsafe** — a normal pilot-triggered RTL skips
straight to navigation.

### `FLIGHT_OPTIONS` bit 23 — `ALLOW_GLIDING_IN_AUTO_THR_MODES`
With this bit, pulling the throttle stick below `THR_DZ` in any
auto-throttle mode *except* TAKEOFF and AUTO triggers the TECS
gliding path (throttle to 0, target airspeed dropped to
`AIRSPEED_MIN`). Releasing the throttle restores auto-throttle.

### `FLIGHT_OPTIONS` bit 24 — `RTL_MANUAL_ALT_CONTROL`
With this bit, RTL hands altitude control to the pilot's pitch stick
via the FBW-B controller while outside of RC failsafe — push to climb,
pull to descend — the same way altitude works in FBWB/CRUISE. The L1
controller still flies the plane home; only the vertical axis is
under the pilot. Useful when you want to RTL but choose a different
return altitude than `ALT_HOLD_RTL` without leaving the mode.

The instant RC failsafe fires (or you clear the bit mid-flight), RTL
snaps back to its normal climb-to-`ALT_HOLD_RTL` behaviour cleanly:
`prev_WP_loc` is reset to the current position so TECS doesn't get a
sudden altitude jump.

Interactions:
- `RTL_ALT_HOME` is suppressed while manual control is active — the
  descend-to-home-altitude logic doesn't fight the stick.
- `RTL_CLIMB_MIN` / `CLIMB_BEFORE_TURN`'s initial-climb bump is
  skipped — TECS isn't pushed to climb when the pilot already owns
  altitude.
- Pitch stick mixing is bypassed (the FBWB path is the sole consumer
  of the pitch stick), so deflection isn't applied twice.

> ⚠️ Bit relocated from upstream-fork bit 20 to bit 24 in this rebase
> because upstream 4.6.3 took bit 20 for the unrelated
> `COURSE_HOLD_HEADING_CONTROL_WITH_YAW_STICK` feature. Update your
> `FLIGHT_OPTIONS` saved value when migrating from the 2022 fork:
> `1<<20` becomes `1<<24`.

### Lost-vehicle audio alarm during RC failsafe
Once armed in RC failsafe, the lost-vehicle alarm fires. Reuses the
existing notify channel; no new param.

### Mission restart on AUTO if not flying
If you switch to AUTO before takeoff and the mission's first
non-jump item is a NAV_TAKEOFF, the mission restarts at item 1 each
time you enter AUTO from a non-flying state. Prevents accidentally
resuming a mid-mission item after a failed launch.

## Emergency landing (RC failsafe)

A state machine that initiates a controlled glide to the home area
after a sustained RC failsafe. Once engaged it cycles through:

1. **DELAY** — wait `FS_ELAND_DELAY` seconds at the home loiter
2. **SINKING_TO_GLIDE_ALTITUDE** — descend (with tightened loiter
   radius if `FS_ELAND_LOTRAD` is set) to `FS_ELAND_GLDALT` m AGL
3. **ALIGNMENT_INTO_WIND** — bank into the wind (if `FS_ELAND_UPWIND`)
4. **GLIDING** — wings-level glide (throttle to 0, `AIRSPEED_MIN`)
5. **GLIDING_NO_RETURN** — past the no-return altitude, full commit
6. **Auto-disarm on landing**

The leveling-altitude phase forces wings-level below `FS_ELAND_LVLALT`
AGL to avoid wingtip strikes.

- `FS_ELAND_DELAY = -1` — feature disabled (default). Any non-negative
  integer sec enables it.
- `FS_ELAND_LVLALT` (m AGL, default -1 = disabled) — force wings-level
  below this altitude
- `FS_ELAND_GLDALT` (m AGL, default 15) — target altitude to start
  gliding from
- `FS_ELAND_UPWIND` (default 1) — align into wind before gliding
- `FS_ELAND_LOTRAD` (m, default 0 = use `RTL_RADIUS`) — tighter loiter
  radius during the sinking phase

Terrain-aware via `AP_Terrain` where available.

## Auto-trim improvements

- Servo auto-trim only triggers when `SERVO_AUTO_TRIM = 1` (one-shot);
  the param self-disables back to 0 after a successful trim run, so a
  follow-up flight doesn't accidentally re-trim.
- GCS reports trim progress per surface: "Ailerons trim finished",
  "Elevator trim finished", then "Servo auto trim finished".
- `SAT_FINISHTHRESH` (default 20, range 0–255) — the PWM-microsecond
  threshold under which a surface's cumulative 10 s trim adjustment is
  considered "good enough". Lower = stricter (autotrim runs longer);
  higher = more lenient. Default raised from a hardcoded 4 to match
  ArduCustom, so users with mechanical slop or noisy inputs see
  autotrim settle faster.

## AUTO → FBWA stick takeover

Move the pitch or roll stick past 10 % deflection while in AUTO and the
plane immediately switches to FBWA. Lets the pilot take over instantly
without reaching for a mode switch. The takeover is **sticky** —
returning the stick to centre does **not** re-enter AUTO; you'd need
to flip the mode switch back manually.

- `RC_OPTIONS` bit 20 = `AUTO_SWITCH_TO_FBWA_WITH_STICKS`
- **Default ON** in this build. New-storage `RC_OPTIONS` default is
  `1049120` (= bits 5+9+20).
- Clear bit 20 to disable (`param set RC_OPTIONS 544`).

Works both pre-launch (sitting on the runway waiting for `TKOFF_THR_MINSPD`)
and in-flight — `ModeAuto::update()` runs every loop AUTO is the active
mode.

> ℹ️ AUTO only — RTL and other nav modes aren't covered. If you want
> stick-takeover from RTL too, holler and I'll extend it.

## Manual airspeed control in nav modes

In RTL / LOITER / CIRCLE (always) and AUTO (if `FLIGHT_OPTIONS` bit
19 = `MODE_AUTO_MANUAL_AIRSPEED_CONTROL`), the pilot's throttle stick
position maps to target airspeed across the `AIRSPEED_MIN` to
`AIRSPEED_MAX` range — same UI as FBWB/CRUISE. Lets you slow down
to loiter or speed up to penetrate wind without leaving the auto mode.

## Throttle output deadzone

### `THR_DZ` — output throttle deadzone
Below `THR_DZ` % output, the throttle is forced to 0. Keeps the
motor truly off when TECS commands a low but non-zero value (typical
under glide / descent). Default 4 %.

## Throttle voltage compensation in MANUAL mode

Upstream applies `FWD_BAT_VOLT_*` throttle voltage compensation in every
auto-throttle mode but skips it in MANUAL (and all the other manual-
throttle modes: STABILIZE, TRAINING, ACRO, FBWA, AUTOTUNE). This fork
extends voltage compensation to MANUAL too, so the pilot's stick
position maps to a more consistent thrust as the battery sags.

- Default: **enabled** in MANUAL (matches the legacy fork's PR #139).
  Set `FWD_BAT_VOLT_MIN` / `FWD_BAT_VOLT_MAX` to configure; comp is
  inactive when either is 0.
- Opt out: set `RC_OPTIONS` bit 22 (`PLANE_DISABLE_MAN_BAT_COMP`) to
  revert to upstream's "MANUAL is pure passthrough" behaviour.

The other manual-throttle modes (STABILIZE / TRAINING / ACRO / FBWA /
AUTOTUNE) keep upstream's behaviour and **do not** get voltage comp.
Fork PR #33 (which extended comp into those modes on the 2022 fork) is
*not* re-ported because upstream's `Mode::use_battery_compensation()`
virtual already provides the same partitioning that PR #33 was aiming
at -- the work is upstream.

## Smarter RC relays

The relay aux functions (`RCx_OPTION = RELAY1..6`) now treat your
configured `RCx_MIN` / `RCx_MAX` as the **PWM range that activates
the relay**, instead of the default ArduPilot behaviour where the
relay only turned on past the 3-position-switch HIGH threshold
(~1700 µs).

You can now use one RC channel to control both the relay (e.g. VTX
power pad) and an OSD layout / mode, as long as the relay-active
PWM band is what you set via `RCx_MIN/MAX`.

`RCx_REVERSED = 1` inverts the in-range test — handy when the
hardware pad is active-low (Matek video-power pads behave this way).
Gated by `RC_OPTIONS` bit 7 (`ALLOW_SWITCH_REV`) as before.

**Boot-time relay state now mirrors `RCx_REVERSED`.** Previously
the relay was left at the HAL default until the first RC frame
arrived (usually LOW). Now `RCx_REVERSED=0` → relay starts LOW on
boot, `RCx_REVERSED=1` → starts HIGH. Solves the "VTX powered on
at boot before the radio is turned on" problem on Matek FCs where
the video-power pad is active-low.

## Steering assist supports reverse driving

`calc_nav_yaw_ground()` now flips the steering output sign when the
plane is actually moving backward on the ground. Below 0.5 m/s the
GPS heading vector is too noisy, so the firmware falls back to the
`reversed_throttle` flag (the pilot's commanded reverse). Above 0.5
m/s it compares ground-track direction to aircraft yaw — if they're
180° apart (within ±90°) the plane is reversing and the steering
sign is negated.

Without this, ground steering on a rolling reverse takeoff or taxi
would kick the plane the wrong way. No parameter — behaviour is
automatic. Mainly useful for taildraggers and pusher props that
back up under power.

## MAVFTP reliability on low-RAM F4 boards

`MAVFTP` (the MAVLink file-transfer protocol used for log download,
script upload, param backup, etc.) sometimes failed to initialise
at boot on F4-class FCs (192 KB RAM) because the lazy init from the
first FTP request raced with RAM still being fragmented during
startup. Symptom: the first FTP transaction from your GCS would
fail outright.

This build retries `ftp_init()` once per second from the main vehicle
loop until it succeeds. The function is idempotent (returns true
immediately if the buffer is already allocated), so it's a no-op
once init succeeds.

Boards that benefit: every F4 board in the supported list
(MatekF405-Wing, revo-mini-sd, SkystarsF405DJI, qUark mini wing v4,
SpeedyBee F405 / Wing / V3 / V4 / AIO / Mini, CoreWing F405 Wing,
Lefei Longbow F405 Wing, FlyingRC F405 mini). H7 boards have
abundant RAM and weren't affected.

## `is_flying` heuristic — fixed thresholds

The internal "is this thing actually airborne" detector
(`update_is_flying_5Hz`) used to scale its ground-speed and airspeed
thresholds against your configured `MIN_GROUNDSPEED` and
`AIRSPEED_MIN`. Those params exist for TECS / navigation bounds, not
for flying detection, and the coupling produced wrong answers on
airframes far from default tuning.

Now uses fixed thresholds:
- **Ground speed:** 1.5 m/s (the `GPS_IS_FLYING_SPEED_CMS` constant)
- **Airspeed:** 25 km/h (~6.94 m/s)

If your `MIN_GROUNDSPEED > 1.7 m/s`, the plane will be considered
"flying" slightly earlier in the takeoff roll. If your
`AIRSPEED_MIN > 9.3 m/s`, "flying" triggers at a lower airspeed than
before. Defaults are unaffected. The downstream consumers (failsafe,
crash detection, auto state) all become slightly more responsive.

## Stick mixing default

`STICK_MIXING` now defaults to `0` (no FBW-style pilot override) in
auto/RTL/guided. Pilots used to upstream's behaviour can set
`STICK_MIXING = 1` to restore it.

## Aileron / elevator differential throws

Asymmetric control-surface throws, set in firmware so they survive radio
swaps and reflashes. Useful for airframes that want more up-throw than
down-throw on the ailerons (less adverse yaw) or asymmetric elevator
behaviour for high-alpha handling.

- `AILERON_DIFF` (% in `[-90, 90]`, default 0) — applied to the split
  aileron outputs, to flaperons (after the auto-flap split), and to
  dspoiler elevons when not in flying-wing mode. Positive value reduces
  the down side; negative reduces the up side.
- `ELEVATOR_DIFF` (% in `[-90, 90]`, default 0) — applied to the
  `k_elevator` output after the elevon/vtail mixers have read from it.
  Same sign convention as `AILERON_DIFF`. Does not apply to elevons (use
  `AILERON_DIFF` for the aileron-side asymmetry on flying wings).

Two new servo functions are added so a single physical aileron channel
can split into left/right outputs with the diff applied per side:

- **200** — `AileronLeft` (mirrors `k_aileron` with sign inverted and
  `AILERON_DIFF` applied)
- **201** — `AileronRight` (same as `k_aileron` but with `AILERON_DIFF`
  applied)

A "classic" build that wires both ailerons to one channel with a Y-cable
keeps using `4:Aileron` and `AILERON_DIFF` is a no-op. A split build
uses `200/201` on two channels and gains per-side differential throw.

Both knobs are also exposed to in-flight RC tuning via `TUNE_PARAM = 90`
(AILERONS_DIFF) and `TUNE_PARAM = 91` (ELEVATOR_DIFF), with the standard
single-param multiplicative-scaling rules (must be seeded non-zero to
search a range; sign of the seed picks which polarity is swept).

> ⚠️ **Deepstall landing is disabled** in this fork (`HAL_LANDING_DEEPSTALL_ENABLED`
> default flipped to `0`). The deepstall code path writes elevator PWM
> directly via `set_output_pwm`, which bypasses the `ELEVATOR_DIFF`
> attenuation in `set_output_scaled`. The two features are mechanically
> incompatible. Re-enable per-board via hwdef define if you actually
> use deepstall and don't need `ELEVATOR_DIFF`.

## Throttle / flap → elevator PWM offset mixes

Two additive feed-forward mixes that shift the calculated elevator PWM
in proportion to throttle or flap position. Useful for compensating
aerodynamic pitching moments — e.g. high-thrust-line airframes that
pitch up when throttle goes in, or planes that pitch up on flap
extension and need a bit of down-elevator to stay trimmed.

- `KFF_THRAT2ELEV` (PWM µs in `[-500, 500]`, default 0) — shift applied
  at full throttle, linearly interpolated from 0 at `TRIM_THROTTLE`.
  Below trim there's no shift. Sign: positive shifts the elevator in
  the same direction as a positive (pitch-up) command, so set negative
  to *reduce* pitch-up tendency with throttle.
- `KFF_FLAP2ELEV` (PWM µs in `[-500, 500]`, default 0) — shift applied
  at 100% flap deployment, linearly scaled with current flap percent.
  Same sign convention.

Both are post-processing offsets applied in `servos_output()` *after*
the standard mixers (elevon, vtail, dspoiler) and after the diff-throws
have run. The shift is broadcast to every elevator-equivalent channel
(`k_elevator`, both elevons, both vtails, and on flying-wing builds the
outer + inner dspoilers). Servo reversal is honored: a positive
`KFF_*2ELEV` always means "elevator-up direction" regardless of how the
individual channels are wired.

Both knobs are exposed to in-flight RC tuning:
- `TUNE_PARAM = 82` (`THRAT2ELEV`)
- `TUNE_PARAM = 83` (`FLAP2ELEV`)

Standard single-param multiplicative-scaling caveat applies: must be
seeded non-zero to search a range, sign of the seed picks polarity.

---

# OSD additions

## New elements

| Param family       | Element                                       |
|--------------------|-----------------------------------------------|
| `OSDx_LOIT_RAD`    | Loiter radius (active loiter / RTL)           |
| `OSDx_RC_THR`      | Input (pilot) throttle %                      |
| `OSDx_TUNED_PN`    | Tuning-knob target parameter **name**         |
| `OSDx_TUNED_PV`    | Tuning-knob target parameter **value**        |
| `OSDx_AOA`         | Angle-of-attack                               |
| `OSDx_ACC_LONG`    | Longitudinal G                                |
| `OSDx_ACC_LAT`     | Lateral G                                     |
| `OSDx_ACC_VERT`    | Vertical G (with optional warning-flash)      |
| `OSDx_AVG_EFG`     | Average ground efficiency (standalone)        |
| `OSDx_AVG_EFA`     | Average air efficiency (standalone)           |
| `OSDx_ASPD_DEM`    | TECS demanded airspeed                        |
| `OSDx_AUTO_FLP`    | Active auto-flap deflection                   |
| `OSDx_RC_FS`       | RC failsafe indicator                         |
| `OSDx_PEAK_RR`     | Peak roll rate (since last reset)             |
| `OSDx_PEAK_PR`     | Peak pitch rate                               |
| `OSDx_STATS_EN`    | **Flight stats grid** — 11-row post-flight summary (see below) |

## OSD options (`OSD_OPTIONS` bits)

| Bit | Name                                       | Effect                                              |
|----:|--------------------------------------------|-----------------------------------------------------|
| 18  | `OPTION_TWO_DECIMALS_VERTICAL_SPEED`       | Vspeed shown to 2 decimals                          |
| 21  | `OPTION_ONE_DECIMAL_ATTITUDE`              | Pitch / roll shown to 1 decimal (default ON)        |
| 23  | `OPTION_SHORTEN_PLUSCODE`                  | Strip leading digits from plus codes when home is near |

## Tunables added

| Param             | Default | Purpose                                             |
|-------------------|--------:|-----------------------------------------------------|
| `OSD_AH_PITCH_MAX`|  20 °   | Artificial-horizon max pitch indicator angle        |
| `OSD_W_ASPD_LOW`  |   0     | Underspeed warning threshold (0 = off)              |
| `OSD_W_ASPD_HIGH` |   0     | Overspeed warning threshold (0 = off)               |
| `OSD_W_VERT_ACC`  |   0     | Vertical-G warning threshold (0 = off)              |
| `OSD_TUNE_DTMOUT` |   4 s   | Tuned-param OSD display timeout                     |
| `OSD_EFF_UNIT`    | Wh      | Default efficiency unit (Wh vs mAh — upstream is now also Wh-default) |

## Display polish

- **Vspeed LPF** (alpha 0.33) — smoothed display
- **Efficiency LPF** (alpha 0.2) — smoothed ground / air / climb efficiency
- **Rangefinder altitude** — attitude-corrected to vertical, with
  `RNGFNDx_GNDCLEAR` subtracted (so the OSD shows altitude above ground,
  not above the sensor mount)
- **Pitch / roll numeric fix** — VTOL-aware via
  `AP::vehicle()->get_osd_roll_pitch_rad()` and proper level-symbol
  thresholds
- **RSSI clamp** — `MIN(99, …)` prevents 3-char widening at value=1.0
- **Home placeholder** — `<HOME>- 0.0m` rendered before GPS fix as a
  placement aid
- **Distance display** — per-magnitude format ladder with leading-space
  alignment for column-stable values across magnitude boundaries and
  sign flips
- **Speed/arrow split** — `draw_speed` can render magnitude only;
  arrow-direction variant available for wind/groundspeed
- **DJI FPV / HDZero numeric attitude** — uses
  `get_osd_roll_pitch_rad()` so the goggles match the horizon
- **Extended CRSF link-stats elements on by default** —
  `AP_OSD_LINK_STATS_EXTENSIONS_ENABLED` flipped 0 → 1 so
  `OSDx_LINK_Q`, `OSDx_RC_PWR`, `OSDx_RSSIDBM`, `OSDx_RC_SNR`,
  `OSDx_RC_ANT`, and `OSDx_RC_LQ` are compiled in without needing
  the custom build server. Costs ~3-5 KB flash; fork users typically
  fly ELRS/CRSF and want these readings on the OSD. Per-board hwdef
  can still override by defining the macro to 0 before include.
- **5 OSD screens** default (was 4)
- **Widened OSD sidebars** — wider value columns on the artificial-
  horizon sidebars so 3- and 4-digit values fit cleanly. Inherited
  from upstream 4.6.3 (port by [@mf0o](https://github.com/mf0o))
- **DJI O3 home / waypoint arrow direction fix** — the BF MSP
  DisplayPort symbol mapping for the home and waypoint direction
  arrows had the wrong indices on DJI O3 goggles, making the arrows
  point in confusing directions. Inherited from upstream 4.6.3
  (port by [@mf0o](https://github.com/mf0o))

## Stats grid

`OSDx_STATS_EN = 1` enables an 11-row post-flight statistics grid
showing min battery / cell voltage, avg/max current and power,
consumed mAh/Wh, avg/max ground/air/wind speed, avg efficiency
(ground & air), distance travelled (ground & air), max home distance,
max altitude, and flight time. Renders on disarm or with the screen
selector.

Stats themselves are armed-relative (pre-arm baseline captured,
deltas accumulated armed); wind stats are gated until 30 s of flight
time has accumulated.

### Persistent maximum-flight-time stat

`STAT_FLT_TIME_MX` (seconds, read-only, persisted) — the longest
single-flight duration ever recorded since last reset. Captured at
takeoff/landing transition (`set_flying(false)` path) and at every
30 s flush tick. Useful for endurance comparisons across batteries
or aircraft. Reset alongside the other stats via `STAT_RESET`.

---

# Quadplane / VTOL

## `Q_TRIM_PITCH` is now knob-tunable

`Q_TRIM_PITCH` (the AHRS pitch trim used by QuadPlane's hover
attitude reference) can be assigned to the in-flight tuning knob via
`TUNE_PARAM = 92` (TUNING_Q_TRIM_PITCH). Same shape as `PTCH_TRIM_DEG`
being knob-tunable for fixed-wing.

- `TUNE_PARAM = 92`, `TUNE_CHAN = <RC channel>`, `TUNE_RANGE = <max-multiplier>`
- Active only when QuadPlane is enabled
- OSD `OSDx_TUNED_PN` / `OSDx_TUNED_PV` display the live value

## `Q_M_FRTB_RATIO` — Tri-frame front/rear thrust balance

Scales the rear motor's thrust output relative to the front pair on
Tri-class VTOL planes. Lets you bias the hover thrust split without
rebuilding the airframe or re-tuning the rate loop.

- `Q_M_FRTB_RATIO = 1.0` (default) — unchanged from upstream behaviour
- `< 1.0` — push the rear motor harder (front motors do less work)
- `> 1.0` — push the front motors harder (rear motor does less work)
- Range: 0.5–2.0, step 0.001, Q_M_ subgroup

Only the `AP_MotorsTri` backend uses this; ignored for quad / hex /
oct frames.

# Battery monitoring (AP_BattMonitor)

A larger toolbox of per-cell and Wh-based limits, plus an
internal-resistance-aware consumed-Wh calculation.

## New params

| Param             | Purpose                                                |
|-------------------|--------------------------------------------------------|
| `BATTn_LOW_CV`    | Low-cell-voltage failsafe threshold                    |
| `BATTn_CRT_CV`    | Critical-cell-voltage failsafe threshold               |
| `BATTn_CELL_VFULL`| Full-cell voltage (defines pack-full-when-plugged-in)  |
| `BATTn_CAPA_WH`   | Pack capacity in Wh (alternative to mAh)               |
| `BATTn_LOW_WH`    | Low-Wh failsafe                                        |
| `BATTn_CRT_WH`    | Critical-Wh failsafe                                   |
| `BATTn_ARM_WH`    | Minimum Wh required to arm                             |
| `BATTn_CELL_DT_V` | Per-cell delta voltage for "full when plugged in"      |
| `BATTn_CELL_COUNT`| Manual override of detected cell count                 |

## Behaviour changes

- `consumed_wh` now integrates with `voltage_resting_estimate` (was
  raw `voltage`). Reported Wh increases by ~10 % under load vs
  upstream — this is the *energy delivered*, internal-resistance
  losses included. A new `consumed_wh_without_losses` field preserves
  the upstream semantic for tools that need it.
- `BATTn_OPTIONS` is now uint32 (was uint16) — bit 23 =
  `Use_Wh_for_remaining_percent_calc`, so the OSD battery % is
  computed off Wh instead of mAh when both `CAPA_WH` and the bit are
  set.

---

# Stats (AP_Stats)

30+ tracked statistics vs upstream's 4 (boot count / flying time /
run time / reset). Includes per-flight max airspeed/groundspeed,
flight count, average distance from home, total energy consumed,
etc. Used by the OSD stats grid above.

Schedule frequency increased 1 Hz → 100 Hz so peak values are
captured accurately.

---

# TECS / throttle

Several control-loop ports from the fork plus tuning hooks. All
have safe defaults — set them only if you actually need them.

## Default changes

- `TECS_INTEG_GAIN` default `0.3` → `0.4` (saved values preserved)

## New params

| Param            | Default | Purpose                                                                 |
|------------------|--------:|-------------------------------------------------------------------------|
| `THR_FF_FILT`    | 1.0     | Throttle feed-forward low-pass-filter alpha (1.0 = no filter)           |
| `THR_FF_DAMP`    | 1.0     | Throttle FF dampening multiplier (1.0 = full FF)                        |
| `RTL_SINK_MAX`   | 0       | Max sink rate in RTL (m/s, 0 = use `TECS_SINK_MAX`)                     |
| `RTL_CLIMB_MAX`  | 0       | Max climb rate in RTL                                                   |
| `AUTO_SINK_MAX`  | 0       | Max sink rate in AUTO                                                   |
| `AUTO_CLIMB_MAX` | 0       | Max climb rate in AUTO                                                  |

`THR_FF_FILT` / `THR_FF_DAMP` are also exposed as a tuning set —
`TUNE_PARAMSET = 111` (`TUNING_SET_TECS_THR_FF`) sweeps both at once.

## Behaviour changes

- Throttle slew rate is now applied **after** the integrator sum, so
  the rate limit catches PD + FF + integrator together (was: rate
  limit applied only to PD+FF, then integrator added on top)
- Throttle command in suppressed / non-auto-throttle phases is now
  written into TECS via `set_throttle_demand()`, giving a smooth
  hand-off when auto-throttle takes over (no THR_CRUISE snap)
- TECS gliding integrates with the legacy `set_gliding_requested_flag`
  hook, shared with AP_Soaring

---

# EKF / state estimator

**EKF3-only. EKF2 is not compiled in.**

Upstream 4.6.3 already defaults `HAL_NAVEKF2_AVAILABLE` to 0 (in
`AP_AHRS_config.h:29`, with the comment "EKF2 slated compiled out by
default in 4.5, slated to be removed"). This fork honours that default
on every board it targets — neither the firmware binary nor the param
list contains any EKF2 surface. Set `AHRS_EKF_TYPE = 3` for normal
operation (already the default).

Boards that explicitly re-enable EKF2 in their hwdef (e.g. upstream's
`CubeRed-EKF2` test target) are excluded from this fork's CI matrix.

---

# Telemetry

## CRSF

- Vehicle UID transmitted (for goggle OSD systems that key on it)
- Relative altitude (above home) sent in addition to AMSL
- CRSF system-info messages suppressed by default (cuts noise)

## MSP / DJI

- DJI FPV / HDZero goggles get attitude from the same source as the
  OSD horizon (VTOL-aware), so the numeric pitch/roll displays match
  the artificial horizon
- MSP wind reported in km/h (was m/s)
- Optional Link Quality output instead of raw RSSI

## FrSky SPort passthrough

- Airspeed bit now toggles between airspeed and groundspeed each frame
- `FRSKY_OPTIONS` default flipped 0 → 1 to enable the alternation

## Common

- Throttle/scaled-RC values promoted int16 → float across all
  vehicle types (Plane/Copter/Rover/Sub/Blimp). Affects
  `VFR_HUD.throttle`, `RC_Channel::get_control_in()`, scripting
  `gcs:get_hud_throttle()`. Resolution improves; widening is
  source-compatible for callers that took `int8/16` (implicit
  conversion).

---

# Behaviour changes vs upstream — migration notes

These are the user-visible differences if you save params from a
stock 4.6.3 build and then load this firmware:

| Default change                                                                                  | Effect                                                                       |
|-------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------|
| `STICK_MIXING` 1 → 0                                                                            | Pilot stick no longer overrides nav controllers in AUTO/RTL/GUIDED.          |
| `TECS_INTEG_GAIN` 0.3 → 0.4                                                                     | Slightly snappier altitude tracking.                                         |
| `OSD_OPTIONS` default gains bit 18 (2-decimal vspeed) and bit 21 (1-decimal attitude)           | Cosmetic OSD-only; saved values preserved.                                   |
| `OSD_EFF_UNIT` default Wh                                                                       | Was mAh in pre-2024 upstream; upstream now also Wh-default.                  |
| `FRSKY_OPTIONS` default 0 → 1                                                                   | SPort passthrough rotates airspeed/groundspeed (FrSky users only).           |
| `consumed_wh` integrates with `voltage_resting_estimate`                                        | Reported Wh ~10 % higher under load; use `consumed_wh_without_losses` for the old value. |
| `AP_Stats` scheduled at 100 Hz                                                                  | Peak-stat accuracy; CPU cost is negligible.                                  |
| `BATTn_OPTIONS` is uint32                                                                       | Bit 23 = use Wh for remaining %. Backwards-compat with uint16 saved values.  |
| `HAL_LANDING_DEEPSTALL_ENABLED` (BOARD_FLASH_SIZE > 1024) → 0                                   | Deepstall landing compiled out; mechanically incompatible with `ELEVATOR_DIFF`. Per-board hwdef can opt back in. |
| MANUAL mode now applies `FWD_BAT_VOLT_*` throttle voltage compensation                          | Pilot's throttle stick maps to consistent thrust as battery sags. Set `RC_OPTIONS` bit 22 (`PLANE_DISABLE_MAN_BAT_COMP`) to opt back into upstream's pure-passthrough behaviour. Only takes effect if `FWD_BAT_VOLT_MIN`/`MAX` are configured (non-zero). |

---

# Deliberately *not* included

Items evaluated and deliberately skipped, with reasoning:

- **Angle-control-as-PID rewrite** — large architectural risk; sticking
  with upstream's controllers
- **Throttle curves** — upstream now has its own
- **Stick-mixing removal entirely** — keeping the param exposed is
  more discoverable than deleting it
- **Throttle-expo (fixed-wing)** — interlocked with deferred TECS
  reconciliation work
- **Auto-throttle nudge rewrite** — upstream already rewrote nudge
  (`8aafe85f6f`); needs reconciliation, not a parallel port
- **`#164` Plane-only OSD element gating** — namespace cleanup with no
  Plane-only benefit; this build targets Plane only

---

# Documentation and tooling

## Full parameter reference

Machine-generated parameter metadata for **this fork** lives at
[`docs/parameters/`](docs/parameters/). It covers every parameter
the firmware exposes — including every fork-added bit on
`FLIGHT_OPTIONS`, `RC_OPTIONS`, `OSD_OPTIONS`, and so on.

### 👉 Browse the parameter reference

Click either link below to read the full HTML parameter reference
in your browser — no download, no GitHub account:

- **Light branch** — <https://raw.githack.com/erwinquilloy/ardupilot/master_custom_4.6.3_light/docs/parameters/Parameters.html>
- **Full branch** — <https://raw.githack.com/erwinquilloy/ardupilot/master_custom_4.6.3/docs/parameters/Parameters.html>

(`raw.githack.com` serves the HTML with the right `Content-Type` so
your browser actually renders it. `raw.githubusercontent.com` would
serve the same file as plain-text source code.)

### Other formats

All six formats `param_parse.py` produces are committed in the same
directory:

| File | Use case |
|---|---|
| [`apm.pdef.json`](docs/parameters/apm.pdef.json) | Best for tooling — easy to parse in JS / Python |
| [`apm.pdef.xml`](docs/parameters/apm.pdef.xml) | Alternative structured format |
| [`Parameters.html`](docs/parameters/Parameters.html) | Browsable HTML (rendered URLs above) |
| [`Parameters.md`](docs/parameters/Parameters.md) | GitHub-rendered Markdown |
| [`Parameters.rst`](docs/parameters/Parameters.rst) | Sphinx-flavoured reStructuredText |
| [`ParametersLatex.rst`](docs/parameters/ParametersLatex.rst) | Book-style RST tuned for LaTeX |

For tooling that fetches the raw JSON / XML directly, use
`raw.githubusercontent.com` (it serves the file's actual bytes with
`text/plain`, which is what JSON/XML parsers expect):

```
https://raw.githubusercontent.com/erwinquilloy/ardupilot/master_custom_4.6.3_light/docs/parameters/apm.pdef.json
https://raw.githubusercontent.com/erwinquilloy/ardupilot/master_custom_4.6.3/docs/parameters/apm.pdef.json
```

The directory's own [README](docs/parameters/README.md) lists the
fork-specific bitmask bits and the most useful params to bring up
in a calculator UI.

## Bitmask calculator

A self-contained HTML/JS bitmask calculator for **this fork's**
parameter set. It loads the JSON above, lets you tick bits to
compute the integer value (or paste a value to see which bits are
set), and shows decimal / hex / binary representations. Runs
entirely in your browser — no server, no analytics, no GitHub
account needed.

### 👉 Open the calculator

Click either link below to use the calculator in your browser:

- **Light branch** — <https://raw.githack.com/erwinquilloy/ardupilot/master_custom_4.6.3_light/docs/bitmask_calculator.html>
- **Full branch** — <https://raw.githack.com/erwinquilloy/ardupilot/master_custom_4.6.3/docs/bitmask_calculator.html>

`raw.githack.com` is a free public CDN — no account, no login. It
just proxies GitHub raw files with the correct `Content-Type` so
HTML renders instead of showing source code. The URLs always serve
the latest committed version of the file from the named branch;
bookmark whichever matches the firmware variant you flash.

### Computing or reverse-engineering a value

**To compute a value:**
1. Pick the param (e.g. `FLIGHT_OPTIONS`) from the dropdown.
2. Tick the bits you want. The decimal / hex / binary fields update live.
3. Hit "Copy" and paste into Mission Planner / MAVProxy / QGroundControl.

**To reverse-engineer an existing value:** type or paste the integer
into the Value (decimal) field; the bit checkboxes will reflect which
bits are set.

Inspired by Stavros' [ArduPilot bitmask calculator](https://notes.stavros.io/ardupilot/bitmask-calculator/);
this version uses **this fork's parameter set** so the fork-added bits
on `FLIGHT_OPTIONS` (21 / 22 / 23 / 24), `RC_OPTIONS` bit 22, the
`OSD_OPTIONS` bits, etc. are visible without leaving the page.

### Want it offline / want to host it yourself?

The page also accepts a local JSON file upload, so you can keep a
copy on a tablet for the field:

```bash
curl -O https://raw.githubusercontent.com/erwinquilloy/ardupilot/master_custom_4.6.3_light/docs/bitmask_calculator.html
curl -O https://raw.githubusercontent.com/erwinquilloy/ardupilot/master_custom_4.6.3_light/docs/parameters/apm.pdef.json
```

Open the HTML file in your browser, then in the "Source" dropdown
pick **Upload file…** and point at the JSON.

---

# Branch / repository layout

- `master_custom_4.6.3` — current mainline. All ongoing work goes here.
- `master_custom` — legacy 2022-era fork from upstream 4.3.0-dev.
  Frozen reference, do not develop on it.
- `legacy-pre-rebase-2026` — tag pointing at the last commit of the
  legacy mainline.

---

# Credits

- **Michel Pastor ([shellixyz](https://github.com/shellixyz))** —
  bulk of the per-feature work, both in the classic 2022
  `shellixyz/ardupilot` and the maintained successor at
  [github.com/ArduCustom/ardupilot](https://github.com/ArduCustom/ardupilot).
  This branch re-ports a curated subset of that work onto a modern
  4.6.3 base.
- **[@mf0o](https://github.com/mf0o)** — widened OSD sidebars, the
  DJI O3 home/waypoint arrow direction fix, and the `SpeedyBeeF405WING`
  target. All three landed in ArduCustom, made their way into upstream
  4.6.3, and are present in this fork by inheritance.
- **[@wvarty](https://github.com/wvarty)** — the CRSF `RSSIDBM` /
  `RC_SNR` / `RC_ANT` OSD elements and the RF-Mode-on-LQ element
  (originally ArduCustom PR #78), plus the "un-cap RF_Mode for CRSF
  protocol" change (PR #205). Same path — landed in ArduCustom, then
  upstream merged it, then we inherited it in 4.6.3.
- **Upstream ArduPilot dev team** — the underlying platform.
- Authoring assistance via Claude Code (Anthropic).
