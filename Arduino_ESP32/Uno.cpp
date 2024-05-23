#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <Wire.h>

// Network settings for Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress mqtt_server(192, 168, 0, 101); // IP address of your MQTT broker
#define mqtt_port 1883

EthernetClient ethClient;
PubSubClient client(ethClient);

EthernetClient ethClient;
PubSubClient client(ethClient);

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int LM35_PIN = A2;
const int RED_PIN = 4;
const int GREEN_PIN = 3;
const int BLUE_PIN = 2;
const int BUZZER_PIN = 5;
const int ANALOG_PIN = A5;
const int NOISE_PIN = 7;
const int BUTTON = 8;
const int BUTTON_LED = 13;

const int TEMPERATURE_THRESHOLD_HOT = 20;
const int TEMPERATURE_THRESHOLD_COLD = 10;
const int NOISE_THRESHOLD = 500;

int buttonState = 0;
bool buttonPressed = false;

void setup() {
    Serial.begin(9600);
    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(ANALOG_PIN, INPUT);
    pinMode(NOISE_PIN, INPUT);
    pinMode(BUTTON_LED, OUTPUT);
    pinMode(BUTTON, INPUT);

    lcd.begin(16, 2);
    lcd.init();
    lcd.backlight();
    setLCD("OK");

    Ethernet.begin(mac);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    
    reconnect();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    readButton();
    updateButtonLED();

    float temperature = readTemperature();
    int digitalValue = digitalRead(NOISE_PIN);

    updateDisplayAndLED(temperature, digitalValue);
    delay(1000);

    // Send sensor data to ESP32 via MQTT
    String sensorData = "Temperature: " + String(temperature) + " C, Noise: " + (digitalValue == HIGH ? "High" : "Low");
    client.publish("/IoT_Grupo1", sensorData.c_str());
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (message == "BUZZER_ON") {
        tone(BUZZER_PIN, 1000);
    } else if (message == "BUZZER_OFF") {
        noTone(BUZZER_PIN);
    } else if (message == "TOGGLE_NOISE") {
        buttonPressed = !buttonPressed;
        updateButtonLED();
    }
}

void reconnect() {
    while (!client.connected()) {
        if (client.connect("ArduinoClient")) {
            client.subscribe("/IoT_Grupo1/commands");
        } else {
            delay(5000);
        }
    }
}

void readButton() {
    buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH && !buttonPressed) {
        buttonPressed = true;
        digitalWrite(BUTTON_LED, HIGH);
    } else if (buttonState == LOW && buttonPressed) {
        buttonPressed = false;
        digitalWrite(BUTTON_LED, LOW);
    }
}

void updateButtonLED() {
    if (buttonPressed) {
        digitalWrite(BUTTON_LED, HIGH);
        tone(BUZZER_PIN, 1000, 5000);
    } else {
        digitalWrite(BUTTON_LED, LOW);
        noTone(BUZZER_PIN);
    }
}

void updateDisplayAndLED(float temperature, int digitalValue) {
    if (temperature >= TEMPERATURE_THRESHOLD_HOT) {
        setColor(0, 0, 255); // Blue for air conditioning
    } else if (temperature <= TEMPERATURE_THRESHOLD_COLD) {
        setColor(255, 0, 0); // Red for heater
    } else {
        setLCD("OK");
        setColor(0, 255, 0); // Green for normal condition
    }

    if (digitalValue == HIGH) {
        setLCD("Ruido Alto");
    }
}

void setColor(int redValue, int greenValue, int blueValue) {
    analogWrite(RED_PIN, redValue);
    analogWrite(GREEN_PIN, greenValue);
    analogWrite(BLUE_PIN, blueValue);
}

void setLCD(String message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
}

float readTemperature() {
    float voltage = (float)analogRead(LM35_PIN) * 5.0 / 800.0;
    return (voltage - 0.5) * 100.0;
}
