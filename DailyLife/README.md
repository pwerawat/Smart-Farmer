# DailyLife (iOS)

A SwiftUI companion app for the Smart-Farmer project: a daily dashboard for
the farm plus a manual activity log ("daily life" of the farm/farmer).

## What's here

- **Dashboard** — sensor cards (temperature, humidity, soil moisture, light),
  mirroring the telemetry JSON the Smart-Farmer boards publish on
  `smart-farmer/<device>/telemetry` (see firmware in PRs #1/#2 of this repo).
- **Relays** — toggle switches for pump/light/fan-style relays, mirroring
  `smart-farmer/<device>/relay/<index>`.
- **Daily Log** — manual notes (watering, feeding, harvest, maintenance,
  observations) persisted locally as JSON in the app's Documents directory.
- **Settings** — placeholder MQTT broker host/port/topic fields.

## Architecture

- `Models/` — `SensorReading`, `RelayDevice`, `DailyLogEntry` (plain
  `Codable` structs).
- `Stores/` — `DashboardStore` and `DailyLogStore` are `ObservableObject`s
  the views bind to. `TelemetryProvider` is a protocol abstracting the data
  source; `MockTelemetryProvider` is the only implementation today.
- `Views/` — one SwiftUI view per tab, plus `AddLogEntryView` presented as a
  sheet.

## Not implemented yet

- **Live MQTT connection.** The dashboard and relay controls currently run
  against `MockTelemetryProvider` (randomized values). Wiring up a real MQTT
  client means implementing `TelemetryProvider` against a Swift MQTT library
  and reading the broker settings from `SettingsView`.
- **Build verification.** This project was authored in an environment
  without Xcode or the Swift toolchain, so it has not been opened or built
  in Xcode. Open `DailyLife.xcodeproj` and build before relying on it.

## Requirements

- Xcode 15+, iOS 17+ deployment target.
