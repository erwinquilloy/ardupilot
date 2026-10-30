# SpeedyBee F405 WING (custom: tone buzzer on S11 + LED segments)

Fork-only variant of `SpeedyBeeF405WING-Buzz` (see its Readme for the S11
buzzer wiring). It adds INAV-style addressable LED segments to the WS2812
strip on the **LED** pad (output 12, `SERVO12_FUNCTION 120` by default).

* `NTF_LED_LEN` = number of LEDs, `NTF_LS_ENABLE 1` to draw segments.
* Up to 8 segments `NTF_LS1_*` .. `NTF_LS8_*` (START/LEN/FUNC/COLOR/OVL):
  fixed colour, flight mode (`NTF_LS_MODECLR`), arm state, battery, GPS,
  link quality, throttle or the stock status pattern, with blink, strobe,
  scanner, failsafe-warning, bar, reverse and armed-only effects.
* The strip is the only output on TIM1, so it uses a 1-channel DMA buffer:
  about 108 bytes of RAM per LED (26 LEDs ~2.8 KB).
* Name: `SpeedyBeeF405WING-Buzz-LED` is over the 23-character board-name
  limit, hence `-LED`. The buzzer on S11 is included.

Costs ~2.5 KB of flash compared with `SpeedyBeeF405WING-Buzz`.
