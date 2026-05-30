# SCT013 Multi-Channel Wiring and Calibration

## Scope

This document describes a 3-channel SCT013 setup for ESP32 using ADC1 pins.

## Channel Mapping

- L1 -> GPIO34 (ADC1)
- L2 -> GPIO35 (ADC1)
- L3 -> GPIO32 (ADC1)

## Circuit Notes

- Midpoint bias network is required for AC waveform sampling.
- Typical midpoint for ESP32 ADC with 3.3V rail is around 1.65V.
- The firmware computes RMS from each sampling window by removing DC offset automatically.
- The firmware uses stored zero-current offsets and does not overwrite them on every boot.
- A fresh zero calibration can be triggered manually by sending `z` over the serial monitor while all clamps are empty.
- Current conversion is controlled by `ampsPerVolt` per channel in `src/main.cpp`.

## Important Hardware Clarification

- Verify capacitor unit in the schematic. For this type of bias filtering, values are usually in uF (for example 10uF to 100uF), not mF.
- If your sensor variant is SCT013-50A/1V, no external burden resistor is typically required at the CT output.
- If your sensor variant is a current-output CT, an external burden resistor is mandatory.

## Calibration Workflow

1. With all CT clamps empty, open the serial monitor and send `z`.
2. Check reported `zero_offset` and `mean` voltage per channel.
3. Verify corrected no-load current is near `0.000 A`.
4. Apply a known load and compare measured current to a reference meter.
5. Adjust `ampsPerVolt` per channel until measured current matches reference.
6. Repeat for all channels.

## Latest No-Load Baseline

- L1 GPIO34: zero offset about `0.203 A`, mean about `1.206 V`
- L2 GPIO35: zero offset about `0.198 A`, mean about `1.184 V`
- L3 GPIO32: zero offset about `0.137 A`, mean about `1.172 V`

Corrected readings after zero calibration were effectively `0.000 A` on all three channels during validation.

## Latest Saved Zero Calibration

- L1 GPIO34: `0.005 A` at about `1.206 V`
- L2 GPIO35: `0.002 A` at about `1.184 V`
- L3 GPIO32: `0.004 A` at about `1.172 V`

After refreshing zero calibration with empty clamps, all three corrected readings stayed at `0.000 A`.

## First Load Calibration

- Reference load: `46 W` incandescent lamp
- Assumed mains voltage: `230 V`
- Expected current: about `0.200 A`
- Observed pre-calibration L2 reading: about `28.1 A`
- Updated L2 `ampsPerVolt`: `0.356`
- Verified post-calibration L2 reading: about `0.189 A` to `0.193 A`

This calibration currently applies to `L2` only. Calibrate `L1` and `L3` separately with the same method.

## Phase-Matched Cross Calibration

- All three clamps were placed on the same phase conductor.
- `L2` was used as the reference channel because it had already been calibrated with the 46 W incandescent load.
- Observed reference current on `L2`: about `0.188 A` to `0.192 A`
- Updated `L1 ampsPerVolt`: `0.613`
- Updated `L3 ampsPerVolt`: `0.788`
- Verified post-calibration readings on the same phase:
	- `L1`: about `0.177 A` to `0.184 A`
	- `L2`: about `0.189 A` to `0.192 A`
	- `L3`: about `0.184 A` to `0.190 A`

This aligns all three channels to the same live conductor. A future reference-meter check is still recommended for final accuracy.

## 60W Validation and Fine Tuning

- Reference load: `60 W` incandescent lamp
- Assumed mains voltage: `230 V`
- Expected current: about `0.261 A`
- Observed pre-adjustment readings:
	- `L1`: about `0.205 A` to `0.206 A`
	- `L2`: about `0.245 A` to `0.249 A`
	- `L3`: about `0.236 A` to `0.242 A`
- Updated scale factors after fine tuning:
	- `L1 ampsPerVolt = 0.778`
	- `L2 ampsPerVolt = 0.377`
	- `L3 ampsPerVolt = 0.863`
- Verified post-tuning readings:
	- `L1`: about `0.261 A`
	- `L2`: about `0.260 A` to `0.263 A`
	- `L3`: about `0.261 A` initially, with occasional rise toward `0.274 A`

## Validation Tips

- Use only ADC1 pins for stable readings while WiFi is active.
- Keep analog signal wires short and reduce noise coupling.
- Use common ground for ESP32 and analog front-end.
