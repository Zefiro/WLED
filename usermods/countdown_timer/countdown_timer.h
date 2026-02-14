#pragma once

#include "wled.h"

// Enable or disable Serial debugging for this usermod
#define CT_DEBUG 1

#if CT_DEBUG
  #define CT_DEBUG_PRINT(x)   Serial.print(x)
  #define CT_DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define CT_DEBUG_PRINT(x)
  #define CT_DEBUG_PRINTLN(x)
#endif

// Countdown Timer usermod for WLED 0.15.0 (v2 API).
// - Configure / monitor via:
//   * Usermod config UI
//   * JSON state (HTTP /json/state, MQTT deviceTopic/api)
// - JSON namespace: "ct"
//
// Example JSON payload sent to deviceTopic/api:
// {"ct":{"seconds":90,"preset":12}}
// can also give a preset (ps2) to apply directly
// {"ct":{"seconds":90,"preset":12,"ps2":5}}
// or just reset the counter
// {"ct":{"clear":0}}

class CountdownTimerUsermod : public Usermod {
private:
  // Configurable fields
  uint32_t countdownSeconds = 0;     // configured countdown duration in seconds
  uint32_t targetPreset     = 0;     // preset to apply on expiry (0 = disabled)

  // Runtime fields
  bool     countdownActive  = false;
  uint32_t remainingSeconds = 0;
  uint32_t lastMillis       = 0;

  // Timestamps (UNIX-ish seconds, using localTime)
  uint32_t lastResetTs      = 0;
  uint32_t lastTriggerTs    = 0;

  // WLED 0.15 provides localTime via Time.cpp.
  // We use it as "now"; if 0, fall back to millis()/1000.
uint32_t getNowTs() {
  if (localTime > 1000000000UL) {
    return localTime;
  }
  return millis() / 1000;
}

// Format timestamp as "dd.mm.yyyy hh:mm:ss" or "hh:mm:ss" (boot-relative)
String formatTime(uint32_t ts) {
  if (ts == 0) {
    return F("never");
  }

  if (localTime > 1000000000UL) {
    // NTP/localTime available: format as full date+time
    time_t epoch = ts;
    struct tm* timeinfo = localtime(&epoch);

    if (timeinfo) {
      char buffer[64];
      // %02d = 2 digits padded, %04d = 4 digits
      snprintf(buffer, sizeof(buffer), "%02d.%02d.%04d %02d:%02d:%02d",
               timeinfo->tm_mday,
               timeinfo->tm_mon + 1,
               timeinfo->tm_year + 1900,
               timeinfo->tm_hour,
               timeinfo->tm_min,
               timeinfo->tm_sec);
      return String(buffer);
    }
  }

  // Fallback: uptime in hh:mm:ss
  uint32_t hours = ts / 3600;
  uint32_t mins  = (ts % 3600) / 60;
  uint32_t secs  = ts % 60;

  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours, mins, secs);
  return String(buffer);
}

void resetCountdownFromConfig() {
    if (countdownSeconds == 0 || targetPreset == 0) {
      countdownActive  = false;
      remainingSeconds = 0;
      CT_DEBUG_PRINTLN(F("[CT] Countdown disabled by config (seconds=0 or preset=0)."));
      return;
    }

    remainingSeconds = countdownSeconds;
    countdownActive  = true;
    lastMillis       = millis();
    lastResetTs      = getNowTs();

    CT_DEBUG_PRINT(F("[CT] Countdown reset from config: "));
    CT_DEBUG_PRINT(remainingSeconds);
    CT_DEBUG_PRINT(F(" seconds, preset "));
    CT_DEBUG_PRINTLN(targetPreset);
  }

  void resetCountdownFromJson(uint32_t seconds, uint32_t preset, bool clear) {
    if (clear) {
      countdownActive   = false;
      remainingSeconds  = 0;
      countdownSeconds  = 0;
      targetPreset      = 0;
      CT_DEBUG_PRINTLN(F("[CT] Countdown cleared via JSON."));
      return;
    }

    if (seconds > 0) {
      countdownSeconds = seconds;
    }
    if (preset > 0) {
      targetPreset = preset;
    }

    if (countdownSeconds == 0 || targetPreset == 0) {
      countdownActive  = false;
      remainingSeconds = 0;
      CT_DEBUG_PRINTLN(F("[CT] JSON update disabled countdown (seconds=0 or preset=0)."));
      return;
    }

    remainingSeconds = countdownSeconds;
    countdownActive  = true;
    lastMillis       = millis();
    lastResetTs      = getNowTs();

    CT_DEBUG_PRINT(F("[CT] Countdown reset from JSON: "));
    CT_DEBUG_PRINT(remainingSeconds);
    CT_DEBUG_PRINT(F(" seconds, preset "));
    CT_DEBUG_PRINTLN(targetPreset);
  }

  void tickCountdown() {
    if (!countdownActive) return;

    uint32_t nowMillis = millis();
    if (nowMillis - lastMillis >= 1000) {
      uint32_t elapsedSec = (nowMillis - lastMillis) / 1000;
      lastMillis += elapsedSec * 1000;

      if (elapsedSec >= remainingSeconds) {
        remainingSeconds = 0;
      } else {
        remainingSeconds -= elapsedSec;
      }

      if (remainingSeconds == 0) {
        countdownActive = false;
        lastTriggerTs   = getNowTs();

        CT_DEBUG_PRINT(F("[CT] Countdown reached zero. Applying preset "));
        CT_DEBUG_PRINTLN(targetPreset);

        if (targetPreset > 0 && currentPreset != targetPreset) { // checking currentPreset seems to be unnecessary, probably already done internally
          applyPreset(targetPreset, CALL_MODE_BUTTON_PRESET); // pretend we're a button, otherwise presets containing chaining won't work, see presets.cpp handlePresets()
        }
      }
    }
  }

  
