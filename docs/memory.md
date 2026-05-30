# Project Memory

This document stores persistent project knowledge.

## Entries

### 2026-05-30
- [DECISION] Use a modular, layer-oriented architecture baseline documented in `docs/architecture.md`.
- [DECISION] Establish `main` + short-lived feature/fix/docs branch strategy for collaboration.
- [DECISION] Select PlatformIO with ESP32 Arduino framework as the default implementation stack.
- [TASK] Runtime stack selection completed; maintain `platformio.ini` as source of truth.
- [TASK] Add implementation skeleton under `src/` following layer mapping and initial ESP32 startup telemetry.
- [IDEA] Add automated CI checks for docs, linting, and tests once stack is finalized.
- [ISSUE] Sensor-specific requirements and pin mapping are not finalized yet.
- [DECISION] Use `upload_speed = 115200` due observed transfer instability at 921600 on `/dev/cu.usbserial-10`.
- [TASK] Confirm if the same serial adapter path persists across reboots or if dynamic port detection script is needed.
- [IDEA] Add a `scripts/read-device-info.sh` helper to automate chip/flash/port checks.
- [ISSUE] Connected board verified as ESP32-D0WD-V3, 4MB flash, MAC `e4:65:b8:71:45:b0`; keep this as current hardware fingerprint for diagnostics.
- [DECISION] Implement 3-channel RMS sensing on GPIO34/GPIO35/GPIO32 with per-window DC offset removal.
- [DECISION] Run automatic startup zero calibration and subtract idle RMS offsets from all SCT013 channels.
- [DECISION] Persist zero-current offsets and refresh them manually with serial command `z` instead of recalibrating on every boot.
- [TASK] Perform per-channel calibration against a reference meter and update `ampsPerVolt` constants.
- [TASK] Keep CT clamps empty during boot whenever a fresh zero baseline is required.
- [ISSUE] Latest validated zero offsets are L1 `0.203 A`, L2 `0.198 A`, L3 `0.137 A`; corrected no-load readings are approximately `0.000 A`.
- [DECISION] Calibrate L2 with a 46 W incandescent reference load at 230 V and set `ampsPerVolt` to `0.356`.
- [DECISION] Cross-calibrate L1 and L3 against L2 while all three clamps are on the same phase.
- [TASK] Verify all three channels later against an external reference meter.
- [ISSUE] L2 validation under the 46 W lamp is approximately `0.189 A` to `0.193 A`, which is acceptable for the current baseline.
- [ISSUE] Verified same-phase readings after cross-calibration are L1 `0.177-0.184 A`, L2 `0.189-0.192 A`, L3 `0.184-0.190 A`.
- [DECISION] Fine-tune all three channels against a 60 W incandescent reference load at 230 V.
- [ISSUE] Pre-adjustment 60 W readings were L1 `0.205-0.206 A`, L2 `0.245-0.249 A`, L3 `0.236-0.242 A` versus expected `0.261 A`.
- [ISSUE] Post-adjustment 60 W readings are approximately L1 `0.261 A`, L2 `0.260-0.263 A`, L3 `0.261-0.274 A`; L3 still shows slight upward drift.
- [TASK] Latest manual zero refresh saved offsets of L1 `0.005 A`, L2 `0.002 A`, L3 `0.004 A` with all clamps empty.
- [ISSUE] Corrected no-load readings after the latest zero refresh are stable at `0.000 A` on all channels.
- [DECISION] Add DHT11 monitoring on GPIO4 for maintenance shaft temperature and humidity.
- [TASK] Validate DHT11 values in real shaft conditions and define alert thresholds.
- [ISSUE] DHT11 is intended for trend-level monitoring, not precision-grade climate measurement.
- [ISSUE] Runtime verification succeeded: DHT11 initially reports waiting state and then valid samples (for example 29.3 C and 27.0 %RH).
- [DECISION] The long-term target is a Whirlpool station with 3-phase consumption, water temperature, water pH, outdoor temperature/humidity, and daylight brightness monitoring.
- [DECISION] The platform target includes Wi-Fi networking, MQTT telemetry and MQTT-triggered calibration workflows for sensors.
- [DECISION] The platform target also includes OTA updates, a password-protected web dashboard, an HTTP JSON status endpoint, and a local display with date/time, pH, and water temperature.

## Update Rules

- Update existing entries when status changes instead of creating duplicates.
- Keep tags consistent: `[DECISION]`, `[TASK]`, `[ISSUE]`, `[IDEA]`.
- Add date headers for new update batches.
