"# bms-sk-esp32" 

create a secrets.h file in the root directory.
Copy this code and Update the certificates, Urls, Topic, and SSID creds
```cpp
#ifndef SECRETS_H
#define SECRETS_H

// === WiFi Credentials ===
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

// === AWS IoT Core MQTT Endpoint (NO https://) ===
const char* AWS_MQTT_ENDPOINT = "<code>.iot.<REGION>.amazonaws.com";
const int AWS_MQTT_PORT = 8883;

// === MQTT Topics ===
const char* MQTT_PUBLISH_TOPIC = "esp32/topic";
const char* MQTT_SUBSCRIBE_TOPIC = "esp32/topic";

// === MQTT Config ===
const char* MQTT_CLIENT_ID       = "ESP32Client";          // You can append MAC address if needed


const char* AWS_ROOT_CA_PEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

const char* AWS_DEVICE_CERT_PEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

const char* AWS_PRIVATE_KEY_PEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
)EOF";

#endif

```