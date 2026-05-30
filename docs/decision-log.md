# Decision Log

This file records relevant architectural and process decisions.

## 2026-05-30 - Build System and Firmware Baseline

### Context
The project required a practical implementation baseline for ESP-WROOM-32 development and reliable USB flash/monitor workflows.

### Decision
Use PlatformIO in VS Code as the primary build and upload workflow, with environment `esp32dev` and Arduino framework for the initial firmware baseline.

### Consequences
- Reproducible build/upload setup with versioned configuration.
- Faster iteration through integrated serial monitor and device tooling.
- `esptool` remains optional for low-level diagnostics, not mandatory for daily workflow.

## 2026-05-30 - Upload Reliability Setting

### Context
Initial flashing at 921600 baud established a connection but failed during packet transfer verification.

### Decision
Set `upload_speed = 115200` in `platformio.ini` for stable USB flashing on the current hardware/cable setup.

### Consequences
- More robust flashing process with lower transfer rate.
- Reduced risk of intermittent upload failures during active development.

## 2026-05-30 - Initial 3-Channel Current Sensing Layout

### Context
Three current sensing channels were assembled around SCT013 sensors and an analog bias network.

### Decision
Implement first firmware support for three channels on ADC1 pins `GPIO34`, `GPIO35`, and `GPIO32`, using RMS estimation with configurable per-channel `biasVoltage` and `ampsPerVolt` factors.

### Consequences
- Immediate multi-channel telemetry is available for integration tests.
- Calibration remains mandatory before production-grade measurements.
- Hardware bill of materials and exact sensor variant must be confirmed to lock final conversion coefficients.

## 2026-05-30 - Startup Zero Calibration for SCT013 Channels

### Context
With empty CT clamps, residual analog noise and frontend asymmetry produced non-zero idle RMS readings on all three channels.

### Decision
Store zero-current calibration offsets and refresh them only when explicitly requested over serial, instead of recalibrating on every boot.

### Consequences
- No-load readings can be restored without risking accidental recalibration during a reboot under load.
- A fresh zero baseline is now taken by sending `z` while clamps are empty.
- Load calibration using a reference meter remains necessary for accurate absolute current values.

## 2026-05-30 - First Load Calibration on L2

### Context
One channel was tested with a `46 W` incandescent lamp on a `230 V` phase, corresponding to an expected current of roughly `0.200 A`.

### Decision
Adjust `L2` conversion factor in firmware from `50.0` to `0.356` based on the observed corrected reading of approximately `28.1 A` before calibration.

### Consequences
- `L2` now has a first empirical calibration tied to a known resistive load.
- Validated L2 reading is approximately `0.192 A`, which is close to the expected `0.200 A`.
- `L1` and `L3` still require their own load calibration.
- Final accuracy should still be verified with a proper reference meter.

## 2026-05-30 - Cross Calibration of L1 and L3

### Context
All three CT clamps were placed on the same live phase, allowing direct relative comparison between channels.

### Decision
Use calibrated `L2` as the temporary reference and adjust `L1` and `L3` conversion factors to match the shared phase current.

### Consequences
- All three channels can now report comparable current on the same conductor.
- Absolute accuracy still depends on the correctness of the original CH2 reference load assumption.
- A final verification with an external meter remains recommended.
- Post-calibration validation on the same phase showed all channels clustered around `0.18 A` to `0.19 A`.

## 2026-05-30 - 60W Fine Tuning Pass

### Context
A `60 W` incandescent lamp provided a second resistive reference load, corresponding to about `0.261 A` at `230 V`.

### Decision
Apply a follow-up scale adjustment on all three channels to bring the measured values closer to the 60 W reference current.

### Consequences
- Absolute current should now better match small resistive loads.
- The calibration remains based on assumed mains voltage and incandescent behavior.
- A meter-based final verification is still the preferred endpoint.
- Post-tuning validation with the 60 W lamp landed essentially on the expected `0.261 A`, with minor drift remaining on CH3.

## 2026-05-30 - DHT11 Maintenance Shaft Monitoring

### Context
The Whirlpool maintenance shaft also requires environmental monitoring to detect high humidity and temperature trends.

### Decision
Integrate a DHT11 sensor on `GPIO4` with periodic polling and serial reporting for temperature and humidity.

### Consequences
- Climate data is now available alongside phase current telemetry.
- Firmware reports waiting/stale states to make sensor quality visible.
- DHT11 precision is acceptable for trend monitoring but not for high-accuracy instrumentation.

## 2026-05-30 - Initial Workspace Baseline

### Context
The repository started without structure or baseline documentation.

### Decision
Create a scalable, technology-agnostic project skeleton with dedicated folders for source code, documentation, scripts, config, tests, and GitHub collaboration templates.

### Consequences
- Faster onboarding and clearer contributor expectations.
- Immediate place for architecture and setup evolution.
- Technology-specific tooling can be added later without restructuring the repository.
