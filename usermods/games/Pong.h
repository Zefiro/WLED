#include "Arduino.h"
#include "settings.h"

// -----------------------------------------------------
// TODO stuff moved from main file (should be refactored)
#define PARTICLE_COUNT 100
Particle particlePool[PARTICLE_COUNT] = {
    Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle(), Particle()
};

// -----------------------------------------------------
// TODO copied over (and adapted) from main .ino, should better be refactored

int getLED2(int pos){
    // The world is 1000 pixels wide, this converts world units into an LED number
    return constrain((int)map(pos, 0, VIRTUAL_LED_COUNT, 0, user_settings.led_count-1)+user_settings.led_offset, 0, user_settings.led_count-1);
}

bool pong_tickParticles(CRGB* leds){
  bool stillActive = false;
  uint8_t brightness;
  for(int p = 0; p < PARTICLE_COUNT; p++){
    if(particlePool[p].Alive()){
      particlePool[p].Tick(USE_GRAVITY);
      
      if (particlePool[p]._power < 5)
      {
        brightness = (5 - particlePool[p]._power) * 10;
        leds[getLED2(particlePool[p]._pos)] += CRGB(brightness, brightness/2, brightness/2);\
      }
      else      
      leds[getLED2(particlePool[p]._pos)] += CRGB(particlePool[p]._power, 0, 0);
      
      stillActive = true;
    }
  }
  return stillActive;
}

void pong_tickDie(long mm, long gamestate_since, CRGB* leds, uint16_t pos) { // a short bright explosion...particles persist after it.
  const int duration = 200; // milliseconds
  const int width = 20;     // half width of the explosion

  if(gamestate_since+duration > mm) {// Spread red from player position up and down the width
  
    int brightness = map((mm-gamestate_since), 0, duration, 255, 150); // this allows a fade from white to red
    
    // fill up
    int n = _max(map(((mm-gamestate_since)), 0, duration, pos, pos+width), 0); // TODO Zefiro: shouldn't this be min(..., user_settings.led_count) ?
    for(int i = pos; i<= n; i++){
      leds[i+user_settings.led_offset] = CRGB(255, brightness, brightness);
    }
    
    // fill to down
    n = _max(map(((mm-gamestate_since)), 0, duration, pos, pos-width), 0);
    for(int i = pos; i>= n; i--){
      leds[i+user_settings.led_offset] = CRGB(255, brightness, brightness);
    }
  }
}

// -----------------------------------------------------
// Generic function to check if a value is in a given set - provided by chatGPT
template<typename T>
bool isInSet(T value, std::initializer_list<T> set) {
    static_assert(std::is_enum<T>::value, "T must be an enum type");
    for (T v : set) {
        if (value == v) {
            return true;
        }
    }
    return false;
}

#define PONG_MAX_SCORE 5

class Pong {

  public:
    Pong(CRGB* leds, Bounce2::Button& butPlayerRight, Bounce2::Button& butPlayerLeft);
    void resetGame();
    void loop(bool redraw);
    enum Gamestates {
      STARTUP,
      FIRST_SERVE,
      PLAY,
      SERVE_RIGHT,
      SERVE_LEFT,
      LOST_RIGHT,
      LOST_LEFT,
      GAME_OVER
    } gamestate;
    uint16_t hitzone_length = 12;
    uint16_t hitzone_short_length = 6;
    uint16_t hitzone_mini_length = 2;
    float hitzone_speedup = 0.1;
    float hitzone_short_speedup = 0.15;
    float hitzone_mini_speedup = 0.2;
    uint8_t maximum_speed = 10;
    CRGB col_Hitzone_idle = CRGB(0, 0, 15);
    CRGB col_Hitzone_hit = CRGB(100, 100, 150);
    CRGB col_Hitzone_hit2 = CRGB(0, 0, 150);
    CRGB col_ball = CRGB(50, 50, 50);
    CRGB col_ball_tail = CRGB(40, 30, 20);
    CRGB col_score_left = CRGB(100, 0, 0);
    CRGB col_score_right = CRGB(0, 100, 0);
    uint8_t score_left, score_right;

