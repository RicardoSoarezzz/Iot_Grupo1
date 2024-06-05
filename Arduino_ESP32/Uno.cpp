#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

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

SoftwareSerial sw(0,1);
bool buttonPressed = false;

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

    // Send data to ESP-32
    Serial.println("Sending data to ESP32");
    Serial.print("{\"topico\":");
    Serial.print("\"/ic/Grupo1\"");  //sensor id
    Serial.print(",");
    Serial.print("\"tagName\":");
    Serial.print("\"TEMP\"");
    Serial.print(",");
    Serial.print("\"valor\":");
    Serial.print("\"");
    Serial.print(temperature);
    Serial.print("\"");
    Serial.print("}");
    Serial.println();

    sw.print("{\"topico\":");
    sw.print("\"/ic/Grupo1\"");  //sensor id
    sw.print(",");
    sw.print("\"tagName\":");
    sw.print("\"TEMP\"");
    sw.print(",");
    sw.print("\"valor\":");
    sw.print("\"");
    sw.print(temperature);
    sw.print("\"");
    sw.print("}");
    sw.println();
    delay(500);
    Serial.println("Sending data to ESP32");
    Serial.print("{\"topico\":");
    Serial.print("\"/ic/Grupo1\"");  //sensor id
    Serial.print(",");
    Serial.print("\"tagName\":");
    Serial.print("\"NOISE\"");
    Serial.print(",");
    Serial.print("\"valor\":");
    Serial.print("\"");
    Serial.print(noiseONOFF);
    Serial.print("\"");
    Serial.print("}");
    Serial.println();

    sw.print("{\"topico\":");
    sw.print("\"/ic/Grupo1\"");  //sensor id
    sw.print(",");
    sw.print("\"tagName\":");
    sw.print("\"NOISE\"");
    sw.print(",");
    sw.print("\"valor\":");
    sw.print("\"");
    sw.print(noiseONOFF);
    sw.print("\"");
    sw.print("}");
    sw.println();
    delay(500);

/*
    // Check for incoming commands from ESP-32
    if (sw.available() > 0) {
        String command = Serial.readStringUntil('\n');
        handleCommand(command);
    }


  if (sw.available() > 0) {
      char bfr[501];
      memset(bfr, 0, 501);
      sw.readBytesUntil('\n', bfr, 500);
      String json = String(bfr);
      Serial.println(json);

      String tagName = getAtrib(json, "tagName");
      if (tagName.equals("ALARM")) {
        String valor = getAtrib(json, "valor");
        digitalWrite(PIN_LED, valor.equals("ON") ? HIGH : LOW);
        Serial.println("Alarme " + valor);
      }
  }*/
    delay(1000);
}




void readButton() {
    buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH && !buttonPressed) {
        buttonPressed = true;
    } else if (buttonState == LOW && buttonPressed) {
        buttonPressed = false;
        digitalWrite(BUTTON_LED, LOW);
    }
}

void updateButtonLED() {
    if (buttonPressed) {
        Serial.println("Sending data to ESP32");
    Serial.print("{\"topico\":");
    Serial.print("\"/ic/Grupo1\"");
    Serial.print(",");
    Serial.print("\"tagName\":");
    Serial.print("\"ALARM\"");
    Serial.print(",");
    Serial.print("\"valor\":");
    Serial.print("\"");
    Serial.print(1);
    Serial.print("\"");
    Serial.print("}");
    Serial.println();

    sw.print("{\"topico\":");
    sw.print("\"/ic/Grupo1\"");
    sw.print(",");
    sw.print("\"tagName\":");
    sw.print("\"ALARM\"");
    sw.print(",");
    sw.print("\"valor\":");
    sw.print("\"");
    sw.print(1);
    sw.print("\"");
    sw.print("}");
    sw.println();
        for (int i = 0; i < 3; i++) {
            digitalWrite(BUTTON_LED, HIGH);
            tone(BUZZER_PIN, 1000);
            delay(500);

            digitalWrite(BUTTON_LED, LOW);
            noTone(BUZZER_PIN);
            delay(500);
        }
    } else {
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
    float voltage = (float)analogRead(LM35_PIN) /1000;
    return voltage * 100.0;
}

void handleCommand(String command) {
    static float tempValue = 0;
    static int noiseState = 0;

    if (command.startsWith("ALARM")) {
        buttonPressed = true;
        updateButtonLED();
    } else if (command.startsWith("TEMP:")) {
        tempValue = command.substring(5).toFloat();
        Serial.print("Received TEMP: ");
        Serial.println(tempValue);
    } else if (command.startsWith("NOISE:")) {
        noiseState = command.substring(6).toInt();
        Serial.print("Received NOISE: ");
        Serial.println(noiseState);
        if (noiseState == 1) {
            digitalWrite(NOISE_PIN, HIGH);
        } else {
            digitalWrite(NOISE_PIN, LOW);
        }
    }

    updateDisplayAndLED(tempValue, noiseState);
}
String tiraAspas(String s) {
  if (s.charAt(0) == '"')
    s = s.substring(1);
  if (s.charAt(s.length() - 1) == '"')
    s = s.substring(0, s.length() - 1);
  return s;
}


String getAtrib(String json, String atrib) {
  char sep = ',';
  int inicio = 1;
  int fim = -1;
  int maxIndex = json.length() - 1;

  json = json.substring(1, json.length() - 2);  // tira chavetas

  for (int i = 0; i <= maxIndex; i++) {
    if (json.charAt(i) == sep || i == maxIndex) {
      inicio = fim + 1;
      fim = (i == maxIndex) ? i + 1 : i;
      String a = json.substring(inicio, fim);
      int posdp = a.indexOf(":");
      if (posdp < 0)
        continue;
      String nomeAtrib = a.substring(1, posdp - 1);  // filtra "
      String valorAtrib = a.substring(posdp + 1);
      // Serial.println("nome: "+nomeAtrib+"  valor: "+valorAtrib);
      if (nomeAtrib.equals(atrib))
        return tiraAspas(valorAtrib);
    }
  }

  return "";
}

