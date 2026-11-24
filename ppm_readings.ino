#include "DHT.h"
#include <math.h>

// ================ PIN CONFIGURATION ================

#define DHTPIN 4
#define DHTTYPE DHT21

#define MQ2_PIN 34
#define MQ135_PIN 35

DHT dht(DHTPIN, DHTTYPE);

// ================ HARDWARE PARAMETERS ================

const float VC = 5.0;

const float RL_MQ2 = 1000.0;
const float RL_MQ135 = 1000.0;

const float DIVIDER_FACTOR = 1.5;

const float ADC_REFERENCE = 3.3;
const float ADC_MAX = 4095.0;

// ================ CLEAN AIR RATIOS ================

const float CLEAN_RATIO_MQ2 = 9.8;
const float CLEAN_RATIO_MQ135 = 3.6;

// ================ PPM COEFFICIENTS ================

// MQ2 - Propane (C3H8)
const float MQ2_M = -0.683;
const float MQ2_B = 1.002;

// MQ135 - Hydrogen (H2)
const float MQ135_M = -0.523;
const float MQ135_B = 0.347;

// ================ RO CALIBRATION ================

const int CALIBRATION_SAMPLES = 60;

float Ro_MQ2 = 0;
float Ro_MQ135 = 0;

bool calibrated = false;

// ================ ADC READING ================

float readADC(int pin) {

  uint32_t sum = 0;

  for (int i = 0; i < 20; i++) {
    sum += analogRead(pin);
    delay(5);
  }

  return sum / 20.0;
}

// ================ VOLTAGE CONVERSION ================

float calculateVoltage(float adc) {

  float vadc =
    (adc / ADC_MAX) * ADC_REFERENCE;

  return vadc * DIVIDER_FACTOR;
}

// ================ SENSOR RESISTANCE ================

float calculateRs(float voltage, float RL) {

  if (voltage <= 0.01 || voltage >= VC) {
    return NAN;
  }

  return RL * ((VC - voltage) / voltage);
}

// ================ PPM CALCULATION ================

float calculatePPM(
  float ratio,
  float slope,
  float intercept
) {

  if (!isfinite(ratio) || ratio <= 0 ||
      !isfinite(slope) || slope == 0) {
    return NAN;
  }

  float logPPM =
    (log10(ratio) - intercept) / slope;

  float ppm = pow(10.0, logPPM);

  if (!isfinite(ppm) || ppm <= 0) {
    return NAN;
  }

  return ppm;
}

// ================ RO CALIBRATION ================

void calibrateSensors() {

  Serial.println("# CLEAN AIR CALIBRATION START");

  double sumMQ2 = 0;
  double sumMQ135 = 0;

  int countMQ2 = 0;
  int countMQ135 = 0;

  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {

    float adc2 = readADC(MQ2_PIN);
    float adc135 = readADC(MQ135_PIN);

    float v2 = calculateVoltage(adc2);
    float v135 = calculateVoltage(adc135);

    float rs2 = calculateRs(v2, RL_MQ2);
    float rs135 = calculateRs(v135, RL_MQ135);

    if (isfinite(rs2)) {
      sumMQ2 += rs2;
      countMQ2++;
    }

    if (isfinite(rs135)) {
      sumMQ135 += rs135;
      countMQ135++;
    }

    delay(1000);
  }

  if (countMQ2 == 0 || countMQ135 == 0) {
    Serial.println("# CALIBRATION FAILED");
    return;
  }

  Ro_MQ2 =
    (sumMQ2 / countMQ2) / CLEAN_RATIO_MQ2;

  Ro_MQ135 =
    (sumMQ135 / countMQ135) / CLEAN_RATIO_MQ135;

  calibrated = true;

  Serial.println("# CALIBRATION COMPLETE");

  Serial.print("# MQ2_Ro=");
  Serial.println(Ro_MQ2, 4);

  Serial.print("# MQ135_Ro=");
  Serial.println(Ro_MQ135, 4);
}

// ================ SETUP ================

void setup() {

  Serial.begin(115200);

  analogReadResolution(12);

  analogSetPinAttenuation(MQ2_PIN, ADC_11db);
  analogSetPinAttenuation(MQ135_PIN, ADC_11db);

  dht.begin();

  delay(2000);

  Serial.println("# TRINETRA SENSOR LOGGER");

  Serial.println("# Stabilization period...");
  delay(60000);

  calibrateSensors();

  if (!calibrated) return;

  Serial.println(
    "Time_ms,Temperature_C,Humidity_percent,"
    "MQ2_ADC,MQ2_Vadc,MQ2_Vout,MQ2_Rs,"
    "MQ2_Ro,MQ2_Ratio,MQ2_PPM_C3H8,"
    "MQ135_ADC,MQ135_Vadc,MQ135_Vout,"
    "MQ135_Rs,MQ135_Ro,MQ135_Ratio,MQ135_PPM_H2"
  );
}

// ================ MAIN LOOP ================

void loop() {

  if (!calibrated) {
    delay(1000);
    return;
  }

  float adc2 = readADC(MQ2_PIN);
  float adc135 = readADC(MQ135_PIN);

  float vadc2 =
    (adc2 / ADC_MAX) * ADC_REFERENCE;

  float vadc135 =
    (adc135 / ADC_MAX) * ADC_REFERENCE;

  float vout2 = vadc2 * DIVIDER_FACTOR;
  float vout135 = vadc135 * DIVIDER_FACTOR;

  float rs2 = calculateRs(vout2, RL_MQ2);
  float rs135 = calculateRs(vout135, RL_MQ135);

  float ratio2 = rs2 / Ro_MQ2;
  float ratio135 = rs135 / Ro_MQ135;

  float ppm2 =
    calculatePPM(ratio2, MQ2_M, MQ2_B);

  float ppm135 =
    calculatePPM(ratio135, MQ135_M, MQ135_B);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // ================ CSV OUTPUT ================

  Serial.print(millis());
  Serial.print(",");

  if (isfinite(temperature))
    Serial.print(temperature, 2);
  else Serial.print("NA");

  Serial.print(",");

  if (isfinite(humidity))
    Serial.print(humidity, 2);
  else Serial.print("NA");

  Serial.print(",");

  Serial.print(adc2, 2);
  Serial.print(",");
  Serial.print(vadc2, 4);
  Serial.print(",");
  Serial.print(vout2, 4);
  Serial.print(",");
  Serial.print(rs2, 2);
  Serial.print(",");
  Serial.print(Ro_MQ2, 2);
  Serial.print(",");
  Serial.print(ratio2, 4);
  Serial.print(",");

  if (isfinite(ppm2))
    Serial.print(ppm2, 2);
  else Serial.print("NA");

  Serial.print(",");

  Serial.print(adc135, 2);
  Serial.print(",");
  Serial.print(vadc135, 4);
  Serial.print(",");
  Serial.print(vout135, 4);
  Serial.print(",");
  Serial.print(rs135, 2);
  Serial.print(",");
  Serial.print(Ro_MQ135, 2);
  Serial.print(",");
  Serial.print(ratio135, 4);
  Serial.print(",");

  if (isfinite(ppm135))
    Serial.println(ppm135, 2);
  else Serial.println("NA");

  delay(1000);
}