  private:
    void setGamestate(Gamestates newState);
    void calculateGame(unsigned long mm);
    void reactOnPlayer();
    void moveBall();
    void loosePlayerRight();
    void loosePlayerLeft();
    void drawLost(unsigned long mm);
    void drawGameOver(unsigned long mm);
    void drawGame(unsigned long mm);
    CRGB* leds;
    float position_ball;
    float speed_ball = 0.1; // speed & direction (+ = left, - = right)
    bool hitzone_hit_right; // towards idx=0
    bool hitzone_hit_left; // towards idx=hitzone_length
    uint16_t hitzone_animation_right;
    uint16_t hitzone_animation_left;
    unsigned long gamestate_since;
    Bounce2::Button& butPlayerRight;
    Bounce2::Button& butPlayerLeft;
};

Pong::Pong(CRGB* leds, Bounce2::Button& butPlayerRight, Bounce2::Button& butPlayerLeft)
    : leds(leds), butPlayerRight(butPlayerRight), butPlayerLeft(butPlayerLeft) {}

void Pong::loop(bool redraw) {
  if (!redraw) return;
  unsigned long mm = millis();

  // if playing:
  calculateGame(mm);

  if (gamestate == LOST_RIGHT || gamestate == LOST_LEFT) {
    drawLost(mm);
  } else if (gamestate == GAME_OVER) {
    drawGameOver(mm);
  } else {
    drawGame(mm);
  }
}

void Pong::resetGame() {
  setGamestate(STARTUP);
  score_left = 0;
  score_right = 0;
}

void Pong::setGamestate(Gamestates newState) {
  if (gamestate == newState) return;
  switch(newState) {
    case LOST_RIGHT: {
      for(int p = 0; p < PARTICLE_COUNT; p++){
          particlePool[p].Spawn(position_ball);
      } 
    } break;
    case LOST_LEFT: {
      for(int p = 0; p < PARTICLE_COUNT; p++){
          particlePool[p].Spawn(position_ball);
      }
    } break;
    default: {
      speed_ball = 0;
      position_ball = 0;
    }
  }
  hitzone_hit_left = false;
  hitzone_hit_right = false;
  gamestate = newState;
  gamestate_since = millis();
}

void Pong::calculateGame(unsigned long mm) {
// TODO use everywhere: if (isInSet(gamestate, { FIRST_SERVE, SERVE_RIGHT }));
  if ((gamestate == FIRST_SERVE || gamestate == SERVE_RIGHT) && butPlayerRight.pressed()) {
    setGamestate(PLAY);
    speed_ball = random8(6, 12)/10.0;
  } else if ((gamestate == FIRST_SERVE || gamestate == SERVE_LEFT) && butPlayerLeft.pressed()) {
    setGamestate(PLAY);
    position_ball = user_settings.led_count-1;
    speed_ball = -(random8(6, 12)/10.0);
  } else if (gamestate == FIRST_SERVE || gamestate == SERVE_LEFT || gamestate == SERVE_RIGHT) {
    // waiting for button presses
  } else if (gamestate == PLAY) {
    reactOnPlayer();
    moveBall();
  } else if (gamestate == LOST_RIGHT || gamestate == LOST_LEFT) {
    //
  } else if (gamestate == GAME_OVER) {
    //
  } else if (gamestate == STARTUP) {
    setGamestate(FIRST_SERVE);
  } else {
    // shouldn't happen
    Serial.printf("!!!!!!!! unexpected PONG gamestate=%d !!!!!!!!\n", gamestate);
    setGamestate(GAME_OVER);
  }
}

