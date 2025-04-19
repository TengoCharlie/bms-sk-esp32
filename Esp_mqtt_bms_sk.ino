#include "init.h"

void setup() {
  Serial.begin(115200);

  // Initialize INA219 sensors
  if (!ina219_1.begin()) { Serial.println("Failed to find INA219 at 0x40"); while (1); }
  if (!ina219_2.begin()) { Serial.println("Failed to find INA219 at 0x41"); while (1); }
  if (!ina219_3.begin()) { Serial.println("Failed to find INA219 at 0x44"); while (1); }
  if (!ina219_4.begin()) { Serial.println("Failed to find INA219 at 0x45"); while (1); }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

   // Attempt Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 1) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    attempts++;
  }
  Serial.println("Connected to WiFi");

  // Display IP address on OLED
  String ipAddress = WiFi.localIP().toString();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("IP Address:");
  display.println(ipAddress);
  display.display();  // Show the IP address on the OLED

  // Start the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web server started");
  Serial.println(WiFi.localIP());

  // Setup cutoff pin
  pinMode(cutoffPin, OUTPUT);
  digitalWrite(cutoffPin, HIGH);  // Initially set to HIGH (no cutoff)
}

void loop() {
  // Read INA219 sensor values for total voltage and current
  float Voltage_1 = ina219_1.getBusVoltage_V();
  float Voltage_2 = ina219_2.getBusVoltage_V();
  float Voltage_3 = ina219_3.getBusVoltage_V();
  float Voltage_4 = ina219_4.getBusVoltage_V();

  current_mA_1 = ina219_1.getCurrent_mA();
  current_mA_2 = ina219_2.getCurrent_mA();
  current_mA_3 = ina219_3.getCurrent_mA();
  current_mA_4 = ina219_4.getCurrent_mA();
  float power_mW_4 = ina219_4.getPower_mW();

  // Calculate individual cell voltages
  cellVoltage_1 = Voltage_1;
  cellVoltage_2 = Voltage_2 - Voltage_1;
  cellVoltage_3 = Voltage_3 - Voltage_2;
  cellVoltage_4 = Voltage_4 - Voltage_3;

  // Get temperatures from NTC thermistors
  temperature1 = getTemperature(36);
  temperature2 = getTemperature(39);
  temperature3 = getTemperature(34);
  temperature4 = getTemperature(35);

  // Print to Serial Monitor
  Serial.print("Cell 1 Voltage: "); Serial.print(cellVoltage_1); Serial.print(" V, Current: "); Serial.print(current_mA_1); Serial.println(" mA");
  Serial.print("Cell 2 Voltage: "); Serial.print(cellVoltage_2); Serial.print(" V, Current: "); Serial.print(current_mA_2*100); Serial.println(" mA");
  Serial.print("Cell 3 Voltage: "); Serial.print(cellVoltage_3); Serial.print(" V, Current: "); Serial.print(current_mA_3); Serial.println(" mA");
  Serial.print("Cell 4 Voltage: "); Serial.print(cellVoltage_4); Serial.print(" V, Current: "); Serial.print(current_mA_4); Serial.print(" mA, Power: "); Serial.print(power_mW_4); Serial.println(" mW");

  Serial.print("Temperature 1: "); Serial.print(temperature1); Serial.println(" C");
  Serial.print("Temperature 2: "); Serial.print(temperature2); Serial.println(" C");
  Serial.print("Temperature 3: "); Serial.print(temperature3); Serial.println(" C");
  Serial.print("Temperature 4: "); Serial.print(temperature4); Serial.println(" C");

  // Check cutoff conditions for voltage, current, and temperature
  checkCellConditions(cellVoltage_1, cellVoltage_2, cellVoltage_3, cellVoltage_4, current_mA_1, current_mA_2, current_mA_3, current_mA_4, temperature1, temperature2, temperature3, temperature4);

  // Serve web page
  server.handleClient();

  delay(1000);  // Delay between readings
}

