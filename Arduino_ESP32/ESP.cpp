#include <WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "DuckNet";
const char* password = "DuckieUPT";
const char* mqtt_server = "192.168.0.101";
#define mqtt_port 1883
#define TOPIC "/ic/Grupo1"

#define RXp2 16
#define TXp2 17

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        char* clientId = "Grupo1";
        if (mqttClient.connect(clientId)) {
            Serial.println("MQTT Connected Successfully");
            mqttClient.subscribe(TOPIC);
        } else {
            Serial.print("Connection failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println("Trying again...");
            delay(2500);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, RXp2, TXp2);
    setup_wifi();
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    reconnect();
}

void loop() {
    mqttClient.loop();
    if (Serial1.available() > 0) {
        String data = Serial1.readStringUntil('\n');
        publishData(data.c_str());
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message:");
    Serial.print(topic);
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Forward the command to the Arduino
    Serial1.write(payload, length);
    Serial1.write('\n'); // Ensure the command ends with a newline
}

void publishData(const char* data) {
    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.publish(TOPIC, data);
    Serial.print("Publishing to ");
    Serial.print(TOPIC);
    Serial.print(" : ");
    Serial.println(data);
}
