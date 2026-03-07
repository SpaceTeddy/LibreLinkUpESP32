# LibreLinkUp Library - Quick Start

This README is for users who want to use the module directly in their own ESP32/PlatformIO project.

License: GNU General Public License v2.0 (`GPL-2.0-only`). See `LICENSE`.

## 1. What the library does

`LIBRELINKUP` handles:
- LibreLinkUp/LibreView authentication
- Current glucose value retrieval
- Historical/graph data retrieval
- Re-authentication when tokens expire

## 2. Requirements

- ESP32 + Arduino framework
- Active Wi-Fi connection
- Valid LibreLinkUp credentials (email + password)
- LittleFS available (if certificate mode `2` is used)

## 3. Include and instantiate

```cpp
#include "librelinkup.h"
```

Create an instance:
```cpp
LIBRELINKUP librelinkup;
```

## 4. Minimal usage flow (important)

```cpp
void setup() {
  // 1) Wi-Fi must already be connected

  // 2) Provide credentials (once or after loading config)
  librelinkup.set_credentials("you@example.com", "your_password");

  // 3) Initialize
  // 0 = insecure, 1 = embedded cert, 2 = cert from file
  // Recommended for first setup: 1 (fewer external dependencies)
  librelinkup.begin(1);
}

void loop() {
  // 4) Fetch data
  if (librelinkup.get_graph_data() == 1) {
    const auto& g = librelinkup.glucose_data();
    Serial.printf("Glucose: %u mg/dL, Trend: %s\n",
                  g.glucoseMeasurement,
                  g.str_trendArrow.c_str());
  }

  delay(60000); // e.g. every 60s
}
```

## 5. Where to read data

After a successful `get_graph_data()` call:

- Current values: `librelinkup.glucose_data()`
- Sensor status: `librelinkup.status()` and `librelinkup.sensor_data()`
- History: `librelinkup.sensor_history_data()`
- Remaining sensor lifetime: `librelinkup.sensor_lifetime()`
- Login/token data: `librelinkup.login_data()`

## 6. Re-auth behavior

- If token/user ID is missing or invalid, the library tries to re-auth automatically.
- Credentials must be set first (`set_credentials(...)` or at least one successful `auth_user(...)`).
- If no credentials are available, requests will fail as expected.

## 7. Common issues

1. `Auth User failed` / `no stored credentials available`  
   Cause: `set_credentials(...)` was never called.

2. HTTP/TLS error during startup  
   Cause: wrong certificate mode or missing cert file in LittleFS when using `begin(2)`.
   Tip: start with `begin(1)` first, then move to `begin(2)` if needed.

3. No data even though build is successful  
   Cause: Wi-Fi/NTP/LibreLinkUp account/region configuration issue.

## 8. API reference (short)

- Initialization:
  - `uint8_t begin(uint8_t use_cert)`
  - `void set_credentials(const String& user_email, const String& user_password)`
  - `bool has_credentials() const`

- Network/API:
  - `uint16_t auth_user(String user_email, String user_password)`
  - `uint16_t tou_user()`
  - `uint16_t get_connection_data()`
  - `uint16_t get_graph_data()`

- Data access:
  - `glucose_data()`
  - `status()`
  - `sensor_data()`
  - `sensor_history_data()`
  - `sensor_lifetime()`
  - `login_data()`

## 9. Note

The library uses internal logging (`uuid::log::Logger`) and is designed for ESP32/Arduino.

## 10. Certificate mode recommendation

- `begin(1)`:
  - Best default for new users
  - No LittleFS certificate file required

- `begin(2)`:
  - Use when you explicitly want certs loaded from filesystem
  - Requires valid cert file handling in LittleFS

- `begin(0)`:
  - Insecure (testing only)

## 11. Integration checklist (Git/PlatformIO)

Use this checklist when adding the library from Git into another project.

1. Add library dependency
   - In `platformio.ini`:
```ini
lib_deps =
  https://github.com/<your-user>/LibreLinkUpESP32.git
  bblanchon/ArduinoJson@^6.21.5
  https://github.com/nomis/mcu-uuid-log.git
```

2. Ensure framework/platform
   - `framework = arduino`
   - `platform = espressif32`

3. Ensure Wi-Fi is connected before first API call
   - Call `WiFi.begin(...)` and wait for `WL_CONNECTED`.

4. Set credentials before fetching
```cpp
librelinkup.set_credentials(email, password);
```

5. Initialize once
   - Start with:
```cpp
librelinkup.begin(1);
```
   - Only use `begin(2)` when your LittleFS cert flow is ready.

6. Fetch and read values
```cpp
if (librelinkup.get_graph_data() == 1) {
  const auto& g = librelinkup.glucose_data();
  // use g.glucoseMeasurement, g.str_trendArrow, ...
}
```

7. If fetch fails
   - Verify credentials were set.
   - Verify Wi-Fi is still connected.
   - Try `begin(1)` before debugging cert files.
   - Check serial logs for auth/TLS messages.
