#include <Arduino.h>
#include <WiFi.h>
#include <librelinkup.h>

LIBRELINKUP librelinkup;

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* LLU_EMAIL = "you@example.com";
const char* LLU_PASS  = "your_password";

static bool connectWifi(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > timeoutMs) {
      return false;
    }
    delay(250);
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  if (!connectWifi(15000)) {
    Serial.println("WiFi connect timeout");
    return;
  }

  librelinkup.set_credentials(LLU_EMAIL, LLU_PASS);
  if (!librelinkup.begin(1)) {
    Serial.println("LLU begin failed");
    return;
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost, reconnecting...");
    if (!connectWifi(15000)) {
      Serial.println("WiFi reconnect failed");
      delay(5000);
      return;
    }
  }

  const uint16_t ok = librelinkup.get_graph_data();
  if (ok == 1) {
    const auto& g = librelinkup.glucose_data();
    Serial.printf("Glucose: %u mg/dL | Trend: %s\n",
                  g.glucoseMeasurement,
                  g.str_trendArrow.c_str());
  } else {
    const auto& login = librelinkup.login_data();
    Serial.printf("Fetch failed | user_id=%s token=%s\n",
                  login.user_id.c_str(),
                  librelinkup.masked_user_token().c_str());
  }

  delay(60000);
}
