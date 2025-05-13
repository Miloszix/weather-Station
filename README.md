# Weather Station with Substation

## Overview

This project involves the creation of a **weather monitoring system** consisting of a **main station** and a **substation**. The substation collects environmental data and transmits it wirelessly to the main station for processing and display.

## Components

### Main Station
- **ESP32** – microcontroller
- **BME280** – temperature, humidity, and pressure sensor
- **TFT ILI9341** – 240x320px display
- **RTC DS3231** – real-time clock
- **NRF24L01** – RF module

### Substation
- **ESP32** – microcontroller
- **BME280** – environmental sensor
- **NRF24L01** – RF module
- **OLED SSD1306** – 128x64px display

## System Schematics

*Schematics for both the main station and substation are included in the documentation.*

## How It Works

### Main Station

1. **Initialization**  
   - Connects to Wi-Fi for NTP synchronization  
   - Initializes RTC, RF24, and TFT display  

2. **Environmental Data Measurement**  
   - Reads data from BME280  
   - Receives remote data from substation via RF24  

3. **Data Processing & Display**  
   - Combines local and remote sensor data  
   - Shows values (temperature, humidity, pressure) with icons on TFT  
   - Displays current time and date  

4. **Time Synchronization**  
   - Syncs RTC with NTP server hourly when Wi-Fi is available  

5. **Timers & Interrupts**  
   - Two timers for sensor reading and display updates  
   - RF24 interrupt for new data reception  

### Substation

1. **OLED Configuration**  
   - Uses I2C to communicate  
   - Displays changing screens for temperature, pressure, humidity every 3s  

2. **RF24 Configuration**  
   - Unique addresses for reliable transmission  
   - Sends structured data to main station  

3. **Sensor Configuration**  
   - BME280 reads data every 5s using hardware timer  

4. **Timers**  
   - Timer1: sensor reading  
   - Timer2: OLED screen switching  

5. **Main Loop**  
   - Handles data reading, screen update, and RF24 data transmission  

## Code Overview

### `Sensor.h`
- Handles BME280 initialization, calibration, and data conversion
- Key functions:
  - `initBME280()`
  - `readCalibrationData()`
  - `readRawData()`
  - `getTemp()`, `getHum()`, `getPress()`

### `displayBMP.h`
- Loads and renders BMP images from SPIFFS memory to TFT

### `Bitmaps.h`
- Contains bitmap icons used in the display interface

### `Main.c` (Main Station)
- Initializes all components (SPI, I2C, RTC, RF24, Wi-Fi)
- Uses timers for periodic data collection and display
- Displays both local and remote sensor data on the TFT screen

### `Main.c` (Substation)
- Initializes OLED, RF24, sensors, and timers
- Displays environmental data with rotating screens
- Sends sensor data wirelessly every 5s

## Used Libraries

- `Arduino.h`, `Wire.h`, `SPI.h` – Base libraries
- `Adafruit_SSD1306.h`, `Adafruit_GFX.h` – OLED handling
- `nRF24L01.h`, `RF24.h` – RF module communication
- `FS.h`, `SPIFFS.h` – File system for TFT images
- `TFT_eSPI.h` – TFT display handling
- `WiFi.h` – Wi-Fi connectivity and NTP sync

## Summary

The system continuously monitors environmental conditions from two locations and displays them on a user interface. It combines RF communication, real-time synchronization, and sensor data processing in a flexible and user-friendly way. Ideal for home weather stations or environmental monitoring setups.
