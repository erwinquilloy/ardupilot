# ArduPlane Custom Build — **LIGHT VARIANT**

> 🪶 **This is the light variant of the fork** — a stripped-down build that
> fits more easily on 1 MB F4 boards.
> **⬇ Download:** grab the newest `light-v*` build from the
> [**Releases** page](https://github.com/erwinquilloy/ardupilot/releases)
> (every version, newest first — the latest is `light-v0.3-beta`).
> **Full variant** (H7 / 2 MB, every backend compiled in): see the
> [full variant `README_CUSTOM.md`](https://github.com/erwinquilloy/ardupilot/blob/master_custom_4.6.3/README_CUSTOM.md),
> or the `full-v*` builds on the same
> [Releases page](https://github.com/erwinquilloy/ardupilot/releases).

## Why flash this instead of stock ArduPlane 4.6.3?

Everything stock ArduPlane 4.6.3 does still works underneath — this fork
only **adds** on top (both variants carry the identical feature set; they
differ only in which hardware backends are compiled in). If you fly
long-range or FPV fixed-wing, these are the additions most worth flashing
for. Each group links to its full reference below.

### 🖥️ A much richer FPV / long-range OSD

The single biggest visible difference from stock — dozens of new elements
aimed at efficiency and situational awareness:

- **Efficiency in Wh/km or mAh/km**, **resting cell voltage** (sag-free
  pack health at a glance), and **battery used since arming**.
- **End-of-flight stats grid** (distance, time, peak values) plus a
  **persistent max-flight-time** record.
- **Angle of attack**, **demanded vs actual airspeed**, attitude-corrected
  **rangefinder AGL altitude**, longitudinal/lateral/vertical **G**, **peak
  roll/pitch rates**, **auto-flap %**, **input throttle %**, **live tuning
  param name + value**, and **loiter radius**.
- **Under/over-speed warning flashes**, one/two-decimal polish on speed,
  vertical speed and attitude, and a shortenable **plus code** home locator.

→ [OSD additions](#osd-additions)

### 🛟 Wind-aware autoland & a real failsafe glide-in

Protecting the airframe when the link drops or the battery runs marginal:

- **`LAND_WIND_BIAS` auto-picks the upwind `DO_LAND_START`**, with
  **`LAND_WIND_DIST`** capping how far the wind bias may reach and
  **`LAND_WIND_STRICT`** turning that cap into a hard fence — so a
  battery-marginal RTL doesn't fly away to a distant wind-aligned approach.
- **Emergency-landing state machine on RC failsafe**: instead of orbiting
  home until the battery dies, it sinks to a glide altitude, aligns **into
  wind**, glides down and disarms. Fully parameterised (`FS_ELAND_*`).
- **RTL Autoland Commit** (`RCx_OPTION 251`): hold over home until you flip
  a switch, then commit to the landing sequence (bypassed on failsafe).

→ [Emergency landing](#emergency-landing-rc-failsafe) ·
[RTL & failsafe behaviour](#rtl--failsafe-behaviour)

### 🚀 Hand-launch & arming ergonomics

Built for throwing a wing off your hand safely:

- **`TKOFF_THR_IDLE`** idles the prop once armed and the stick is raised —
  a clear "armed and ready" cue in the palm — with **`TKOFF_IDL_DELAY`** to
  pause before the ramp.
- **Pre-launch audio cues** and a **lost-vehicle alarm** on failsafe.
- **Arm-switch safety**: an in-flight disarm **cuts throttle and drops to
  FBWA** rather than killing the servos and dropping the plane out of the
  sky. **`ARMING_MODE_SW`** auto-switches to TAKEOFF/AUTO after arming.
- **`FLTMODE_EXT`** gives a full **12 flight-mode positions** on one channel.

→ [Arming & takeoff](#arming--takeoff) · [Flight modes](#flight-modes)

### 🎛️ Tune and take control in the air

- **Knob-tunable pitch trim** (`PTCH_TRIM_DEG`, plus `Q_TRIM_PITCH` on
  VTOL) and **three switchable tuning sets** (`TUNE_PARAM`/`PARAM2`/`PARAM3`)
  — retune live without a laptop.
- **AUTO → FBWA stick takeover**: nudge the sticks mid-mission to grab
  manual control instantly, hand back by re-centring.
- **Throttle stick sets target airspeed** in RTL/LOITER/CIRCLE/AUTO, plus
  **pilot loiter radius + direction** control in LOITER/RTL.
- Extra modes: **Course Hold** (heading hold) and **Auto Trim** as a mode.

→ [Pitch trim & tuning](#pitch-trim--tuning) ·
[Manual airspeed](#manual-airspeed-control-in-nav-modes) ·
[AUTO → FBWA takeover](#auto--fbwa-stick-takeover)

### 📡 Long-range extras

- **Peer-aircraft radar** (iNav Radar / FormationFlight) — see other
  aircraft on your OSD.
- **Richer telemetry**: CRSF relative altitude + link quality, MSP/DJI
  numeric-attitude fix, wind in km/h, vehicle UID.
- **Expanded battery monitoring**: Wh accounting, cell count/voltage, and
  capacity-remaining plus low/critical thresholds feeding the OSD readouts.

→ [Peer-aircraft radar](#peer-aircraft-radar-inav-radar--formationflight) ·
[Telemetry](#telemetry) ·
[Battery monitoring](#battery-monitoring-ap_battmonitor)

> Most additions are opt-in via parameters and change nothing until you
> enable them — but a few **defaults do differ** from stock. Skim
> [Behaviour changes vs upstream](#behaviour-changes-vs-upstream--migration-notes)
> before your first flight.

## What's stripped vs. the full variant

Aligned to the **strict 2022 light-variant definition** mf0o released,
with one deliberate addition (FPORT2, noted below):
- Only **1 GPS** and **1 magnetometer**
- **No CAN** anywhere (no DroneCAN GPS / RC / battery / airspeed / rangefinder)
- RC: **SBUS, CRSF, IBUS, FPORT, FPORT2** — CRSF/SBUS/IBUS/FPORT are the
  strict-2022 set; **FPORT2 is kept in addition** (mf0o's 2022 strip
  dropped it) because it shares the FPORT driver and is common on modern
  FrSky receivers
- GPS: **UBLOX** only
- Airspeed: **Analog** or **MS4525** pressure sensors only
- Rangefinder: **Benewake LIDARs** only

A per-board hwdef can re-enable any of the stripped items via the
matching `define AP_<X>_ENABLED 1` if a specific airframe needs it.

| Family | Kept (in light) | Stripped (not in light) |
|---|---|---|
| **GPS instances** | 1 | (no redundant / blended GPS) |
| **Magnetometers** | 1 | (no second compass averaging) |
| **CAN entirely** | — | all DroneCAN backends: GPS, RC, BattMon, Rangefinder, Airspeed; plus the **PiccoloCAN** ESC driver (~30 KB of generated protocol code) |
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

> ⚠️ **Upstream 1 MB-flash-gated features are OFF on the F4 fleet — this is
> not a light strip.** Separately from the strips above, ArduPilot compiles
> a set of features out on 1 MB boards via `BOARD_FLASH_SIZE > 1024`. This
> is **upstream behaviour** — identical on stock ArduPlane and on the full
> variant when run on the same 1 MB F4 (the H7 boards, being 2 MB, keep
> them). Notable ones absent on the F4 boards: **VisualOdom / external-nav,
> ADSB, EFI, GyroFFT, object-avoidance proximity, gimbal mounts
> (CADDX/Gremsy/Viewpro/Xacti), Lua scripting serial devices, INA2xx
> battery monitors, and EK3 drag / body-odom / external-nav fusion.**
>
> **Common migration gotcha:** a saved parameter with `EK3_SRCx_* =
> ExternalNav` triggers the prearm **`EK3 sources require VisualOdom`** —
> VisualOdom isn't compiled in on 1 MB F4. Set those sources off
> `ExternalNav`: use the `GPS`/`Baro`/`Compass` defaults, or `RangeFinder`
> (`EK3_SRC1_POSZ = 2`) if you want a downward lidar as the EKF height
> source. **A Benewake TFmini is a *rangefinder*, not external nav — it is
> fully supported on light** (all Benewake serial variants are kept); only
> the `ExternalNav` source value needs the (absent) VisualOdom backend.

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

### Firmware version string

Mavlink `AUTOPILOT_VERSION` (the banner that shows up in MissionPlanner
on connect, and in `arduplane --help`) is built up as:

```
ArduPlane V<x.y.z> ArduCustom <FORK_VERSION>[ Light] (<git-hash>)
```

For example:

```
ArduPlane V4.6.3 ArduCustom v0.3-beta (f345f9a)        # full branch
ArduPlane V4.6.3 ArduCustom v0.3-beta Light (f345f9a)  # light branch
```

The upstream `ArduPlane V4.6.3` prefix is kept so GCS tools that parse
the upstream version string (MissionPlanner, MAVProxy, autotest
scripts) keep working. `FORK_VERSION` lives in
`libraries/AP_Common/AP_FWVersionDefine.h` — bump it on each release.
The `Light` suffix only appears on the `master_custom_4.6.3_light`
branch (set in the same header).

## Supported boards

> ⚠️ **The full variant has outgrown 1 MB F4 flash.** As of v0.2-beta (and still in v0.3-beta) the
> full build overflows the ~1 MB flash on `MatekF405-Wing` (by a few KB) and
> similar 1 MB F4 boards — the accumulated fork feature set no longer fits
> alongside *every* upstream hardware backend. **Flash those boards from the
> [`light`](../../tree/master_custom_4.6.3_light) variant instead**, which
> strips the rarely-used backends to make room. The board-specific fixes
> listed below (LED-pad relay, U3 DMA, F4 mount re-enable) still apply — the
> light variant shares the same hwdefs. H7 / F7 / larger-flash F4 boards run
> the full variant fine. (This is why the full branch's `test chibios` CI
> shows MatekF405-Wing red: it is the expected overflow, not a regression.)

Fork-specific board work covers:

- **MatekF405-Wing** — LED-pad-as-relay output, U3 DMA fix, bootloader
  serial wiring
- **revo-mini-sd** *(custom target)*
- **SkystarsF405DJI** *(custom target)*
- **qUark mini wing v4** *(custom target)*
- **NeutronRC_H7_BT** *(custom target, H743, ported from ArduCustom)*
- **OMNIBUSF7V2** — quadplane disabled to fit the firmware
- **F4 mount support re-enabled** on `speedybeef4v3`, `MatekF405-TE`,
  `MatekF405-Wing`, `LongBowF405WING`, `SpeedyBeeF405WING`. Upstream's
  `minimize_common.inc` zeros `HAL_MOUNT_ENABLED` on flash-constrained
  boards, which breaks gimbal users on F4 (MissionPlanner reports
  "Invalid channel option" for `RCx_OPTION = 212/213/214` — the
  MOUNT1_ROLL/PITCH/YAW input labels). Each F4 hwdef now does an
  `undef`/`define` after the `minimize_*` include to turn mount back
  on with **servo + CADDX** backends enabled (other backends stay off
  to preserve flash budget). LongBow's existing upstream attempt used
  `#undef`/`#define` which is a hwdef comment, not a directive —
  fixed by dropping the `#`.
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

> 📡 **Yaapu telemetry users:** stock Yaapu's plane-mode table stops
> at mode 25 (Thermal) so Course Hold shows as a blank chip + no
> audio cue. Drop-in `plane.lua` and `coursehold.wav` are in
> [`Tools/yaapu-coursehold/`](Tools/yaapu-coursehold/README.md) —
> see that directory's README for install paths on both the radio
> SD card and the Yaapu GCS desktop tool.

### Auto Trim (mode 27)
Flies like Course Hold (locked ground-track heading, pilot trims
roll/pitch by stick) and simultaneously runs the **servo auto-trim
loop** in the background. Use it to push small trim into the saved
servo trims while still actively flying — no need to land between
adjustments. Exiting the mode stops the loop.

- **Enable:** set any `FLTMODE_n = 27` (or use FLTMODE_EXT slots 7..12)
- The same `SERVO_AUTO_TRIM` / `SAT_FINISHTHRESH` parameters that
  control the always-on auto-trim apply here too.
- The mode shares Course Hold's `CRUISE_YAW_RATE` + `FLIGHT_OPTIONS`
  bit 20 (yaw-stick steers locked heading) behaviour because it
  inherits from `ModeCourseHold`.
- Same post-4.6.3 mode-number caveat as Course Hold (upstream may
  give a meaning to 27 in a later release).

### RC aux switch entries (Plane)
Plane's `RCx_OPTION` enum gains two entries so the pilot can put
autotrim and autotune on momentary aux switches without burning a
FLTMODE_n slot:

- **`RCx_OPTION = 17` — AUTOTUNE Mode.** Switches into Plane's
  AUTOTUNE flight mode (mode 8) on switch HIGH. Upstream had this
  enum value wired only for Copter; the fork extends it to Plane.
  This is *flight-mode entry*, distinct from `RCx_OPTION = 107`
  (`FW_AUTOTUNE`), which toggles the autotune loop without changing
  flight mode.
- **`RCx_OPTION = 200` — Servos Auto Trim.** Starts the servo
  auto-trim accumulator on switch HIGH (with a clean prepare so it
  doesn't resume from a previous run) and stops it on LOW. Stays in
  whatever flight mode you're already in (FBWA / CRUISE / etc.) — it
  does **not** force AUTO_TRIM mode. GCS prints "Servos auto trim
  started" / "stopped" as it toggles. Slot 200 matches mf0o's port,
  so saved `RCx_OPTION = 200` params from mf0o-based builds keep
  working. (The legacy 2022 fork used slot 162 for the same option;
  re-save the param if you're migrating from the old fork.)

### `FLTMODE_EXT` — 12-position FLTMODE_CH
Default `0` keeps the upstream 6-position behaviour bit-for-bit. Set
`1` and ArduPlane reads the FLTMODE_CH PWM as a 12-bin selector
instead of 6 — useful when your radio's source for `FLTMODE_CH` is
an OpenTX/EdgeTX multi-position switch with more than 6 positions.

- Bins are 75 µs wide starting at 1126 µs (`1126/1201/.../1876`).
- Positions 1..6 use the existing `FLTMODE1..6` params (no change).
- Positions 7..12 use new params **`FLTMODE7..FLTMODE12`** (default
  FBWA each).
- Stable digital RC links recommended (SBUS / CRSF / ELRS). PPM /
  analogue PWM is **not** recommended — 75 µs bins are too tight
  for jitter typical of analogue links.

Originally Stavros Korokithakis 2021 (`e8ff5a8dfd`), forward-ported
through ArduCustom; this re-port adapts to upstream 4.6.3's
RC_Channel-based mode-switch refactor.

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
circle around the target waypoint. The pitch (elevator) stick now
adjusts altitude during this loiter phase the same way FBW-B does it —
pull back to climb, push forward to descend — so the pilot can settle
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

> **REQUIRED: `PTCH_TRIM_DEG` must be non-zero before you engage the knob.**
> AP_Tuning's knob math is purely multiplicative around the parameter's
> current value — the knob sweeps `[seed / TUNE_RANGE, seed * TUNE_RANGE]`.
> If the seed is exactly `0`, every multiplier is still `0`, so the knob
> appears dead and the value will not change no matter where you turn it.
>
> Workflow:
> 1. Set `PTCH_TRIM_DEG` to a small non-zero value (e.g. `0.5`, or
>    `-0.5` to explore nose-down) via the GCS *before* engaging the knob.
> 2. Re-centre the knob (selector switch low, <500 ms hold) so the new
>    seed becomes the mid-point.
> 3. To cross zero or flip sign, re-seed manually and re-centre — the
>    multiplicative scheme cannot cross zero on its own.
>
> The same caveat applies to `Q_TRIM_PITCH` via `TUNE_PARAM = 92`.

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

### `LAND_WIND_BIAS` — pick the upwind `DO_LAND_START`
Upstream picks the **nearest** `DO_LAND_START` whenever ArduPlane
needs to autoland — RTL autoland, fence breach, battery failsafe, or
a `MAV_CMD_DO_LAND_START` from the GCS. In wind, the nearest
landing is frequently the *downwind* one.

`LAND_WIND_BIAS` (default `0` = upstream behaviour preserved)
shifts the selection toward the landing whose final approach faces
into the wind. Each candidate is scored as

```
score = distance * (1 - LAND_WIND_BIAS * cos(approach_heading - wind_from))
```

where `cos` returns `+1` for a perfect headwind approach (no
penalty) and `-1` for a perfect tailwind approach (multiplier
`1 + LAND_WIND_BIAS`). The lowest-scoring candidate wins.
`LAND_WIND_BIAS = 1` lets a direct-tailwind 500 m approach lose to a
direct-headwind 1000 m approach; `0.3` is a gentler tilt that still
breaks ties in favour of upwind landings.

The approach heading for each `DO_LAND_START` is derived from the
bearing toward the next `NAV_*` waypoint in the mission — typically
the first leg of the descending approach. If no following nav cmd
can be found, that candidate is scored on distance alone.

Falls back to nearest-distance (the upstream behaviour) when:
- `LAND_WIND_BIAS = 0` (any value `<= 0`)
- AHRS wind estimate magnitude is below 1 m/s (unreliable on a
  pre-takeoff plane, or a GPS-only build before the plane has
  turned enough for the estimator to settle)
- A candidate's approach heading cannot be derived

When wind-bias changes the choice, a `DO_LAND_START N (upwind,
nearest was M)` GCS line is sent next to the standard `Landing
sequence start` text, so you can see the bias actually fired in
flight logs.

> **Note on airframes without an airspeed sensor (any variant):**
> ArduPlane's wind estimator needs the plane to turn to converge
> when there is no pitot. Straight-line flight just after takeoff
> may not produce a usable estimate, so the fallback keeps
> selection nearest-distance until a turn or two has populated
> the estimator. Airframes with a working pitot get a usable wind
> estimate much sooner. The light variant is no different from the
> full variant here — the limitation is in the AHRS estimator, not
> the build.

### `LAND_WIND_DIST` — why: cap how far the wind bias can reach
**The problem it solves:** on missions with several `DO_LAND_START`
items spread across a wide area, a strong `LAND_WIND_BIAS` can pull
selection toward a much farther landing just because its approach
happens to align with the wind. On a battery-marginal RTL that
extra travel can cost you the airframe. `LAND_WIND_DIST` gives you
a distance ceiling: any `DO_LAND_START` whose start-of-approach
sits farther than this many metres from the plane's decision-time
position is **excluded** from selection outright. The wind bias
then picks only among the candidates that actually survive the cap.

**What "decision time" means depends on `RTL_AUTOLAND`:**

| `RTL_AUTOLAND` | When the pick runs | Cap effectively measured from |
|---|---|---|
| `1` (return then autoland) | after the plane reaches the RTL loiter point | HOME (plane is there) |
| `2` (autoland immediately) | as soon as RTL is entered | plane's current position |
| Battery failsafe | when the failsafe fires | plane's current position |
| GCS `MAV_CMD_DO_LAND_START` | when the command arrives | plane's current position |

So for `RTL_AUTOLAND=2` and mid-mission failsafes the cap prevents
the bias from pulling the plane *away* from where it already is;
for `RTL_AUTOLAND=1` it behaves as a home-radius fence, which is
what you probably had in mind if you set it up on the bench.

**Behaviour cases (`LAND_WIND_DIST = 500`):**
- Only one `DO_LAND_START` within 500 m of the plane → it is
  picked, tailwind or not.
- Multiple within 500 m → `LAND_WIND_BIAS` decides between them
  (into-wind approach preferred).
- **Zero** within 500 m → what happens is controlled by
  `LAND_WIND_STRICT` (below). Default (`= 0`) falls back to
  upstream nearest-across-all; strict (`= 1`) refuses to autoland.

`LAND_WIND_BIAS = 0` still short-circuits the whole feature, so
`LAND_WIND_DIST` has no effect on its own.

### `LAND_WIND_STRICT` — why: make the cap a *hard* fence
**The problem it solves:** by default, if `LAND_WIND_DIST` filters
out every candidate, the code falls through to upstream nearest-
across-all so the plane still lands somewhere — even if that
"somewhere" is far outside the cap you configured. That preserves
the failsafe safety net (the plane will always try to land) but
partly defeats the point of the cap. `LAND_WIND_STRICT` lets you
opt into a stricter interpretation: **when no `DO_LAND_START` is
within the cap, refuse to autoland at all**. RTL stays in RTL, the
plane keeps loitering at HOME/rally, and the pilot has to
intervene.

**Values:**

| `LAND_WIND_STRICT` | Behaviour when zero candidates in cap |
|---|---|
| `0` (default, soft fallback) | Fall through to upstream nearest-across-all; the plane still lands, even if the pick is far outside `LAND_WIND_DIST`. Wind bias is skipped for that one pick. |
| `1` (hard fence) | Refuse the autoland. Return `false` from the picker; RTL stays in RTL, plane loiters at HOME/rally. GCS text: `No DO_LAND_START within LAND_WIND_DIST`. Pilot must intervene. |

**When to use each:**
- **`= 0`** — you want `LAND_WIND_DIST` to gently guide selection
  toward nearby landings, but you'd rather the plane always
  attempt a landing on failsafe (avoid "loiter until battery
  dies"). This is the safer default for most operations.
- **`= 1`** — you have several DO_LAND_STARTs spread across a
  large area and would rather fly the plane home manually than
  have it commit to an unexpected 5 km cross-country to satisfy
  autoland. Common on missions where the "far" landings are
  spare alternates you don't actually want the autopilot to
  choose without you.

**⚠️ Safety warning:** under `LAND_WIND_STRICT = 1`, a **battery
failsafe** that triggers when no landing is within the cap will
**not attempt to land** — the plane will loiter at HOME/rally
until the battery dies and comes down uncontrolled. Only enable
strict mode if you are able to monitor the aircraft in flight and
take manual control on failsafe. If you cannot guarantee that,
leave `LAND_WIND_STRICT = 0` and accept the occasional far
landing.

Strict mode is silently ignored when `LAND_WIND_DIST = 0` (no
cap set) or `LAND_WIND_BIAS = 0` (whole feature off).

## Pilot control of loiter radius and direction (LOITER, RTL)

Ported from ArduCustom PR #180 (shellixyz `dd65f3275f`; carried
across the 4.5 → 4.6 API rename from
`fbwa_throttle_to_pitch_compensation` /
`pitch_limit_max_cd` to `adjust_nav_pitch_throttle` /
`pitch_limit_max`).

**In LOITER (mode 12) and RTL (mode 11)**, once the plane is
established in the loiter circle:

- **Rudder-stick more than half deflection to the left** — snap
  loiter direction to **counter-clockwise** (`loiter.direction = -1`).
- **Rudder-stick more than half deflection to the right** — snap
  loiter direction to **clockwise** (`loiter.direction = 1`).
- **Roll-stick** integrates the loiter radius at 20 m/s scaled by
  direction, so pushing "outward" always grows the radius regardless
  of which way the plane is orbiting. Radius is clamped to `[20, 1000]` m.

On mode entry, radius and direction are seeded from `WP_LOITER_RAD`
(LOITER) or `RTL_RADIUS` with a `WP_LOITER_RAD` fallback (RTL).
The sign of these params still determines the starting direction
(negative = CCW).

**Bails during RC failsafe.** The pilot-control helper does not
run when `plane.failsafe.rc_failsafe` is true, so the emergency-
landing state machine's `FS_ELAND_LOTRAD` override and the
GLIDING/GLIDING_NO_RETURN wings-level enforcement still own the
loiter geometry during the sink.

No parameter to enable — behaviour is on whenever the plane is
in LOITER or RTL and not in RC failsafe.

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

## Serial port numbering matched to chip UART numbers

On the boards whose stock ArduPilot `SERIAL_ORDER` was scrambled, `SERIALn`
now equals the silkscreen `UARTn` pad again — restoring the mapping that
shipped in ArduCustom `custom-v11.3` and earlier. The pad marked `TX2/RX2`
is `SERIAL2` (not `SERIAL6`), `TX3/RX3` is `SERIAL3`, and so on, removing the
mismatch that confused users (e.g. an ELRS receiver wired to the `RX2/TX2`
pads showing up on `SERIAL6`).

Affected boards: `MatekF405-TE(-bdshot)`, `MatekF405-STD`, `omnibusf4pro`,
`MatekH743(-bdshot)`. `omnibusf4pro` has no USART2 / UART5, so `SERIAL2` /
`SERIAL5` are `EMPTY` placeholders that keep `USART3=SERIAL3` and
`USART6=SERIAL6` aligned. `KakuteH7-Wing` is intentionally left on upstream
numbering (it was never remapped in v11.3, and its connectors are labelled
functionally rather than by chip UART number).

Like v11.3 this is a `SERIAL_ORDER`-only change (no protocol defaults added),
so default protocols fall on the stock per-index defaults (S1/S2 MAVLink,
S3/S4 GPS) — set `SERIALx_PROTOCOL` to match your wiring.

> ⚠️ **Upgrade note:** moving from stock-upstream serial numbering (any prior
> v0.2-beta build) on the four affected boards means saved `SERIALx_PROTOCOL`
> / `_BAUD` / `_OPTIONS` values now point at different physical UARTs.
> Re-check your serial config after flashing — GPS, telemetry and RC
> (CRSF/ELRS) may need reassigning to the new `SERIALn`.

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
| 23  | `OPTION_SHORTEN_PLUSCODE`                  | Strip leading digits from the plus code panel when near home |

> **Plus code (OLC):** the `OSDx_PLUSCODE_*` Open Location Code panel is
> compiled in on **both variants** from v1.0 — the light variant forces it on
> for every board including the 1 MB F4s (~2–3 KB flash), and the full variant
> uses the upstream `BOARD_FLASH_SIZE > 1024` gate (its boards are H7).
> `OSD_OPTIONS` bit 23 shortens the code when the aircraft is near home.

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
  from upstream 4.6.3 (port by [@mf0o](https://github.com/mf0o)).
  Upstream replaced ArduCustom's binary `OSD_OPTIONS` bit 17 toggle
  with two continuous-offset top-level params that are strictly
  more flexible: **`OSD_SB_H_OFS`** (horizontal offset of the
  altitude column, default 0; use `14` to match ArduCustom's
  wide-sidebar look — `16 + 14 = 30`) and **`OSD_SB_V_EXT`**
  (extra vertical bar height each side of centre, default 0).
  These are **`OSD_*`** (single global value), not per-screen
  `OSDx_*`. This is why we don't backport the bit-17 toggle.
- **DJI O3 home / waypoint arrow direction fix** — the BF MSP
  DisplayPort symbol mapping for the home and waypoint direction
  arrows had the wrong indices on DJI O3 goggles, making the arrows
  point in confusing directions. Inherited from upstream 4.6.3
  (port by [@mf0o](https://github.com/mf0o))
- **DJI OSD battery-bar data source — `OSD_BATBAR_TYPE`** —
  default `0` keeps the goggle's battery bar driven by mAh used
  vs `BATT_CAPACITY` (stock behaviour). Set `1` to drive the bar
  by Wh used vs `BATT_CAPACITY_WH` instead. The DJI Air Unit only
  consumes mAh + capacity_mah in `MSP_BATTERY_STATE`, so when set
  to Wh we rescale the reported capacity to make the resulting
  percentage equal `consumed_wh / capacity_wh`. Useful on packs
  whose Wh capacity is configured but whose mAh rating is unknown
  or unreliable. Originally
  [ArduCustom @shellixyz + @stavros](https://github.com/ArduCustom/ardupilot)
  via [@mf0o](https://github.com/mf0o)'s `osd/dji_batt_bar_source_option`
  branch (commit `93c530cdb2`)

## Setting up cell voltage on the OSD

Two related elements display cell voltage. Both are upstream features,
but the setup is non-obvious because cell count has to come from
somewhere.

| Element | Shows |
|---|---|
| `OSDx_AVGCELLV` | Avg cell voltage **under load** (`pack_voltage / cell_count`) |
| `OSDx_ACRVOLT` | Avg cell **resting** voltage — sag-corrected; closer to true state-of-charge in flight |

`OSDx_ACRVOLT` is generally the more useful in-flight reading because
it back-calculates what the cell voltage would be with no load — a
current spike won't make the bar dive.

### Steps

1. **Battery monitor must be configured.** Confirm `BATT_MONITOR` is
   non-zero (`4` = analog volt+amp on most Matek/SpeedyBee boards) and
   that pack voltage shows in your GCS. If not, set up `BATT_MONITOR` /
   `BATT_VOLT_PIN` / `BATT_VOLT_MULT` for your hardware first.

2. **Tell the OSD how many cells to divide by:**

   ```
   OSD_CELL_COUNT = -1   disables both avgcellv and acrvolt elements
   OSD_CELL_COUNT = 0    autodetect at battery connection (assumes well-
                         charged LiPo/LiIon — power up with a full pack)
   OSD_CELL_COUNT = N    manual override (e.g. 4 for a 4S pack)
   ```

   Autodetect (`0`) is the easiest if your packs are always the same
   chemistry / fully charged on connection.

3. **Enable the element on each screen you want it on**, with a
   position (column `X`, row `Y`). For screen 1, e.g. column 24 row 4:

   ```
   OSD1_AVGCELLV_EN = 1
   OSD1_AVGCELLV_X  = 24
   OSD1_AVGCELLV_Y  = 4
   ```

   Substitute `OSD2_..OSD5_` for additional screens. Same pattern for
   `OSDx_ACRVOLT_EN/X/Y` if you want the resting-voltage variant.

4. **Optional low-voltage flash thresholds**, both are top-level
   `OSD_*` params (single global value, not per-screen):

   ```
   OSD_W_AVGCELLV = 3.6   AVGCELLV flashes below this cell voltage
   OSD_W_ACRVOLT  = 3.6   ACRVOLT flashes below this cell voltage
   ```

   Default `3.6 V` is sensible for LiPo. Lower for LiIon (e.g. `3.3`).

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

# Peer-aircraft radar (iNav Radar / FormationFlight)

Forward-ported from [MUSTARDTIGERFPV/ArduPilot @ `35f5f0ef4f`](https://github.com/MUSTARDTIGERFPV/ArduPilot/commit/35f5f0ef4f1a3e64f1be9a05e9eda3ce5b40e2dd)
(March 2023). Lets ArduPlane consume peer-aircraft position frames
from an ESP32 running [ESP32-INAV-Radar](https://github.com/OlivierC-FR/ESP32-INAV-Radar)
or its active successor [FormationFlight](https://github.com/FormationFlight/FormationFlight)
and display peers as a small symbol on the OSD. Peer-to-peer radio
(LoRa SX12xx, E22 sub-GHz, or ESP-NOW) is handled entirely by the
ESP32 — the flight controller just sees an MSP serial stream.

**Wire protocol:** `MSP2_SET_RADAR_POS` (`0x100B`) — identical frame
layout to iNav's, so any existing ESP32-INAV-Radar / FormationFlight
hardware works without reflashing.

## Setup

Hardware: connect the ESP32 module's MSP TX/RX/GND to a free UART on
the flight controller.

Parameters:

| Param | Value | Notes |
|---|---|---|
| `RADAR_TYPE` | `1` | `1`=MSP, `0`=disabled (defaults to MSP) |
| `SERIALn_PROTOCOL` | `33` | MSP — the same protocol used by DJI/Walksnail OSD telemetry; can share that port |
| `SERIALn_BAUD` | `115` | 115200 |
| `OSDx_RADAR_EN` | `1` | Per-screen enable for the radar OSD element |
| `OSDx_RADAR_X` / `_Y` | grid coords | Element placement on screen `x` (1..5) |

If the FC is already running MSP DisplayPort for goggle OSD on one
UART, the radar can share that same UART — MSP demultiplexes by
message ID.

## OSD element

Cycles through up to 6 healthy peers (one every 2 s) and renders:

```
A→
 1.2km ↑15m
```

- **A..F** — peer slot letter (the ESP32 assigns slot 0..5; we display 'A'+slot).
- **arrow** — bearing to the peer, body-frame, drawn from the
  existing `get_arrow_font_index()` direction-arrow helper.
- **distance** — horizontal distance to the peer in the OSD's
  configured units (m/km, ft/mi, or NM).
- **vertical distance** — relative altitude with up/down arrow
  (`↑`/`↑↑` if the peer is above; `↓`/`↓↓` if below — heavy arrow
  past 25 m).
- A peer goes "stale" (and the element blanks) after `RADAR_PEER_FRESH_TIME_MS`
  = 3 s without a fresh frame.

## Build gating

```
AP_RADAR_ENABLED        = HAL_MSP_ENABLED       (default)
HAL_MSP_RADAR_ENABLED   = AP_RADAR_ENABLED && HAL_MSP_ENABLED
```

i.e. radar follows MSP. Override in a hwdef with `define AP_RADAR_ENABLED 0`
to force-strip it on a flash-constrained board.

## Full release fleet (`full-v1.0`)

The fork ships two parallel per-board release tracks — `full-v1.0`
(this branch) and `light-v1.0` (the light branch). Both carry the
**identical fork feature set**; they differ only in which hardware
backends are compiled in. A board's version string tells you which you
flashed: the light build appends ` Light`
(`... ArduCustom v1.0 Light (…)`), the full build has no suffix.

The **full** fleet is built from `master_custom_4.6.3` with every upstream
backend plus the fork additions, scoped to boards with the flash headroom
to carry all of it — H7 / F7 / 2 MB targets. (1 MB F4 boards overflow the
full build; flash those from the light release instead.)

| Board | MCU / Flash | Status |
|---|---|---|
| MatekF765-Wing | F7 / 2 MB | Ships — Matek F765-WING (most flash headroom in the fleet, 521 KB free) |
| MatekH743-bdshot | H7 / 2 MB | Ships — Matek H743-WING (bidirectional DShot) |
| KakuteH7-Wing | H7 / 2 MB | Ships — Holybro Kakute H743 Wing |

Both keep hundreds of KB free with the full backend set (exact figures in
the release notes). `KakuteH7-Wing`, an H7 with generous flash, is carried
on **both** tracks — it belongs here where the extra headroom is an asset,
and is also offered on the light release for convenience.

## Light release fleet (`light-v1.0`) — 10 boards (8 × 1 MB F4 + 1 × H7 + 1 × F7)

Measured against the **light variant** at v1.0 with radar +
LAND_WIND_DIST + LAND_WIND_STRICT on:

| Board | Flash | Free after fork features | Status |
|---|---:|---:|---|
| MatekF765-Wing | 2 MB | **613.2 KB** | Ships — Matek F765-WING (F7; also on full) |
| KakuteH7-Wing | 2 MB | **281.9 KB** | Ships — Holybro Kakute H743 Wing (H7; also on full) |
| KakuteF4-Wing-Buzz | 1 MB | **80.2 KB** | Ships — Kakute F4 Wing + tone buzzer (see below) |
| speedybeef4v3 | 1 MB | **52.0 KB** | Ships — most F4 headroom |
| MatekF405-STD | 1 MB | **45.6 KB** | Ships |
| MatekF405-TE-bdshot | 1 MB | **41.2 KB** | Ships |
| omnibusf4pro | 1 MB | **40.4 KB** | Ships — Omnibus F4 Pro FPV FC |
| SpeedyBeeF405WING | 1 MB | **28.7 KB** | Ships |
| SpeedyBeeF405WING-Buzz | 1 MB | **28.4 KB** | Ships — SpeedyBee F405 WING + tone buzzer (see below) |
| MatekF405-Wing-bdshot | 1 MB | **26.3 KB** | Ships — tightest of the F4 fleet |

`MatekF405-Wing-bdshot` is now the canary board — first to bump the
1 MB ceiling in any future upstream rebase or feature add. Worth
watching going forward. (`KakuteH7-Wing` and `MatekF765-Wing`, 2 MB H7/F7
targets, are not flash-constrained here — they are carried on both tracks
for convenience.)

### Tone-buzzer board variant: `SpeedyBeeF405WING-Buzz`

A fork-only variant of `SpeedyBeeF405WING` that adds a hardware
**ToneAlarm** (pitched-tone) buzzer on the **S11** pad (PB15 remapped
from the TIM1 PWM group to `TIM12_CH2`, marked `ALARM`). This lets the
FC play the full ArduPilot tunes — arming, GPS lock, the auto-takeoff/
launch cues above, failsafe — on a dedicated passive buzzer instead of
routing them through the DShot ESCs. Cost is one servo output (S11 /
channel 11); outputs 1-10 and 12 (WS2812 LED) are unchanged. Board ID
is inherited (1106) so the `.apj` flashes over the stock SpeedyBee
bootloader as a normal update. Flash footprint is essentially identical
to stock `SpeedyBeeF405WING` (~29 KB free on light). Shipped as an extra
asset on the light release alongside the other boards above.

A sibling variant, **`KakuteF4-Wing-Buzz`**, applies the same idea to the
Holybro **Kakute F4 Wing** — which has *no* buzzer at all on the stock
board. The tone buzzer goes on the WS2812 **LED** pad (`PA1`, remapped from
`TIM5_CH2` to the unused `TIM2_CH2`, marked `ALARM`); it costs the
addressable-LED-strip output, **no** servo/motor output, and keeps the
onboard status LED. Board ID is inherited (`Holybro-KakuteF4-Wing`).
For `KakuteF4-Wing-Buzz`, wire a *passive* buzzer to the LED pad — see the
board's `Readme.md`.

The comparable stock-upstream H7 board **`MatekH743-bdshot`** (Matek
**H743-WING**, bidirectional DShot) is *not* a fork `*-Buzz` variant: it
already carries an onboard buzzer (GPIO single-tone on `PA15`, since
bidirectional DShot claims the `TIM2` timer the non-bdshot base uses for a
multi-tone `ALARM`), so no external buzzer wiring is needed. It shipped as a
light asset through v0.2-beta; **as of v0.3-beta it ships on the full track
only** (`full-v0.3-beta`) — the light H7 slot is now filled by
`KakuteH7-Wing`.

A sibling variant, **`KakuteF4-Wing-Buzz`**, applies the same idea to the
Holybro **Kakute F4 Wing** — which has *no* buzzer at all on the stock
board. The tone buzzer goes on the WS2812 **LED** pad (`PA1`, remapped from
`TIM5_CH2` to the unused `TIM2_CH2`, marked `ALARM`); it costs the
addressable-LED-strip output, **no** servo/motor output, and keeps the
onboard status LED. Board ID is inherited (`Holybro-KakuteF4-Wing`).
For `KakuteF4-Wing-Buzz`, wire a *passive* buzzer to the LED pad — see the
board's `Readme.md`.

Alongside it, the **`MatekH743-bdshot`** (Matek **H743-WING**, bidirectional
DShot) is also added as a **light-v0.2-beta** asset — but it is a *stock
upstream* board, **not** a fork `*-Buzz` variant: it already carries an
onboard buzzer (GPIO single-tone on `PA15`, since bidirectional DShot claims
the `TIM2` timer the non-bdshot base uses for a multi-tone `ALARM`), so no
external buzzer wiring is needed.

#### Hardware requirements

- **A passive buzzer** — a bare piezo or a magnetic *passive* buzzer.
  Do **not** use an active buzzer: it has a fixed internal oscillator
  and can only switch on/off, so it cannot reproduce the pitched tunes.
- **Recommended:** an LS3040 passive piezo (~4 kHz resonance, rated
  2 mA, 1–30 V). Its draw is well under the MCU pin limit, so it wires
  straight to the pad — no transistor, no diode (a piezo is capacitive,
  not inductive).
- **For a louder or magnetic buzzer:** a small NPN transistor
  (2N2222 / S8050 / MMBT2222) or a logic-level N-MOSFET, plus a 1 kΩ
  base resistor (~100 Ω for a MOSFET gate), a 10 kΩ base/gate pull-down
  to GND, and — for a magnetic (coil) buzzer only — a 1N4148 flyback
  diode.
- **Signal pad:** S11 (**PB15**). Power the buzzer high side / transistor
  stage from the **5 V servo rail** (confirm the BEC feeds it) and GND.
  This costs servo output 11; outputs 1–10 and 12 (WS2812 LED) remain.

#### Wiring

Direct-drive passive piezo (recommended — e.g. LS3040):

```
S11 (PB15) ------[ LS3040 piezo ]------ GND
```

Polarity does not matter; a bare piezo needs no transistor and no diode.
Driven off the 3.3 V pin swing it is a little quieter than its rated
80 dB but normally plenty audible.

Magnetic passive buzzer, or a louder piezo (transistor-driven):

```
5V (servo rail) ---+----------------+
                   |                |
              [ buzzer ]        1N4148  (band/cathode -> 5V)
                   |                |
                   +----------------+
                   |
               collector
S11 (PB15) -[1k]- base    NPN (2N2222 / S8050)
               emitter
                   |
         [10k] ----+   (base->GND pull-down: keeps it silent at boot)
                   |
                  GND
```

Omit the 1N4148 for a bare piezo (no coil to snub). The board leaves the
stock active buzzer on PC15 in place — ignore it, or remove
`HAL_BUZZER_PIN` from the hwdef if you don't want two noise-makers. Full
notes and wiring photos live in the board's `Readme.md`.

**v0.2-beta fleet change vs v0.1-beta:**
- **Added:** `MatekF405-STD` — a standard (non-bdshot) alternative
  in the F4 lineup, comfortable ~47 KB free headroom.
- **Dropped:** `LongBowF405WING` (was unmeasured in v0.1-beta and
  no bench-verified data yet) and `KakuteH7-Wing` (2 MB H7 board,
  outside the "tight 1 MB F4" focus of the light variant).

**Post-v0.2-beta fleet addition:**
- **Added:** `omnibusf4pro` (Omnibus F4 Pro) — user-requested. A stock
  upstream 1 MB F405 FPV flight controller (MPU6000/BMI270 IMU, BMP280
  baro, onboard OSD), so no fork hwdef work was needed — it already pulls
  in `minimize_fpv_osd.inc`. Measured **41.6 KB free** on the light variant
  (940,436 B of 1 MB used), right in the middle of the F4 fleet's headroom
  band. Shipped as a light-release asset alongside the boards above.

**v0.3-beta fleet change vs v0.2-beta:**
- **Serial numbering** restored to `SERIALn == UARTn` on `MatekF405-TE-bdshot`,
  `MatekF405-STD`, and `omnibusf4pro` (see the serial-port section above).
- **Re-added:** `KakuteH7-Wing` — the 2 MB H7 board dropped from the light
  fleet at v0.2-beta is back as a light asset (**105.3 KB free**), so it can
  be pulled from either track; it has ample flash for both variants.
- **Now full-only:** `MatekH743-bdshot` — the other H7 board no longer ships
  as a light asset; it lives on `full-v0.3-beta`.

**v1.0 fleet change vs v0.3-beta:**
- **Added:** `MatekF765-Wing` (Matek **F765-WING**, F7 / 2 MB) — the first
  F7 board in either fleet, shipped on **both** tracks (`full-v1.0` and
  `light-v1.0`). Stock upstream board, no fork hwdef work needed. It has the
  most headroom of any board here: **521.0 KB free** on full, **613.2 KB
  free** on light.
- Plus code (OLC) is now compiled into **every** light board, including the
  1 MB F4s (it was already on full and on the H7 light build). The ~2–3 KB
  cost trimmed each F4's free-flash figure above by roughly 1 KB versus
  v0.3-beta; every board still clears its ceiling (tightest is
  `MatekF405-Wing-bdshot` at 26.3 KB free).

If a 1 MB F4 board overflows, the cheapest first trim is
`define HAL_SOARING_ENABLED 0` in that board's hwdef (Plane soaring
is on by default and unused on twin-motor FPV wings; buys ~8–12 KB).

**Bench-tested end-to-end** on a SpeedyBee F405 WING with a
FormationFlight ESP32 module — peer detection works in the
FormationFlight web UI, OSD element draws correctly once both planes
have GPS lock (the element stays blank without lock by design — see
"Limitations" below).

## Limitations / scope

- **GPS lock required on both planes for the element to draw.** Your
  own plane needs `ahrs.get_location()` to compute bearing/distance,
  and the peer plane needs a fix to broadcast non-zero coords (a peer
  with `lat=lon=alt=0` is treated as unhealthy). With no fix anywhere
  the OSD shows only a blinking letter ("A"); element switches to the
  full `B→ 1.2km ↑15m` form within ~2 s of both planes having lock.
- **One backend.** Only `MSP` is implemented — no MAVLink, no
  proprietary radio backends. The mavlink message handler is a stub
  inherited from MustardTiger's design and currently does nothing.
- **No avoidance integration.** Peers are *displayed*, not fed into
  `AP_Avoidance` — formation flight intentionally violates avoidance
  geometry, and ADS-B-style fan-out would have cost more flash than
  the whole feature.
- **Vertical arrow thresholds are fixed** (25 m) — no parameter to
  tune them yet. Easy to add later if anyone asks.
- **No iNav-style "information-to-display" upstream channel.**
  `MSP2_SET_RADAR_ITD` (`0x100C`) is reserved in the protocol header
  but not yet parsed.
- **No GCS "peer detected / lost" announcements** in this release.
  t413's upstream PR #32333 adds `STATUSTEXT` events for peer state
  changes; we may cherry-pick that in a follow-up.

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

> **REQUIRED: `Q_TRIM_PITCH` must be non-zero before you engage the knob.**
> Same multiplicative-knob caveat as `PTCH_TRIM_DEG` — a `0` seed means
> the knob has no effect. Set a small non-zero value first (e.g. `0.5`
> or `-0.5`), then re-centre with the selector switch.

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

- `master_custom_4.6.3` — full-variant mainline (this branch). All
  ongoing work goes here; the light branch follows via periodic merge.
- `master_custom_4.6.3_light` — light variant. Same fork feature set,
  with rarely-used hardware backends stripped to fit 1 MB F4 flash.
  Merged forward from `master_custom_4.6.3` after each change.
- `master_custom` — legacy 2022-era fork from upstream 4.3.0-dev.
  Frozen reference, do not develop on it.
- `legacy-pre-rebase-2026` — tag pointing at the last commit of the
  legacy mainline.

**Releases:** two parallel per-board tracks — `full-v0.3-beta` (H7 / 2 MB
boards, built from this branch) and `light-v0.3-beta` (1 MB F4 boards plus
the H7 `KakuteH7-Wing`, built from the light branch). Each asset is
`<board>_plane_bin_<variant>-<tag>_.zip` with the 4 build products
(`arduplane`, `.apj`, `.bin`, `_with_bl.hex`).

---

# Credits

- **Michel Pastor ([shellixyz](https://github.com/shellixyz))** —
  bulk of the per-feature work, both in the classic 2022
  `shellixyz/ardupilot` and the maintained successor at
  [github.com/ArduCustom/ardupilot](https://github.com/ArduCustom/ardupilot).
  This branch re-ports a curated subset of that work onto a modern
  4.6.3 base. Recent additions sourced from him include `OSD_BATBAR_TYPE`
  (DJI battery-bar Wh source, co-authored with Stavros, originally
  `93c530cdb2`), `MatekH743` ADSB+EFI strip (`26597c8805`), the
  `COURSE_HOLD` flight mode (mode 26) and the `AUTO_TRIM` flight
  mode (mode 27, ArduCustom v11.2 `9e84d8a1ee`) that extends it.
  The v0.2-beta Course Hold `adjust_nav_pitch_throttle()` restore
  followed his ArduCustom original after mf0o's `master_custom_light`
  mirror flagged the missing call.
- **[Stavros Korokithakis ([@skorokithakis](https://github.com/skorokithakis))]** —
  `FLTMODE_EXT` 12-position FLTMODE_CH mechanism (original 2021 commit
  `e8ff5a8dfd`) and co-author of the DJI battery-bar source option.
- **[@mf0o](https://github.com/mf0o)** — widened OSD sidebars, the
  DJI O3 home/waypoint arrow direction fix, the `SpeedyBeeF405WING`
  target (all three landed in ArduCustom and inherited here via
  upstream 4.6.3), and stewardship of the
  [`master_custom_light`](https://github.com/mf0o/ardupilot/tree/master_custom_light)
  pattern that our `master_custom_4.6.3_light` variant mirrors —
  including the `MatekH743` ADSB+EFI strip and routing the
  DJI-batt-bar-source patch.
- **[@giacomo892](https://github.com/giacomo892)** — `PiccoloCAN` ESC
  driver strip used by the light variant to save ~30 KB of generated
  protocol code (originally `8837116f9f`, picked up from mf0o's light
  fork).
- **[@wvarty](https://github.com/wvarty)** — the CRSF `RSSIDBM` /
  `RC_SNR` / `RC_ANT` OSD elements and the RF-Mode-on-LQ element
  (originally ArduCustom PR #78), plus the "un-cap RF_Mode for CRSF
  protocol" change (PR #205). Same path — landed in ArduCustom, then
  upstream merged it, then we inherited it in 4.6.3.
- **[@jaroszmm](https://github.com/jaroszmm)** — proposed the
  distance-cap idea that became `LAND_WIND_DIST` (and, by extension,
  `LAND_WIND_STRICT`) after flagging that `LAND_WIND_BIAS` alone
  could pull the plane toward a much farther landing purely for
  wind alignment. Shaped the anchor semantics (measure from the
  plane's decision-time position, not always from home) and the
  soft-fallback-vs-hard-fence design.
- **[MUSTARDTIGERFPV](https://github.com/MUSTARDTIGERFPV)** — the
  ArduPilot `AP_Radar` peer-aircraft radar implementation
  (`35f5f0ef4f`, March 2023) that this fork forward-ported wholesale
  into the 4.6.3 base. See the
  [Peer-aircraft radar](#peer-aircraft-radar-inav-radar--formationflight)
  section.
- **[OlivierC-FR](https://github.com/OlivierC-FR)** and the
  **[FormationFlight](https://github.com/FormationFlight/FormationFlight)
  project** — the original
  [ESP32-INAV-Radar](https://github.com/OlivierC-FR/ESP32-INAV-Radar)
  and its active successor FormationFlight, whose
  `MSP2_SET_RADAR_POS` (`0x100B`) wire format the radar feature
  consumes and stays compatible with.
- **Upstream ArduPilot dev team** — the underlying platform.
- The `LAND_WIND_BIAS` / `LAND_WIND_DIST` / `LAND_WIND_STRICT`
  wind-aware DO_LAND_START selection family is original to this
  fork (not from any upstream contributor), built on top of
  AP_Mission's existing landing-sequence selection.
- Authoring assistance via Claude Code (Anthropic).
