# Build, calibration and tuning

## Mechanical build

### BASIC 2WD

Use two matching geared motors with identical wheel diameters. Put the drive wheels on the same axle line and a low-friction caster or skid at the opposite side.

### ADVANCED 4WD mecanum

Use four matching mecanum wheels and install them in the standard mirrored roller arrangement. Verify that the two front and two rear wheels are oriented as the wheel manufacturer's pattern requires.

Keep the battery low and near the robot center. A low center of gravity makes rapid starts/stops much easier to control.

## First electrical test

1. Lift the robot so the wheels cannot touch the floor.
2. Disconnect the kicker/dribbler power if possible.
3. Power the receiver and MCU.
4. Move the transmitter sticks slowly.
5. Confirm neutral sticks give zero motor command.
6. Test one direction at a time.
7. Confirm the kicker is disabled while the robot drive is being tested.
8. Only then place the robot on the floor.

## RC calibration

The sketches assume approximately:

- 1000 µs = full one direction
- 1500 µs = center
- 2000 µs = full opposite direction

If your transmitter endpoints differ, change `RC_MIN_US`, `RC_CENTER_US`, `RC_MAX_US` and the deadband in the sketch.

## Correcting a backwards motor

Do not immediately move wires around. First change:

- BASIC: `REVERSE_THROTTLE`, `REVERSE_STEERING`, `REVERSE_LEFT_MOTOR`, `REVERSE_RIGHT_MOTOR`
- ADVANCED: `REVERSE_STRAFE`, `REVERSE_FORWARD`, `REVERSE_ROTATE`, and the four `INV_*` motor flags

## Mecanum tuning order

1. Test forward only.
2. Test reverse only.
3. Test pure strafe.
4. Test pure rotation.
5. Test diagonal movement.
6. Check that combined commands do not saturate one motor unexpectedly.

If diagonal movement feels uneven, first check wheel orientation and motor gearboxes before changing software.

## Servo kicker tuning

Start with a small angle. Increase the kick angle gradually until the ball leaves the robot reliably. Never let the horn bind against the mechanical stop.

## Dribbler tuning

The dribbler motor should pull the ball inward without stalling continuously. If the motor gets hot quickly, reduce PWM or use a motor/gear ratio with more suitable torque.
