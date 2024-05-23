#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "DuckNet";
const char* password = "DuckieUPT";

// MQTT broker settings
const char* mqtt_server = "192.168.0.101";
#define mqtt_port 1883
#define TOPIC_PUBLISH "/IOT_GRUPO1/sensor"
#define TOPIC_SUBSCRIBE "/IOT_GRUPO1/actuator"

WiFiClient espClient;
PubSubClient client(espClient);

// Function to connect to WiFi
void setup_wifi() {
  delay(10);
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

// Callback function for handling received MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Send the received message to the Arduino
  Serial.write(payload, length);
}

// Function to reconnect to the MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(TOPIC_SUBSCRIBE);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read data from the Arduino and publish it to the MQTT broker
  if (Serial.available() > 0) {
    String serialData = Serial.readStringUntil('\n');
    client.publish(TOPIC_PUBLISH, serialData.c_str());
    Serial.print("Published: ");
    Serial.println(serialData);
  }
}
