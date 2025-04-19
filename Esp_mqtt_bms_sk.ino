#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Optional: OLED display (can be skipped if not used)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define to use mock data
// #define USE_MOCK_SENSORS

#ifdef USE_MOCK_SENSORS
float getVoltage(int ch) { return 3.5 + (ch * 0.05); }
float getCurrent(int ch) { return 1000 + (ch * 10); }
float getTemperature(int pin) { return 25.0 + (pin % 3); }
void initSensors() { Serial.println("🔧 Mock sensors initialized."); }
#else
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219_1(0x40), ina219_2(0x41), ina219_3(0x44), ina219_4(0x45);
void initSensors() {
  ina219_1.begin(); ina219_2.begin(); ina219_3.begin(); ina219_4.begin();
}
float getVoltage(int ch) {
  switch (ch) {
    case 1: return ina219_1.getBusVoltage_V();
    case 2: return ina219_2.getBusVoltage_V();
    case 3: return ina219_3.getBusVoltage_V();
    case 4: return ina219_4.getBusVoltage_V();
    default: return 0.0;
  }
}
float getCurrent(int ch) {
  switch (ch) {
    case 1: return ina219_1.getCurrent_mA();
    case 2: return ina219_2.getCurrent_mA();
    case 3: return ina219_3.getCurrent_mA();
    case 4: return ina219_4.getCurrent_mA();
    default: return 0.0;
  }
}
float getTemperature(int pin) {
  int raw = analogRead(pin);
  float resistance = ((4095.0 / raw) - 1) * 100000.0;
  resistance = 100000.0 / resistance;
  float tempK = log(resistance);
  tempK = 1 / (0.001129148 + (0.000234125 * tempK) + (0.0000000876741 * tempK * tempK * tempK));
  return tempK - 273.15;
}
#endif

WiFiClientSecure net;
PubSubClient client(net);

const int cutoffPin = 4;
const float MIN_VOLTAGE = 2.0, MAX_VOLTAGE = 4.2, MAX_CURRENT = 6000;
const float MIN_TEMPERATURE = -20.0, MAX_TEMPERATURE = 50.0;

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000); Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected"); Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 ["); Serial.print(topic); Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
}

void connectToAWS() {
  net.setCACert(AWS_ROOT_CA_PEM);
  net.setCertificate(AWS_DEVICE_CERT_PEM);
  net.setPrivateKey(AWS_PRIVATE_KEY_PEM);
  client.setServer(AWS_MQTT_ENDPOINT, AWS_MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("✅ Connected to AWS IoT");
      client.subscribe("esp32/#");
    } else {
      Serial.print("Retrying MQTT... ");
      delay(2000);
    }
  }
}

int voltageToPercentage(float v) {
  return constrain((int)((v - 3.2) / (4.2 - 3.2) * 100), 0, 100);
}

bool checkCellConditions(float voltage, float current, float temp, const String& cellLabel) {
  bool cutoff = false;
  if (voltage < MIN_VOLTAGE) {
    Serial.println(cellLabel + ": Voltage too LOW");
    cutoff = true;
  } else if (voltage > MAX_VOLTAGE) {
    Serial.println(cellLabel + ": Voltage too HIGH");
    cutoff = true;
  }
  if (current > MAX_CURRENT) {
    Serial.println(cellLabel + ": Current too HIGH");
    cutoff = true;
  }
  if (temp < MIN_TEMPERATURE) {
    Serial.println(cellLabel + ": Temp too LOW");
    cutoff = true;
  } else if (temp > MAX_TEMPERATURE) {
    Serial.println(cellLabel + ": Temp too HIGH");
    cutoff = true;
  }
  return cutoff;
}

void publishSensorData() {
  float V[5], I[5], T[5];
  for (int i = 1; i <= 4; i++) {
    V[i] = getVoltage(i);
    I[i] = getCurrent(i);
  }

  V[2] -= V[1]; V[3] -= V[2] + V[1]; V[4] -= V[3] + V[2] + V[1];

  T[1] = getTemperature(36);
  T[2] = getTemperature(39);
  T[3] = getTemperature(34);
  T[4] = getTemperature(35);

  bool trigger = false;
  for (int i = 1; i <= 4; i++) {
    String id = "BAT" + String(i);
    trigger |= checkCellConditions(V[i], I[i], T[i], id);
  }

  digitalWrite(cutoffPin, trigger ? LOW : HIGH);

  StaticJsonDocument<1024> doc;
  doc["timestamp"] = millis();  // Use NTP or RTC in real apps
  JsonArray batteries = doc.createNestedArray("batteries");

  for (int i = 1; i <= 4; i++) {
    JsonObject bat = batteries.createNestedObject();
    bat["battery_id"] = "BAT" + String(i);
    bat["percentage"] = voltageToPercentage(V[i]);
    bat["temperature_celsius"] = round(T[i] * 100) / 100.0;
    bat["voltage"] = round(V[i] * 100) / 100.0;
    bat["current"] = round(I[i] * 100) / 100.0;
  }

  char jsonStr[1024];
  serializeJson(doc, jsonStr);
  client.publish(MQTT_PUBLISH_TOPIC, jsonStr);
  Serial.println("📤 JSON published to MQTT:");
  Serial.println(jsonStr);
}

void setup() {
  Serial.begin(115200);
  pinMode(cutoffPin, OUTPUT);
  digitalWrite(cutoffPin, HIGH);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Booting...");
    display.display();
  }

  connectToWiFi();
  connectToAWS();
  initSensors();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("MQTT Connected");
  display.display();
}

void loop() {
  if (!client.connected()) connectToAWS();
  client.loop();

  publishSensorData();
  delay(10000);
}
