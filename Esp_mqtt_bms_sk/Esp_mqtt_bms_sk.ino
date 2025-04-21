#include "init.h"

void setup() {
  Serial.begin(115200);
  Logger::log("🔧 Booting ESP32 BMS...");

  initAllSensors();
  initializeDisplay();
  connectToWiFi();
  connectToAWS();
  initTimeUTC();
  initializeWebServer();
  initializeCutoffPin();
  Logger::log("✅ Setup complete");
}


void loop() {
  #ifdef USE_MOCK_SENSORS;
  updateSensorOffsets();
  randomSeed(analogRead(A0)); //Uconnected Analog pin for garbage value
  #endif
  
  readSensorValues();
  calculateVoltages();
  readTemperatures();

  if (!client.connected()) {
    connectToAWS();
  }
  client.loop();

  publishPeriodically();
  printToConsole();
  checkCutoffConditions();
  server.handleClient();

  delay(1000);
}
