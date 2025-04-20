#pragma once
#include "Logger.h"
#include "secrets.h"
#define USE_MOCK_SENSORS  // ✅ Enable sensor mocks (INA219, SSD1306, analogRead)
// Always include real Arduino + WebServer headers
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

// Conditionally include sensor-related headers
#ifdef USE_MOCK_SENSORS
  #include "mock_sensors.h"
#else
  #include <Wire.h>
  #include <Adafruit_INA219.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif

// INA219 sensor instances
Adafruit_INA219 ina219_1(0x40);
Adafruit_INA219 ina219_2(0x41);
Adafruit_INA219 ina219_3(0x44);
Adafruit_INA219 ina219_4(0x45);

// OLED display parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1  // No reset pin on the OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Clients
WiFiClientSecure net;
PubSubClient client(net);

// Reconnect Safety
int reconnectAttempts = 0;
const int MAX_RECONNECTS = 10;

// Web server on port 80
WebServer server(80);

// Setup pin for cutoff control
const int cutoffPin = 4;  // Pin D25 to control cutoff

const float MIN_VOLTAGE = 2.0;
const float MAX_VOLTAGE = 4.2;
const float MAX_CURRENT = 6000;  // 3A in mA

// Global variables for temperature limits
const float MIN_TEMPERATURE = -20.0;   // Minimum safe temperature in Celsius
const float MAX_TEMPERATURE = 50.0;  // Maximum safe temperature in Celsius

// Shared global sensor readings
float Voltage_1, Voltage_2, Voltage_3, Voltage_4;
float power_mW_4;

// Already declared somewhere:
float current_mA_1, current_mA_2, current_mA_3, current_mA_4;
float cellVoltage_1, cellVoltage_2, cellVoltage_3, cellVoltage_4;
float temperature1, temperature2, temperature3, temperature4;

// To Hold last Publish battery state
StaticJsonDocument<1024> lastPublishedDoc;
bool firstPublishDone = false;
bool forcePublish = false;

