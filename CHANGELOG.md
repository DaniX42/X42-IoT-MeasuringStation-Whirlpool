# Changelog

All notable changes to this project will be documented in this file.

The format is inspired by Keep a Changelog and follows semantic release-style categories.

## [Unreleased]

### Added
- Initial workspace scaffolding with scalable folder layout.
- Baseline project documentation in `docs/`.
- GitHub collaboration templates for issues and pull requests.
- PlatformIO baseline configuration for ESP-WROOM-32 development.
- Initial ESP32 firmware skeleton with startup and chip telemetry output.
- Initial 3-channel SCT013 current sensing firmware on GPIO34/GPIO35/GPIO32.
- Dedicated wiring and calibration guide in `docs/sensors-sct013.md`.
- DHT11 maintenance shaft temperature/humidity monitoring in firmware.
- Dedicated DHT11 wiring and deployment guide in `docs/sensors-dht11.md`.
- Embedded HTTP web server with password-based login and configuration forms.
- HTTP status endpoint returning JSON telemetry.
- OTA update support through browser upload and Arduino OTA.
- MQTT telemetry publishing for integrated sensors.

### Changed
- RMS measurement logic now removes DC offset per sample window for more stable baseline readings.
- Firmware now uses persisted zero-current calibration and supports manual zero refresh via serial command `z`.
- CH2 current conversion factor was empirically calibrated against a 46W incandescent reference load.
- CH1 and CH3 current conversion factors were cross-calibrated against CH2 on the same phase conductor.
- All three current conversion factors were fine-tuned against a 60W incandescent reference load.
- Channel labels were renamed from CH1/CH2/CH3 to L1/L2/L3 for three-phase readability.
- Wi-Fi, MQTT, hostname, sensor topic names, and publish interval are now runtime-configurable via web UI.

### Fixed
- N/A
