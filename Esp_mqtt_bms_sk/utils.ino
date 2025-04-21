// Function to convert raw analog value from NTC thermistors to temperature
float getTemperature(int pin) {
  int rawValue = analogRead(pin);
  float resistance = ((4095.0 / rawValue) - 1) * 100000.0;  // Assuming 100k reference resistor
  resistance = 100000.0 / resistance;

  // Using Steinhart-Hart equation
  float temperature = log(resistance);
  temperature = 1 / (0.001129148 + (0.000234125 * temperature) + (0.0000000876741 * temperature * temperature * temperature));
  temperature = (temperature - 273.15) / 20;  // Convert Kelvin to Celsius

  return temperature;
}


bool checkCellConditions(float cellVoltage_1, float cellVoltage_2, float cellVoltage_3, float cellVoltage_4,
                         float current_mA_1, float current_mA_2, float current_mA_3, float current_mA_4,
                         float temp1, float temp2, float temp3, float temp4) {
  // Check if any condition exceeds the limits and trigger cutoff
  if (cellVoltage_1 < MIN_VOLTAGE || cellVoltage_1 > MAX_VOLTAGE || current_mA_1 > MAX_CURRENT || temp1 < MIN_TEMPERATURE || temp1 > MAX_TEMPERATURE || cellVoltage_2 < MIN_VOLTAGE || cellVoltage_2 > MAX_VOLTAGE || current_mA_2 > MAX_CURRENT || temp2 < MIN_TEMPERATURE || temp2 > MAX_TEMPERATURE || cellVoltage_3 < MIN_VOLTAGE || cellVoltage_3 > MAX_VOLTAGE || current_mA_3 > MAX_CURRENT || temp3 < MIN_TEMPERATURE || temp3 > MAX_TEMPERATURE || cellVoltage_4 < MIN_VOLTAGE || cellVoltage_4 > MAX_VOLTAGE || current_mA_4 > MAX_CURRENT || temp4 < MIN_TEMPERATURE || temp4 > MAX_TEMPERATURE) {

    digitalWrite(cutoffPin, LOW);  // Cutoff if any condition fails
    Serial.println("Cutoff triggered due to out-of-range voltage, current, or temperature.");

    // Log conditions when cutoff occurs
    scanAndLogCellConditions(cellVoltage_1, cellVoltage_2, cellVoltage_3, cellVoltage_4,
                             current_mA_1, current_mA_2, current_mA_3, current_mA_4, temp1, temp2, temp3, temp4);

    return true;  // Return true if cutoff occurred
  } else {
    digitalWrite(cutoffPin, HIGH);  // No cutoff if all conditions are within limits
    return false;                   // Return false if no cutoff
  }
}

void scanAndLogCellConditions(float cellVoltage_1, float cellVoltage_2, float cellVoltage_3, float cellVoltage_4,
                              float current_mA_1, float current_mA_2, float current_mA_3, float current_mA_4,
                              float temp1, float temp2, float temp3, float temp4) {

  // Log conditions when cutoff occurs
  Serial.println("System Cutoff. Logging cell conditions:");

  // Cell 1
  if (cellVoltage_1 < MIN_VOLTAGE) {
    Serial.println("Cell 1: Voltage too low");
  } else if (cellVoltage_1 > MAX_VOLTAGE) {
    Serial.println("Cell 1: Voltage too high");
  }
  if (current_mA_1 > MAX_CURRENT) {
    Serial.println("Cell 1: Current too high");
  }
  if (temp1 < MIN_TEMPERATURE) {
    Serial.println("Cell 1: Temp too low");
  } else if (temp1 > MAX_TEMPERATURE) {
    Serial.println("Cell 1: Temp too high");
  }

  // Cell 2
  if (cellVoltage_2 < MIN_VOLTAGE) {
    Serial.println("Cell 2: Voltage too low");
  } else if (cellVoltage_2 > MAX_VOLTAGE) {
    Serial.println("Cell 2: Voltage too high");
  }
  if (current_mA_2 > MAX_CURRENT) {
    Serial.println("Cell 2: Current too high");
  }
  if (temp2 < MIN_TEMPERATURE) {
    Serial.println("Cell 2: Temp too low");
  } else if (temp2 > MAX_TEMPERATURE) {
    Serial.println("Cell 2: Temp too high");
  }

  // Cell 3
  if (cellVoltage_3 < MIN_VOLTAGE) {
    Serial.println("Cell 3: Voltage too low");
  } else if (cellVoltage_3 > MAX_VOLTAGE) {
    Serial.println("Cell 3: Voltage too high");
  }
  if (current_mA_3 > MAX_CURRENT) {
    Serial.println("Cell 3: Current too high");
  }
  if (temp3 < MIN_TEMPERATURE) {
    Serial.println("Cell 3: Temp too low");
  } else if (temp3 > MAX_TEMPERATURE) {
    Serial.println("Cell 3: Temp too high");
  }

  // Cell 4
  if (cellVoltage_4 < MIN_VOLTAGE) {
    Serial.println("Cell 4: Voltage too low");
  } else if (cellVoltage_4 > MAX_VOLTAGE) {
    Serial.println("Cell 4: Voltage too high");
  }
  if (current_mA_4 > MAX_CURRENT) {
    Serial.println("Cell 4: Current too high");
  }
  if (temp4 < MIN_TEMPERATURE) {
    Serial.println("Cell 4: Temp too low");
  } else if (temp4 > MAX_TEMPERATURE) {
    Serial.println("Cell 4: Temp too high");
  }
}

// === Device ID ===
const char* getDeviceID() {
  return MQTT_CLIENT_ID;
}


// === Debug: Heap Usage ===
void logHeap(const char* label) {
  Serial.printf("[HEAP] %s: %u bytes free\n", label, ESP.getFreeHeap());
}


void connectToWiFi() {
  Serial.printf("🔌 Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retries = 0;

  while (WiFi.status() != WL_CONNECTED && retries < 15) {
    delay(1000);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Display IP on OLED if available
    #ifdef SCREEN_WIDTH
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println("IP Address:");
    display.println(WiFi.localIP().toString());
    display.display();
    #endif

  } else {
    Serial.println("\n❌ WiFi failed. Rebooting...");
    delay(3000);
    ESP.restart();
  }

  logHeap("Post WiFi Connect");
}

void logCell(const String& label, float voltage, float current, float temp) {
  Serial.printf("[%s] V: %.2f V | I: %.2f mA | T: %.2f °C\n", 
                label.c_str(), voltage, current, temp);
}




String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "1970-01-01T00:00:00Z";

  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000000Z", &timeinfo);  // ISO format + Z
  return String(buf);
}


// Have to setup the copnstants and values in Init.h

int voltageToPercentage(float v) {
  return constrain((int)((v - 3.2) / (4.2 - 3.2) * 100), 0, 100);
}

float estimateDischargeTime(float voltage, float current_mA) {
  float capacity_mAh = 3000;  // Example fixed capacity TODO: Make it In INIT.h
  return current_mA > 0 ? (capacity_mAh / current_mA) * 60 : 0;  // Return in minutes
}

float estimateBatteryHealth(float voltage) {
  return constrain((voltage / 4.2) * 100.0, 50.0, 100.0);
}