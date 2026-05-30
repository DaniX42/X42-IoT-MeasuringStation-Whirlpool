# X42-IoT-MeasuringStation-Whirlpool

A long-term, maintainable project workspace for an IoT measuring station focused on Whirlpool-related monitoring and expansion.

Current runtime baseline: PlatformIO + ESP32 Arduino framework targeting ESP-WROOM-32 modules.

## Purpose

This repository is prepared as a scalable foundation for implementation, experimentation, and operations around an IoT measuring station.

## Project Structure

- `src/`: Application source code
- `docs/`: Project documentation and decision history
- `scripts/`: Automation and developer scripts
- `config/`: Configuration templates and environment-specific settings
- `tests/`: Test suites and test helpers
- `.github/`: Collaboration templates for issues and pull requests

## Getting Started

1. Read `docs/setup-guide.md`
2. Review architecture and constraints in `docs/architecture.md`
3. Check `docs/decision-log.md` for key decisions
4. Use `TODO.md` for current priorities
5. For current sensing hardware, review `docs/sensors-sct013.md`

## PlatformIO Quick Start

1. Connect the ESP-WROOM-32 board via USB.
2. Open the PlatformIO panel in VS Code.
3. Build the firmware using the configured environment.
4. Upload to the device.
5. Open Serial Monitor at 115200 baud.

The default firmware prints startup telemetry and chip information.
For three-phase current sensing, the firmware labels channels as `L1`, `L2`, and `L3` on GPIO34, GPIO35, and GPIO32.

## Collaboration

- Branch strategy: `main` + short-lived feature branches (`feature/<topic>`)
- Commit messages: clear, imperative, and scoped (e.g. `docs: add initial architecture baseline`)
- Pull requests should include rationale, testing notes, and documentation updates when relevant.

## Status

Initial workspace baseline prepared and ready for implementation.