public:
  // === Core v2 hooks ===

  void setup() override {
    CT_DEBUG_PRINTLN(F("[CT] CountdownTimerUsermod setup()"));
  // DISABLE COUNTDOWN ON BOOT (as readFromConfig() is called first and might trigger a start)
  countdownActive  = false;
  remainingSeconds = 0;
  }

  void loop() override {
    tickCountdown();
  }

  void addToJsonState(JsonObject& root) override {
    JsonObject ct = root.createNestedObject("ct");
    ct["seconds_cfg"] = countdownSeconds;
    ct["preset"]      = targetPreset;
    ct["remaining"]   = remainingSeconds;
    ct["active"]      = countdownActive;
    ct["last_reset_raw"]  = lastResetTs;
    ct["last_triggered_raw"]= lastTriggerTs;
    ct["last_reset"]  = formatTime(lastResetTs);
    ct["last_triggered"]= formatTime(lastTriggerTs);
  }

  bool readFromConfig(JsonObject& root) override {
    // Config block (for persistent usermod settings)
    JsonObject top = root["CountdownTimer"];
    if (!top.isNull()) {
      if (top.containsKey("seconds")) {
        countdownSeconds = top["seconds"].as<uint32_t>();
      }
      if (top.containsKey("preset")) {
        targetPreset = top["preset"].as<uint32_t>();
      }
      // On config change, restart countdown
      resetCountdownFromConfig();
    }
    return true;
}

void readFromJsonState(JsonObject &root) override
{
    // Sets the countdown timer and target preset, optionally also a desired current preset. This implicitely starts the countdown.
    // {"ct":{"seconds":90,"preset":12,"clear":0,"ps2":1}}
    if (root.containsKey("ct"))
    {
        JsonObject ct = root["ct"];
        uint32_t seconds = ct["seconds"] | 0;
        uint32_t preset = ct["preset"] | 0;
        uint8_t clear = ct["clear"] | 0;
        uint8_t targetPreset = ct["ps2"] | 0;

        resetCountdownFromJson(seconds, preset, clear != 0);

        if (targetPreset > 0 && currentPreset != targetPreset)
        {
            applyPreset(targetPreset, CALL_MODE_BUTTON_PRESET); // pretend we're a button, otherwise presets containing chaining won't work, see presets.cpp handlePresets()
        }
    }
}

  // Config UI section
  void addToConfig(JsonObject& root) override {
    JsonObject top = root.createNestedObject("CountdownTimer");

    // Editable
    top["seconds"] = countdownSeconds;
    top["preset"]  = targetPreset;

    // Read-only status
    top["remaining"] = remainingSeconds;
    top["last_reset"] = formatTime(lastResetTs);
    top["last_triggered"] = formatTime(lastTriggerTs);
  }

  // getId is uint16_t in 0.15; you can return 0 if you don't need a static ID.
  uint16_t getId() override {
    return USERMOD_ID_UNSPECIFIED; // or 0; WLED defines USERMOD_ID_UNSPECIFIED.
  }
};
