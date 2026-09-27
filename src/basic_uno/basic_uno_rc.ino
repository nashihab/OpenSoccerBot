/*
  SoccerBot BASIC — Arduino UNO + standard RC receiver
  Manual-control version only. No autonomy, no ball tracking.

  Receiver interface: standard servo PWM outputs.
  Example: FlySky FS-iA6B or another PWM-output RC receiver.

  Default channel mapping:
    CH1 = steering (left/right)
    CH2 = throttle (forward/reverse)
    CH5 = kick switch (ON/high position = kick)

  Receiver pulse range is normally about 1000..2000 us with ~1500 us center.
  Tune RC_* constants when using a different radio.
*/

#include <Servo.h>

// ---------------- USER CONFIG ----------------
const uint8_t RC_CH1_PIN = 2;   // steering
const uint8_t RC_CH2_PIN = 3;   // throttle
const uint8_t RC_CH5_PIN = A0;  // kick switch

// TB6612FNG
const uint8_t PWMA = 5;
const uint8_t AIN1 = 7;
const uint8_t AIN2 = 8;
const uint8_t PWMB = 6;
const uint8_t BIN1 = 9;
const uint8_t BIN2 = 10;
const uint8_t STBY = 4;

const uint8_t KICK_SERVO_PIN = A3;
const int KICK_REST = 25;
const int KICK_ANGLE = 115;
const uint32_t KICK_COOLDOWN_MS = 1000;
const uint32_t KICK_HOLD_MS = 130;

const uint16_t RC_MIN_US = 1000;
const uint16_t RC_CENTER_US = 1500;
const uint16_t RC_MAX_US = 2000;
const uint16_t RC_DEADBAND_US = 45;
const uint32_t DRIVE_TIMEOUT_US = 6000;
const uint32_t AUX_TIMEOUT_US = 5000;
const uint16_t RC_FAILSAFE_MS = 120;

// Reverse a channel here instead of rewiring anything.
const bool REVERSE_THROTTLE = false;
const bool REVERSE_STEERING = false;
const bool REVERSE_LEFT_MOTOR = false;
const bool REVERSE_RIGHT_MOTOR = true;

const uint8_t MAX_DRIVE_PWM = 230;

Servo kicker;
uint32_t lastGoodDriveMs = 0;
uint32_t lastKickMs = 0;
uint32_t kickStartMs = 0;
bool kickWasHigh = false;
bool kicking = false;

// ---------------- HELPERS ----------------
uint16_t readPulse(uint8_t pin, uint32_t timeoutUs) {
  return (uint16_t)pulseIn(pin, HIGH, timeoutUs);
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
  digitalWrite(STBY, LOW);
  setMotor(PWMA, AIN1, AIN2, 0, REVERSE_LEFT_MOTOR);
  setMotor(PWMB, BIN1, BIN2, 0, REVERSE_RIGHT_MOTOR);
}

void driveDifferential(int throttle, int steering) {
  digitalWrite(STBY, HIGH);

  long left = (long)throttle + steering;
  long right = (long)throttle - steering;

  long peak = max(1000L, max(abs(left), abs(right)));
  int leftPwm = (int)(left * MAX_DRIVE_PWM / peak);
  int rightPwm = (int)(right * MAX_DRIVE_PWM / peak);

  setMotor(PWMA, AIN1, AIN2, leftPwm, REVERSE_LEFT_MOTOR);
  setMotor(PWMB, BIN1, BIN2, rightPwm, REVERSE_RIGHT_MOTOR);
}

void startKick() {
  uint32_t now = millis();
  if (kicking || (now - lastKickMs < KICK_COOLDOWN_MS)) return;

  stopDrive();
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
  pinMode(RC_CH5_PIN, INPUT);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  kicker.attach(KICK_SERVO_PIN);
  kicker.write(KICK_REST);
  stopDrive();

  Serial.println(F("SoccerBot BASIC RC receiver ready."));
}

void loop() {
  updateKick();

  // Keep the drive stopped while the kicker is moving.
  if (kicking) {
    stopDrive();
    return;
  }

  uint16_t steerPulse = readPulse(RC_CH1_PIN, DRIVE_TIMEOUT_US);
  uint16_t throttlePulse = readPulse(RC_CH2_PIN, DRIVE_TIMEOUT_US);

  if (steerPulse >= 900 && steerPulse <= 2100 &&
      throttlePulse >= 900 && throttlePulse <= 2100) {

    int steering = rcAxis(steerPulse, REVERSE_STEERING);
    int throttle = rcAxis(throttlePulse, REVERSE_THROTTLE);
    driveDifferential(throttle, steering);
    lastGoodDriveMs = millis();
  } else {
    stopDrive();
  }

  // Mechanism channel is intentionally sampled at a slower rate.
  static uint32_t lastAuxRead = 0;
  if (millis() - lastAuxRead >= 40) {
    lastAuxRead = millis();
    uint16_t kickPulse = readPulse(RC_CH5_PIN, AUX_TIMEOUT_US);
    bool kickHigh = (kickPulse >= 1600 && kickPulse <= 2100);
    if (kickHigh && !kickWasHigh) startKick();
    kickWasHigh = kickHigh;
  }

  if (millis() - lastGoodDriveMs > RC_FAILSAFE_MS) {
    stopDrive();
  }
}
