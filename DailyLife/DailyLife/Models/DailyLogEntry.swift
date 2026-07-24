import Foundation

/// A manual note the farmer records about a day's work (watering, feeding,
/// harvest, observations, ...).
struct DailyLogEntry: Codable, Equatable, Identifiable {
    enum Category: String, Codable, CaseIterable {
        case watering = "Watering"
        case feeding = "Feeding"
        case harvest = "Harvest"
        case maintenance = "Maintenance"
        case observation = "Observation"
    }

    var id: UUID
    var date: Date
    var category: Category
    var note: String

    init(id: UUID = UUID(), date: Date = Date(), category: Category, note: String) {
        self.id = id
        self.date = date
        self.category = category
        self.note = note
    }
}
