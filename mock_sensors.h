#pragma once

#define USE_MOCK_SENSORS  // comment out this line to use real sensors

#ifdef USE_MOCK_SENSORS

#include <Arduino.h>

// === Add these missing constants ===
#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_WHITE        1

// ===================== Mock INA219 Class ===================== //
class Adafruit_INA219 {
  public:
    Adafruit_INA219(uint8_t addr) : address(addr) {}

    bool begin() {
      Serial.printf("[MOCK] INA219 sensor at 0x%02X initialized.\n", address);
      return true;
    }

    float getBusVoltage_V() {
      switch (address) {
        case 0x40: return 3.5;                   // Cell 1
        case 0x41: return 3.5 + 3.6;             // Cell 1 + 2
        case 0x44: return 3.5 + 3.6 + 3.7;       // Cell 1 + 2 + 3
        case 0x45: return 3.5 + 3.6 + 3.7 + 3.8; // Total
        default: return 3.3;
      }
    }

    float getCurrent_mA() {
      return 1200.0 + random(-100, 100);  // Simulate 1100–1300 mA
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
    case 36: return 1200;
    case 39: return 1400;
    case 34: return 1350;
    case 35: return 1250;
    default: return 1300;
  }
}

#endif  // USE_MOCK_SENSORS
