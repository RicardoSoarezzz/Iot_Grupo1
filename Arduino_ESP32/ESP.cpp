#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>

// WiFi and MQTT configuration
const char* ssid = "DuckNet";
const char* password = "DuckieUPT";
const char* mqtt_server = "192.168.0.101";
#define mqtt_port 1883
#define TOPIC "/ic/Grupo1"
#define TOPIC_INTERFACE "/ic/Grupo1Interface"


// Define Serial1 RX and TX pins
#define rxPin 16
#define txPin 17

HardwareSerial sw(2);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Unique identifier for this ESP32
const char* esp_id = "ESP32_1";

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
        char clientId[50];
        snprintf(clientId, sizeof(clientId), "Grupo1_RM_%s", esp_id);
        if (mqttClient.connect(clientId)) {
            Serial.println("MQTT Connected Successfully");
            boolean res = mqttClient.subscribe(TOPIC);
            Serial.print(TOPIC_INTERFACE);
            Serial.println(res ? "  true" : "  false");
        } else {
            Serial.print("Connection failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Trying again in 5 seconds...");
            delay(5000);
        }
    }
}

void setup() {
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);

    Serial.begin(115200);
    sw.begin(19200, SERIAL_8N1, 16, 17);

    setup_wifi();
    mqttClient.setServer(mqtt_server, mqtt_port);


    mqttClient.setCallback(callback);
    reconnect();
}

void loop() {
    mqttClient.loop();
    if (sw.available() > 0) {
        char bfr[501];
        memset(bfr, 0, 501);
        sw.readBytesUntil('\n', bfr, 500);
        String b = String(bfr);
        publishSerialData(b);
    }
    delay(200);
}

// Function to publish serial data
void publishSerialData(String json) {
    if (!mqttClient.connected()) {
        reconnect();
    }

    Serial.print("Raw JSON: ");
    Serial.println(json);

    StaticJsonDocument<200> doc;

    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        return;
    }

    const char* topico = doc["topico"];
    const char* tagName = doc["tagName"];
    int valor = doc["valor"];

    doc["esp_id"] = esp_id;

    String dados = String(tagName) + ":" + valor; // TEMP:x | NOISE:1/0 | ALARM:1/0

    mqttClient.publish(topico, dados.c_str());

    Serial.print("Publish : ");
    Serial.print(topico);
    Serial.print(" : ");
    Serial.print(dados);
    Serial.println();
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.print(topic);
    String st = "";
    for (unsigned int i = 0; i < length; i++) {
        st += (char)payload[i];
    }
    Serial.println(st);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, st);

    if (error) {
        Serial.println("Deserialization failed");
        return;
    }

    const char* received_esp_id = doc["esp_id"];
    if (received_esp_id && strcmp(received_esp_id, esp_id) == 0) {
        Serial.println("Ignoring message from this ESP32");
        return; 
    }

    sw.println(st);
}