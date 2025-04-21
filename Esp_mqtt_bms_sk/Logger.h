#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

class Logger {
public:
  template<typename T, typename... Args>
  static void log(T first, Args... rest) {
    prefix("📘");
    printAll(first, rest...);
  }

  template<typename T, typename... Args>
  static void info(T first, Args... rest) {
    prefix("ℹ");
    printAll(first, rest...);
  }

  template<typename T, typename... Args>
  static void warn(T first, Args... rest) {
    prefix("⚠");
    printAll(first, rest...);
  }

  template<typename T, typename... Args>
  static void error(T first, Args... rest) {
    prefix("❌");
    printAll(first, rest...);
  }

  template<typename T, typename... Args>
  static void debug(T first, Args... rest) {
    prefix("🐞");
    printAll(first, rest...);
  }

  static void json(const JsonDocument& doc) {
    prefix("📦 JSON");
    serializeJsonPretty(doc, Serial);
    Serial.println();
  }

  static void clear() {
    for (int i = 0; i < 50; i++) Serial.println();
  }

  // Optionally send logs to MQTT (you must declare client elsewhere)
  // static void mqtt(const char* topic, const String& message) {
  //   if (client.connected()) {
  //     client.publish(topic, message.c_str());
  //   }
  // }

private:
  static void prefix(const char* emoji) {
    Serial.print("[" + String(millis()) + " ms] ");
    Serial.print(emoji);
    Serial.print(" ");
  }

  template<typename T>
  static void printAll(T last) {
    Serial.println(last);
  }

  template<typename T, typename... Args>
  static void printAll(T first, Args... rest) {
    Serial.print(first);
    Serial.print(" ");
    printAll(rest...);
  }
};

#endif
