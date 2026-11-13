/*
  AgriGuard - Member 2: Animal Detection Logic
  Board : ESP32 Dev Module (Arduino framework)

  Logic : PIR motion HIGH  AND  ultrasonic distance <= threshold
          => intrusion confirmed => LED ON + buzzer beeping.
          Alert stays on for ALERT_HOLD_MS after the last detection.

  Pins (connect hardware later):
    PIR OUT      -> GPIO 27
    HC-SR04 TRIG -> GPIO 5
    HC-SR04 ECHO -> GPIO 18  (use a 1k + 2k voltage divider, ECHO is 5V)
    Buzzer (+)   -> GPIO 26  (active buzzer)
    LED anode    -> GPIO 25  (through 220 ohm resistor)
*/

#include <Arduino.h>

// ---------- Pins ----------
#define PIR_PIN     27
#define TRIG_PIN    5
#define ECHO_PIN    18
#define BUZZER_PIN  26
#define LED_PIN     25

// ---------- Settings (tune these during testing) ----------
const int  DISTANCE_THRESHOLD_CM = 100;     // object closer than this counts as "near"
const int  MIN_VALID_CM          = 2;       // ignore readings below this (noise)
const int  MAX_VALID_CM          = 400;     // HC-SR04 max range
const unsigned long PIR_WARMUP_MS      = 30000;  // PIR needs ~30 s to stabilise
const unsigned long SENSOR_INTERVAL_MS = 200;    // how often sensors are read
const unsigned long ALERT_HOLD_MS      = 5000;   // keep alert on after last detection
const unsigned long BEEP_INTERVAL_MS   = 300;    // buzzer on/off toggle time

// ---------- Shared state (other members can read these later for Firebase) ----------
bool  motionDetected = false;   // PIR result
int   distanceCm     = -1;      // -1 = no valid reading
bool  intrusionAlert = false;   // final confirmed intrusion

// ---------- Internal ----------
unsigned long lastSensorRead = 0;
unsigned long lastDetection  = 0;
unsigned long lastBeepToggle = 0;
bool buzzerState = false;
bool lastAlertPrinted = false;

// Single ultrasonic measurement in cm, -1 if no echo
int readDistanceOnce() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // 30 ms timeout
  if (duration == 0) return -1;

  int cm = (int)(duration * 0.0343 / 2.0);
  if (cm < MIN_VALID_CM || cm > MAX_VALID_CM) return -1;
  return cm;
}

// Median of 3 readings to remove random spikes
int readDistanceCm() {
  int a = readDistanceOnce(); delay(10);
  int b = readDistanceOnce(); delay(10);
  int c = readDistanceOnce();

  int valid[3]; int n = 0;
  if (a > 0) valid[n++] = a;
  if (b > 0) valid[n++] = b;
  if (c > 0) valid[n++] = c;

  if (n == 0) return -1;
  if (n == 1) return valid[0];
  if (n == 2) return (valid[0] + valid[1]) / 2;

  // n == 3 -> median
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) { int t = a; a = b; b = t; }
  return b;
}

void setAlertOutputs(bool on) {
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  if (!on) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  setAlertOutputs(false);

  Serial.println("AgriGuard animal detection starting...");
  Serial.println("PIR warm-up (30 s) - keep still, LED blinks meanwhile.");

  unsigned long start = millis();
  while (millis() - start < PIR_WARMUP_MS) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(500);
  }
  setAlertOutputs(false);
  Serial.println("Ready. Monitoring field.");
}

void loop() {
  unsigned long now = millis();

  // ----- 1. Read sensors every SENSOR_INTERVAL_MS -----
  if (now - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = now;

    motionDetected = (digitalRead(PIR_PIN) == HIGH);
    distanceCm = readDistanceCm();

    bool objectNear = (distanceCm > 0 && distanceCm <= DISTANCE_THRESHOLD_CM);

    // ----- 2. Combine: intrusion only if BOTH agree -----
    if (motionDetected && objectNear) {
      intrusionAlert = true;
      lastDetection = now;
    } else if (intrusionAlert && (now - lastDetection > ALERT_HOLD_MS)) {
      intrusionAlert = false;
    }

    // Debug print
    Serial.print("PIR: ");       Serial.print(motionDetected ? "MOTION" : "none");
    Serial.print(" | Dist: ");   Serial.print(distanceCm > 0 ? String(distanceCm) + " cm" : "no echo");
    Serial.print(" | Status: "); Serial.println(intrusionAlert ? "ALERT" : "SAFE");

    if (intrusionAlert != lastAlertPrinted) {
      Serial.println(intrusionAlert ? ">>> INTRUSION CONFIRMED <<<" : ">>> Field safe again <<<");
      lastAlertPrinted = intrusionAlert;
    }
  }

  // ----- 3. Drive LED and buzzer -----
  if (intrusionAlert) {
    digitalWrite(LED_PIN, HIGH);
    if (now - lastBeepToggle >= BEEP_INTERVAL_MS) {
      lastBeepToggle = now;
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
    }
  } else {
    setAlertOutputs(false);
  }
}