/*
  SoccerBot ADVANCED — ESP32 + standard RC receiver
  Manual-control version only. No autonomy, no ball tracking.

  Four-wheel mecanum / omni drive + dribbler + servo kicker.

  Default mapping for a typical 6-channel RC transmitter:
    CH1 = strafe left/right (aileron)
    CH2 = forward/reverse (elevator)
    CH4 = rotate left/right (rudder)
    CH5 = kick switch
    CH6 = dribbler switch

  This code reads ordinary servo PWM pulses, so the robot is not tied to
  one transmitter brand. A FlySky FS-i6/i6X + FS-iA6B is one example.

  Required library:
    ESP32Servo
*/

#include <ESP32Servo.h>

// ---------------- RECEIVER INPUTS ----------------
const uint8_t RC_CH1_PIN = 34;  // strafe
const uint8_t RC_CH2_PIN = 35;  // forward/reverse
const uint8_t RC_CH4_PIN = 36;  // rotate
const uint8_t RC_CH5_PIN = 39;  // kick
const uint8_t RC_CH6_PIN = 13;  // dribbler

// ---------------- MOTOR OUTPUTS ----------------
// Driver A: FL + FR
const uint8_t FL_PWM = 25;
const uint8_t FL_IN1 = 4;
const uint8_t FL_IN2 = 16;
const uint8_t FR_PWM = 26;
const uint8_t FR_IN1 = 17;
const uint8_t FR_IN2 = 18;

// Driver B: RL + RR
const uint8_t RL_PWM = 27;
const uint8_t RL_IN1 = 19;
const uint8_t RL_IN2 = 21;
const uint8_t RR_PWM = 14;
const uint8_t RR_IN1 = 22;
const uint8_t RR_IN2 = 23;

// Tie both TB6612 STBY pins to 3.3 V. This keeps the wiring simple.
// Both motor-driver boards must share GND with the ESP32.

const uint8_t KICK_SERVO_PIN = 32;
const uint8_t DRIBBLER_PIN = 33;  // logic-level MOSFET gate/module input

// ---------------- RC TUNING ----------------
const uint16_t RC_MIN_US = 1000;
const uint16_t RC_CENTER_US = 1500;
const uint16_t RC_MAX_US = 2000;
const uint16_t RC_DEADBAND_US = 45;
const uint32_t DRIVE_TIMEOUT_US = 5500;
const uint16_t RC_FAILSAFE_MS = 120;

// Reverse a channel here if your transmitter orientation is opposite.
const bool REVERSE_STRAFE = false;
const bool REVERSE_FORWARD = false;
const bool REVERSE_ROTATE = false;

// Reverse individual motor directions here after the first wheel-off-ground test.
const bool INV_FL = false;
const bool INV_FR = true;
const bool INV_RL = false;
const bool INV_RR = true;

const uint8_t MAX_DRIVE_PWM = 235;
const uint8_t KICK_REST = 25;
const uint8_t KICK_ANGLE = 115;
const uint32_t KICK_COOLDOWN_MS = 1000;
const uint32_t KICK_HOLD_MS = 130;

Servo kicker;
uint32_t lastGoodDriveMs = 0;
uint32_t lastKickMs = 0;
uint32_t kickStartMs = 0;
bool kicking = false;

// ---------------- HELPERS ----------------
uint16_t readPulse(uint8_t pin) {
  return (uint16_t)pulseIn(pin, HIGH, DRIVE_TIMEOUT_US);
}

int rcAxis(uint16_t pulseUs, bool reverseAxis) {
  if (pulseUs == 0) return 0;
  pulseUs = constrain(pulseUs, RC_MIN_US, RC_MAX_US);
  int value = (int)pulseUs - (int)RC_CENTER_US;
  if (abs(value) <= (int)RC_DEADBAND_US) value = 0;

  long out = 0;
  if (value > 0) out = map(value, RC_DEADBAND_US, RC_MAX_US - RC_CENTER_US, 0, 1000);
  else if (value < 0) out = map(value, -(RC_CENTER_US - RC_MIN_US), -RC_DEADBAND_US, -1000, 0);

  out = constrain(out, -1000L, 1000L);
  return reverseAxis ? -(int)out : (int)out;
}

