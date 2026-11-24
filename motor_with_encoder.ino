#include <Arduino.h>

// ==========================================
// 1. MOTOR DRIVER PIN DEFINITIONS
// ==========================================
// Motor 1 - BTS7960 #1
#define M1_RPWM 25
#define M1_LPWM 26

// Motor 2 - L298N #1 (ENA tied to 5V rail)
#define M2_IN1  14
#define M2_IN2  13

// Motor 3 - BTS7960 #2
#define M3_RPWM 18
#define M3_LPWM 19

// Motor 4 - L298N #2 (ENA tied to 5V rail)
#define M4_IN1  22
#define M4_IN2  23

// ==========================================
// 2. JGB37-520 ENCODER PIN DEFINITIONS
// ==========================================
#define M1_ENC_A 32
#define M1_ENC_B 33

#define M2_ENC_A 27
#define M2_ENC_B 4

#define M3_ENC_A 16
#define M3_ENC_B 17

#define M4_ENC_A 21
#define M4_ENC_B 5

const float TICKS_PER_REV = 1000.0;

// ==========================================
// 3. ENCODER INTERRUPT SERVICE ROUTINES
// ==========================================
volatile long countM1 = 0;
volatile long countM2 = 0;
volatile long countM3 = 0;
volatile long countM4 = 0;

void IRAM_ATTR isrM1() {
  if (digitalRead(M1_ENC_B) == HIGH) countM1++;
  else countM1--;
}

void IRAM_ATTR isrM2() {
  if (digitalRead(M2_ENC_B) == HIGH) countM2++;
  else countM2--;
}

void IRAM_ATTR isrM3() {
  if (digitalRead(M3_ENC_B) == HIGH) countM3++;
  else countM3--;
}

void IRAM_ATTR isrM4() {
  if (digitalRead(M4_ENC_B) == HIGH) countM4++;
  else countM4--;
}

// ==========================================
// 4. MOTOR CONTROL FUNCTIONS
// ==========================================
void moveAllForward() {
  // Motor 1 (BTS7960 #1 - Full speed to match L298N)
  analogWrite(M1_RPWM, 255);
  analogWrite(M1_LPWM, 0);

  // Motor 2 (L298N #1)
  digitalWrite(M2_IN1, HIGH);
  digitalWrite(M2_IN2, LOW);

  // Motor 3 (BTS7960 #2 - Full speed to match L298N)
  analogWrite(M3_RPWM, 255);
  analogWrite(M3_LPWM, 0);

  // Motor 4 (L298N #2)
  digitalWrite(M4_IN1, HIGH);
  digitalWrite(M4_IN2, LOW);
}

void stopAllMotors() {
  analogWrite(M1_RPWM, 0);
  analogWrite(M1_LPWM, 0);

  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, LOW);

  analogWrite(M3_RPWM, 0);
  analogWrite(M3_LPWM, 0);

  digitalWrite(M4_IN1, LOW);
  digitalWrite(M4_IN2, LOW);
}

// ==========================================
// 5. SETUP & MAIN LOOP
// ==========================================
void setup() {
  Serial.begin(115200);

  // Configure Motor Driver Pins
  pinMode(M1_RPWM, OUTPUT);
  pinMode(M1_LPWM, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);
  pinMode(M3_RPWM, OUTPUT);
  pinMode(M3_LPWM, OUTPUT);
  pinMode(M4_IN1, OUTPUT);
  pinMode(M4_IN2, OUTPUT);

  // Configure Encoder Pins with Internal Pull-ups
  pinMode(M1_ENC_A, INPUT_PULLUP);
  pinMode(M1_ENC_B, INPUT_PULLUP);
  pinMode(M2_ENC_A, INPUT_PULLUP);
  pinMode(M2_ENC_B, INPUT_PULLUP);
  pinMode(M3_ENC_A, INPUT_PULLUP);
  pinMode(M3_ENC_B, INPUT_PULLUP);
  pinMode(M4_ENC_A, INPUT_PULLUP);
  pinMode(M4_ENC_B, INPUT_PULLUP);

  // Attach Interrupts
  attachInterrupt(digitalPinToInterrupt(M1_ENC_A), isrM1, RISING);
  attachInterrupt(digitalPinToInterrupt(M2_ENC_A), isrM2, RISING);
  attachInterrupt(digitalPinToInterrupt(M3_ENC_A), isrM3, RISING);
  attachInterrupt(digitalPinToInterrupt(M4_ENC_A), isrM4, RISING);

  // Start driving all 4 motors forward
  moveAllForward();
  Serial.println("All 4 motors running forward + reading encoders...");
}

void loop() {
  // Safely snapshot volatile encoder counts
  noInterrupts();
  long c1 = countM1;
  long c2 = countM2;
  long c3 = countM3;
  long c4 = countM4;
  interrupts();

  // Print Ticks and Revolutions for all 4 motors
  Serial.print("M1: "); Serial.print(c1); Serial.print(" ("); Serial.print(c1 / TICKS_PER_REV, 1); Serial.print(" rev) | ");
  Serial.print("M2: "); Serial.print(c2); Serial.print(" ("); Serial.print(c2 / TICKS_PER_REV, 1); Serial.print(" rev) | ");
  Serial.print("M3: "); Serial.print(c3); Serial.print(" ("); Serial.print(c3 / TICKS_PER_REV, 1); Serial.print(" rev) | ");
  Serial.print("M4: "); Serial.print(c4); Serial.print(" ("); Serial.print(c4 / TICKS_PER_REV, 1); Serial.println(" rev)");

  delay(200);
}
