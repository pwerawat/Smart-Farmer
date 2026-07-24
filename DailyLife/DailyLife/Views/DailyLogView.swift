import SwiftUI

struct DailyLogView: View {
    @EnvironmentObject private var store: DailyLogStore
    @State private var isPresentingAddEntry = false

    var body: some View {
        NavigationStack {
            Group {
                if store.entries.isEmpty {
                    ContentUnavailableView(
                        "No Entries Yet",
                        systemImage: "book.closed",
                        description: Text("Log watering, feeding, harvest, and other daily farm work.")
                    )
                } else {
                    List {
                        ForEach(store.entries) { entry in
                            VStack(alignment: .leading, spacing: 4) {
                                HStack {
                                    Text(entry.category.rawValue)
                                        .font(.subheadline.bold())
                                    Spacer()
                                    Text(entry.date.formatted(date: .abbreviated, time: .shortened))
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                }
                                Text(entry.note)
                                    .font(.body)
                            }
                            .padding(.vertical, 4)
                        }
                        .onDelete(perform: store.delete)
                    }
                }
            }
            .navigationTitle("Daily Log")
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button {
                        isPresentingAddEntry = true
                    } label: {
                        Image(systemName: "plus")
                    }
                }
            }
            .sheet(isPresented: $isPresentingAddEntry) {
                AddLogEntryView()
            }
        }
    }
}

#Preview {
    DailyLogView()
        .environmentObject(DailyLogStore())
}
