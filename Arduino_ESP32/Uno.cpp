#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

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

float lastTemperature = -999.0;
int lastNoise = -1;
int lastAlarmState = -1;

SoftwareSerial sw(0, 1);

void setup() {
    Serial.begin(19200);
    sw.begin(115200);
    pinMode(RED_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_LED, OUTPUT);

    pinMode(BUTTON, INPUT);
    pinMode(ANALOG_PIN, INPUT);
    pinMode(NOISE_PIN, INPUT);

    lcd.begin(16, 2);
    lcd.init();
    lcd.backlight();
    setLCD("OK");
}

void loop() {
    float temperature = readTemperature();
    int digitalValue = digitalRead(NOISE_PIN);

    int noiseONOFF = (digitalValue >= 1) ? 1 : 0;

    readButton();
    updateButtonLED();
    updateDisplayAndLED(temperature, digitalValue);

    // Send data to ESP-32 if there are significant changes
    if (abs(temperature - lastTemperature) > 3) {
        sendData("TEMP", temperature);
        lastTemperature = temperature;
    }

    if (noiseONOFF != lastNoise) {
        sendData("NOISE", noiseONOFF);
        lastNoise = noiseONOFF;
    }

    if ((buttonPressed ? 1 : 0) != lastAlarmState) {
        sendData("ALARM", buttonPressed ? 1 : 0);
        lastAlarmState = buttonPressed ? 1 : 0;
    }

  
    // Read from Serial
       if (sw.available() > 0) {
        char bfr[501];
        memset(bfr, 0, 501);
        sw.readBytesUntil('\n', bfr, 500);
        String b = String(bfr);
        handleSerialMessage(b);
    }
    

    delay(100);
}

void handleSerialMessage(String message) {
    StaticJsonDocument<200> doc;

    const char* topico = doc["topico"];
    const char* tagName = doc["tagName"];
    int valor = doc["valor"];

    if( tagName == "ALARM" && valor==1){
        digitalWrite(BUTTON_LED, LOW);
        noTone(BUZZER_PIN);
    }

    
}

void readButton() {
    buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH && !buttonPressed) {
        buttonPressed = true;
    } else if (buttonState == LOW && buttonPressed) {
        buttonPressed = false;
    }
}

void updateButtonLED() {
    if (buttonPressed) {
        digitalWrite(BUTTON_LED, HIGH);
        tone(BUZZER_PIN, 1000);
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
    float voltage = (float)analogRead(LM35_PIN) / 500;
    return voltage * 100.0;
}

void sendData(String tagName, float value) {
    String data = "{\"topico\":\"/ic/Grupo1\",\"tagName\":\"" + tagName + "\",\"valor\":\"" + String(value) + "\"}";
    Serial.println("Sending data to ESP32");
    Serial.println(data);
    sw.println(data);
}

