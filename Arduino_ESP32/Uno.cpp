#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

    readButton();
    updateButtonLED(temperature);
    updateDisplayAndLED(temperature, digitalValue);

    // Send data to ESP-32
    Serial.print("TEMP:");
    Serial.println(temperature);
    Serial.print("NOISE:");
    Serial.println(digitalValue);

    // Check for incoming commands from ESP-32
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        handleCommand(command);
    }

    delay(1000);
}

void readButton() {
    buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH && !buttonPressed) {
        buttonPressed = true;
        digitalWrite(BUTTON_LED, HIGH);
        Serial.println("RED-1");
    } else if (buttonState == LOW && buttonPressed) {
        buttonPressed = false;
        digitalWrite(BUTTON_LED, LOW);
        Serial.println("RED-0");
    }
}

void updateButtonLED(float temp) {
    if (buttonPressed && temp >= TEMPERATURE_THRESHOLD_HOT) {
        tone(BUZZER_PIN, 1000, 5000);
        Serial.println("BUZ-1");
    } else {
        noTone(BUZZER_PIN);
        Serial.println("BUZ-0");
    }
}

void updateDisplayAndLED(float temperature, int digitalValue) {
    if (temperature >= TEMPERATURE_THRESHOLD_HOT) {
        setColor(0, 0, 255); // Blue for air conditioning
        Serial.println("LED-B");
    } else if (temperature <= TEMPERATURE_THRESHOLD_COLD) {
        setColor(255, 0, 0); // Red for heater
        Serial.println("LED-R");
    } else {
        setLCD("OK");
        setColor(0, 255, 0); // Green for normal condition
        Serial.println("LED-G");
    }

    if (digitalValue == HIGH) {
        Serial.println("NOI-1");
        setLCD("Ruido Alto");
    } else {
        Serial.println("NOI-0");
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
    float voltage = (float)analogRead(LM35_PIN) * 5.0 / 1023.0;
    return voltage * 100.0;
}

void handleCommand(String command) {
    if (command.startsWith("BUZZER_ON")) {
        tone(BUZZER_PIN, 1000); // Turn on buzzer
    } else if (command.startsWith("BUZZER_OFF")) {
        noTone(BUZZER_PIN); // Turn off buzzer
    } else if (command.startsWith("TEMP:")) {
        // Handle any temperature-specific commands if necessary
    } else if (command.startsWith("NOISE:")) {
        // Handle any noise-specific commands if necessary
    }
}
