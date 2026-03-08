# Changelog

All notable changes to this project will be documented in this file.

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