void setMotor(uint8_t pwmPin, uint8_t in1, uint8_t in2, int command, bool invert) {
  if (invert) command = -command;
  command = constrain(command, -255, 255);

  if (command > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (command < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  analogWrite(pwmPin, abs(command));
}

void stopDrive() {
  setMotor(FL_PWM, FL_IN1, FL_IN2, 0, INV_FL);
  setMotor(FR_PWM, FR_IN1, FR_IN2, 0, INV_FR);
  setMotor(RL_PWM, RL_IN1, RL_IN2, 0, INV_RL);
  setMotor(RR_PWM, RR_IN1, RR_IN2, 0, INV_RR);
}

void driveMecanum(int strafe, int forward, int rotate) {
  float fl = (float)forward + strafe + rotate;
  float fr = (float)forward - strafe - rotate;
  float rl = (float)forward - strafe + rotate;
  float rr = (float)forward + strafe - rotate;

  float peak = max(1000.0f, max(max(abs(fl), abs(fr)), max(abs(rl), abs(rr))));

  int flCmd = (int)(fl * MAX_DRIVE_PWM / peak);
  int frCmd = (int)(fr * MAX_DRIVE_PWM / peak);
  int rlCmd = (int)(rl * MAX_DRIVE_PWM / peak);
  int rrCmd = (int)(rr * MAX_DRIVE_PWM / peak);

  setMotor(FL_PWM, FL_IN1, FL_IN2, flCmd, INV_FL);
  setMotor(FR_PWM, FR_IN1, FR_IN2, frCmd, INV_FR);
  setMotor(RL_PWM, RL_IN1, RL_IN2, rlCmd, INV_RL);
  setMotor(RR_PWM, RR_IN1, RR_IN2, rrCmd, INV_RR);
}

void setDribbler(bool on) {
  digitalWrite(DRIBBLER_PIN, on ? HIGH : LOW);
}

void startKick() {
  uint32_t now = millis();
  if (kicking || (now - lastKickMs < KICK_COOLDOWN_MS)) return;
  stopDrive();
  setDribbler(false);
  kicker.write(KICK_ANGLE);
  kickStartMs = now;
  lastKickMs = now;
  kicking = true;
}

void updateKick() {
  if (kicking && millis() - kickStartMs >= KICK_HOLD_MS) {
    kicker.write(KICK_REST);
    kicking = false;
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(RC_CH1_PIN, INPUT);
  pinMode(RC_CH2_PIN, INPUT);
  pinMode(RC_CH4_PIN, INPUT);
  pinMode(RC_CH5_PIN, INPUT);
  pinMode(RC_CH6_PIN, INPUT);

  const uint8_t outputPins[] = {
    FL_PWM, FL_IN1, FL_IN2, FR_PWM, FR_IN1, FR_IN2,
    RL_PWM, RL_IN1, RL_IN2, RR_PWM, RR_IN1, RR_IN2,
    DRIBBLER_PIN
  };
  for (uint8_t pin : outputPins) pinMode(pin, OUTPUT);

  setDribbler(false);
  stopDrive();

  kicker.setPeriodHertz(50);
  kicker.attach(KICK_SERVO_PIN, 500, 2400);
  kicker.write(KICK_REST);

  Serial.println(F("SoccerBot ADVANCED RC receiver ready."));
}

void loop() {
  updateKick();

  // Always fail safe while a kick is physically moving.
  if (kicking) {
    stopDrive();
    return;
  }

  // Read three drive channels.
  uint16_t p1 = readPulse(RC_CH1_PIN);
  uint16_t p2 = readPulse(RC_CH2_PIN);
  uint16_t p4 = readPulse(RC_CH4_PIN);

  bool driveValid =
    p1 >= 900 && p1 <= 2100 &&
    p2 >= 900 && p2 <= 2100 &&
    p4 >= 900 && p4 <= 2100;

  if (driveValid) {
    int strafe = rcAxis(p1, REVERSE_STRAFE);
    int forward = rcAxis(p2, REVERSE_FORWARD);
    int rotate = rcAxis(p4, REVERSE_ROTATE);
    driveMecanum(strafe, forward, rotate);
    lastGoodDriveMs = millis();
  } else {
    stopDrive();
  }

  // Mechanisms are sampled independently so missing mechanism pulses never
  // make the drivetrain unsafe.
  static uint32_t lastMechanismRead = 0;
  if (millis() - lastMechanismRead >= 35) {
    lastMechanismRead = millis();

    uint16_t p5 = readPulse(RC_CH5_PIN);
    uint16_t p6 = readPulse(RC_CH6_PIN);

    bool kickRequest = (p5 >= 1600 && p5 <= 2100);
    bool dribbleRequest = (p6 >= 1600 && p6 <= 2100);

    static bool previousKick = false;
    if (kickRequest && !previousKick) startKick();
    previousKick = kickRequest;

    setDribbler(dribbleRequest);
  }

  if (millis() - lastGoodDriveMs > RC_FAILSAFE_MS) {
    stopDrive();
    setDribbler(false);
  }
}
