import Foundation

/// A single farm telemetry snapshot, matching the JSON payload published by the
/// Smart-Farmer boards on `smart-farmer/<device>/telemetry` (see firmware PRs #1/#2).
struct SensorReading: Codable, Equatable {
    var temperatureC: Double
    var humidityPercent: Double
    var soilMoisturePercent: Double
    var lightLux: Double
    var timestamp: Date

    static let placeholder = SensorReading(
        temperatureC: 27.4,
        humidityPercent: 63,
        soilMoisturePercent: 41,
        lightLux: 8600,
        timestamp: Date()
    )
}
