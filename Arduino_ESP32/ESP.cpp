#include <WiFi.h>
#include <PubSubClient.h>
//#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>


// WiFi and MQTT configuration
const char* ssid = "DuckNet";
const char* password = "DuckieUPT";
const char* mqtt_server = "192.168.0.101";
#define mqtt_port 1883
#define TOPIC "/ic/Grupo1"

// Define Serial1 RX and TX pins
#define rxPin 16
#define txPin 17

HardwareSerial sw(2);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

//SoftwareSerial sw =  SoftwareSerial(rxPin, txPin);



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
        char* clientId = "Grupo1_RM";
        if (mqttClient.connect(clientId)) {
            Serial.println("MQTT Connected Successfully");
            boolean res = mqttClient.subscribe(TOPIC);
            Serial.print(TOPIC);
            Serial.println(res?"  true":"  false");
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
    //sw.begin(19200);
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
      memset(bfr,0,501);
      sw.readBytesUntil ('\n',bfr,500);
      String b = String(bfr);
      publishSerialData(b);


    }
    delay(200);

}

String tiraAspas( String s) {
  if (s.charAt(0) == '"')
    s = s.substring(1);
  if (s.charAt(s.length()-1) == '"')
    s = s.substring(0,s.length()-1);
  return s;
}


// Function to publish serial data
void publishSerialData(String json) {
  if (!mqttClient.connected()) {
    reconnect();
  }

  // Print the raw JSON string for debugging
  Serial.print("Raw JSON: ");
  Serial.println(json);

  // Create a StaticJsonDocument with enough capacity
  StaticJsonDocument<200> doc;

  // Deserialize the JSON string
  DeserializationError error = deserializeJson(doc, json);

  // Check if deserialization failed
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Extract values from the JSON object
  const char* topico = doc["topico"];
  const char* tagName = doc["tagName"];
  const char* valor = doc["valor"];

  // Debug output for extracted values
  Serial.print("Extracted topico: ");
  Serial.println(topico ? topico : "null");
  Serial.print("Extracted tagName: ");
  Serial.println(tagName ? tagName : "null");
  Serial.print("Extracted valor: ");
  Serial.println(valor ? valor : "null");

  // Create the payload
  String dados = String(tagName) + ":"+ String(valor);

  // Publish the data
  mqttClient.publish(topico, dados.c_str());

  // Debug output for published message
  Serial.print("Publish : ");
  Serial.print(topico);
  Serial.print(" : ");
  Serial.print(dados);
  Serial.println();
}


String getAtrib(String json, String atrib)
{
  char sep = ',';
  int inicio = 1;
  int fim = -1;
  int maxIndex = json.length()-1;

  json = json.substring(1,json.length()-2);

    for(int i=0; i<=maxIndex; i++){
    if( json.charAt(i)==sep || i==maxIndex){
      inicio = fim+1;
      fim = (i == maxIndex) ? i+1 : i;
      String a = json.substring(inicio, fim);
      int posdp = a.indexOf(":");
      if (posdp < 0)
        continue;
      String nomeAtrib = a.substring(1,posdp-1);
      String valorAtrib = a.substring(posdp+1);
      if (nomeAtrib.equals(atrib))
        return tiraAspas(valorAtrib);
    }
  }

  return "";
}


void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.print(topic);
    String st = "";
    for (unsigned int i = 0; i < length; i++) {
        st +=(char)payload[i];
    }
    Serial.println(st);
    sw.println(st);


}

void publishData(const char* data) {
    Serial.println("PublishData");
    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.publish(TOPIC, data);
    Serial.print("Publishing to ");
    Serial.print(TOPIC);
    Serial.print(" : ");
    Serial.println(data);
    delay(500);
}