void Pong::reactOnPlayer() {
  uint16_t int_ball = position_ball;
  uint16_t midfield = user_settings.led_count / 2;
  uint16_t relpos_ball = (int_ball < midfield) ? int_ball : user_settings.led_count - int_ball - 1;

  if (butPlayerRight.pressed()) {
    hitzone_hit_right = true;
    hitzone_animation_right = 0;
    if (speed_ball > 0 || int_ball > midfield) { 
      // buttons are ignored (no penalty) if the ball moves away from the player or is in the other half
    } else if (relpos_ball < hitzone_length) {
      if (relpos_ball < hitzone_mini_length) {
        // mini hit
        speed_ball = -speed_ball + hitzone_mini_speedup;
      } else if (relpos_ball < hitzone_short_length) {
        // short hit
        speed_ball = -speed_ball + hitzone_short_speedup;
      } else {
        // normal hit
        speed_ball = -speed_ball + hitzone_speedup;
      }
      if (speed_ball <= -maximum_speed || speed_ball > maximum_speed) {
        Serial.printf("!!!!!! Speeding ticket: %f\n", speed_ball);
        loosePlayerRight(); // safeguard, shouldn't happen
      }
      Serial.printf("Right hit - pos=%3.1f (relpos=%3d), speed=%2.1f\n", position_ball, relpos_ball, speed_ball);
    } else { // pressed too early (outside hit zone)
      Serial.printf("Right lost - too early - pos=%3.1f (relpos=%3d), speed=%2.1f\n", position_ball, relpos_ball, speed_ball);
      loosePlayerRight();
    }
  }

  if (butPlayerLeft.pressed()) {
    hitzone_hit_left = true;
    hitzone_animation_left = 0;
    if (speed_ball < 0 || int_ball < midfield) { 
      // buttons are ignored (no penalty) if the ball moves away from the player or is in the other half
    } else if (relpos_ball < hitzone_length) {
      if (relpos_ball < hitzone_mini_length) {
        // mini hit
        speed_ball = -speed_ball - hitzone_mini_speedup;
      } else if (relpos_ball < hitzone_short_length) {
        // short hit
        speed_ball = -speed_ball - hitzone_short_speedup;
      } else {
        // normal hit
        speed_ball = -speed_ball - hitzone_speedup;
      }
      if (speed_ball <= -maximum_speed || speed_ball > maximum_speed) {
        Serial.printf("!!!!!! Speeding ticket: %f\n", speed_ball);
        loosePlayerLeft(); // safeguard, shouldn't happen
      }
      Serial.printf("Left hit - pos=%3.1f (relpos=%3d), speed=%2.1f\n", position_ball, relpos_ball, speed_ball);
    } else { // pressed too early (outside hit zone)
      Serial.printf("Left lost - too early - pos=%3.1f (relpos=%3d), speed=%2.1f\n", position_ball, relpos_ball, speed_ball);
      loosePlayerLeft();
    }
  }
}

void Pong::moveBall() {
  position_ball += speed_ball;

  if (position_ball >= user_settings.led_count) { // leaving field to the left
    Serial.printf("Left lost - missed - pos=%3.1f, speed=%2.1f\n", position_ball, speed_ball);
    loosePlayerLeft();
  } else if (position_ball < 0) { // leaving field to the right
    Serial.printf("Right lost - missed - pos=%3.1f, speed=%2.1f\n", position_ball, speed_ball);
    loosePlayerRight();
  }

}

void Pong::loosePlayerRight() {
  score_left++;
  if (score_left < PONG_MAX_SCORE) {
    Serial.printf("Left %d : %d Right. Right serves.\n", score_left, score_right);
    setGamestate(LOST_RIGHT);
  } else {
    Serial.printf("Left %d : %d Right. Game Over. Left wins.\n", score_left, score_right);
    setGamestate(GAME_OVER);
  }
}

void Pong::loosePlayerLeft() {
  score_right++;
  if (score_right < PONG_MAX_SCORE) {
    Serial.printf("Left %d : %d Right. Left serves.\n", score_left, score_right);
    setGamestate(LOST_LEFT);
  } else {
    Serial.printf("Left %d : %d Right. Game Over. Right wins.\n", score_left, score_right);
    setGamestate(GAME_OVER);
  }
}

void Pong::drawLost(unsigned long mm) {
  FastLED.clear();
  pong_tickDie(mm, gamestate_since, leds, position_ball);
  if (!pong_tickParticles(leds)) {
    setGamestate(gamestate == LOST_RIGHT ? SERVE_RIGHT : SERVE_LEFT);
  }
}

void Pong::drawGameOver(unsigned long mm) {
  if (millis() - gamestate_since < 3000) {
    bool leftWon = score_left > score_right;
    uint16_t midfield = user_settings.led_count / 2;
    CRGB col;
    uint16_t x1, x2;
    if (leftWon) {
      col = col_score_left;
      x1 = midfield;
      x2 = user_settings.led_count;
    } else {
      col = col_score_right;
      x1 = 0;
      x2 = midfield;
    }
    col = col.fadeToBlackBy(sin8(mm/4));
    for(uint16_t x = x1; x < x2; x++) {
      leds[x+user_settings.led_offset] = col;
    }
  } else {
    resetGame();
  }
}

