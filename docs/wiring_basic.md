# BASIC UNO wiring — 2WD

## Signal side

### RC receiver

Use the receiver's individual PWM outputs. The example channel assignment is:

| Receiver | Arduino Uno | Function |
|---|---:|---|
| CH1 | D2 | Steering |
| CH2 | D3 | Throttle |
| CH5 | A0 | Kick switch |
| 5V | 5V regulated rail | Receiver power |
| GND | GND | Common ground |

The code is intentionally not tied to FlySky. Any receiver that produces standard servo-style PWM pulses can be used by changing the three input pins in the sketch.

## Motor driver — TB6612FNG

| TB6612FNG | Uno |
|---|---:|
| PWMA | D5 |
| AIN1 | D7 |
| AIN2 | D8 |
| PWMB | D6 |
| BIN1 | D9 |
| BIN2 | D10 |
| STBY | D4 |
| VCC | 5 V |
| VM | Battery motor rail |
| GND | Common GND |

Connect motor A to the left side and motor B to the right side. If either side spins the wrong way, use the `REVERSE_*_MOTOR` flags in the sketch rather than swapping wires.

## Kicker servo

- Signal: A3
- +5 V: regulated 5 V/BEC
- GND: common GND

Do **not** power a servo from an overloaded USB/logic rail. A small buck/BEC with sufficient current headroom is strongly recommended.

## Driver limits

Toshiba specifies the TB6612FNG VCC operating range as 2.7–5.5 V and VM as 2.5–13.5 V, with 1.2 A average output current per channel and short-duration higher peak capability under datasheet conditions. Check the actual motor stall current before selecting this driver.

Reference: https://toshiba.semicon-storage.com/info/docget.jsp?did=10660
