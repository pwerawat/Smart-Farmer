import SwiftUI

struct RelaysView: View {
    @EnvironmentObject private var store: DashboardStore

    var body: some View {
        NavigationStack {
            List(store.relays) { relay in
                Toggle(isOn: Binding(
                    get: { relay.isOn },
                    set: { _ in Task { await store.toggleRelay(relay) } }
                )) {
                    Label(relay.name, systemImage: relay.isOn ? "bolt.fill" : "bolt.slash")
                }
            }
            .navigationTitle("Relays")
            .task { await store.refresh() }
            .refreshable { await store.refresh() }
        }
    }
}

#Preview {
    RelaysView()
        .environmentObject(DashboardStore())
}
