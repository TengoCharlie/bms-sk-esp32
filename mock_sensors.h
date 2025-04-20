#pragma once

#define USE_MOCK_SENSORS  // comment out this line to use real sensors

#ifdef USE_MOCK_SENSORS

#include <Arduino.h>

// === Add these missing constants ===
#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_WHITE 1

unsigned long lastFluctuationTime = 0;
float voltageOffset = 0.0;
float currentOffset = 0.0;

void updateSensorOffsets() {
  unsigned long currentTime = millis();
  if (currentTime - lastFluctuationTime >= 5000) {  // Every 5 seconds
    lastFluctuationTime = currentTime;

    // Random voltage offset between -0.05V and +0.05V
    voltageOffset = random(-50, 51) / 1000.0;

    // Random current offset between -50mA and +50mA
    currentOffset = random(-50, 51);
  }
}

// ===================== Mock INA219 Class ===================== //
class Adafruit_INA219 {
public:
  Adafruit_INA219(uint8_t addr)
    : address(addr) {}

  bool begin() {
    Serial.printf("[MOCK] INA219 sensor at 0x%02X initialized.\n", address);
    return true;
  }

  float getBusVoltage_V() {
    // Base voltages for each cell
    float baseVoltage;
    switch (address) {
      case 0x40: baseVoltage = 3.5; break;                    // Cell 1
      case 0x41: baseVoltage = 3.5 + 3.6; break;              // Cell 1 + 2
      case 0x44: baseVoltage = 3.5 + 3.6 + 3.7; break;        // Cell 1 + 2 + 3
      case 0x45: baseVoltage = 3.5 + 3.6 + 3.7 + 3.8; break;  // Total
      default: baseVoltage = 3.3; break;
    }

    // Apply voltage offset
    return baseVoltage + voltageOffset + random(-20, 20)/100;
  }

  float getCurrent_mA() {
    // Base current with offset
    return 1200.0 + currentOffset + + random(-300, 300)/100;
  }

  float getPower_mW() {
    return getBusVoltage_V() * getCurrent_mA();
  }



private:
  uint8_t address;
};


// ===================== Mock OLED Display ===================== //
#include <Adafruit_GFX.h>  // Can be included or stubbed too

class Adafruit_SSD1306 {
public:
  Adafruit_SSD1306(int w, int h, void* wire, int rst) {}

  bool begin(int vcc, int addr) {
    Serial.println("[MOCK] SSD1306.begin() success");
    return true;
  }

  void clearDisplay() {
    Serial.println("[MOCK] OLED cleared");
  }

  void setTextSize(int size) {}
  void setTextColor(int color) {}
  void setCursor(int x, int y) {}
  void println(const String& msg) {
    Serial.println("[MOCK OLED] " + msg);
  }

  void display() {}
};

// ===================== Mock Temperature Reader ===================== //
float analogRead(int pin) {
  switch (pin) {
    case 36: return random(0, 1200);
    case 39: return random(0, 1200);
    case 34: return random(0, 1200);
    case 35: return random(0, 1200);
    default: return random(0, 1200);
  }
}

#endif  // USE_MOCK_SENSORS
