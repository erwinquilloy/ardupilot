# CoreWingF405WMiniV2-Buzz (fork tone-buzzer variant)

A fork-only variant of `CoreWingF405WMiniV2` that plays ArduPilot's full pitched
tunes (arming, GPS lock, launch cues, failsafe) on a **passive** buzzer. The
stock buzzer output (`PC15`) has no hardware timer, so it can only beep on
and off at one pitch.

## What changed vs stock CoreWingF405WMiniV2

- The **LED** pad (`PA8`, output 12) is marked `ALARM` on `TIM1_CH1`.
- Output 11 (`PB15`) moves from TIM1 to `TIM12_CH2`, because a tone timer
  can drive nothing else. Outputs 1-11 are unchanged (output 11, the S11 pad, now runs on TIM12, still plain PWM as on the stock board).
- **Cost:** output 12 only. On the stock board that pad can't run an LED strip
  in ArduPilot (no DMA stream is left for TIM1), so it was a plain PWM pad.
- The stock `PC15` buzzer output still works. With an active buzzer on it
  as well you hear both; set `NTF_BUZZ_PIN -1` to silence it.
- Board ID is inherited, so the `.apj` flashes as a normal update.

## Wiring

```
LED pad ------[ passive piezo, e.g. LS3040 ]------ GND
```

A bare piezo needs no transistor and no diode, and polarity doesn't matter.
For a louder or magnetic buzzer, drive it through an NPN transistor from 5 V
(1 kΩ base resistor, 10 kΩ base pull-down, 1N4148 across a coil buzzer).
An **active** (self-oscillating) buzzer won't play pitches.

Everything else (ports, sensors, OSD, DShot) is as on the stock board; see
[its README](../CoreWingF405WMiniV2/README.md).
