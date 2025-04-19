#include <Wire.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

// Wi-Fi credentials
const char* ssid = "        ";  // Replace with your network name
const char* password = "        ";  // Replace with your password

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

// Define variables to hold temperature values
float temperature1, temperature2, temperature3, temperature4;

// Define variables to hold cell voltages and currents
float cellVoltage_1, cellVoltage_2, cellVoltage_3, cellVoltage_4;
float current_mA_1, current_mA_2, current_mA_3, current_mA_4;

// Function to convert raw analog value from NTC thermistors to temperature
float getTemperature(int pin) {
  int rawValue = analogRead(pin);
  float resistance = ((4095.0 / rawValue) - 1) * 100000.0; // Assuming 100k reference resistor
  resistance = 100000.0 / resistance;

  // Using Steinhart-Hart equation
  float temperature = log(resistance);
  temperature = 1 / (0.001129148 + (0.000234125 * temperature) + (0.0000000876741 * temperature * temperature * temperature));
  temperature = (temperature - 273.15)/20; // Convert Kelvin to Celsius

  return temperature;
}

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
  WiFi.begin(ssid, password);
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

  delay(100);  // Delay between readings
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>INA219 Sensor Readings</h1>";
  html += "<p>Cell 1 -> Voltage: " + String(cellVoltage_1) + " V, Current: " + String(current_mA_1) + " mA</p>";
  html += "<p>Cell 2 -> Voltage: " + String(cellVoltage_2) + " V, Current: " + String(current_mA_2) + " mA</p>";
  html += "<p>Cell 3 -> Voltage: " + String(cellVoltage_3) + " V, Current: " + String(current_mA_3) + " mA</p>";
  html += "<p>Cell 4 -> Voltage: " + String(cellVoltage_4) + " V, Current: " + String(current_mA_4) + " mA, Power: " + String(ina219_4.getPower_mW()) + " mW</p>";
  
  // Add temperatures to the web page
  html += "<h2>Temperatures</h2>";
  html += "<p>Temperature 1: " + String(temperature1) + " C</p>";
  html += "<p>Temperature 2: " + String(temperature2) + " C</p>";
  html += "<p>Temperature 3: " + String(temperature3) + " C</p>";
  html += "<p>Temperature 4: " + String(temperature4) + " C</p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

bool checkCellConditions(float cellVoltage_1, float cellVoltage_2, float cellVoltage_3, float cellVoltage_4, 
                          float current_mA_1, float current_mA_2, float current_mA_3, float current_mA_4, 
                          float temp1, float temp2, float temp3, float temp4) {
  // Check if any condition exceeds the limits and trigger cutoff
  if (cellVoltage_1 < MIN_VOLTAGE || cellVoltage_1 > MAX_VOLTAGE || current_mA_1 > MAX_CURRENT || temp1 < MIN_TEMPERATURE || temp1 > MAX_TEMPERATURE ||
      cellVoltage_2 < MIN_VOLTAGE || cellVoltage_2 > MAX_VOLTAGE || current_mA_2 > MAX_CURRENT || temp2 < MIN_TEMPERATURE || temp2 > MAX_TEMPERATURE ||
      cellVoltage_3 < MIN_VOLTAGE || cellVoltage_3 > MAX_VOLTAGE || current_mA_3 > MAX_CURRENT || temp3 < MIN_TEMPERATURE || temp3 > MAX_TEMPERATURE ||
      cellVoltage_4 < MIN_VOLTAGE || cellVoltage_4 > MAX_VOLTAGE || current_mA_4 > MAX_CURRENT || temp4 < MIN_TEMPERATURE || temp4 > MAX_TEMPERATURE) {
      
    digitalWrite(cutoffPin, LOW);  // Cutoff if any condition fails
    Serial.println("Cutoff triggered due to out-of-range voltage, current, or temperature.");

    // Log conditions when cutoff occurs
    scanAndLogCellConditions(cellVoltage_1, cellVoltage_2, cellVoltage_3, cellVoltage_4, 
                             current_mA_1, current_mA_2, current_mA_3, current_mA_4, temp1, temp2, temp3, temp4);

    return true;  // Return true if cutoff occurred
  } else {
    digitalWrite(cutoffPin, HIGH);  // No cutoff if all conditions are within limits
    return false;  // Return false if no cutoff
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