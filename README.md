# open-weather-station-revamp

## 1. Short Description

This is the source code of my personal implementation of a weather station. This is an Open Source project and is built using common components that could be easily found in any hardware store. The main objective is to build a station capable of measure temperature, humidity, atmospheric pressure and other relevant weather data for astronomical studies purpose. This is part of the instrumentation of my personal astronomical observatory and will be continuously updated according to studies needs.

## 2. Component List

- ESP8266 Node MCU board;
- DHT22 Temperatur/Humidity Sensor;
- BMP180 Pressure Sensor;
- GY-30, a BH1750 Luminance sensor board;
- LM393/YL-38 Rain sensor
- LEDs and resistors;
- Jumpers and protoboards;


## 3. Features

### 3.1 Temperature and Humidity readings

Temperature is read in Celsius degrees by both sensors DHT22 and BMP180 while and Humidity is read in percentage only by DHT22. 

### 3.2 Atmospheric Pressure readings

Atmospheric pressure is read by BMP180 in hPa (Hectopascal).

### 3.3 Luminance readings

Luminance is read in lux by BH1750 sensor

### 3.4 Rain Sensor

The rain is a boolean status and is detected when the resistance is above an edge value manually adjusted in the trimpot of the YL-38 board.

### 3.5 Status Leds

There is a pair of LEDs to indicate status of the system. The Green LED indicate the status of the WIFI connection, blinking when searching for the preset network. The Red LED indicate when the system is acquiring readings from sensor and sending data to the cloud.

### 3.6 Data Visualization

Currently the system sends data using HTTP Requests and REST API to the ThingSpeak IoT service provided free of charge by MathWorks.