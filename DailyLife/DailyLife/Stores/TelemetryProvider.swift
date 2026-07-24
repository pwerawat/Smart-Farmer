import Foundation

/// Source of live farm data. The Smart-Farmer boards publish telemetry and accept
/// relay commands over MQTT (see the firmware in PRs #1/#2); this protocol lets a
/// real MQTT-backed implementation be dropped in later without touching the UI.
protocol TelemetryProvider {
    func fetchLatestReading() async throws -> SensorReading
    func fetchRelays() async throws -> [RelayDevice]
    func setRelay(id: Int, isOn: Bool) async throws
}

/// Deterministic in-memory stand-in used until a real MQTT client is wired up.
final class MockTelemetryProvider: TelemetryProvider {
    private var relays = RelayDevice.placeholders

    func fetchLatestReading() async throws -> SensorReading {
        SensorReading(
            temperatureC: Double.random(in: 22...31),
            humidityPercent: Double.random(in: 45...80),
            soilMoisturePercent: Double.random(in: 20...65),
            lightLux: Double.random(in: 500...12000),
            timestamp: Date()
        )
    }

    func fetchRelays() async throws -> [RelayDevice] {
        relays
    }

    func setRelay(id: Int, isOn: Bool) async throws {
        guard let index = relays.firstIndex(where: { $0.id == id }) else { return }
        relays[index].isOn = isOn
    }
}
