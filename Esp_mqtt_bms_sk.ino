#include <Wire.h>
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "secrets.h"
#include <ArduinoJson.h>

// INA219 sensors
Adafruit_INA219 ina219_1(0x40);
Adafruit_INA219 ina219_2(0x41);
Adafruit_INA219 ina219_3(0x44);
Adafruit_INA219 ina219_4(0x45);

// OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MQTT client
WiFiClientSecure net;
PubSubClient client(net);

// Cutoff control
const int cutoffPin = 4;
const float MIN_VOLTAGE = 2.0;
const float MAX_VOLTAGE = 4.2;
const float MAX_CURRENT = 6000;
const float MIN_TEMPERATURE = -20.0;
const float MAX_TEMPERATURE = 50.0;

// Sensor variables
float temperature1, temperature2, temperature3, temperature4;
float cellVoltage_1, cellVoltage_2, cellVoltage_3, cellVoltage_4;
float current_mA_1, current_mA_2, current_mA_3, current_mA_4;

// Get temperature from analog NTC thermistor
float getTemperature(int pin) {
  int rawValue = analogRead(pin);
  float resistance = ((4095.0 / rawValue) - 1) * 100000.0;
  resistance = 100000.0 / resistance;

  float tempK = log(resistance);
  tempK = 1 / (0.001129148 + (0.000234125 * tempK) + (0.0000000876741 * tempK * tempK * tempK));
  return tempK - 273.15;
}

// MQTT callback (optional for commands)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
}

// Connect to Wi-Fi
void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  Serial.println("\n✅ WiFi connected");
  Serial.println(WiFi.localIP());
}

// Connect to AWS IoT
void connectToAWS() {
  net.setCACert(AWS_ROOT_CA_PEM);
  net.setCertificate(AWS_DEVICE_CERT_PEM);
  net.setPrivateKey(AWS_PRIVATE_KEY_PEM);
  client.setServer(AWS_MQTT_ENDPOINT, AWS_MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("🔄 Connecting to AWS IoT... ");
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("✅ Connected!");
      client.subscribe("esp32/#");
    } else {
      Serial.print("❌ Failed, rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// Publish JSON data to MQTT
void publishSensorData() {
  StaticJsonDocument<512> doc;
  doc["cell1_voltage"] = cellVoltage_1;
  doc["cell2_voltage"] = cellVoltage_2;
  doc["cell3_voltage"] = cellVoltage_3;
  doc["cell4_voltage"] = cellVoltage_4;

  doc["current1_mA"] = current_mA_1;
  doc["current2_mA"] = current_mA_2;
  doc["current3_mA"] = current_mA_3;
  doc["current4_mA"] = current_mA_4;

  doc["temp1_C"] = temperature1;
  doc["temp2_C"] = temperature2;
  doc["temp3_C"] = temperature3;
  doc["temp4_C"] = temperature4;

  char buffer[512];
  serializeJson(doc, buffer);
  client.publish(MQTT_PUBLISH_TOPIC, buffer);
  Serial.println("📤 JSON published to MQTT");
}

void setup() {
  Serial.begin(115200);

  if (!ina219_1.begin() || !ina219_2.begin() || !ina219_3.begin() || !ina219_4.begin()) {
    Serial.println("❌ INA219 sensor init failed");
    while (true);
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED init failed");
    while (true);
  }

  pinMode(cutoffPin, OUTPUT);
  digitalWrite(cutoffPin, HIGH);

  connectToWiFi();
  connectToAWS();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("MQTT Connected");
  display.display();
}

void loop() {
  float V1 = ina219_1.getBusVoltage_V();
  float V2 = ina219_2.getBusVoltage_V();
  float V3 = ina219_3.getBusVoltage_V();
  float V4 = ina219_4.getBusVoltage_V();

  current_mA_1 = ina219_1.getCurrent_mA();
  current_mA_2 = ina219_2.getCurrent_mA();
  current_mA_3 = ina219_3.getCurrent_mA();
  current_mA_4 = ina219_4.getCurrent_mA();

  cellVoltage_1 = V1;
  cellVoltage_2 = V2 - V1;
  cellVoltage_3 = V3 - V2;
  cellVoltage_4 = V4 - V3;

  temperature1 = getTemperature(36);
  temperature2 = getTemperature(39);
  temperature3 = getTemperature(34);
  temperature4 = getTemperature(35);

  publishSensorData();

  client.loop();
  delay(10000);  // Send every 10 seconds
}
