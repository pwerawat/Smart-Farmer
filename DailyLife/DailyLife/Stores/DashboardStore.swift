import Foundation

@MainActor
final class DashboardStore: ObservableObject {
    @Published private(set) var reading: SensorReading = .placeholder
    @Published private(set) var relays: [RelayDevice] = RelayDevice.placeholders
    @Published private(set) var isRefreshing = false
    @Published var lastError: String?

    private let provider: TelemetryProvider

    init(provider: TelemetryProvider = MockTelemetryProvider()) {
        self.provider = provider
    }

    func refresh() async {
        isRefreshing = true
        defer { isRefreshing = false }
        do {
            async let readingTask = provider.fetchLatestReading()
            async let relaysTask = provider.fetchRelays()
            reading = try await readingTask
            relays = try await relaysTask
            lastError = nil
        } catch {
            lastError = error.localizedDescription
        }
    }

    func toggleRelay(_ relay: RelayDevice) async {
        let newValue = !relay.isOn
        if let index = relays.firstIndex(where: { $0.id == relay.id }) {
            relays[index].isOn = newValue
        }
        do {
            try await provider.setRelay(id: relay.id, isOn: newValue)
        } catch {
            lastError = error.localizedDescription
            if let index = relays.firstIndex(where: { $0.id == relay.id }) {
                relays[index].isOn = !newValue
            }
        }
    }
}
