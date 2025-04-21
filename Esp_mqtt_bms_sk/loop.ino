void readSensorValues() {
  Voltage_1 = ina219_1.getBusVoltage_V();
  Voltage_2 = ina219_2.getBusVoltage_V();
  Voltage_3 = ina219_3.getBusVoltage_V();
  Voltage_4 = ina219_4.getBusVoltage_V();

  current_mA_1 = ina219_1.getCurrent_mA();
  current_mA_2 = ina219_2.getCurrent_mA();
  current_mA_3 = ina219_3.getCurrent_mA();
  current_mA_4 = ina219_4.getCurrent_mA();

  power_mW_4 = ina219_4.getPower_mW();
}

void calculateVoltages() {
  cellVoltage_1 = Voltage_1;
  cellVoltage_2 = Voltage_2 - Voltage_1;
  cellVoltage_3 = Voltage_3 - Voltage_2;
  cellVoltage_4 = Voltage_4 - Voltage_3;
}

void readTemperatures() {
  temperature1 = getTemperature(36);
  temperature2 = getTemperature(39);
  temperature3 = getTemperature(34);
  temperature4 = getTemperature(35);
}

void publishPeriodically() {
  static unsigned long lastPub = 0;
  if (millis() - lastPub > 10000) {
    prepareMqttPayload();
    logHeap("Post Publish");
    lastPub = millis();
  }
}

void printToConsole() {
  Logger::info("Cell 1 => Voltage:", cellVoltage_1, "V | Current:", current_mA_1, "mA");
  Logger::info("Cell 2 => Voltage:", cellVoltage_2, "V | Current:", current_mA_2 * 100, "mA");
  Logger::info("Cell 3 => Voltage:", cellVoltage_3, "V | Current:", current_mA_3, "mA");
  Logger::info("Cell 4 => Voltage:", cellVoltage_4, "V | Current:", current_mA_4, "mA | Power:", power_mW_4, "mW");

  Logger::debug("Temperatures => T1:", temperature1, "°C | T2:", temperature2, "°C | T3:", temperature3, "°C | T4:", temperature4, "°C");
}

void checkCutoffConditions() {
  checkCellConditions(
    cellVoltage_1, cellVoltage_2, cellVoltage_3, cellVoltage_4,
    current_mA_1, current_mA_2, current_mA_3, current_mA_4,
    temperature1, temperature2, temperature3, temperature4);
}
