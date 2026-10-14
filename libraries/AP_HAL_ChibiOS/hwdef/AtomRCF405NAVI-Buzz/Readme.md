# AtomRCF405NAVI-Buzz (fork tone-buzzer variant)

A fork-only variant of `AtomRCF405NAVI` that plays ArduPilot's full pitched
tunes (arming, GPS lock, launch cues, failsafe) on a **passive** buzzer. The
stock buzzer output (`PC5`) has no hardware timer, so it can only beep on and
off at one pitch.

## What changed vs stock AtomRCF405NAVI

- The WS2812 **LED** pad (next to SBUS, `PB7`, servo output 9) is marked
  `ALARM` on `TIM4_CH2`. TIM4 drives nothing else on this board.
- **Cost:** the LED-strip output. Servo/motor outputs 1-8 are unchanged.
- The stock `PC5` buzzer output still works. With an active buzzer on it
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
