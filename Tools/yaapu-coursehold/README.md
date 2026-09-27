# Yaapu telemetry — Course Hold and Auto Trim mode support

This fork adds **Course Hold** (Plane flight mode 26) and **Auto Trim**
(mode 27) — see `README_CUSTOM.md` → Flight modes. Stock Yaapu telemetry
(both the on-radio script and the GCS desktop tool) ships with the
upstream Plane mode list, which knows neither: Course Hold shows up as a
blank mode name with no audio cue, and the stock table has no entry at
all for mode 27, which stops the Yaapu script in Auto Trim.

The files in this directory patch that:

- **`plane.lua`** — drop-in replacement of Yaapu's plane-mode-name
  table. Adds `CourseHold` at index 27 and `AutoTrim` at index 28
  (Yaapu indexes `flight_mode + 1`, so index 27 = our mode 26 =
  `COURSE_HOLD`, index 28 = mode 27 = `AUTO_TRIM`).
- **`coursehold.wav`** and **`autotrim.wav`** — 32 kHz mono PCM speech
  cues that Yaapu plays when the mode changes into Course Hold or Auto
  Trim.

> ⚠️ **Flying Auto Trim with stock Yaapu (or an older copy of this
> `plane.lua`) stops the Yaapu script** — the mode has no table entry.
> Install this `plane.lua` before you put mode 27 on a switch.

## Install — Yaapu on-radio telemetry script

**The sound folder depends on which Yaapu build you run, and getting it
wrong is silent — the mode name still appears on the HUD, you just never
hear the cue.** Yaapu derives the WAV filename from the same table entry
as the display name, so a correct `plane.lua` with a misplaced WAV gives
you exactly that: right name, no audio.

| Build | `coursehold.wav` and `autotrim.wav` go in |
|---|---|
| EdgeTX widget (TX16S) | `WIDGETS/yaapu/sounds/en/` |
| OpenTX / Horus script | `SOUNDS/yaapu0/en/` |
| FrSky Ethos | `scripts/yaaputelemetry/audio/en/` |

Substitute your language folder for `en` throughout.

### EdgeTX widget (e.g. RadioMaster TX16S) — verified

On the radio's SD card:

1. Replace Yaapu's `plane.lua` under your `WIDGETS/yaapu/` install with
   **this** `plane.lua`.
2. Delete the matching `plane.luac` alongside it (Yaapu's pre-compiled
   cache — it regenerates from the new `.lua` on next run).
3. Copy `coursehold.wav` and `autotrim.wav` into **`WIDGETS/yaapu/sounds/en/`**.

Use your language folder in place of `en` if your radio isn't English.
Filename case doesn't matter — the SD card is FAT.

### Older OpenTX / Horus script layout

The pre-widget script build keeps its files elsewhere:

1. Replace `SCRIPTS/YAAPU/LIB/plane.lua` with **this** `plane.lua`.
2. Delete `SCRIPTS/YAAPU/LIB/plane.luac`.
3. Copy `coursehold.wav` and `autotrim.wav` into **`SOUNDS/yaapu0/en/`**.

### FrSky Ethos

Ethos uses a third layout — neither path above applies:

1. Replace Yaapu's `plane.lua` in your `scripts/yaaputelemetry/` install
   with **this** `plane.lua`, and delete any `plane.luac` beside it.
2. Copy `coursehold.wav` and `autotrim.wav` into **`scripts/yaaputelemetry/audio/en/`**.

Note this is `audio/`, not `sounds/`, and it lives under `scripts/`
rather than `WIDGETS/`.

## Install — Yaapu GCS (desktop)

On the GCS install directory:

1. Replace `SCRIPTS/TOOLS/yaapu/plane.lua` with **this** `plane.lua`.
2. Delete `SCRIPTS/TOOLS/yaapu/plane.luac`.

The desktop GCS uses the same WAV path lookup as the on-radio script,
so copying the WAVs as described above covers GCS too if both
are installed on the same machine.

## Verify

After install, switch into Course Hold, then Auto Trim, on the radio. Expect:

- Yaapu HUD shows `CourseHold`, then `AutoTrim`, in the top-right mode chip.
- Audio cues say "course hold" and "auto trim" (or whatever your WAVs
  contain — feel free to record your own at 32 kHz mono PCM, named
  `coursehold.wav` / `autotrim.wav`).

**If the name shows but there's no sound**, the `plane.lua` edit landed
and the WAV is simply in a folder Yaapu isn't reading — re-check the
sound path for *your* build above. That's the common failure, and it
looks like a broken audio cue rather than a misplaced file.

## Source

All three files were provided by the fork user — `plane.lua` is a
straightforward extension of Yaapu's stock table with two new
mode entries; `coursehold.wav` and `autotrim.wav` are custom-recorded
sound cues.

These files are unrelated to ArduPilot's `AP_Scripting` Lua subsystem
(which runs inside the autopilot). They live entirely on the
**radio side** (transmitter) or **GCS host**.
