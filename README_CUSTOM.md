# ArduPlane Custom Build — **FULL VARIANT**

> 🛠️ **This is the full variant of the fork** — every hardware backend
> upstream ArduPlane 4.6.3 ships, plus this fork's additions. Larger
> binary, fits comfortably on H7 / F7 / large-flash F4 targets.
> **⬇ Download:** grab the newest `full-v*` build from the
> [**Releases** page](https://github.com/erwinquilloy/ardupilot/releases)
> (every version, newest first — the current release is `full-v1.2`).
> **Light variant** (1 MB F4, fewer GPS / RC / battery / rangefinder backends): see the
> [light variant `README_CUSTOM.md`](https://github.com/erwinquilloy/ardupilot/blob/master_custom_4.6.3_light/README_CUSTOM.md),
> or the `light-v*` builds on the same
> [Releases page](https://github.com/erwinquilloy/ardupilot/releases).

A curated fork of ArduPlane, rebased onto upstream **ArduPlane 4.6.3**
(tag `Plane-4.6.3`, commit `3fc7011a7d`). This branch carries a set of
additions and behaviour changes on top of stock 4.6.3, mostly ported
from [shellixyz's classic 2022 fork](https://github.com/shellixyz/ardupilot)
and the maintained successor at [ArduCustom/ardupilot](https://github.com/ArduCustom/ardupilot),
plus a few additions originating in this fork.

> If you fly stock ArduPlane and don't recognise the param names in this
> document, you are on the wrong firmware — go to [ardupilot.org](https://ardupilot.org)
> instead.

> 📋 **Read first — your GCS won't describe this fork's parameters.**
> Mission Planner and QGroundControl ship the *upstream* parameter
> metadata, so every parameter this fork adds (and every fork-added bit
> on `FLIGHT_OPTIONS`, `RC_OPTIONS`, `OSD_OPTIONS`, …) shows up with a
> blank description, no value list, and no bitmask checkboxes. The
> parameter still works — your GCS just doesn't know what it is. Use
> this fork's own docs instead of guessing:
>
> - **[Full parameter reference](#full-parameter-reference)** — every
>   parameter this firmware exposes, with descriptions, units and ranges.
> - **[Bitmask calculator](#bitmask-calculator)** — tick the bits you
>   want and copy out the integer to paste into your GCS (or paste a
>   value back to see which bits are set).
>
> Both are per-variant — use the **full** links if you flash this build,
> the **light** links if you flash the light variant.

---

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
  a switch, then commit to the landing sequence. On RC failsafe it waits
  `RTL_AUTOLAND_DLY` seconds (default 30) before committing, so a brief
  dropout can't trigger the landing.

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
- **End the climb in FBWA** (`FLIGHT_OPTIONS` bit 21) instead of loitering
  out at `TKOFF_DIST` — manual control the moment the plane is up.
- **`FLTMODE_EXT`** gives a full **12 flight-mode positions** on one channel.

→ [Arming & takeoff](#arming--takeoff) · [Flight modes](#flight-modes)

### 🎛️ Tune and take control in the air

- **Knob-tunable pitch trim** (`PTCH_TRIM_DEG`, plus `Q_TRIM_PITCH` on
  VTOL) and **three switchable tuning sets** (`TUNE_PARAM`/`PARAM2`/`PARAM3`)
  — retune live without a laptop.
- **Move sticks to cancel auto launch**: nudge pitch or roll during an
  automatic takeoff and the plane drops to FBWA so you can abort a bad
  launch — iNav-style, active only while the takeoff is running.
- **Throttle stick sets target airspeed** in RTL/LOITER/CIRCLE/AUTO, plus
  **pilot loiter radius + direction** control in LOITER/RTL, and in the
  TAKEOFF-mode loiter once the climb finishes.
- Extra modes: **Course Hold** (heading hold) and **Auto Trim** as a mode.

→ [Pitch trim & tuning](#pitch-trim--tuning-knob) ·
[Manual airspeed](#manual-airspeed-control-in-nav-modes) ·
[Cancel auto launch](#move-sticks-to-cancel-auto-launch)

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

### 🎥 Head-tracked FPV gimbal (MSP)

- **Point your gimbal with your head** — head-tracker angles come from the
  goggles over the MSP DisplayPort link and drive the mount (pan/tilt/roll).
  Drives **any gimbal ArduPilot supports** on the full (H7) build — Siyi,
  SToRM32-serial, Topotek, Viewpro, CADDX (CADDXFPV GM-series), servo, …
- Switch to **enable** head tracking (`RCx_OPTION 252`) and to **centre + FPV-lock**
  it (`253`). You can also use the **Walksnail Goggles X Gimbal Lock** feature to
  switch the gimbal to FPV mode.

→ [Walksnail Headtracking via MSP and Serial Gimbal](#walksnail-headtracking-via-msp-and-serial-gimbal)

> Most additions are opt-in via parameters and change nothing until you
> enable them — but a few **defaults do differ** from stock. Skim
> [Behaviour changes vs upstream](#behaviour-changes-vs-upstream--migration-notes)
> before your first flight.

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
  `MatekF405-Wing`, `LongBowF405WING`, `SpeedyBeeF405WING`,
  `AtomRCF405NAVI` (v1.1). Upstream's
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

#### 📡 Yaapu telemetry — showing and announcing Course Hold

Stock Yaapu's plane-mode table stops short of mode 26, so Course Hold
shows as a blank chip with no audio cue. Two drop-in files fix it —
[`Tools/yaapu-coursehold/`](Tools/yaapu-coursehold/README.md) in this
repo, and attached to every release as `plane.lua` + `coursehold.wav`
so you don't need to clone anything.

On an **EdgeTX widget install** (e.g. RadioMaster TX16S):

1. Replace Yaapu's `plane.lua` under `WIDGETS/yaapu/` with the one
   provided — this adds `CourseHold` at index 27 (Yaapu indexes
   `flight_mode + 1`, so 27 = our mode 26).
2. Delete the `plane.luac` next to it; it's a compiled cache and will
   regenerate.
3. Copy `coursehold.wav` into **`WIDGETS/yaapu/sounds/en/`** (your
   language folder if not English).

> ⚠️ **Right name but no sound?** That's the WAV being in a folder
> Yaapu doesn't read — the two things are looked up separately, so the
> HUD text can work while the audio silently doesn't. The sound folder
> depends on which Yaapu build you run:
>
> | Build | `coursehold.wav` goes in |
> |---|---|
> | EdgeTX widget (TX16S) | `WIDGETS/yaapu/sounds/en/` |
> | OpenTX / Horus script | `SOUNDS/yaapu0/en/` |
> | FrSky Ethos | `scripts/yaaputelemetry/audio/en/` |
>
> None of these is EdgeTX's own `SOUNDS/en/` folder, which is for
> Special Functions and unrelated to Yaapu.

The same `plane.lua` works for the Yaapu **GCS desktop tool** — see
[`Tools/yaapu-coursehold/README.md`](Tools/yaapu-coursehold/README.md)
for that and for recording your own cue.

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
Plane's `RCx_OPTION` enum gains three entries — two so the pilot can
put autotrim and autotune on momentary aux switches without burning a
FLTMODE_n slot, plus the RTL Autoland Commit gate:

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
- **`RCx_OPTION = 251` — RTL Autoland Commit.** Gates the RTL → landing
  jump when `RTL_AUTOLAND = 1`: the plane holds at the home loiter until
  you flip this switch HIGH, then commits to `DO_LAND_START`. See
  [RTL & failsafe behaviour](#rtl--failsafe-behaviour) for the full
  behaviour and the `RTL_AUTOLAND_DLY` failsafe grace timer.

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

**Plain GPIO buzzers (v1.1).** The table above needs a **ToneAlarm** output
(a pitched buzzer on a PWM/`ALARM` pin) to play actual tones. Boards with
only a plain on/off GPIO buzzer — `AtomRCF405NAVI` among them — previously
got no pre-launch cues at all. They now play the same three states as
distinct **cadences** instead of pitches:

| State | GPIO buzzer cadence |
|-------|---------------------|
| `TKOFS_WAITING_TO_RAISE_THROTTLE` | Slow lone beeps |
| `TKOFS_WAITING_FOR_IDLE_THROTTLE` | Medium double-beeps |
| `TKOFS_WAITING_FOR_LAUNCH` | Fast continuous |

The three are deliberately far apart in rhythm so they stay tellable apart
by ear on a buzzer that can only be on or off. Nothing to configure — the
cues appear automatically on boards that have a GPIO buzzer and no
ToneAlarm; boards with a ToneAlarm are unchanged and still play the tones
in the first table.

> **Timing note.** The GPIO buzzer enforces a 3.3 s minimum gap between
> patterns (32 bits × 100 ms). State-change cues bypass that gap so they
> sound the moment the state changes — otherwise a "ready to launch" cue
> could arrive up to 3.3 s late, well after a hand launch has happened.
> The repeat of the launch-ready cue still respects the gap.

### Pilot altitude control during TAKEOFF loiter

After the initial climb in `TAKEOFF` mode, the plane enters a loiter
circle around the target waypoint. The pitch (elevator) stick now
adjusts altitude during this loiter phase the same way FBW-B does it —
pull back to climb, push forward to descend — so the pilot can settle
the plane at the right altitude before flipping to `AUTO` or `RTL`.

No parameter to enable; behaviour is on whenever TAKEOFF is the active
mode and `flight_stage` is past the initial climb — and whenever the
loiter phase runs at all. Set
[`FLIGHT_OPTIONS` bit 21](#flight_options-bit-21--end-takeoff-in-fbwa-instead-of-loitering)
to skip that phase entirely and drop into FBWA at the end of the climb
instead.

> ⚠️ **Do not assume LOITER behaves the same way.** This always-on
> behaviour is specific to the TAKEOFF loiter. In LOITER mode the same
> stick gesture does nothing to the target altitude unless
> [`FLIGHT_OPTIONS` bit 12](#flight_options-bit-12--fbw-b-style-loiter-altitude-control-upstream)
> is set — without it the plane climbs while you hold the stick and sinks
> back the moment you release. Bit 12 is a fork default from v1.2 on, but
> upgrades keep whatever `FLIGHT_OPTIONS` you already had saved.

**Since v1.1 the roll and rudder sticks work here too** — radius and
turn direction, from the ArduCustom PR #180 port. See
[pilot control of loiter radius and direction](#pilot-control-of-loiter-radius-and-direction-loiter-rtl-takeoff).
During the climb itself the pitch and roll sticks instead
[cancel the launch](#move-sticks-to-cancel-auto-launch); the two phases
never overlap.

> 🐞 **Fixed in v1.1 — stale climb target on airframes *with* an airspeed
> sensor.** `ModeTakeoff::navigate()` wrote the pilot-adjustable target
> altitude back into the loiter waypoint unconditionally, from the moment
> the mode started — before that value had been seeded off the loiter
> waypoint. Entering TAKEOFF from FBWB could therefore latch the climb
> target at ground level, so the plane would level off almost immediately
> instead of climbing to `TKOFF_ALT`. Aircraft without an airspeed sensor
> were unaffected, because their climb is pitch-driven rather than
> TECS-driven. The write-back is now gated on the altitude having been
> seeded.

### `FLIGHT_OPTIONS` bit 21 — end TAKEOFF in FBWA instead of loitering

**New in v1.2.** With this bit set, reaching the end of the `TAKEOFF`
climb hands the plane straight to you in **FBWA** instead of entering the
loiter circle. The mode changes, so everything above stops applying:
there is no loiter at `TKOFF_DIST`, no pitch-stick altitude nudging, no
radius or direction control — you are simply flying the aircraft, with a
`"Takeoff complete - FBWA"` message on the GCS and OSD.

The climb itself is unchanged. `TKOFF_ALT`, `TKOFF_LVL_ALT`,
`TKOFF_LVL_PITCH` and the launch-cancel window all behave exactly as they
do with the bit clear; only the end of the sequence differs.

| `FLIGHT_OPTIONS` bit 21 | At the end of the climb |
|---|---|
| clear (**default**) | loiter at `TKOFF_ALT` / `TKOFF_DIST`, still in TAKEOFF mode |
| set (**2097152**) | switch to FBWA |

Useful if you hand launch into a small field and want manual control the
moment the plane is up, rather than watching it circle out to
`TKOFF_DIST` before you can take over. The alternative — cancelling the
launch with the sticks — drops to FBWA *during* the climb and abandons
it; this option lets the climb finish first.

> ⚠️ **The switch is suppressed under RC failsafe.** FBWA is a
> pilot-flown mode, so if the link is down at the end of the climb (or a
> long failsafe is pending), the plane keeps the legacy loiter and lets
> the failsafe action decide. You only get FBWA when there is a
> transmitter to fly it with.

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

### `FLIGHT_OPTIONS` bit 12 — FBW-B style loiter altitude control *(upstream)*

Not a fork addition, but documented here because the fork's own
pitch-stick altitude features sit either side of it and the asymmetry
catches people out.

In **LOITER**, pitch-stick altitude control needs two things:

| Param | Value | Why |
|---|---|---|
| `FLIGHT_OPTIONS` | **4096** (bit 12) — **default in this fork since v1.2** | "Enable FBWB style loiter altitude control" |
| `STICK_MIXING` | non-zero, not `VTOL_YAW` — this fork defaults to `FBW` | `ModeLoiter::update()` gates on `stick_mixing_enabled()` |

Upstream defaults `FLIGHT_OPTIONS` to `0`, so on stock ArduPlane both of
these are off. This fork ships both on, so LOITER altitude control works out
of the box.

> ⚠️ **Upgrading from v1.1 or earlier?** Changing a default only affects
> fresh parameter storage. Your saved `FLIGHT_OPTIONS` carries over
> untouched, so you still need to add 4096 by hand — same as the
> `STICK_MIXING` change in v1.1.

> ⚠️ **Without bit 12 the pitch stick still moves the plane — and that is
> the trap.** It falls through to ordinary stick mixing, which adds a
> temporary offset to `nav_pitch_cd`. The aircraft climbs while you hold
> the stick, then TECS returns it to the original loiter altitude the
> moment you let go. It looks like altitude control that "forgets" the new
> altitude, when in fact no altitude command was ever issued.

With the bit set, `update_fbwb_speed_height()` owns the pitch stick and
calls `set_target_altitude_current()` when the stick returns to centre,
so the new altitude is latched and held. `FBWB_CLIMB_RATE` (default
2.0 m/s) sets how fast the stick moves the target, and the result is
clamped by any altitude fence and by `CRUISE_ALT_FLOOR`.

**Contrast with the fork's own two:** [RTL](#flight_options-bit-24--rtl_manual_alt_control)
needs bit 24, while the
[TAKEOFF loiter](#pilot-altitude-control-during-takeoff-loiter) needs no
parameter at all. Three modes, three different enable conditions — LOITER
is the only one relying on an upstream option.

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

### RTL Autoland Commit (`RCx_OPTION 251`) — hold at home, commit on your command
With `RTL_AUTOLAND = 1` the plane normally flies home and then jumps
straight into the nearest `DO_LAND_START` landing sequence. Assign
`RCx_OPTION = 251` to a two-position switch and that jump becomes
**pilot-gated**: the plane holds at the home loiter (circling) until you
flip the switch HIGH, then commits to the landing sequence. This lets
you loiter over the field and pick the moment to land, and it defers the
wind-biased `DO_LAND_START` selection (see `LAND_WIND_BIAS`) until the
wind estimate has settled.

Behaviour at a glance (with `RTL_AUTOLAND = 1` and the switch assigned):

| Switch (`RCx_OPTION 251`) | RC link | What the plane does |
| --- | --- | --- |
| **LOW / hold** | OK | Circles the home loiter **indefinitely**, waiting for you. GCS: `RTL: holding, AUTOLAND switch to land` |
| **HIGH / commit** | OK | **Commits immediately** — GCS `RTL autoland: commit`, RTL → AUTO, flies `DO_LAND_START` |
| **LOW / hold** | RC failsafe, `RTL_AUTOLAND_DLY > 0` | Keeps circling home, then **commits after `RTL_AUTOLAND_DLY` s** (timed from reaching the loiter). GCS: `RTL autoland: FS, committing in <n>s` |
| **LOW / hold** | RC failsafe, link recovers before the timer expires | Timer **resets**, gate re-engages → back to holding for you |
| **LOW / hold** | RC failsafe, `RTL_AUTOLAND_DLY = 0` | **Commits immediately** (pre-v1.0 behaviour) |
| **HIGH / commit** | RC failsafe | Already committed before the link dropped → lands |
| **Unassigned** (any) | any | No hold — exactly **stock ArduPlane** (commits per `RTL_AUTOLAND` on reaching home) |

> `RTL_AUTOLAND = 2` (go directly to the landing sequence) never loiters
> at home, so it is **never gated** by this switch — only `RTL_AUTOLAND = 1`
> is.

**`RTL_AUTOLAND_DLY` — failsafe grace timer (default 30 s).** During RC
failsafe you can no longer flip the switch, so the plane would otherwise
commit to the landing immediately. `RTL_AUTOLAND_DLY` keeps it circling
home for this many seconds (counted from when it settles at the home
loiter) before committing — long enough that a brief dropout or a
single-frame RC glitch can't trigger the landing. If the link recovers
inside the window the gate re-engages and the plane goes back to holding
for you; GCS shows `RTL autoland: FS, committing in <n>s` when the timer
starts. Set `0` to commit immediately on failsafe (the pre-v1.0
behaviour). Has no effect when the commit switch is unassigned or already
in the commit position.

> Why this exists: on a real flight a ~0.4 s ELRS dropout (with the
> switch still in hold) dropped the gate and committed the landing. The
> grace timer makes transient link glitches harmless while still
> guaranteeing the plane lands if the link is genuinely gone.

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

## Pilot control of loiter radius and direction (LOITER, RTL, TAKEOFF)

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

Together with the two altitude/speed features documented elsewhere, all
four sticks are live in LOITER:

| Stick | Effect in LOITER | Needs |
|---|---|---|
| Pitch | target altitude, latched when the stick centres | [`FLIGHT_OPTIONS` bit 12](#flight_options-bit-12--fbw-b-style-loiter-altitude-control-upstream) **and** a non-zero `STICK_MIXING` — both fork defaults since v1.2 |
| Roll | loiter radius, clamped `[20, 1000]` m | — |
| Rudder | loiter direction, beyond 50 % deflection | — |
| Throttle | [target airspeed](#manual-airspeed-control-in-nav-modes) across `AIRSPEED_MIN`…`AIRSPEED_MAX` | — |

So the whole orbit — height, size, direction and speed — can be reshaped
without leaving the mode.

> ⚠️ **Roll input is integrated, not proportional.** The radius keeps
> growing or shrinking for as long as the stick is held, and stays where
> you left it when you centre — it does not spring back to
> `WP_LOITER_RAD`. Rudder is a snap: past 50 % it flips direction once
> rather than integrating. Pitch behaves differently again — it holds the
> altitude the aircraft is actually at when the stick centres, not the
> altitude the demand had reached.

**In TAKEOFF mode (mode 13)**, the same control applies once the climb
finishes and the plane settles into its loiter — see
[cancel auto launch](#move-sticks-to-cancel-auto-launch) for why the sticks
mean something different during the climb itself. The plane stays in TAKEOFF
mode while circling, so this is pilot control of a TAKEOFF-mode loiter, not a
switch to LOITER. Combined with the pitch-stick altitude control below, all
three axes are live in that phase:

| Stick | Effect in the TAKEOFF loiter |
|---|---|
| Pitch | loiter altitude (`FBWB_CLIMB_RATE`, release to lock) |
| Roll | loiter radius, clamped `[20, 1000]` m |
| Rudder | loiter direction, beyond 50% deflection |

During the climb none of this applies: roll and pitch beyond 10 % cancel the
launch instead, and normal `STICK_MIXING` is in effect below that threshold.

On mode entry, radius and direction are seeded from `WP_LOITER_RAD`
(LOITER, TAKEOFF) or `RTL_RADIUS` with a `WP_LOITER_RAD` fallback (RTL).
The sign of these params still determines the starting direction
(negative = CCW). In TAKEOFF mode the seeding happens when the loiter phase
begins rather than at mode entry, since the climb comes first.

> ⚠️ **Stick mixing is suppressed in the TAKEOFF loiter phase.** With all
> three sticks assigned to loiter geometry, `stabilize_stick_mixing_fbw()`
> returns early there, so the same input is not applied twice. Mixing is
> unaffected everywhere else, including during the takeoff climb.

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

## Move sticks to cancel auto launch

Move the pitch or roll stick past 10 % deflection **during an automatic
takeoff** and the plane switches to FBWA, handing you manual control so
you can save a bad launch without reaching for the mode switch. Modelled
on iNav's "move sticks to cancel auto launch".

There are two windows, matching iNav's fixed-wing launch behaviour:

- **Before launch — available immediately.** Once armed and waiting for the
  throw, moving pitch or roll cancels the takeoff. Same as iNav, where stick
  movement in `NAV_STATE_LAUNCH_WAIT` aborts the launch and leaves the mode.
  Nothing has been thrown yet, so there is no throw motion to guard against.
- **During the throw — locked for `TKOFF_CNCL_DLY`** (default **1 s**, from
  launch detection). You throw one-handed while holding the transmitter, so
  the throwing motion must not be able to abort the launch it just started.
  Equivalent to iNav's `nav_fw_launch_min_time`. Set `0` to disable the
  lock-out.
- **Climb-out — available again** until the target altitude is reached:
  `TKOFF_ALT` in TAKEOFF mode, or the `NAV_TAKEOFF` item's altitude in AUTO.
  Either takeoff timeout closes it too.
- **After the takeoff — never.** No stick takeover for the rest of the flight.

| Parameter | Default | Meaning |
|---|---:|---|
| `TKOFF_CNCL_DLY` | `1.0` s | Stick input is ignored for this long after launch. Range 0-10 s. |

Active in **TAKEOFF mode** and in **AUTO** while a `NAV_TAKEOFF` item is
the command in progress. The takeover is **sticky**: you stay in FBWA
until you flip the mode switch.

Two GCS announcements make it visible:

| When | Message |
|---|---|
| Window open — once before launch, then every 2 s while airborne | `Move sticks to cancel auto launch` |
| Sticks moved, takeover fires | `Auto launch cancelled` |

The first message is your cue that cancelling is now possible — it is sent
when the window actually opens, not at the moment of launch. It therefore
goes quiet for `TKOFF_CNCL_DLY` after launch detection and reappearing is
what tells you the grace period is over.

**Why it repeats.** The OSD holds one message at a time and displays it for
`OSD_MSG_TIME` (default 10 s), so *any* later statustext hides this prompt
for the rest of the climb. In AUTO that is guaranteed: `Holding course …`
is sent once, a couple of seconds after the throw, and would otherwise own
the screen for the entire takeoff. Repeating the prompt keeps the cancel
option on screen for as long as it is actually available. The repeat is
timed from whenever the message slot last changed, not from our own last
send, so any other takeoff message gets a full 2 s of screen time before
the prompt takes it back — you still get to read `Holding course …`, it
just no longer costs you the rest of the climb.

The repeat runs **only once airborne**. Before launch the prompt is sent
once, so it cannot bury the `TKOFF idle THR timer started` cue while you
are standing there waiting to throw.

### What it deliberately does *not* do

- **No stick takeover during the throw, or after the climb.** The window
  shuts for `TKOFF_CNCL_DLY` after launch detection and closes for good at the
  target altitude (`Takeoff complete` in AUTO). Switching back to AUTO later
  in the flight gives you a normal mission with no takeover — a stick bump at
  altitude can no longer end your mission. Before launch the window *is*
  open, as described above.
- **No option bit.** There is no `RC_OPTIONS` bit for this. Because it is
  scoped to the takeoff phase it is always available, with nothing to
  configure.
- **VTOL takeoffs are excluded.** `NAV_VTOL_TAKEOFF` is not covered, so
  quadplane takeoffs behave exactly as upstream.

> ⚠️ **Changed in v1.1 — read this if you used the old behaviour.**
> Earlier builds had `RC_OPTIONS` bit 20
> (`AUTO_SWITCH_TO_FBWA_WITH_STICKS`), which fired **any time** you were
> in AUTO, at any altitude, mid-mission. That was easy to trigger by
> accident and silently ended missions. **Bit 20 is gone** — the option no
> longer exists and the behaviour is now takeoff-only. If your saved
> `RC_OPTIONS` still has bit 20 set it is simply ignored; you can clear it
> for tidiness but nothing depends on it. For mid-mission pilot nudges
> use `STICK_MIXING`, which is what it is for and now defaults to `1`.

### How this interacts with `STICK_MIXING`

They no longer overlap, which was the point of the rescope. The cancel
check runs only during takeoff; `STICK_MIXING` governs pilot input for the
rest of the mission.

`stick_mixing_enabled()` only permits mixing in modes that are both
auto-throttle **and** auto-navigation. Once a cancel fires you are in
FBWA, which is neither — and FBWA's sticks command attitude directly — so
mixing simply stops being reachable.

| Stick deflection | During an automatic takeoff | Rest of the mission (`STICK_MIXING = 1`) |
|---|---|---|
| Under 10 % | Nothing; the takeoff controller flies | Mixing nudges the plane against the nav controller |
| Over 10 % | **Cancel → FBWA** | Mixing nudges harder; the mission continues |

The old build had a threshold split in AUTO where a small nudge biased the
mission and a slightly larger one ended it, with an invisible boundary in
between. That is gone: nudging mid-mission now only ever mixes.

> ℹ️ In TAKEOFF mode the pitch stick has a second job **after** the climb:
> it nudges the loiter altitude (see
> [pilot loiter altitude](#pilot-altitude-control-during-takeoff-loiter)). The
> two never overlap — cancel applies during the takeoff phase, altitude
> nudging only once the plane reaches the loiter.

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

## Revert to MANUAL after disarming

A feature of the original 2022 fork, restored in v1.2. Off by default —
enable it with **`RC_OPTIONS` bit 21** (add `2097152`).

With the bit set, two things happen:

- **On disarm**, the aircraft switches to MANUAL immediately, so it is
  never left sitting on the ground in AUTO / RTL / a stale auto-throttle
  mode. If you are already in MANUAL, nothing happens. The mode change is
  logged with `ModeReason` 57 (`DISARMED`).
- **On arming**, the flight mode switch is re-read — but **only if you are
  still in the MANUAL that the disarm put you in**. Without this half,
  arming after a revert would leave you flying MANUAL while the switch
  still reads e.g. FBWA, until you happened to toggle it.

⚠️ That second condition matters. The re-read only ever undoes the fork's
own revert: it fires when the current mode is MANUAL **and** was set with
`ModeReason` 57 (`DISARMED`). Arm in a mode you chose yourself — a GCS or
mission-initiated TAKEOFF or AUTO, or any mode you selected after the
revert — and the switch is **not** re-read. An earlier build re-read
unconditionally, which dumped an AUTO takeoff into the mode switch position
three seconds into the climb; caught in SITL before release.

The re-read on arming also only applies when `ARMING_MODE_SW` is `0`
(Disabled). If you use `ARMING_MODE_SW` to jump straight to TKOFF or AUTO on
arming, that takes precedence and the switch is not re-read — the two
features would otherwise fight over the mode 3 s after arming.

> **Note:** the disarm revert is immediate, unlike the 3 s delay on the
> `ARMING_MODE_SW` path. That delay exists so a single aux-switch arm can
> start a takeoff; there is no disarm equivalent.

**This does not fight the in-flight throttle cut.** Flipping the arm switch
while flying does not disarm — it cuts the throttle and bumps you to FBWA
(see [arm-switch safety](#arm-switch-safety--in-flight-disarm-cuts-throttle-doesnt-disarm)).
That path
never reaches the real disarm, so no MANUAL revert happens and the FBWA bump
stands. Likewise, re-arming out of a throttle cut restores the pre-cut mode
rather than re-reading the switch.

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

## `EKF3 allocation failed` on 1 MB F4 — allocation order and the fix

On 1 MB F4 boards (192 KB RAM) the runtime limit is **RAM, not flash**. If
the heap runs short, EKF3 can't allocate its core, the estimator falls back
to DCM, and prearm refuses to arm. Nothing crashes — the vehicle is
protecting itself.

Symptoms, at boot, any of:

```
EKF3 allocation failed
EKF3 not enough memory
Terrain: Allocation failed
PreArm: AHRS: EKF3 not started
PreArm: Terrain out of memory
AHRS: DCM active
```

**These boards cannot fit the log buffer, the terrain cache and the EKF3
core all at once.** You are choosing which one goes short — and the choice
is made for you by the order they allocate in:

1. **Logger**, at boot — the write buffer, plus filesystem structures once a
   log file is actually opened.
2. **Terrain**, about a second later, on its first height lookup.
3. **EKF3**, last, several seconds in — its init is deliberately delayed.

**EKF3 asks last, so it gets the leftovers.** Every counter-intuitive result
below follows from that one fact.

### The fix

```
LOG_DISARMED     0
TERRAIN_CACHE_SZ 9
```

Reboot. `LOG_DISARMED 0` stops a log file being opened while you're
disarmed, so the filesystem allocations that come with an open file aren't
held during boot — which is the window EKF3 needs. `TERRAIN_CACHE_SZ 9`
(~16.2 KB rather than the default 12's ~21.6 KB) then leaves room for the
estimator. On the board this was diagnosed on, that combination brought up
EKF3 cleanly with satellites locked, where `CACHE_SZ 12` still failed with
`Terrain: Allocation failed`.

> ⚠️ **`LOG_DISARMED 0` moves the allocation to arming time — it doesn't
> remove it.** When you arm, the log file opens and allocates. EKF3 is safe
> by then (it already holds its core), but **logging can fail silently and
> you'd fly with no dataflash log**. The internal "out of memory for
> logging" complaint never reaches your GCS, so no error message is *not*
> evidence it worked. Arm on the bench with **props off**, confirm a log
> file is actually created and grows, and confirm EKF3 stays healthy.

> ⚠️ **Don't try to free RAM by lowering `TERRAIN_CACHE_SZ` further — below
> 9 it backfires twice.** Because terrain allocates *before* EKF3, a
> *smaller* cache is more likely to succeed, which means terrain takes the
> RAM EKF3 then can't have. Observed on an F405: `CACHE_SZ 12` (21.6 KB) was
> too big, terrain failed, and **EKF3 started fine**; `CACHE_SZ 9` (16.2 KB)
> fitted, terrain took it, and **EKF3 failed** — until `LOG_DISARMED 0`
> freed enough for both. Separately, terrain prefetches a **3×3 ring of 9
> blocks** every update, so a cache below 9 thrashes its LRU and pending
> requests may never reach zero, leaving you at `PreArm: waiting for terrain
> data` forever. `9` is the floor, and it has no spare for the home, mission
> and rally blocks the default 12 accounts for — expect extra reloads.

> ℹ️ `TERRAIN_SPACING` costs **zero RAM** — the cache is `CACHE_SZ` blocks of
> ~1.8 KB regardless of spacing; spacing only changes how much ground each
> block covers. Raising it also invalidates every `.DAT` on your SD card
> (spacing is recorded inside each block, and filenames are lat/lon only),
> forcing a full re-download. Leave it at `100` unless you have a reason.

> ℹ️ Only trust a result taken with a **3D GPS fix**. Indoors, terrain never
> really populates, so a configuration can look healthy on the bench and
> then fail once you have satellites.

### Reading the messages

The two EKF3 messages are **different faults** — check which one you have
before changing anything:

| Message | Meaning |
|---|---|
| `EKF3 not enough memory` | The free-memory precheck failed. Genuinely out of heap. |
| `EKF3 allocation failed` | The precheck *passed* but the allocation still failed — heap fragmentation. |

Either way EKF3 disables itself in RAM only, so a reboot always retries and
each boot prints at most one of these.

Several messages that look like this problem are **not** memory faults:

- `PreArm: AHRS: EKF3 not started` **with** `GPS: Bad fix` — on a plane EKF3
  won't finish init until a **3D GPS fix**. It's waiting for satellites.
- `PreArm: AHRS: Not healthy` — EKF3 is running but not yet healthy; also
  usually a GPS fix away. Note this message *replacing* `EKF3 not started`
  is progress, not a regression.
- `PreArm: waiting for terrain data` — terrain can't fetch blocks until it
  knows where it is. Clears with GPS.
- `AHRS: DCM active` in the first seconds of boot — normal, DCM covers the
  window before EKF3 initialises.

Confirmation you're actually fixed is `EKF3 IMU0 initialised` followed by
`EKF3 IMU0 tilt alignment complete`. Those only print from a core that
allocated *and* initialised. Then check `freemem` (Mission Planner → Status
tab) for your remaining margin.

> ℹ️ **Scope, honestly:** this was diagnosed on a SoloGood F405 Wing running
> the `SpeedyBeeF405WING-Buzz` build (there is no SoloGood target — the
> SpeedyBee one is a stand-in). A genuine SpeedyBee F405 WING runs the same
> binary with the same defaults and shows none of it. That difference is
> **unexplained**: same chip, same 192 KB, same firmware, so it's params or
> detected hardware rather than the build. Treat this as a lever to reach for
> **if you see the messages above**, not as a setting every F4 board needs.

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

`STICK_MIXING` defaults to `1` (FBW-style pilot override in
auto/RTL/guided), matching upstream. Earlier builds of this fork shipped
`0`, which made sense while the old always-on AUTO→FBWA takeover existed —
running both gave you an invisible threshold where a small nudge biased the
mission and a larger one ended it. Now that the takeover is scoped to
[auto-launch cancel](#move-sticks-to-cancel-auto-launch) only, mixing is
the right tool for mid-mission nudges and there is no conflict. Set
`STICK_MIXING = 0` if you want pilot input ignored in nav modes.

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

# Walksnail Headtracking via MSP and Serial Gimbal

Point a camera gimbal with your head. A **head-tracker device** — such as the
Walksnail Goggles X — transmits its head angles over a **side channel** (the MSP DisplayPort
link that already carries your OSD) rather than over the RC link, and the flight
controller drives the mount to follow your head in pan, tilt and roll. The gimbal
is steered the same way it is for normal RC/manual pointing, but while head
tracking is active it **ignores the RC axis inputs and uses the head-tracker
angles instead**.

This implements the Walksnail head-tracker-over-MSP protocol
(`MSP2_SENSOR_HEADTRACKER`, message `0x1F07`), **introduced to iNav by
[mmosca](https://github.com/mmosca)** — full credit and thanks to mmosca and the
[iNav project](https://github.com/iNavFlight/inav). This is an independent
ArduPilot implementation of that protocol. (Upstream ArduPilot has the CADDX
gimbal *output* driver but no MSP head-tracker *input*, so this fills that gap.)

Verified on hardware with **Walksnail Goggles X + a CADDX GM3 gimbal**, and with
a plain PWM servo pan/tilt.

## How it flows

    goggles ──air link──► VTX ──MSP DisplayPort──► FC ──serial/PWM──► gimbal

The head angles ride the existing DisplayPort/OSD link into the FC; the FC
scales them and drives whatever mount backend is configured — so it works with
**any gimbal ArduPilot supports**. The **full (H7) build carries the complete
backend set** (Siyi, SToRM32-serial, Alexmos, Topotek, Viewpro, CADDX, servo, …);
the light variant's 1 MB F4 boards are trimmed to CADDX serial + PWM servo to
save flash. (Verified with a GM3, but nothing here is GM3-specific.)

## Setup

| Param | Value | Purpose |
|---|---|---|
| `MNT1_TYPE` | e.g. `13` (CADDX), `8` (Siyi), `7` (Servo, FPV/no-stab), `1` (Servo, stabilised) | gimbal backend |
| `MNT1_DEFLT_MODE` | `3` (RC_TARGETING) | control when head tracking is off |
| `SERIALx_PROTOCOL` | `42` (MSP DisplayPort) | head-tracker **input** (goggles/VTX UART) |
| `SERIALy_PROTOCOL` | `8` (Gimbal) | serial-gimbal **output** UART |
| `MNT1_YAW_MIN` / `MNT1_YAW_MAX` | e.g. `-170` / `170` | yaw travel — doubles as the pan gain |
| `MNT1_PITCH_*`, `MNT1_ROLL_*` | your gimbal's travel | pitch/roll gain |
| `RCa_OPTION` | `252` | head-tracking **enable** switch |
| `RCb_OPTION` | `253` | **centre-lock** switch |

Servo gimbal: assign the outputs with `SERVOn_FUNCTION` = `6` (Mount1Yaw),
`7` (Mount1Tilt), `8` (Mount1Roll).

> 💡 **For head tracking, use `MNT1_TYPE = 7` (BrushlessPWM), not `1` (Servo).**
> Both drive the same servo outputs and the same `SERVOn_FUNCTION` pins, but
> type `1` earth-frame stabilises — the pan/tilt servos counter-rotate to hold
> the horizon as the airframe moves, which fights the head tracker. Type `7`
> drives the servos in body frame with no stabilisation, so pan/tilt follow only
> your head (FPV behaviour). `MNT1_TYPE` is reboot-required.

## Controls

- **`RCx_OPTION 252` — head-tracking enable.** HIGH follows your head; LOW hands
  the gimbal back to `MNT1_DEFLT_MODE` (normal RC/manual gimbal control).
- **`RCx_OPTION 253` — centre-lock.** HIGH recenters the gimbal and locks it
  **FPV-style** — bolted to the airframe on all three axes with **no horizon
  stabilisation** (roll and pitch follow the aircraft rather than self-levelling).
  Handy to reset the camera if you lose orientation. LOW resumes head tracking.
  (FPV-lock is implemented for the CADDX backend; other backends centre to their
  neutral.)
- **The Walksnail Goggles X Gimbal Lock feature.** You can also switch the gimbal
  to FPV mode straight from the goggles — the Gimbal Lock button stops the
  head-tracker stream, and the FC treats **~1 s of silence** as a centre + lock,
  resuming when the stream returns. A genuine link dropout parks the gimbal the
  same way — a handy failsafe.

## Notes

- Head-tracker yaw is body-frame, so the usable range is **±180°** (±180 is
  straight back). Keep `MNT1_YAW_MIN/MAX` within that; a GM3 realistically
  reaches ~±170°, so use `-170 / 170`.
- Head-forward always maps to gimbal-forward, even with asymmetric limits.
- **Which boards have it.** On the full build every board is F7/H7 and
  mount-capable, so head tracking is available across the whole full fleet.
  In the light fleet it runs on the mount-enabled F4 "wing" boards and, as of
  **v1.1**, on `AtomRCF405NAVI` — that board now enables the servo (PWM) and
  CADDX mount backends instead of shipping with the mount subsystem off.
  (`KakuteF4-Wing-Buzz` and `MatekF405-STD` still ship exactly as upstream with
  the mount subsystem off to save flash, so no gimbal there by default. Want it
  on one? It's a small hwdef change — just ask.)

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

> ⚠️ **ArduPilot's MSP identity depends on the backend, and it is not
> always `ARDU`.** Worth knowing before writing anything that branches on
> `MSP_FC_VARIANT`:
>
> | `SERIALn_PROTOCOL` | Backend | Reports |
> |---|---|---|
> | `32` | `SerialProtocol_MSP` | `ARDU` |
> | `33` | `SerialProtocol_DJI_FPV` | **`BTFL`** — hard-coded |
> | `42` | `SerialProtocol_MSP_DisplayPort` | `BTFL` if the Betaflight-fonts `MSP_OPTIONS` bit is set, else `ARDU` |
>
> Both `33` and `42` impersonate Betaflight so DJI and Walksnail goggles
> accept the link. This bit the peer-callsign work: an ESP32 asking "who
> are you?" over a protocol-33 port is told `BTFL`, so logic that tests
> for ArduPilot fails on the exact setup it was written for. Prefer
> testing for the flight controller you want to **exclude** rather than
> the one you want to include.

## OSD element

Cycles through up to 6 healthy peers (one every 2 s) and renders:

```
BOB→
   1.2km ↑15m
```

- **callsign** — the peer's 3-character name when the radio supplies one,
  otherwise the slot letter `A`..`F` as before. See
  [Peer callsigns on the radar OSD](#peer-callsigns-on-the-radar-osd) — this
  needs a patched ESP32; with stock firmware you get the letters.
  The label is always three columns wide so the fields to the right do not
  shift when a name arrives part-way through a flight.
- **arrow** — bearing to the peer, body-frame, drawn from the
  existing `get_arrow_font_index()` direction-arrow helper.
- **distance** — horizontal distance to the peer in the OSD's
  configured units (m/km, ft/mi, or NM).
- **vertical distance** — relative altitude with up/down arrow
  (`↑`/`↑↑` if the peer is above; `↓`/`↓↓` if below — heavy arrow
  past 25 m).
- A peer goes "stale" (and the element blanks) after `RADAR_PEER_FRESH_TIME_MS`
  = 3 s without a fresh frame.
- **A peer without a GPS fix still shows.** Peer selection is by *freshness*
  (heard in the last 3 s), not by position. A peer transmitting from 0/0
  because it has no fix yet is displayed with its callsign alone, blinking;
  the bearing arrow, distance and relative altitude appear once it reports a
  real position. Upstream selected only peers with a valid position, which
  made a peer invisible even when its callsign and link quality were known —
  unhelpful on the bench and during pre-flight, where you want to confirm
  who is on the mesh before anyone has a fix.

## Build gating

```
AP_RADAR_ENABLED        = HAL_MSP_ENABLED       (default)
HAL_MSP_RADAR_ENABLED   = AP_RADAR_ENABLED && HAL_MSP_ENABLED
```

i.e. radar follows MSP. Override in a hwdef with `define AP_RADAR_ENABLED 0`
to force-strip it on a flash-constrained board.

## Peer callsigns on the radar OSD

By default the element labels peers `A`..`F` by slot. It can show a real
**3-character callsign** instead, but that needs a patched ESP32 —
the stock protocol never tells the flight controller who a peer is.

**Why a patch is needed.** FormationFlight already knows every peer's name:
each node broadcasts its own one character per OTA cycle through the
`air_type0_t` `extra_type`/`extra_value` slots (types 2, 3, 4 carry
`name[0..2]`), and receivers reassemble it into `peer_t.name`. But
`MSPManager::sendRadar()` only forwards the position fields, so the name
stops at the ESP32 and never reaches the FC.

**The wire change.** The ESP32 appends the 4-byte NUL-terminated callsign
to the `MSP2_SET_RADAR_POS` payload, straight after `lq`:

| Offset | Field | Notes |
|---:|---|---|
| 0..18 | existing position fields | 19 bytes, unchanged, iNav-compatible |
| 19..22 | `char name[4]` | 3 printable chars + NUL |

Total payload is 23 bytes with a name, 19 without.

**Compatibility is two-way.** The FC branches on the received payload
length, so a stock iNav Radar / FormationFlight node still works and simply
keeps its slot letter. A short or malformed frame is now rejected instead of
being cast past the end of the buffer, which is what the handler did before.

**Getting names onto the mesh.** Names come from each node's own ESP32:

1. Flash [FormationFlightAPC](https://github.com/erwinquilloy/FormationFlightAPC)
   on **every** node whose name you want to see.
2. Set the name in its WebUI. Only the **first 3 characters** are
   transmitted — `peer_t.name` is `char[NAME_LENGTH + 1]` with
   `NAME_LENGTH 3` — so pick a 3-letter callsign.
3. Expect the name to take a few OTA cycles to appear: it arrives one
   character per cycle, at `ota_nonce % 5`. Until the first character
   lands the OSD stays on the slot letter, and partial or non-printable
   bytes are filtered out rather than drawn.
4. **Case does not matter** — callsigns are folded to upper case on
   receipt. The MAX7456 character map holds symbols, not lowercase
   letters, at those code points, so a lowercase name would otherwise
   draw as arrows. Type `Dol` or `DOL` in the WebUI; both show `DOL`.

Without the WebUI name set, FormationFlight assigns a random 3-character
string from the ESP32 chip ID. It does **not** ask an ArduPilot FC for a
craft name — ArduPilot reuses `MSP_NAME` for the DJI OSD status-text line,
so the reply is the current warning message rather than a name, and
FormationFlight skips ArduPilot hosts for exactly that reason.

## Which file in the zip do I flash?

Every release zip carries four build products; two of them are things you
can actually flash.

| File | Use it when |
|---|---|
| `arduplane.apj` | **Normal upgrade.** Load it from MissionPlanner / QGC "Load custom firmware" onto a board that already has a working ArduPilot bootloader. |
| `arduplane_with_bl.hex` | **The board's bootloader is missing, stale, or rejects the `.apj`.** Flash over **DFU** (board in bootloader mode, e.g. STM32CubeProgrammer or `dfu-util`). This writes the bootloader *and* the firmware, so it works from a bare or mis-flashed chip. |
| `arduplane`, `arduplane.bin` | ELF and raw binary — debugging and toolchain use, not for GCS flashing. |

> 💡 **If the `.apj` refuses to take, go straight to `_with_bl.hex` over
> DFU.** This is not a fork quirk — it means the bootloader on the board
> is not the one the `.apj` path expects. Confirmed on `KakuteH7-Wing`
> with `full-v1.1`: the `.apj` would not flash, the `_with_bl.hex` worked
> first try. Boards that shipped with Betaflight, or that have been
> flashed across firmware families, are the usual candidates.

## Full release fleet (`full-v1.2`)

The fork ships two parallel per-board release tracks — `full-v1.2`
(this branch) and `light-v1.2` (the light branch). Both carry the
**identical fork feature set**; they differ only in which hardware
backends are compiled in. A board's version string tells you which you
flashed: the light build appends ` Light`
(`... ArduCustom v1.2 Light (…)`), the full build has no suffix.

As of v1.1 the two tracks no longer overlap: **full is the F7 / H7 track,
light is the 1 MB F4 track.** A given board appears on exactly one of them,
so there is no longer any ambiguity about which asset to flash.

The **full** fleet is built from `master_custom_4.6.3` with every upstream
backend plus the fork additions, scoped to boards with the flash headroom
to carry all of it — H7 / F7 / 2 MB targets. (1 MB F4 boards overflow the
full build; flash those from the light release instead.)

Measured at **v1.2**:

| Board | MCU / Flash | Free after fork features | Status |
|---|---|---:|---|
| MatekF765-Wing | F7 / 2 MB | **519.1 KB** | Ships — Matek F765-WING (most flash headroom in the fleet) |
| KakuteH7-Wing | H7 / 2 MB | **103.1 KB** | Ships — Holybro Kakute H743 Wing |
| MatekH743-bdshot | H7 / 2 MB | **98.7 KB** | Ships — Matek H743-WING (bidirectional DShot) |
| TBS_LUCID_H7_OEM | H7 / 2 MB | **85.2 KB** | **New in v1.2** — TBS Lucid H7 OEM (FCAPv1 H743, D2FCAP schematic); tightest of the full fleet |

All four fit the full backend set with headroom to spare, though not equally.
`MatekF765-Wing` has five times the room of any H7, because the three H7 hwdefs
reserve more flash — `TBS_LUCID_H7_OEM` leaves only ~1664 KB usable of its 2 MB
part.

**`TBS_LUCID_H7_OEM` is the full track's canary**, the way
`MatekF405-Wing-bdshot` is light's — it is the first board here that a future
upstream rebase or feature add will push against. Note it is ~13 KB tighter than
the other two H7s partly by choice: this fork adds the MPU6000 probe upstream's
4.6.3 parent hwdef lacks, which costs ~7.8 KB but is what lets an
MPU6000-populated board boot at all.

From v1.1 these F7/H7 targets are **full-only**
— they were previously also built for the light release, which made sense
while light was the more complete track, but a 2 MB board has no reason to
run a backend-stripped build.

> **`TBS_LUCID_H7_OEM` is not the AT32 "Lucid Pro".** This is the real
> STM32H743 OEM variant of the Lucid H7, ported from upstream ArduPilot
> (board ID 5255). It differs from the base `TBS_LUCID_H7` in that the
> on-board MAX7456 analog OSD is depopulated — analog OSD is generated by
> an STM32G431 co-processor fed MSP DisplayPort over `SERIAL5`, so
> `OSD_TYPE` defaults to 5 (MSP DisplayPort) rather than 1 (MAX7456). The
> freed SPI2 pins become UART5. See
> [the upstream board README](libraries/AP_HAL_ChibiOS/hwdef/TBS_LUCID_H7_OEM/README.md).

## Light release fleet (`light-v1.2`) — 9 boards (all 1 MB F4)

Measured against the **light variant** at **v1.2** with radar +
LAND_WIND_DIST + LAND_WIND_STRICT on:

| Board | Flash | Free after fork features | vs v1.1 | Status |
|---|---:|---:|---:|---|
| KakuteF4-Wing-Buzz | 1 MB | **79.3 KB** | −0.3 | Ships — Kakute F4 Wing + tone buzzer (see below) |
| speedybeef4v3 | 1 MB | **50.2 KB** | −0.2 | Ships — most F4 headroom |
| MatekF405-STD | 1 MB | **44.6 KB** | −0.3 | Ships |
| omnibusf4pro | 1 MB | **39.5 KB** | −0.3 | Ships — Omnibus F4 Pro FPV FC |
| MatekF405-TE-bdshot | 1 MB | **39.0 KB** | −0.8 | Ships |
| AtomRCF405NAVI | 1 MB | **30.5 KB** | −0.4 | Ships — AtomRC F405 Navi FPV FC |
| SpeedyBeeF405WING | 1 MB | **26.9 KB** | −0.2 | Ships |
| SpeedyBeeF405WING-Buzz | 1 MB | **26.6 KB** | −0.2 | Ships — SpeedyBee F405 WING + tone buzzer (see below) |
| MatekF405-Wing-bdshot | 1 MB | **24.5 KB** | −0.2 | Ships — tightest of the F4 fleet |

The whole v1.2 feature set costs **0.2–0.8 KB** per board, so the ranking is
unchanged and no board moved materially. (`omnibusf4pro` and
`MatekF405-TE-bdshot` were tied at 39.8 KB in v1.1 and have now separated.)

`MatekF405-Wing-bdshot` is now the canary board — first to bump the
1 MB ceiling in any future upstream rebase or feature add. Worth
watching going forward. With the F7/H7 targets moved to full-only in v1.1,
every board on this track is flash-constrained, so the canary matters more
than it did before: nothing in this fleet has spare headroom to absorb a
regression.

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
to stock `SpeedyBeeF405WING` (26.6 KB vs 26.9 KB free on light at v1.2 —
a 0.3 KB difference). Shipped as an extra
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
- **Added (light only):** `AtomRCF405NAVI` (AtomRC **F405 Navi**, 1 MB F4) —
  stock upstream FPV board, no fork hwdef work needed. **44.2 KB free** on
  light, right in the middle of the F4 headroom band. 1 MB F4 → light-only
  (it overflows the full build, like the rest of the F4 fleet).
- Plus code (OLC) is now compiled into **every** light board, including the
  1 MB F4s (it was already on full and on the H7 light build). The ~2–3 KB
  cost trimmed each F4's free-flash figure above by roughly 1 KB versus
  v0.3-beta; every board still clears its ceiling (tightest is
  `MatekF405-Wing-bdshot` at 26.3 KB free).

**v1.1 fleet change vs v1.0:**
- **Removed from light:** `MatekF765-Wing` (F7 / 2 MB) and `KakuteH7-Wing`
  (H7 / 2 MB). Both continue to ship on **full**, unchanged. Carrying 2 MB
  targets on a backend-stripped track never bought anything — the light
  variant exists to fit 1 MB F4 flash, and these boards were never
  constrained. Users on those two boards should flash the **full** asset.
- The tracks are now cleanly split by MCU class: **full = F7/H7**
  (3 boards), **light = 1 MB F4** (9 boards). No board appears on both,
  so there is no longer a choice to get wrong.
- No board was added, and no F4 board changed status. Light goes 11 → 9
  purely by removing the two unconstrained targets.

**v1.2 fleet change vs v1.1:**
- **Added to full:** `TBS_LUCID_H7_OEM` (H7 / 2 MB) — ported from upstream
  ArduPilot, board ID 5255. Full goes **3 → 4 boards**; light is unchanged at
  9. It lands as the tightest board on the full track (**85.2 KB free**),
  so it takes over as that track's canary.
- No board was removed and no board changed track. The F7/H7-vs-1 MB-F4 split
  from v1.1 still holds, and no board appears on both.
- Feature cost across the whole release is **0.2–0.8 KB per board**, so every
  existing board's headroom is essentially where v1.1 left it.

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
| `FLIGHT_OPTIONS` 0 → **4096** (bit 12)                                                          | **New in v1.2.** LOITER pitch-stick altitude control is on out of the box. See [`FLIGHT_OPTIONS` bit 12](#flight_options-bit-12--fbw-b-style-loiter-altitude-control-upstream). Upgrades keep their saved value. |
| `RC_OPTIONS` default is `544` (bits 5+9) | Bit 20 (`AUTO_SWITCH_TO_FBWA_WITH_STICKS`) was **removed in v1.1**. Earlier builds defaulted to `1049120`, where a 10 % stick bump anywhere in AUTO silently ended the mission. The stick takeover now exists only during a takeoff, as an auto-launch cancel, and needs no option bit. A stale bit 20 in saved parameters is ignored. See [move sticks to cancel auto launch](#move-sticks-to-cancel-auto-launch). |
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
on `FLIGHT_OPTIONS` (21 / 22 / 23 / 24), `RC_OPTIONS` bits 21 / 22, the
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
- **[mmosca](https://github.com/mmosca)** — introduced the Walksnail
  head-tracker-over-MSP protocol (`MSP2_SENSOR_HEADTRACKER`) to the
  [iNav project](https://github.com/iNavFlight/inav). This fork's MSP
  head-tracking → serial-gimbal feature is an independent ArduPilot
  implementation of that protocol. See the
  [Walksnail Headtracking](#walksnail-headtracking-via-msp-and-serial-gimbal)
  section.
- **Upstream ArduPilot dev team** — the underlying platform.
- The `LAND_WIND_BIAS` / `LAND_WIND_DIST` / `LAND_WIND_STRICT`
  wind-aware DO_LAND_START selection family is original to this
  fork (not from any upstream contributor), built on top of
  AP_Mission's existing landing-sequence selection.
- Authoring assistance via Claude Code (Anthropic).
