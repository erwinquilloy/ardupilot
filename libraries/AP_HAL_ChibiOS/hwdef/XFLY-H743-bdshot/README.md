# XFLY H743 Flight Controller (ArduCustom)

The XFLY H743 is the STM32H743 flight controller shipped in the XFly VTOL
kits as the "Flight Controller Combo" (part XFW005-14), mounted on the kit's
wiring board. ArduCustom ships it on the **full** track as `XFLY-H743-bdshot`
only: the same hardware definition as the plain `XFLY-H743` target, plus
bidirectional DShot on the motor outputs.

The hardware definition is the manufacturer's own, recovered from the stock
firmware, with three ArduCustom changes: the serial port numbering (see
below), `Q_ENABLE` / `SCHED_LOOP_RATE` moved into `defaults.parm` (they have
no effect as hwdef `define`s), and the bidirectional DShot tagging.

## Features

- MCU - STM32H743, 480 MHz, 2 MB flash
- IMU - Dual ICM42688 (SPI1 and SPI4), with IMU heater
- Barometer - Dual DPS310 (one per I2C bus)
- Compass - IST8310 (internal and external buses are probed)
- OSD - AT7456E analog OSD
- microSD card slot (SDMMC1)
- 6x UARTs plus USB
- CAN (CAN1)
- 2x I2C (I2C2, I2C4)
- 16x PWM outputs, bidirectional DShot on 1-8
- Two analog battery monitors

## Bootloader and board ID

The board ID is **8888**, the same as the manufacturer's firmware, so a board
still on its stock bootloader accepts the `.apj` from Mission Planner's
"Load custom firmware" directly. To (re)install the bootloader, flash
`arduplane_with_bl.hex` over DFU.

## UART mapping: read this before wiring anything

ArduCustom numbers serial ports on this board so that **`SERIALn` is chip
UART `n`**, the same convention it uses on `MatekH743` and the TBS Lucid H7. The manufacturer's stock
firmware numbers them differently, and **the port labels on the XFly wiring
board ("URT1" ... "URT5") follow the stock numbering, not ArduCustom's**. Use
this table to find the right `SERIALn` for each port:

| Wiring board label | Chip UART | **ArduCustom** | Stock firmware | Suggested `SERIALn_PROTOCOL` |
|---|---|---|---|---|
| USB | OTG1 | **SERIAL0** | SERIAL0 | MAVLink2 (default) |
| GPS1 URT3 | USART1 | **SERIAL1** | SERIAL3 | 5 (GPS) |
| RCIN URT5 | USART2 | **SERIAL2** | SERIAL5 | 23 (RCIN) |
| LINK URT2 | USART3 | **SERIAL3** | SERIAL2 | 2 (MAVLink2) |
| LINK URT1 | UART5 | **SERIAL5** | SERIAL1 | 2 (MAVLink2) |
| GPS2 URT4 | UART7 | **SERIAL7** | SERIAL4 | 5 (GPS) |
| DJI O3 | UART8 | **SERIAL8** | SERIAL6 | 42 (DisplayPort) |

`SERIAL4`, `SERIAL6` and `SERIAL9` are empty placeholders: this board has no
UART4, UART6 or a ninth UART. Setting a protocol on them does nothing.

Coming from the stock firmware, move every `SERIALn_*` setting (protocol,
baud, options) to its new number from the table above; a stock parameter file
loaded unchanged will put the GPS, receiver and telemetry on the wrong ports.

## RC input

The receiver goes on the "RCIN URT5" port, which is **`SERIAL2`** here. Set
`SERIAL2_PROTOCOL = 23`. CRSF/ELRS, SBUS (with `SERIAL2_OPTIONS` inversion as
needed) and the other serial RC protocols are auto-detected.

## PWM outputs

| Outputs | Timer | Bidirectional DShot | Notes |
|---|---|---|---|
| 1-4 | TIM1 | Yes | |
| 5-6 | TIM4 | Yes | |
| 7-8 | TIM3 | Yes | |
| 9-10 | TIM12 | No | PWM only, no DMA so no DShot at all |
| 11-12 | TIM8 | No | PWM or DShot |
| 13-16 | TIM2 | No | PWM or DShot |

Outputs sharing a timer must use the same protocol and rate. Enable
bidirectional telemetry with `SERVO_BLH_BDMASK` on the motor outputs, plus
`SERVO_BLH_OTYPE` set to a DShot type and `SERVO_BLH_POLES` for your motors.

The function labels on the wiring board (MOTOR-01 ... MOTOR-05, ailerons,
V-tail) come from the kit's stock parameters. Confirm each output's
`SERVOn_FUNCTION` with the Mission Planner motor and servo tests before
flying.

## Battery monitoring

Defaults: `BATT_MONITOR = 4`, voltage on pin 11 with `BATT_VOLT_MULT = 11`,
current on pin 10 with `BATT_AMP_PERVLT = 33.33`. A second monitor is defined
on pins 4 (voltage, scale 3.2) and 8 (current, scale 1.0). All values are the
manufacturer's; calibrate the current scale against a meter.

## I2C and CAN

Two I2C buses (I2C2 on PB10/PB11 and I2C4 on PD12/PD13). I2C rangefinders,
compasses and airspeed sensors are probed on both. The two CAN connectors
("CAN" and "CAN airspeed") are the one CAN1 bus; set `CAN_P1_DRIVER = 1` and
`CAN_D1_PROTOCOL = 1` (DroneCAN) to use them.

## Defaults

`defaults.parm` sets `Q_ENABLE = 1` (the kits are VTOLs) and
`SCHED_LOOP_RATE = 100`.
