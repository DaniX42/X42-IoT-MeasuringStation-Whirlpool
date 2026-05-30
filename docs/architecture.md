# Architecture

## Overview

This project follows a modular architecture designed for long-term maintainability and incremental delivery.

## Architectural Goals

- Keep domain logic independent from hardware adapters.
- Support easy extension for new sensors and outputs.
- Enable deterministic testing through clear interfaces.
- Preserve decision transparency through continuous documentation.

## Proposed High-Level Layers

1. Ingestion Layer
- Sensor communication and raw measurement collection.

2. Processing Layer
- Validation, filtering, calibration, aggregation, and derived metrics.

3. Storage/Transport Layer
- Local persistence and/or upstream publishing.

4. Application/Orchestration Layer
- Scheduling, startup lifecycle, health checks, and coordination.

5. Interface Layer
- Optional CLI, API, dashboard, or integration endpoints.

## Directory-to-Architecture Mapping

- `src/ingestion/` -> Ingestion Layer
- `src/processing/` -> Processing Layer
- `src/storage/` -> Storage/Transport Layer
- `src/app/` -> Application/Orchestration Layer
- `src/interfaces/` -> Interface Layer

## Design Principles

- Prefer explicit interfaces over implicit coupling.
- Keep side effects at the boundaries of the system.
- Minimize hidden global state.
- Document tradeoffs in `docs/decision-log.md`.

## Non-Functional Priorities

- Reliability for continuous operation.
- Traceability of measurements and transformations.
- Maintainability through clear module boundaries.
- Reproducibility of local development and deployment.
