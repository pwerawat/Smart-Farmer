import SwiftUI

struct DashboardView: View {
    @EnvironmentObject private var store: DashboardStore

    private let columns = [GridItem(.flexible()), GridItem(.flexible())]

    var body: some View {
        NavigationStack {
            ScrollView {
                LazyVGrid(columns: columns, spacing: 16) {
                    SensorCard(title: "Temperature", value: String(format: "%.1f°C", store.reading.temperatureC), icon: "thermometer.medium", tint: .orange)
                    SensorCard(title: "Humidity", value: String(format: "%.0f%%", store.reading.humidityPercent), icon: "humidity.fill", tint: .blue)
                    SensorCard(title: "Soil Moisture", value: String(format: "%.0f%%", store.reading.soilMoisturePercent), icon: "drop.fill", tint: .brown)
                    SensorCard(title: "Light", value: String(format: "%.0f lux", store.reading.lightLux), icon: "sun.max.fill", tint: .yellow)
                }
                .padding()

                Text("Updated \(store.reading.timestamp.formatted(date: .omitted, time: .standard))")
                    .font(.footnote)
                    .foregroundStyle(.secondary)

                if let error = store.lastError {
                    Label(error, systemImage: "exclamationmark.triangle.fill")
                        .font(.footnote)
                        .foregroundStyle(.red)
                        .padding(.top, 4)
                }
            }
            .navigationTitle("Today's Farm")
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    if store.isRefreshing {
                        ProgressView()
                    } else {
                        Button {
                            Task { await store.refresh() }
                        } label: {
                            Image(systemName: "arrow.clockwise")
                        }
                    }
                }
            }
            .task { await store.refresh() }
            .refreshable { await store.refresh() }
        }
    }
}

private struct SensorCard: View {
    let title: String
    let value: String
    let icon: String
    let tint: Color

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Image(systemName: icon)
                .font(.title2)
                .foregroundStyle(tint)
            Text(value)
                .font(.title2.bold())
            Text(title)
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        .padding()
        .background(.thinMaterial, in: RoundedRectangle(cornerRadius: 16))
    }
}

#Preview {
    DashboardView()
        .environmentObject(DashboardStore())
}
