import SwiftUI

@main
struct DailyLifeApp: App {
    @StateObject private var dashboardStore = DashboardStore()
    @StateObject private var logStore = DailyLogStore()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(dashboardStore)
                .environmentObject(logStore)
        }
    }
}
