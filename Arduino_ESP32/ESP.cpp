#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// WiFi settings
const char* ssid = "DuckNet";
const char* password = "DuckieUPT";

const char* mqtt_server = "192.168.0.101"; // IP address of your MQTT broker
#define mqtt_port 1883

WiFiClient espClient;
PubSubClient client(espClient);

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
        if (mqttClient.connect("ESP32Client")) {
            Serial.println("mqttClient connected");
            mqttClient.subscribe(TOPIC);
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(callback);
    reconnect();
}

void loop() {
    mqttClient.loop();
    if (Serial.available() > 0) {
        String serialData = Serial.readStringUntil('\n');
        mqttClient.publish(TOPIC, serialData.c_str());
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}
