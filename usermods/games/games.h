// TODO

#pragma once

#include "wled.h"
#include <Wire.h>
#include "MPU6050.h"   // same library TWANG32 uses

class GamesMod : public Usermod {
private:
  // Hardcoded pins for now (later: make configurable)
  const uint8_t PIN_BUTTON_START = 32;
  const uint8_t PIN_I2C_SDA      = 21;
  const uint8_t PIN_I2C_SCL      = 22;
  const uint8_t PIN_AUDIO_DAC    = 25;

  bool gameActive = false;
  unsigned long lastFrameMs = 0;
  const uint16_t frameTimeMs = 16; // ~60 FPS
  MPU6050 mpu;
  bool lastButton = HIGH;

  void startGame() {
    twangStartNewGame();     // reset game state
    gameActive = true;
  }

  void stopGame() {
    gameActive = false;
  }

  void updateGame() {
    // Read MPU6050 tilt (simplified example)
    mpu.readSensor();
    float ax = mpu.getAccelX_mss();
    // Map accel to joystick range -1..+1
    float joystick = constrain(ax / 4.0f, -1.0f, 1.0f);

    // Attack (twang) detection – for now mapped to the start button
    bool attackPressed = (digitalRead(PIN_BUTTON_START) == LOW);

    uint32_t now = millis();
    twangStep(joystick, attackPressed, now);

    // If game logic says it is over, stop
    if (!twangIsRunning()) {
      stopGame();
    }
  }

  void renderGameToLeds() {
    uint16_t count = strip.getLengthTotal(); // or segment length
    static std::vector<uint8_t> r, g, b;
    if (r.size() != count) {
      r.assign(count, 0);
      g.assign(count, 0);
      b.assign(count, 0);
    }

    twangRender(r.data(), g.data(), b.data(), count);

    for (uint16_t i = 0; i < count; i++) {
      strip.setPixelColor(i, RGBW32(r[i], g[i], b[i], 0));
    }
  }


public:
  void setup() override {
    Serial.println(F("GamesMod setup()"));

    // Button to GND with internal pull-up
    pinMode(PIN_BUTTON_START, INPUT_PULLUP);

    // I2C for MPU6050 (Wire is already used by WLED; re-init is usually safe)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    mpu.setup(0x68); // or the address from TWANG32 config

    // Initialize game with WLED’s LED count
    twangInit(strip.getLengthTotal());
  }

  void loop() override {
    // Do not run game logic too often
    unsigned long now = millis();

    // Start button: active LOW
    static bool lastButton = HIGH;
    bool btn = digitalRead(PIN_BUTTON_START);
    if (btn == LOW && lastButton == HIGH) {
      // Rising edge (button press) → (re)start game
      startGame();
    }
    lastButton = btn;

    if (!gameActive) {
      // Idle: let WLED effects run unmodified
      return;
    }

    // Simple frame limiter
    if (now - lastFrameMs < frameTimeMs) return;
    lastFrameMs = now;

    updateGame();
    renderGameToLeds();
  }

  uint16_t getId() override {
    // TODO: used for pin ownership, see wled00/const.h and pin_manager.h
    return USERMOD_ID_RESERVED;
  }
};