void Pong::drawGame(unsigned long mm) {
  // clear stage
  FastLED.clear();
  
  uint16_t int_ball = position_ball;
  uint16_t midfield = user_settings.led_count / 2;

  // draw left & right hit zone
  CRGB col = col_Hitzone_idle;
  bool drawHitzone = false;
  if (gamestate == PLAY) {
    drawHitzone = true;
    if (hitzone_hit_right) {
      if (hitzone_animation_right < 20) {
        col = col_Hitzone_hit;
      } else if (hitzone_animation_right < 120) {
        col = blend(col_Hitzone_hit2, col_Hitzone_idle, map(hitzone_animation_right, 20, 120, 0, 255));
      } else {
        hitzone_hit_right = false;
      }
      hitzone_animation_right++;
    }
  } else if (gamestate == FIRST_SERVE || gamestate == SERVE_RIGHT) {
    col = blend(col_Hitzone_hit2, col_Hitzone_idle, sin8(mm/4));
    drawHitzone = true;
  } else if (gamestate == SERVE_LEFT) {
    drawHitzone = true;
  }
  if (drawHitzone) {
    for(uint16_t x = 0; x < hitzone_length; x++) {
      leds[x+user_settings.led_offset] = col;
    }
  }
  
  col = col_Hitzone_idle;
  drawHitzone = false;
  if (gamestate == PLAY) {
    drawHitzone = true;
    if (hitzone_hit_left) {
      if (hitzone_animation_left < 20) {
        col = col_Hitzone_hit;
      } else if (hitzone_animation_left < 120) {
        col = blend(col_Hitzone_hit2, col_Hitzone_idle, map(hitzone_animation_left, 20, 120, 0, 255));
      } else {
        hitzone_hit_left = false;
      }
      hitzone_animation_left++;
    }
  } else if (gamestate == FIRST_SERVE || gamestate == SERVE_LEFT) {
    col = blend(col_Hitzone_hit2, col_Hitzone_idle, sin8(mm/4));
    drawHitzone = true;
  } else if (gamestate == SERVE_RIGHT) {
    drawHitzone = true;
  }
  if (drawHitzone) {
    for(uint16_t x = user_settings.led_count-hitzone_length; x < user_settings.led_count; x++) {
      leds[x+user_settings.led_offset] = col;
    }
  }
  
  // draw score (in the middle, darker when ball is near)
  if (gamestate == PLAY || gamestate == FIRST_SERVE || gamestate == SERVE_RIGHT || gamestate == SERVE_LEFT) {
    uint16_t x = midfield;
    uint8_t dimm = map(constrain(abs(int_ball - midfield), 20, 90), 20, 90, 220, 0);
    col = col_score_left;
    col.fadeToBlackBy(dimm);
    for(uint8_t i = 0; i < score_left; i++) {
      x++;
      leds[x++ +user_settings.led_offset] = col;
      leds[x++ +user_settings.led_offset] = col;
      x++;
    }
  
    x = midfield;
    col = col_score_right;
    col.fadeToBlackBy(dimm);
    for(uint8_t i = 0; i < score_right; i++) {
      x--;
      leds[x-- +user_settings.led_offset] = col;
      leds[x-- +user_settings.led_offset] = col;
      x--;
    }
  }
  

  // draw ball & tail (add sparks?)
  if (gamestate == PLAY) {
    col = col_ball;
    leds[int_ball+user_settings.led_offset] = col;
  
    col = col_ball_tail;
    int_ball += (speed_ball > 0) ? -1 : 1; // might wrap around
    if (int_ball >= 0 && int_ball < user_settings.led_count) leds[int_ball+user_settings.led_offset] = col;
  
    int_ball += (speed_ball > 0) ? -1 : 1; // might wrap around
    col.fadeToBlackBy(60);
    if (int_ball >= 0 && int_ball < user_settings.led_count) leds[int_ball+user_settings.led_offset] = col;
  
    int_ball += (speed_ball > 0) ? -1 : 1; // might wrap around
    col.fadeToBlackBy(60);
    if (int_ball >= 0 && int_ball < user_settings.led_count) leds[int_ball+user_settings.led_offset] = col;
  }
}
