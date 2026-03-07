#include <Arduino.h>
#include <WiFi.h>
#include <librelinkup.h>

LIBRELINKUP librelinkup;

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* LLU_EMAIL = "you@example.com";
const char* LLU_PASS  = "your_password";

void setup() {
  Serial.begin(115200);
  delay(500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.println("WiFi connected");

  librelinkup.set_credentials(LLU_EMAIL, LLU_PASS);
  librelinkup.begin(1); // recommended default for first setup
}

void loop() {
  if (librelinkup.get_graph_data() == 1) {
    const auto& g = librelinkup.glucose_data();
    Serial.printf("Glucose: %u mg/dL, trend: %s\n",
                  g.glucoseMeasurement,
                  g.str_trendArrow.c_str());
  } else {
    Serial.println("Fetch failed");
  }

  delay(60000);
}
