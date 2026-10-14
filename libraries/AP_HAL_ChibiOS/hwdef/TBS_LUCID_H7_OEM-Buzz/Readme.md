# TBS_LUCID_H7_OEM-Buzz (fork tone-buzzer variant)

A fork-only variant of `TBS_LUCID_H7_OEM` that plays ArduPilot's full pitched
tunes (arming, GPS lock, launch cues, failsafe) on a **passive** buzzer. The
stock buzzer output (`PA15`) shares its only timer with motor output 1, so it
can only beep on and off at one pitch.

## What changed vs stock TBS_LUCID_H7_OEM

- The **S13/LED** pad (`PA8`, servo output 13; brought out on the PDB as
  "S13/LED") is marked `ALARM` on `TIM1_CH1`. TIM1 drives nothing else.
- **Cost:** the LED-strip output. Outputs 1-12 are unchanged.
  `defaults.parm` leaves `SERVO13_FUNCTION` at 0 (stock sets 120).
- The stock `PA15` buzzer output still works. With an active buzzer on it
  as well you hear both; set `NTF_BUZZ_PIN -1` to silence it.
- Board ID is inherited, so the `.apj` flashes as a normal update.

## Wiring

```
S13/LED ------[ passive piezo, e.g. LS3040 ]------ GND
```

A bare piezo needs no transistor and no diode, and polarity doesn't matter.
For a louder or magnetic buzzer, drive it through an NPN transistor from 5 V
(1 kΩ base resistor, 10 kΩ base pull-down, 1N4148 across a coil buzzer).
An **active** (self-oscillating) buzzer won't play pitches.
