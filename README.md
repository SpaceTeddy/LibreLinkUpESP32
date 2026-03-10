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

### Dependency note (PlatformIO)

Only external libraries are listed in `library.json` dependencies:
- `bblanchon/ArduinoJson`
- `nomis/mcu-uuid-log`

Core headers used by this library are provided by the ESP32 Arduino core/toolchain and are therefore **not** listed as extra dependencies:
- `WiFiClientSecure.h`
- `HTTPClient.h`
- `FS.h`
- `LittleFS.h`
- `mbedtls/sha256.h`
- `string.h`

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

## 9. API stability

Stable API (recommended for long-term integration):
- `begin(...)`
- `set_credentials(...)`
- `auth_user(...)`
- `tou_user()`
- `get_connection_data()`
- `get_graph_data()`
- `glucose_data()`
- `sensor_data()`
- `sensor_history_data()`
- `sensor_lifetime()`
- `status()`
- `login_data()`

Advanced/low-level API (may change between minor versions):
- `setCAfromfile(...)`
- `showCAfromfile(...)`
- `download_root_ca_to_file(...)`
- `check_https_connection(...)`
- `read2String(...)`
- `parseTimestamp(...)`
- `get_wifisecureclient()`

## 10. Note

The library uses internal logging (`uuid::log::Logger`) and is designed for ESP32/Arduino.

## 11. Certificate mode recommendation

- `begin(1)`:
  - Best default for new users
  - No LittleFS certificate file required

- `begin(2)`:
  - Use when you explicitly want certs loaded from filesystem
  - Requires valid cert file handling in LittleFS

- `begin(0)`:
  - Insecure (testing only, not for production)
  - TLS certificate validation is disabled and enables MITM attacks

## 12. Integration checklist (Git/PlatformIO)

Use this checklist when adding the library from Git into another project.

1. Add library dependency
   - In `platformio.ini`:
```ini
lib_deps =
  https://github.com/SpaceTeddy/LibreLinkUpESP32.git
  bblanchon/ArduinoJson@^7.2.2
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

## 13. Examples

- `examples/minimal_fetch/minimal_fetch.ino`: minimal polling loop.
- `examples/error_handling/error_handling.ino`: robust retry and diagnostics flow.

## 14. Release checklist

1. Bump versions manually with:
```bash
./scripts/bump-version.sh 1.2.0
```
2. Validate consistency manually:
```bash
./scripts/check-version-consistency.sh
```
3. Update `CHANGELOG.md`.
4. Run CI and ensure all checks are green.
5. Create a Git tag, for example `v1.2.0`.
6. Test installation from tag in a fresh PlatformIO project:
```ini
lib_deps =
  https://github.com/SpaceTeddy/LibreLinkUpESP32.git#v1.2.0
```

### Optional: One-click release from GitHub Actions

You can also trigger a manual release via GitHub UI:
1. Open `Actions` -> `Manual Release`.
2. Click `Run workflow`.
3. Enter `version` (for example `1.2.0`).
4. Keep `dry_run=true` for a safe check run, set `dry_run=false` to publish.

With `dry_run=false`, the workflow will:
- bump version fields,
- validate consistency,
- commit release changes,
- create and push tag `vX.Y.Z`,
- create a GitHub Release with autogenerated notes.
