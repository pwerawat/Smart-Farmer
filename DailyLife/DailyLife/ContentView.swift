import SwiftUI

struct ContentView: View {
    var body: some View {
        TabView {
            DashboardView()
                .tabItem { Label("Dashboard", systemImage: "leaf.fill") }

            RelaysView()
                .tabItem { Label("Relays", systemImage: "switch.2") }

            DailyLogView()
                .tabItem { Label("Daily Log", systemImage: "book.closed.fill") }

            SettingsView()
                .tabItem { Label("Settings", systemImage: "gearshape.fill") }
        }
    }
}

#Preview {
    ContentView()
        .environmentObject(DashboardStore())
        .environmentObject(DailyLogStore())
}
