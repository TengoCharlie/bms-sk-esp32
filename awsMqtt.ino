// === Send MQTT Message ===
// void prepareMqttPayload() {
//   char payload[100];
//   snprintf(payload, sizeof(payload), "Hello from %s | Uptime: %lu sec", getDeviceID(), millis() / 1000);
//   client.publish(MQTT_PUBLISH_TOPIC, payload);
//   Logger::info("Published to", MQTT_PUBLISH_TOPIC, ":", payload);
// }

void prepareMqttPayload() {
  StaticJsonDocument<1024> doc;
  doc["timestamp"] = getTimestamp();
  JsonArray batteries = doc.createNestedArray("batteries");

  for (int i = 1; i <= 4; i++) {
    JsonObject bat = batteries.createNestedObject();
    float v = i == 1 ? cellVoltage_1 : i == 2 ? cellVoltage_2
                                     : i == 3 ? cellVoltage_3
                                              : cellVoltage_4;
    float c = i == 1 ? current_mA_1 : i == 2 ? current_mA_2
                                    : i == 3 ? current_mA_3
                                             : current_mA_4;
    float t = i == 1 ? temperature1 : i == 2 ? temperature2
                                    : i == 3 ? temperature3
                                             : temperature4;

    bat["battery_id"] = "BAT" + String(i);
    bat["percentage"] = voltageToPercentage(v);
    bat["temperature_celsius"] = round(t * 100) / 100.0;
    bat["voltage"] = round(v * 100) / 100.0;
    bat["current"] = round(c * 100) / 100.0;
    bat["health"] = round(estimateBatteryHealth(v));
    bat["soc"] = voltageToPercentage(v);  // Assuming SoC ≈ percentage
    bat["discharge_min"] = round(estimateDischargeTime(v, c));
  }

  // Only publish if changed, or first time, or forced
  if (!firstPublishDone || forcePublish || !compareJson(doc, lastPublishedDoc)) {
    char payload[1024];
    serializeJson(doc, payload);
    client.publish(MQTT_PUBLISH_TOPIC, payload);
    Logger::info("📤 JSON published to MQTT:", payload);
    lastPublishedDoc = doc;
    firstPublishDone = true;
    forcePublish = false;
  } else {
    Logger::debug("🔁 No change in data, MQTT publish skipped.");
  }
}

bool compareJson(const JsonDocument& a, const JsonDocument& b) {
  String s1, s2;
  serializeJson(a, s1);
  serializeJson(b, s2);
  return s1 == s2;
}

// // === MQTT Callback ===
// void callback(char* topic, byte* payload, unsigned int length) {
//   Logger::log("📩 Message arrived on topic:", topic);
//   Serial.print("🔹 Payload: ");
//   for (unsigned int i = 0; i < length; i++) {
//     Serial.print((char)payload[i]);
//   }
//   Serial.println();
// }


// void callback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("📩 Message arrived on topic: ");
//   Serial.println(topic);

//   // Convert payload to String
//   String json;
//   for (unsigned int i = 0; i < length; i++) {
//     json += (char)payload[i];
//   }
//   Serial.print("🔹 Payload: ");
//   Serial.println(json);

//   // Optional: Parse JSON using ArduinoJson
//   StaticJsonDocument<1024> doc;
//   DeserializationError error = deserializeJson(doc, json);
//   if (error) {
//     Serial.print("❌ JSON parse error: ");
//     Serial.println(error.c_str());
//     return;
//   }

//   // Example: Print each battery's ID and percentage
//   JsonArray batteries = doc["batteries"];
//   for (JsonObject battery : batteries) {
//     Serial.print("🔋 Battery ");
//     Serial.print(battery["battery_id"].as<String>());
//     Serial.print(" - %: ");
//     Serial.println(battery["percentage"].as<int>());
//   }
// }

//Json
// void callback(char* topic, byte* payload, unsigned int length) {
//   Serial.print("📩 Message arrived on topic: ");
//   Serial.println(topic);

//   // Convert payload to null-terminated char array
//   char message[length + 1];
//   memcpy(message, payload, length);
//   message[length] = '\0'; // Ensure it's a proper C-string

//   Serial.print("🔹 Payload: ");
//   Serial.println(message);

//   // === Try to parse as JSON ===
//   StaticJsonDocument<1024> doc;
//   DeserializationError error = deserializeJson(doc, message);

//   if (!error) {
//     Serial.println("✅ Detected JSON format.");

//     // If it's a valid JSON object or array, you can inspect the structure
//     if (doc.containsKey("batteries")) {
//       JsonArray batteries = doc["batteries"];
//       for (JsonObject battery : batteries) {
//         Serial.print("🔋 Battery ");
//         Serial.print(battery["battery_id"].as<const char*>());
//         Serial.print(" | %: ");
//         Serial.print(battery["percentage"].as<int>());
//         Serial.print(" | Temp: ");
//         Serial.print(battery["temperature_celsius"].as<float>());
//         Serial.print("°C | Voltage: ");
//         Serial.print(battery["voltage"].as<float>());
//         Serial.print(" V | Current: ");
//         Serial.print(battery["current"].as<float>());
//         Serial.println(" A");
//       }
//     } else {
//       Serial.println("ℹ️ JSON received but does not match expected battery format.");
//       serializeJsonPretty(doc, Serial);
//       Serial.println();
//     }
//   } else {
//     Serial.println("📦 Received plain text or non-JSON message:");
//     Serial.println(message);
//   }
// }

void callback(char* topic, byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (!error && doc.containsKey("command")) {
    String cmd = doc["command"];
    if (cmd == "reboot") {
      Logger::warn("💣 Reboot command received!");
      delay(1000);
      ESP.restart();
    } else if (cmd == "force_push") {
      Logger::info("📦 Force publish triggered");
      forcePublish = true;
    } else {
      Serial.println("ℹ️ JSON received but does not match expected Command format.");
      serializeJsonPretty(doc, Serial);
      Serial.println();
    }
  } else {
    Serial.println("📦 Received plain text or non-JSON message:");
    Serial.println(message);
  }
}




// === AWS MQTT Connect ===
void connectToAWS() {
  net.setCACert(AWS_ROOT_CA_PEM);
  net.setCertificate(AWS_DEVICE_CERT_PEM);
  net.setPrivateKey(AWS_PRIVATE_KEY_PEM);

  client.setServer(AWS_MQTT_ENDPOINT, AWS_MQTT_PORT);
  client.setBufferSize(2048);
  client.setCallback(callback);

  while (!client.connected()) {
    if (client.connect(MQTT_CLIENT_ID)) {
      Logger::log("✅ Connected to AWS IoT");

      if (client.subscribe(MQTT_SUBSCRIBE_TOPIC)) {
        Logger::log("✅ Subscribed to topic:", MQTT_SUBSCRIBE_TOPIC);
      } else {
        Logger::warn("❌ Failed to subscribe to topic:", MQTT_SUBSCRIBE_TOPIC);
      }
    } else {
      Logger::warn("❌ MQTT connect failed with rc =", client.state(), "Retrying...");
      delay(3000);
    }
  }

  logHeap("🔐 Post AWS Connect");
}
