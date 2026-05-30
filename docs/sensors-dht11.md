# DHT11 Maintenance Shaft Monitoring

## Purpose

This document describes the DHT11 integration for temperature and humidity monitoring in the Whirlpool maintenance shaft.

## Wiring

- DHT11 VCC -> ESP32 3V3
- DHT11 GND -> ESP32 GND
- DHT11 DATA -> ESP32 GPIO4

If you use a bare DHT11 sensor (not a module), add a pull-up resistor (about 4.7k to 10k) between DATA and VCC.

## Firmware Behavior

- Sensor type: `DHT11`
- Data pin: `GPIO4`
- Poll interval: `5000 ms`
- Data stale timeout: `30000 ms`

The firmware prints climate data in a dedicated section:

- `temperature` in C
- `humidity` in %

If no valid sample is available yet, firmware reports waiting state.
If the last sample is too old, firmware reports stale sample age.

## Deployment Notes for Maintenance Shaft

- Keep the sensor away from direct splash water.
- Avoid placing it directly next to hot electronics or heater elements.
- Use a ventilated enclosure to get representative ambient values.
- Prefer short signal wiring or shielded cable if the environment is noisy.

## Calibration/Quality Expectations

DHT11 is suitable for basic operational monitoring and trend detection, not precision climate metrology.
For higher precision and better long-term stability, consider a DHT22/SHT31 class sensor later.
