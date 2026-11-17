#pragma once
#include <Arduino.h>

// ================= CHANGE THESE IF NEEDED =================
#define RELAY_PIN          26      // GPIO wired to relay IN (confirm with Member 5)
#define RELAY_ACTIVE_LOW   true    // most relay modules turn ON with LOW; set false if yours is opposite

// Moisture is a PERCENTAGE (0-100), same as AgriGuard/soil/moisturePercent. LOW = DRY.
#define DRY_PERCENT        30      // at or below this -> soil is dry -> pump ON
#define WET_PERCENT        60      // at or above this -> soil is wet enough -> pump OFF

#define PUMP_MAX_RUN_MS    10000UL // safety timeout: pump never runs longer than this (10 s)
#define PUMP_COOLDOWN_MS   30000UL // wait after any stop so water can soak in (30 s)
// ==========================================================

static bool          g_pumpOn      = false;
static bool          g_hasStopped  = false;
static unsigned long g_pumpStartMs = 0;
static unsigned long g_lastStopMs  = 0;

inline void pumpWrite(bool on) {
  bool high = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(RELAY_PIN, high ? HIGH : LOW);
}

inline void pumpStart() {
  pumpWrite(true);
  g_pumpOn = true;
  g_pumpStartMs = millis();
  Serial.println("[PUMP] ON");
}

inline void pumpStop(const char* reason) {
  pumpWrite(false);
  g_pumpOn = false;
  g_hasStopped = true;
  g_lastStopMs = millis();
  Serial.print("[PUMP] OFF - ");
  Serial.println(reason);
}

inline void irrigationBegin() {
  pumpWrite(false);              // set OFF level first to avoid a pump blip at boot
  pinMode(RELAY_PIN, OUTPUT);
  pumpWrite(false);
  Serial.println("[IRRIGATION] ready");
}

// Call regularly with the latest moisture PERCENT (0-100)
inline void irrigationUpdate(int moisturePercent) {
  unsigned long now = millis();

  // Fail-safe: invalid reading (sensor unplugged, bad data) -> pump OFF
  if (moisturePercent < 0 || moisturePercent > 100) {
    if (g_pumpOn) pumpStop("invalid sensor reading");
    return;
  }

  if (g_pumpOn) {
    if (now - g_pumpStartMs >= PUMP_MAX_RUN_MS) {
      pumpStop("safety timeout");
    } else if (moisturePercent >= WET_PERCENT) {
      pumpStop("soil is wet");
    }
  } else {
    bool cooledDown = !g_hasStopped || (now - g_lastStopMs >= PUMP_COOLDOWN_MS);
    if (moisturePercent <= DRY_PERCENT && cooledDown) {
      pumpStart();
    }
  }
}

inline bool isPumpOn() { return g_pumpOn; }

// Matches what the dashboard reads from AgriGuard/irrigation/pumpStatus
inline const char* pumpStatusString() { return g_pumpOn ? "ON" : "OFF"; }
