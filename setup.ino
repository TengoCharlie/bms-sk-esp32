bool initSensor(Adafruit_INA219& sensor, uint8_t address) {
  if (!sensor.begin()) {
    Serial.printf("❌ Failed to find INA219 at 0x%02X\n", address);
    return false;
  }
  Serial.printf("✅ INA219 initialized at 0x%02X\n", address);
  return true;
}

void initAllSensors() {
  if (!initSensor(ina219_1, 0x40) || !initSensor(ina219_2, 0x41) || !initSensor(ina219_3, 0x44) || !initSensor(ina219_4, 0x45)) {
    Logger::info("💥 One or more INA219 sensors failed. Halting.");
    while (true)
      ;  // Stop execution
  }

  Logger::info("✅ Sensors initialized");
}


void initializeDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Logger::info("❌ OLED display not found!");
    while (true); // halt
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("🚀 Starting...");
  display.display();

  Logger::info("✅ OLED initialized");
}

void initializeWebServer() {
  server.on("/", handleRoot);
  server.begin();
  Logger::info("🌐 Web server started at:");
  Logger::info(WiFi.localIP());
}

void initializeCutoffPin() {
  pinMode(cutoffPin, OUTPUT);
  digitalWrite(cutoffPin, HIGH);
  Logger::info("✅ Cutoff pin set");
}

void initTimeUTC() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC (GMT+0)

  Logger::info("⏳ Waiting for NTP time sync...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Logger::warn("Failed to obtain time, retrying...");
    delay(1000);
  }
  Logger::info("⏰ UTC Time synchronized:", asctime(&timeinfo));
}

