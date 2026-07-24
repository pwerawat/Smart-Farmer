import SwiftUI

struct SettingsView: View {
    @AppStorage("mqttBrokerHost") private var brokerHost = ""
    @AppStorage("mqttBrokerPort") private var brokerPort = 1883
    @AppStorage("mqttDeviceTopic") private var deviceTopic = "smart-farmer"

    var body: some View {
        NavigationStack {
            Form {
                Section("MQTT Broker") {
                    TextField("Host", text: $brokerHost, prompt: Text("e.g. 192.168.1.20"))
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                    Stepper("Port: \(brokerPort)", value: $brokerPort, in: 1...65535)
                    TextField("Device topic prefix", text: $deviceTopic)
                        .textInputAutocapitalization(.never)
                        .autocorrectionDisabled()
                } footer: {
                    Text("Matches the smart-farmer/<device>/telemetry and smart-farmer/<device>/relay/+ topics published by the board firmware. Wiring this up to a live MQTT connection is not implemented yet — the dashboard currently uses mock data.")
                }

                Section("About") {
                    LabeledContent("App", value: "DailyLife")
                    LabeledContent("Project", value: "Smart-Farmer")
                }
            }
            .navigationTitle("Settings")
        }
    }
}

#Preview {
    SettingsView()
}
