# Changelog

All notable changes to this project will be documented in this file.

> Entries for `1.2.2` – `1.2.9` were reconstructed from the Git history after
> the fact and are therefore coarser than the hand-written ones.

## [Unreleased]

### Changed
- `get_graph_data()` releases the response body buffer before re-serializing the
  filtered document into `get_last_graph_json()`. Previously the raw body (~16 kB),
  the parsed document and the re-serialized copy were all resident at the same
  time, which is the worst moment to ask a fragmented heap for a contiguous block.
- Deserialization filters are built once instead of on every fetch. The two
  schemas (connection and graph) used to share a single `JsonDocument` that was
  populated and cleared inside each request — roughly thirty document insertions
  per cycle, and the two schemas were only ever kept apart by the `clear()` at the
  end of each caller. They are now two separate documents, populated lazily on
  first use.

### Removed
- Unused `LIBRELINKUP_JSON_BUFFER_SIZE` and `LIBRELINKUP_FILTER_JSON_BUFFER_SIZE`
  defines. ArduinoJson 7 documents grow elastically; these had looked like active
  tuning knobs since the v7 migration.

### Internal
- The body-read helpers share a single `body_append_available()` routine, so the
  append/NUL-terminate/heap-check sequence exists once instead of twice.

## [1.3.1] - 2026-08-16

### Changed
- `LIBRELINKUP_BODY_IDLE_TIMEOUT_MS` raised to 20 s to tolerate slower API responses.

## [1.3.0] - 2026-08-15

### Fixed
- **Fetch stall.** `get_graph_data()` could hang indefinitely while reading the
  response body. `HTTPClient::getString()` delegates to a loop whose only exit is
  `connected()` going false; on a half-open TLS connection that never happens,
  because `WiFiClientSecure::connected()` does not treat `MBEDTLS_ERR_SSL_WANT_READ`
  as an error. The loop then spins on `delay(1)` forever — and because it yields,
  no watchdog fires. A missing `Content-Length` (i.e. the chunked responses this
  client asks for) made the loop's length condition permanently true as well.
  Replaced by `read_body_bounded()`, which enforces an idle timeout and a total
  deadline, and decodes `Transfer-Encoding: chunked` framing itself.
- `WiFiClientSecure::setTimeout()` was being passed milliseconds, but it takes
  **seconds** — the configured value meant roughly 2 h 46 min instead of 10 s.
- TLS handshake capped at 15 s (the default is 120 s, long enough to look like a
  hang and to outlast a typical application watchdog).
- HTTP connect timeout set explicitly to 10 s instead of relying on the 5 s default.

### Added
- `setCACertBundle(const uint8_t*)` and certificate mode `begin(3)` for using an
  ESP-IDF x509 root CA bundle instead of a single embedded or file-based certificate.
- `breadcrumb()` accessor exposing the current phase of the HTTP/parse pipeline
  (`auth.http_post`, `graph.deserialize`, …) so an external hang detector can
  report *where* a stall sits.
- Deserialization failures now log the body length and its first bytes, which
  distinguishes a truncated body from undecoded chunk framing from a CDN error page.

### Removed
- Baltimore CyberTrust root CA URLs and storage path.

## [1.2.9] - 2026-07-09

### Changed
- Maintenance and CI-only release; no library behavior changes.

## [1.2.8] - 2026-04-12

### Added
- `SENSOR_SN_LIBRE3PLUS_THRESHOLD` define for the serial-number prefix that
  distinguishes Libre 3 Plus from Libre 3.
- `dtid` device field read from the API to support sensor type detection.

### Changed
- Sensor type detection reverted to the serial-number check after the enum-based
  approach was trialled and rolled back.

### Fixed
- Sensor type cache flag was re-evaluating an already-checked sensor.

## [1.2.7] - 2026-04-12

### Fixed
- Certificate file handle is closed when the HTTP code is negative.

## [1.2.6] - 2026-04-11

### Changed
- Internal refactor: buffer initialization and loop guards.
- Libre 3 Plus detection threshold changed to serial number prefix `0G0000`.

## [1.2.5] - 2026-03-22

### Changed
- Dependency tree in `library.json` restructured.

## [1.2.4] - 2026-03-15

### Added
- Unit tests wired into CI.
- HTTP fetch duration is logged.

### Changed
- Logging cleanup.

## [1.2.3] - 2026-03-10

### Changed
- Migrated to ArduinoJson 7 (elastic `JsonDocument`); version-conditional
  `#ifdef` blocks removed and CI updated accordingly.

## [1.2.2] - 2026-03-08

### Changed
- Doxygen descriptions updated; CI build reduced to one example.

## [1.1.0] - 2026-03-08

### Added
- Public getter accessors for runtime data structs (login, glucose, history, status, lifetime).
- `set_credentials(...)` and helper checks for credentials/token presence.
- `masked_user_token(...)` and `clear_sensitive_runtime_data()` helpers.
- New usage example: `examples/error_handling/error_handling.ino`.
- CI workflow for ESP32 builds via PlatformIO.

### Changed
- Improved HTTP status handling in `get_connection_data()` and `get_graph_data()`.
- Unified parser flow for graph data (`parse_graph_json_doc()`) across API and ingest paths.
- Version metadata aligned to `1.1.0`.
- README extended with API stability and release checklist.

### Fixed
- Re-authentication path for unauthorized API responses.
- `check_sensor_type()` cache flag (`already_checked`) was unreachable.
- Certificate file handling now closes file on early return in `setCAfromfile()`.
- Safer certificate buffer allocation in `showCAfromfile()`.
- Timezone lock rejects invalid parsed timestamps (`<= 0`).

### Security
- Documentation now explicitly warns against using `begin(0)` in production.
