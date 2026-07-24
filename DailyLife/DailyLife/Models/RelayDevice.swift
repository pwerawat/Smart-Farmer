import Foundation

/// A controllable relay (pump, fan, light, ...) exposed by a Smart-Farmer board
/// over `smart-farmer/<device>/relay/<index>`.
struct RelayDevice: Codable, Equatable, Identifiable {
    var id: Int
    var name: String
    var isOn: Bool

    static let placeholders: [RelayDevice] = [
        RelayDevice(id: 0, name: "Water Pump", isOn: false),
        RelayDevice(id: 1, name: "Grow Light", isOn: true),
        RelayDevice(id: 2, name: "Vent Fan", isOn: false),
    ]
}
