import SwiftUI

struct AddLogEntryView: View {
    @EnvironmentObject private var store: DailyLogStore
    @Environment(\.dismiss) private var dismiss

    @State private var category: DailyLogEntry.Category = .observation
    @State private var note = ""
    @State private var date = Date()

    var body: some View {
        NavigationStack {
            Form {
                Picker("Category", selection: $category) {
                    ForEach(DailyLogEntry.Category.allCases, id: \.self) { category in
                        Text(category.rawValue).tag(category)
                    }
                }
                DatePicker("Date", selection: $date)
                TextField("Note", text: $note, axis: .vertical)
                    .lineLimit(3...6)
            }
            .navigationTitle("New Entry")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { dismiss() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        store.add(category: category, note: note, date: date)
                        dismiss()
                    }
                    .disabled(note.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
                }
            }
        }
    }
}

#Preview {
    AddLogEntryView()
        .environmentObject(DailyLogStore())
}
