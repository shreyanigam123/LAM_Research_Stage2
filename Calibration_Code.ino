// ================== SET DRIVE POWER HERE ==================
// -100 to 100 (%).  Positive = forward, negative = reverse, 0 = off
int MOTOR1_POWER = 50;
int MOTOR2_POWER = 50;
int MOTOR3_POWER = 0;
// ==========================================================

// ---- PIN MAP (verify against your PCB!) ----
//                 L_PWM R_PWM L_EN R_EN
const int M1[4] = { 32,   33,   13,  26 };
const int M2[4] = { 18,   19,    4,  16 };
const int M3[4] = { 23,   25,   21,  22 };

const int FLOW_PIN = 14;     // FS

const int PWM_FREQ = 20000;  // 20 kHz
const int PWM_BITS = 8;      // 0-255

volatile unsigned long pulses = 0;   // total pulses counted
unsigned long lastPulses = 0;

void IRAM_ATTR countPulse() {
  pulses++;
}

void setupMotor(const int *m) {
  pinMode(m[2], OUTPUT);
  pinMode(m[3], OUTPUT);
  ledcAttach(m[0], PWM_FREQ, PWM_BITS);   // core 3.x
  ledcAttach(m[1], PWM_FREQ, PWM_BITS);
}
// For ESP32 core 2.x, replace ledcAttach/ledcWrite with
// ledcSetup(channel...), ledcAttachPin(pin, channel), ledcWrite(channel, value)

void driveMotor(const int *m, int percent) {
  percent = constrain(percent, -100, 100);
  int duty = abs(percent) * 255 / 100;

  if (percent == 0) {
    digitalWrite(m[2], LOW);
    digitalWrite(m[3], LOW);
    ledcWrite(m[0], 0);
    ledcWrite(m[1], 0);
    return;
  }

  digitalWrite(m[2], HIGH);   // enable both halves
  digitalWrite(m[3], HIGH);

  if (percent > 0) {          // forward: R_PWM
    ledcWrite(m[0], 0);
    ledcWrite(m[1], duty);
  } else {                    // reverse: L_PWM
    ledcWrite(m[1], 0);
    ledcWrite(m[0], duty);
  }
}

void setup() {
  Serial.begin(115200);

  setupMotor(M1);
  setupMotor(M2);
  setupMotor(M3);

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), countPulse, FALLING);

  driveMotor(M1, MOTOR1_POWER);
  driveMotor(M2, MOTOR2_POWER);
  driveMotor(M3, MOTOR3_POWER);
}

void loop() {
  delay(1000);                          // one print per second

  unsigned long total = pulses;         // snapshot
  unsigned long tickRate = total - lastPulses;   // pulses in the last second = Hz
  lastPulses = total;

  Serial.print("Tick rate: ");
  Serial.print(tickRate);
  Serial.print(", Pulses:");
  Serial.println(total);
}
