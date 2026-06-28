# Yaapu telemetry — Course Hold mode support

This fork adds **Course Hold** as Plane flight mode 26 (see
`README_CUSTOM.md` → Flight modes). Stock Yaapu telemetry (both the
on-radio script and the GCS desktop tool) ships with the upstream
Plane mode list, which stops at mode 25 (`Thermal`) — so Course Hold
shows up as a blank / unknown mode name on Frsky-equipped radios and
the audio cue for mode-change announces nothing.

The two files in this directory patch that:

- **`plane.lua`** — drop-in replacement of Yaapu's plane-mode-name
  table. Adds `CourseHold` at index 27 (Yaapu indexes
  `flight_mode + 1`, so index 27 = our mode 26 = `COURSE_HOLD`).
- **`coursehold.wav`** — 32 kHz mono PCM speech cue that Yaapu plays
  when the mode changes into Course Hold.

> **AUTO_TRIM (mode 27):** not currently in `plane.lua`. The fork's
> `AUTO_TRIM` flight mode (Plane mode 27, Yaapu index 28) will still
> show blank in Yaapu. Open an issue if you want it added.

## Install — Yaapu on-radio telemetry script (Frsky / OpenTX / EdgeTX)

On the radio's SD card:

1. Replace `SCRIPTS/YAAPU/LIB/plane.lua` with **this** `plane.lua`.
2. Delete `SCRIPTS/YAAPU/LIB/plane.luac` (Yaapu's pre-compiled cache —
   it'll regenerate from the new `.lua` on next run).
3. Copy `coursehold.wav` into `SOUNDS/yaapu0/en/`.

## Install — Yaapu GCS (desktop)

On the GCS install directory:

1. Replace `SCRIPTS/TOOLS/yaapu/plane.lua` with **this** `plane.lua`.
2. Delete `SCRIPTS/TOOLS/yaapu/plane.luac`.

The desktop GCS uses the same WAV path lookup as the on-radio script,
so step 3 above (copying `coursehold.wav`) covers GCS too if both are
installed on the same machine.

## Verify

After install, switch into Course Hold on the radio. Expect:

- Yaapu HUD shows `CourseHold` in the top-right mode chip.
- Audio cue says "course hold" (or whatever your `coursehold.wav`
  contains — feel free to record your own at 32 kHz mono PCM).

## Source

Both files were provided by the fork user — `plane.lua` is a
straightforward extension of Yaapu's stock table with one new
mode entry; `coursehold.wav` is a custom-recorded sound cue.

These files are unrelated to ArduPilot's `AP_Scripting` Lua subsystem
(which runs inside the autopilot). They live entirely on the
**radio side** (transmitter) or **GCS host**.
