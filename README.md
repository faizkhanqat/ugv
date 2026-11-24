

# Edge-AI Based Mine Safety & Rescue UGV

A four-wheel Unmanned Ground Vehicle (UGV) prototype designed for **remote inspection and environmental monitoring** in underground or otherwise difficult-to-access areas. It allows an operator to observe the surroundings and monitor sensor readings from a safer location.

## Key Features

- Four-wheel mobile platform with differential (skid) steering.
- Environmental monitoring using an AM2301 (DHT21), MQ-2 and MQ-135 sensors.
- Two ultrasonic sensors for nearby obstacle-distance measurement.
- ESP32-CAM for live visual monitoring.
- Wi-Fi telemetry for sensor readings and camera streaming.
- LoRa SX1278 communication link under development for long-range telemetry.

## System Overview

1. Sensors collect temperature, humidity, raw gas-response and distance readings.
2. ESP32 controllers acquire and format the sensor data.
3. Sensor telemetry is transmitted over Wi-Fi to an operator device.
4. The ESP32-CAM provides a separate live camera feed over Wi-Fi.
5. The SX1278 LoRa link is being developed as an additional low-data-rate telemetry path.

## Hardware

- Four-wheel chassis with four 12 V geared DC motors
- Two BTS7960 motor-driver modules
- Two ESP32 DevKit V1 boards
- ESP32-CAM
- AM2301 / DHT21 temperature and humidity sensor
- MQ-2 and MQ-135 gas-response sensor modules
- Two HC-SR04 ultrasonic sensors
- Ai-Thinker RA-02 / SX1278 LoRa modules
- 12 V battery and LM2596 buck converter

## Software and Data

- ESP32 firmware for sensor acquisition and Wi-Fi telemetry
- ESP32-CAM firmware for live video streaming
- Structured CSV logging of timestamped sensor records
- Telemetry fields include temperature, humidity, MQ-2 and MQ-135 raw ADC values, and two ultrasonic distance readings.

## Current Development Status

### Working / Integrated

- Four-wheel UGV prototype
- ESP32 sensor acquisition
- Temperature and humidity readings
- Raw MQ-2 and MQ-135 analogue readings
- Two ultrasonic distance sensors
- ESP32-CAM live feed
- Wi-Fi sensor telemetry and camera feed

### In Progress

- LoRa SX1278 point-to-point telemetry link and packet testing.

### Planned

- Dedicated sensing for methane, carbon monoxide, hydrogen sulphide and oxygen deficiency.
- More robust underground communication.
- Edge-AI processing on a more capable onboard computer.
- Localisation, mapping and autonomous navigation.
- Fault handling, emergency-stop behaviour and mission-level safety logic.

## Safety and Limitations

This is an educational engineering prototype. Its ESP32 electronics, MQ sensor modules and battery system are hobby-grade components and are **not certified as intrinsically safe equipment or as life-safety instruments**.

The prototype must not be deployed in a potentially explosive mine atmosphere. The current system supports operator-monitored testing; autonomous navigation and automatic hazard classification are future development stages.

## Project Stage

**Prototype Integration — November 2026**
