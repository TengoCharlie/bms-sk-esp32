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