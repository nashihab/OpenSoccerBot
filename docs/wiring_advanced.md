# ADVANCED ESP32 wiring — 4WD mecanum

This version uses two TB6612FNG boards. Each board drives two motors.

## RC receiver PWM inputs

| Receiver | ESP32 | Function |
|---|---:|---|
| CH1 | GPIO34 | Strafe |
| CH2 | GPIO35 | Forward/reverse |
| CH4 | GPIO36 | Rotate |
| CH5 | GPIO39 | Kick |
| CH6 | GPIO13 | Dribbler |
| 5 V | regulated receiver rail | Receiver power |
| GND | GND | Common ground |

**ESP32 signal warning:** the ESP32 GPIOs are not 5 V tolerant. Verify the receiver output level. Add a proper divider or level shifter when required.

## TB6612 board A

| Motor | PWM | IN1 | IN2 |
|---|---:|---:|---:|
| Front-left | GPIO25 | GPIO4 | GPIO16 |
| Front-right | GPIO26 | GPIO17 | GPIO18 |

## TB6612 board B

| Motor | PWM | IN1 | IN2 |
|---|---:|---:|---:|
| Rear-left | GPIO27 | GPIO19 | GPIO21 |
| Rear-right | GPIO14 | GPIO22 | GPIO23 |

### Driver supplies

- VCC: 3.3 V logic rail
- VM: battery motor rail
- GND: common GND
- STBY: tie to 3.3 V for the current firmware

Toshiba specifies the TB6612FNG VCC operating range as 2.7–5.5 V and VM as 2.5–13.5 V. Output current is specified as 1.2 A average per channel and 3.2 A peak for short pulses under the datasheet conditions. Verify your actual breakout and motor current before use. 

Reference: https://toshiba.semicon-storage.com/info/docget.jsp?did=10660

## Dribbler

Use a small brushed motor through a suitable logic-level MOSFET stage:

```text
ESP32 GPIO33 → MOSFET input/gate
Battery/motor rail → dribbler motor → MOSFET switched return
```

Add a flyback diode across a bare brushed DC motor if the MOSFET module does not already include suitable flyback protection.

## Kicker servo

- Signal: GPIO32
- +5 V: regulated 5 V/BEC
- GND: common GND

Do not power the servo from an ESP32 I/O pin.

## ESP32 power

Power the DevKit from its intended regulated 5 V/VIN input according to the particular board version. Do not connect the raw 2S/3S/4S motor battery directly to a 3.3 V pin.
