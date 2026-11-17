#include <Arduino.h>
#include "irrigation.h"

#define MOISTURE_PIN 34          // placeholder: use Member 3's pin
#define USE_FAKE_MOISTURE true   // true = type values in Serial Monitor, false = real sensor

// Placeholder calibration for real-sensor mode (Member 3 will have the real values)
#define ADC_DRY 3500             // raw reading in dry air/soil
#define ADC_WET 1500             // raw reading in water/wet soil

int fakeMoisture = 70;           // starts "wet"

void setup() {
  Serial.begin(115200);
  irrigationBegin();
  Serial.println("Type: d = dry (15%), w = wet (70%), or any number 0-100");
}

void loop() {
  int moisture;

  if (USE_FAKE_MOISTURE) {
    if (Serial.available()) {
      String in = Serial.readStringUntil('\n');
      in.trim();
      if (in == "d") fakeMoisture = 15;
      else if (in == "w") fakeMoisture = 70;
      else if (in.length() > 0) fakeMoisture = in.toInt();
      Serial.printf("[TEST] moisture = %d%%\n", fakeMoisture);
    }
    moisture = fakeMoisture;
  } else {
    int raw = analogRead(MOISTURE_PIN);
    moisture = constrain(map(raw, ADC_DRY, ADC_WET, 0, 100), 0, 100);
  }

  irrigationUpdate(moisture);

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2000) {
    lastPrint = millis();
    Serial.printf("moisture=%d%% pump=%s\n", moisture, pumpStatusString());
  }
  delay(100);
}
