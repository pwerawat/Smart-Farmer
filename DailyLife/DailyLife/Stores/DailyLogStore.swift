import Foundation

@MainActor
final class DailyLogStore: ObservableObject {
    @Published private(set) var entries: [DailyLogEntry] = []

    private let fileURL: URL

    init(fileURL: URL? = nil) {
        if let fileURL {
            self.fileURL = fileURL
        } else {
            let documents = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
            self.fileURL = documents.appendingPathComponent("daily_log.json")
        }
        load()
    }

    func add(category: DailyLogEntry.Category, note: String, date: Date = Date()) {
        let entry = DailyLogEntry(date: date, category: category, note: note)
        entries.insert(entry, at: 0)
        entries.sort { $0.date > $1.date }
        save()
    }

    func delete(at offsets: IndexSet) {
        entries.remove(atOffsets: offsets)
        save()
    }

    private func load() {
        guard let data = try? Data(contentsOf: fileURL) else { return }
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        entries = (try? decoder.decode([DailyLogEntry].self, from: data)) ?? []
    }

    private func save() {
        let encoder = JSONEncoder()
        encoder.dateEncodingStrategy = .iso8601
        guard let data = try? encoder.encode(entries) else { return }
        try? data.write(to: fileURL, options: .atomic)
    }
}
