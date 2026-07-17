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

## Install — Yaapu on-radio telemetry script

**The sound folder depends on which Yaapu build you run, and getting it
wrong is silent — the mode name still appears on the HUD, you just never
hear the cue.** Yaapu derives the WAV filename from the same table entry
as the display name, so a correct `plane.lua` with a misplaced WAV gives
you exactly that: right name, no audio.

### EdgeTX widget (e.g. RadioMaster TX16S) — verified

On the radio's SD card:

1. Replace Yaapu's `plane.lua` under your `WIDGETS/yaapu/` install with
   **this** `plane.lua`.
2. Delete the matching `plane.luac` alongside it (Yaapu's pre-compiled
   cache — it regenerates from the new `.lua` on next run).
3. Copy `coursehold.wav` into **`WIDGETS/yaapu/sounds/en/`**.

Use your language folder in place of `en` if your radio isn't English.
Filename case doesn't matter — the SD card is FAT.

### Older OpenTX / Horus script layout

The pre-widget script build keeps its files elsewhere:

1. Replace `SCRIPTS/YAAPU/LIB/plane.lua` with **this** `plane.lua`.
2. Delete `SCRIPTS/YAAPU/LIB/plane.luac`.
3. Copy `coursehold.wav` into **`SOUNDS/yaapu0/en/`**.

> ⚠️ **FrSky Ethos** uses a different layout again — neither path above
> applies. Check your Yaapu Ethos install before copying.

## Install — Yaapu GCS (desktop)

On the GCS install directory:

1. Replace `SCRIPTS/TOOLS/yaapu/plane.lua` with **this** `plane.lua`.
2. Delete `SCRIPTS/TOOLS/yaapu/plane.luac`.

The desktop GCS uses the same WAV path lookup as the on-radio script,
so copying `coursehold.wav` as described above covers GCS too if both
are installed on the same machine.

## Verify

After install, switch into Course Hold on the radio. Expect:

- Yaapu HUD shows `CourseHold` in the top-right mode chip.
- Audio cue says "course hold" (or whatever your `coursehold.wav`
  contains — feel free to record your own at 32 kHz mono PCM).

**If the name shows but there's no sound**, the `plane.lua` edit landed
and the WAV is simply in a folder Yaapu isn't reading — re-check the
sound path for *your* build above. That's the common failure, and it
looks like a broken audio cue rather than a misplaced file.

## Source

Both files were provided by the fork user — `plane.lua` is a
straightforward extension of Yaapu's stock table with one new
mode entry; `coursehold.wav` is a custom-recorded sound cue.

These files are unrelated to ArduPilot's `AP_Scripting` Lua subsystem
(which runs inside the autopilot). They live entirely on the
**radio side** (transmitter) or **GCS host**.
