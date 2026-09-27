# RC controller compatibility

This project uses the receiver's **individual servo-style PWM outputs** as the base control interface.

## Example: FlySky

A practical example is:

- FlySky FS-i6 / FS-i6X transmitter
- FlySky FS-iA6B receiver

FlySky lists the FS-iA6B with six PWM channels and PWM/PPM/i.BUS/S.BUS interfaces. Its specified receiver supply range is 4.0–8.4 V. Official reference:

https://www.flyskytech.com/parts_detail/42.html

## BASIC channel map

| Channel | Function | Firmware input |
|---|---|---|
| CH1 | Steering | Uno D2 |
| CH2 | Throttle | Uno D3 |
| CH5 | Kick | Uno A0 |

## ADVANCED channel map

| Channel | Function | Firmware input |
|---|---|---|
| CH1 | Strafe | ESP32 GPIO34 |
| CH2 | Forward / reverse | ESP32 GPIO35 |
| CH4 | Rotate | ESP32 GPIO36 |
| CH5 | Kick | ESP32 GPIO39 |
| CH6 | Dribbler | ESP32 GPIO13 |

## Using another RC system

A different radio is acceptable when the receiver provides compatible PWM outputs.

Before wiring it to the robot, verify:

1. Receiver output type is PWM.
2. Pulse endpoints are compatible with the sketch, or are calibrated in the sketch.
3. Receiver supply voltage follows the receiver manufacturer's specification.
4. Grounds are common.
5. **ESP32 signal level:** GPIOs are not 5 V tolerant. If a receiver output can exceed 3.3 V, use a proper logic-level interface before the GPIO.

The base sketches do not decode i.BUS, S.BUS or PPM. Those can be added later as a separate receiver-interface layer.
