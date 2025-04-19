#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secrets.h"

// Clients
WiFiClientSecure net;
PubSubClient client(net);



// Reconnect Safety
int reconnectAttempts = 0;
const int MAX_RECONNECTS = 10;

// === Debug: Heap Usage ===
void logHeap(const char* label) {
  Serial.printf("[HEAP] %s: %u bytes free\n", label, ESP.getFreeHeap());
}

// === Device ID ===
const char* getDeviceID() {
  return MQTT_CLIENT_ID;
}

// === Send MQTT Message ===
void prepareMqttPayload() {
  char payload[100];
  snprintf(payload, sizeof(payload), "Hello from %s | Uptime: %lu sec", getDeviceID(), millis() / 1000);
  client.publish(MQTT_PUBLISH_TOPIC, payload);
  Serial.print("📤 Published to ");
  Serial.print(MQTT_PUBLISH_TOPIC);
  Serial.print(": ");
  Serial.println(payload);
}

// === WiFi Connect ===
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
  } else {
    Serial.println("\n❌ WiFi failed. Rebooting...");
    delay(3000);
    ESP.restart();
  }

  logHeap("Post WiFi Connect");
}

// === MQTT Callback ===
// void callback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("📩 Message arrived on topic: ");
//   Serial.println(topic);
//   Serial.print("🔹 Payload: ");
//   for (unsigned int i = 0; i < length; i++) {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println("\n----------------------------------");

//   // Handle command (example)
//   if (strcmp(topic, MQTT_SUBSCRIBE_TOPIC) == 0) {
//     if (strncmp((char*)payload, "reboot", length) == 0) {
//       Serial.println("💣 Reboot command received!");
//       delay(1000);
//       ESP.restart();
//     }
//   }
// }

// // === AWS MQTT Connect ===
// void connectToAWS() {
//   net.setCACert(AWS_ROOT_CA_PEM);
//   net.setCertificate(AWS_DEVICE_CERT_PEM);
//   net.setPrivateKey(AWS_PRIVATE_KEY_PEM);
//   net.setHandshakeTimeout(30);

//   client.setServer(AWS_MQTT_ENDPOINT, AWS_MQTT_PORT);
//   client.setCallback(callback);

//   while (!client.connected() && reconnectAttempts < MAX_RECONNECTS) {
//     Serial.print("⏳ Connecting to AWS IoT... ");
//     if (client.connect(MQTT_CLIENT_ID)) {
//       Serial.println("✅ Connected to AWS IoT!");

//       if (client.subscribe(MQTT_SUBSCRIBE_TOPIC)) {
//         Serial.print("✅ Subscribed to: ");
//         Serial.println(MQTT_SUBSCRIBE_TOPIC);
//       } else {
//         Serial.print("❌ Failed to subscribe to: ");
//         Serial.println(MQTT_SUBSCRIBE_TOPIC);
//       }

//       reconnectAttempts = 0;
//     } else {
//       Serial.printf("❌ MQTT connect failed, rc=%d. Retrying...\n", client.state());
//       delay(3000);
//       reconnectAttempts++;
//     }
//   }

//   if (reconnectAttempts >= MAX_RECONNECTS) {
//     Serial.println("💥 Too many retries. Restarting...");
//     ESP.restart();
//   }

//   logHeap("Post MQTT Connect");
// }

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("📩 [%s] ", topic);
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
}

void connectToAWS() {
  // Setup TLS
  net.setCACert(AWS_ROOT_CA_PEM);
  net.setCertificate(AWS_DEVICE_CERT_PEM);
  net.setPrivateKey(AWS_PRIVATE_KEY_PEM);

  client.setServer(AWS_MQTT_ENDPOINT, AWS_MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("✅ Connected to AWS IoT");
      if (client.subscribe("esp32/#")) {
        Serial.println("✅ Subscribed to 'esp32/#'");
      } else {
        Serial.println("❌ Subscribe failed");
      }
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}


// === Arduino Setup ===
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("🚀 ESP32 AWS MQTT Starting...");
  logHeap("Initial");

  connectToWiFi();
  connectToAWS();
}

// === Arduino Loop ===
void loop() {
  if (!client.connected()) {
    connectToAWS();
  }

  client.loop();

  static unsigned long lastPub = 0;
  if (millis() - lastPub > 10000) {
    prepareMqttPayload();
    logHeap("Post Publish");
    lastPub = millis();
  }
}
