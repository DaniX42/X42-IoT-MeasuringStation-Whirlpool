# Setup Guide

## Prerequisites

- Git installed
- VS Code with PlatformIO IDE extension
- ESP-WROOM-32 development board with USB data cable
- Access to required sensor hardware or mocks

## Clone and Prepare

1. Clone the repository.
2. Enter the workspace root.
3. Open the folder in VS Code.
4. Ensure the PlatformIO extension is enabled.
5. Review `README.md`, `docs/architecture.md`, and `docs/decision-log.md`.

## ESP32 Build and Flash (PlatformIO)

1. Connect the board via USB.
2. In PlatformIO, choose environment `esp32dev`.
3. Build the firmware.
4. Upload firmware to the board.
5. Open Serial Monitor at 115200 baud.

Expected output includes startup logs and chip details used for hardware verification.

## Initial Project Workflow

1. Create a feature branch from `main`.
2. Implement changes in small, reviewable commits.
3. Update docs when architecture or behavior changes.
4. Update `CHANGELOG.md` for meaningful updates.
5. Open a pull request using the provided template.

## Recommended Branching

- Stable branch: `main`
- Working branches:
  - `feature/<topic>`
  - `fix/<topic>`
  - `docs/<topic>`

## Commit Message Convention

Use concise, imperative messages with optional scope:

- `feat: add sensor ingestion skeleton`
- `fix: handle malformed measurement payload`
- `docs: expand setup guide for local testing`

## Reproducibility Rules

- Keep environment assumptions documented.
- Commit configuration templates, never secrets.
- Prefer scripts in `scripts/` over manual repeated steps.
- Keep `platformio.ini` and board environment settings version-controlled.
