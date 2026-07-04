# SpeedyBee F405 WING (custom: hardware tone buzzer on S11)

This is a fork-only variant of the stock `SpeedyBeeF405WING` target. It is
**identical** to that board except it adds a hardware **ToneAlarm** (pitched
tone) buzzer on the **S11** servo pad, so the flight controller can play the
full ArduPilot tunes (arming, GPS lock, auto-takeoff/launch, failsafe, etc.)
on a dedicated buzzer instead of routing them through the DShot ESCs.

## What changed vs. stock SpeedyBeeF405WING

* PWM output 11 (pad **S11**, MCU pin **PB15**) is moved off the TIM1 PWM
  group onto **TIM12_CH2** and marked `ALARM`. This defines `HAL_PWM_ALARM`,
  enabling `AP_ToneAlarm` on that pin.
* TIM12 is otherwise unused on this board, so the remaining PWM outputs
  (TIM1/2/3/4/8) are unaffected.
* `SERVO11_FUNCTION` is defaulted to 0 (disabled) since S11 is now the buzzer.
* The stock active buzzer on PC15 is left in place; you can ignore it or
  remove `HAL_BUZZER_PIN` if you don't want two noise-makers.
* Board ID is inherited (**1106**), so the resulting `.apj` flashes over the
  stock SpeedyBee bootloader as a normal firmware update — no new bootloader.

## Cost

One servo output (S11 / channel 11) is given up for the buzzer. This board
still exposes outputs 1-10 and 12 (12 drives the WS2812 LED by default).

## Wiring a passive buzzer to S11

Use a **passive** buzzer (a bare piezo, or a magnetic passive buzzer). Do NOT
use an active buzzer here — it has a fixed internal oscillator and cannot play
tones.

### Passive piezo, direct drive (recommended — e.g. LS3040)

The LS3040 (passive piezo, ~4 kHz resonance, rated **2 mA**, 1-30 V) draws far
less than the MCU pin's limit, so wire it straight to the pad — no transistor,
no diode (a piezo is capacitive, not inductive):

    S11 (PB15) ------[ LS3040 piezo ]------ GND

Polarity does not matter. Driven off the 3.3 V pin swing it will be a bit
quieter than its rated 80 dB but is normally plenty audible. If you need more
volume, use the transistor stage below to switch the full 5 V rail (still no
diode — it is not a coil).

### Magnetic passive buzzer, or louder piezo (transistor-driven)

Drive it through a small NPN transistor (2N2222 / S8050 / MMBT2222) or a
logic-level N-MOSFET, powered from the servo-rail 5V:

    5V (servo rail) ---+----------------+
                       |                |
                  [ buzzer ]        1N4148  (cathode/band -> 5V)
                       |                |
                       +----------------+
                       |
                   collector
    S11 (PB15) -[1k]- base    NPN (2N2222 / S8050)
                   emitter
                       |
             [10k] ----+   (base->GND pulldown: keeps buzzer
                       |    silent during boot)
                      GND

* Buzzer high side -> servo-rail 5V (confirm the rail is powered by the BEC).
* Buzzer low side  -> transistor collector/drain.
* Transistor base/gate -> S11 via the 1k (use ~100R for a MOSFET gate).
* 10k pulldown from base/gate to GND.
* 1N4148 flyback diode across the magnetic buzzer, band (cathode) to 5V.
  (Omit the diode for a bare piezo.)

## Building

    ./waf configure --board SpeedyBeeF405WING-Buzz
    ./waf plane

Flash `build/SpeedyBeeF405WING-Buzz/bin/arduplane.apj` via Mission Planner
(Install Firmware -> Load custom firmware). It flashes over the existing
SpeedyBee bootloader because the board ID matches (1106).
