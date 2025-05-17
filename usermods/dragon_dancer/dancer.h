#pragma once

#include "wled.h"
#include "../wled00/FX.h"

#define DANCER_DEBUG_OUTPUT true // uses Serial.println for debugging
#define FX_MODE_DANCER 200
/*
Name@ sliders ; colors ; palette ; flags ; commands

sliders: 2..5 + 0..3 buttons. buttons begin after 5 entries in the CVS, which can be left empty if not needed
  SEGMENT.speed/intensity/custom1/custom2/custom3 (0..255 except custom3: 0..31)
  SEGMENT.check1/2/3
colors: just e.g. "1,2,3"
palette: optional, treated as cvs, if anything other than a number (e.g. "!"): "palette" icon
flags: string, optional, a number, defined in index.js/populateEffects() as "m", default 1=1D effect
commands:
  CSV of key=val
  sx=default value for speed
  ix=default value for speed
  c1/c2/c3=default value for the custom sliders
  o1/o2/o3=default value for the checkboxes (0/1)
  pal=set a specific palette (palette and effect IDs are shown in tooltip)
  m12=2D matrix related number, defined in index.js populateSegments map2D
  si= sound sim (shown in segment config)
  


see index.js/setEffectParameters


{
	"0": {
		"id": 0,
		"start": 0,
		"stop": 16,
		"len": 16,
		"grp": 1,
		"spc": 0,
		"of": 0,
		"on": true,
		"frz": false,
		"bri": 255,
		"cct": 127,
		"set": 0,
		"col": [
			[
				8,
				255,
				0
			],
			[
				0,
				0,
				255
			],
			[
				255,
				0,
				0
			]
		],
		"fx": 38,
		"sx": 24,
		"ix": 128,
		"pal": 50,
		"c1": 128,
		"c2": 128,
		"c3": 16,
		"sel": true,
		"rev": false,
		"mi": false,
		"o1": false,
		"o2": false,
		"o3": false,
		"si": 0,
		"m12": 0
	}
}
*/
static const char _data_FX_MODE_DANCER[] PROGMEM = "Dancer@Color speed,Circle speed,Circle Strength,,,Mirrored,Flipped;;;;sx=55,ix=125,c1=111,o2=1";
static const char _data_FX_MODE_DANCER_HELPER[] PROGMEM = "(Dancer Helper)@Slider 1,Slider 2, Slider 3,Slider 4, Slider 5, Box 1, Box 2, Box 3;col1,c2lor,3col,ccl4?;!;012vf;sx=1,ix=2,c1=3,c2=4,c3=5,o1=false,o2=true,o3=true,pal=50,mi=true,bri=128,m12=2,si=1";

#define MAX_BLEND_ENTRIES 20

struct dancer_blendentry {
  CRGB col;
  uint16_t len;
  uint16_t lensum;
};

struct dancer_blendTable {
  uint8_t size;
  uint16_t totalLen;
  struct dancer_blendentry entries[MAX_BLEND_ENTRIES];
};

#define DANCER_BALL_LENGTH 16 // number of LEDs in a ball
#define DANCER_BALL_OFFSET 200 // offset for the ball color in the blend table
// note: lensum and totalLen are not set in this struct, but calculated in initBlendTable()
struct dancer_blendTable blendTable = {
  6, 0,
  { { { 0xFF, 0x00, 0x00 }, 200, 0 },
    { { 0xFF, 0x00, 0x00 }, 150, 0 }, //    0: red
    { { 0xFF, 0x00, 0x00 },  25, 0 },
    { { 0xFF, 0xFF, 0x00 },  25, 0 },
    { { 0x00, 0xFF, 0xFF }, 150, 0 }, //  600: cyan
    { { 0x00, 0xFF, 0xFF },  25, 0 },
    { { 0x00, 0xFF, 0x00 }, 150, 0 }, //  400: green
    { { 0x00, 0xFF, 0x00 },  25, 0 },
    { { 0xFF, 0xFF, 0x00 },  25, 0 },
    { { 0xFF, 0x00, 0xFF }, 150, 0 }, // 1000: violet
    { { 0xFF, 0x00, 0xFF },  25, 0 },
    { { 0xFF, 0x00, 0x00 },  25, 0 },
    { { 0xFF, 0xFF, 0x00 }, 150, 0 }, //  200: yellow
    { { 0xFF, 0xFF, 0x00 },  25, 0 },
    { { 0x00, 0xFF, 0x00 },  25, 0 },
    { { 0x00, 0x00, 0xFF }, 150, 0 }, //  800: blue
    { { 0x00, 0x00, 0xFF },  50, 0 } }
};

// how many leds offset we need to get from led index 0 to the "led on the upper side of the circle" (based on measurement of Dancer hardware)
#define DANCER_MAX_BALLS 6
uint8_t DANCER_CIRCLE_OFFSET[DANCER_MAX_BALLS] = { 4, 5, 3, 10, 9, 10 };

uint8_t map2(int8_t a, int32_t b, int32_t y, uint16_t z) {
	return ((uint8_t) (b * z / y)) + a;
}

class DragonDancerFx : public Usermod {
public:

void setup() {
    if (DANCER_DEBUG_OUTPUT) Serial.println("DragonDancer mod setup()!");
    initBlendTable();
    strip.addEffect(FX_MODE_DANCER, fxDancer, _data_FX_MODE_DANCER);
    strip.addEffect(FX_MODE_DANCER+1, fxDancerCircleOffsetHelper, _data_FX_MODE_DANCER_HELPER);
  }

  /** Calculates lensum and totalLen */
  void initBlendTable() {
    uint16_t lensum = 0;
    for (int j = 0; j < blendTable.size; j++) {
      blendTable.entries[j].lensum = lensum;
      lensum += blendTable.entries[j].len;
    }
    blendTable.totalLen = lensum;
  }

  void loop() {
    // Not needed unless your effect needs background processing
  }

/*
  void setupSegmentEffects() {
    // Iterate through each segment and assign the effect
    int num_segments = strip.getSegmentsNum();
    for (int i = 0; i < num_segments; i++) {
      // Get segment info
      Segment& seg = strip.getSegment(i);
  
      // Set the effect for each segment. In this case, using your custom effect
      seg.mode = FX_MODE_DANCER;  // Your custom effect
      seg.speed = 1000;  // Example speed value
      seg.intensity = 128;  // Example intensity
  
      // Optionally, you can tweak the segment parameters based on their index
//      seg.start = i * 16;  // Adjust starting index for each segment if needed
//      seg.stop = seg.start + 16;  // Adjust stop index for segment length
    }
  
    // Apply any changes to the segment configuration
//    strip.updateSegments();
// doesn't exist, and neither does strip.saveSegments()
  }
//*/
  
private:
/*
  static uint16_t myEffect_orig() {
    // get local offset based on current time and segment index
    // interpolate color from blend blendTable
    // iterate over LEDs in segment
    // 
    strip.getSegment(0);
    for (int i=SEGLEN-1; i > 0; i--) {
      CRGB col = CRGB(0, 0, 0);
      if ((strip.now + i*500/8) % 1000 < 500) {
        col = CRGB(0, 32, 0); // on
      } else {
        col = CRGB(0, 0, 0); // off
      }
      SEGMENT.setPixelColor(i, col);
    }
    return 20;  // delay until next frame (ms)
  }
//*/

/* Ideas
  Intensity: change circle speed (currently 250) and dimmer dropoff rate (px_dimm, relative to 128)

  Default Parameters:
    speed: 55
    intensity: 111
*/

  static uint16_t fxDancer() {
    // configuration parameters.
    uint8_t speed = SEGMENT.speed == 0 ? 0 : 256-SEGMENT.speed; // was: 55 -> 5 steps per second, needs 40 seconds per color (lower is faster)
    uint16_t px_dimm = 256 - SEGMENT.custom1; // was: 111 -> 0..255, 128 = no dimming, 0 = full dimming
    bool mirror = !SEGMENT.check1; // off = hard edge but more noticeable effect, on = more subtle but smooth
    bool toggleCircleDirection = !SEGMENT.check2; // toggle circle movement direction for each ball
    uint_fast16_t curPxDimm = 0;

    CRGB c;
    // current ball index, 0..DANCER_MAX_BALLS-1
    uint8_t ballIdx = DANCER_MAX_BALLS;
    // current pixel index in the ball, 0..DANCER_BALL_LENGTH-1
    uint8_t idxInBall = DANCER_BALL_LENGTH;
    // segment pixel index of first pixel in the current ball
    uint16_t ballStartIdx = 0;
    // current color index in blendTable
    uint8_t cci = 0;
    // optimization: if the same as cci, we don't need to recalculate rgb
    uint8_t cached_cci = -1; // cache starts invalid
    uint16_t len = 0;
    // current rgb color
    uint8_t r = 0, g = 0, b = 0;
    // delta to next color in blendTable
    int16_t dr = 0, dg = 0, db = 0;
    // color offset in blendTable for this ball
    uint8_t color_offset;
    // per-pixel color offset (not used in this effect, as we use dimming instead)
    uint16_t px_offset = 0;
    // number of leds to be computed in a ball
    uint16_t px_count = mirror ? (DANCER_BALL_LENGTH >> 1) + 1 : DANCER_BALL_LENGTH;
    //
    int32_t blend_index_current = 0;
    // speed (and direction) to rotate the circle
    int16_t circleSpeed = 0;
    // current position in the circle
    int32_t circle_offset_current = 0;

    // TODO: this is how fx.cpp does it
    uint32_t cycleTime = (255 - SEGMENT.speed)*20;
    uint32_t rem = strip.now % cycleTime;

    // loop over all pixels we are responsible for in this segment
    for(uint16_t pxIdx = 0; pxIdx < SEGMENT.length(); pxIdx++) {

    idxInBall++;
    if (idxInBall >= DANCER_BALL_LENGTH) {
      idxInBall = 0;
      ballIdx++;
      if (ballIdx >= DANCER_MAX_BALLS) {
        // init for first ball (or wrap around)
        ballIdx = 0;
      }
      // init for each ball
      ballStartIdx = ballIdx * DANCER_BALL_LENGTH;
      color_offset = ballIdx * DANCER_BALL_OFFSET;
      circleSpeed = SEGMENT.intensity == 0 ? 0 : ((ballIdx & 1) && toggleCircleDirection ? SEGMENT.intensity-256 : 256-SEGMENT.intensity) * 2;
//      blend_index_current = (color_offset + (speed == 0 ? 0 : speed > 0 ? strip.now / speed : blendTable.totalLen - 1 + (strip.now / speed))) % blendTable.totalLen;
      blend_index_current = speed == 0 ? 0 : strip.now / speed;
      blend_index_current = (blend_index_current + color_offset) % blendTable.totalLen;

      circle_offset_current = DANCER_CIRCLE_OFFSET[ballIdx % DANCER_MAX_BALLS] + (circleSpeed == 0 ? 0 : circleSpeed > 0 ? strip.now / circleSpeed / 2 : SEGMENT.length() - 1 - ((strip.now / (-circleSpeed/2)) % SEGMENT.length()));
      curPxDimm = SEGMENT.currentBri();
      if (DANCER_DEBUG_OUTPUT && (strip.now % 1000 < FRAMETIME))
        Serial.println("ballIdx=" + String(ballIdx) + ", blend_index_current=" + String(blend_index_current) +
        ", circleSpeed=" + String(circleSpeed) + ", circle_offset=" + String(circle_offset_current) + ", speed=" + String(speed) + 
        ", curPxDimm=" + String(curPxDimm) + ", px_dimm=" + String(px_dimm) + ", pxIdx=" + String(pxIdx) + ", idxInBall=" + String(idxInBall) + 
        ", ballStartIdx=" + String(ballStartIdx));
    }
    if (idxInBall >= px_count) {
      // skip this pixel, as it's already set by the mirrored pixel
      continue;
    }
  
    // limit ball color offset to total range of color-blend table
    if (blend_index_current >= blendTable.totalLen) { blend_index_current %= blendTable.totalLen; }
    if (blend_index_current < 0) { blend_index_current = blendTable.totalLen - 1 - ((blend_index_current) % blendTable.totalLen); }
  
    // calculate current color index (cci) in the color-blend blendTable
      uint16_t lensum = blendTable.entries[cci].lensum;
      while (blend_index_current < lensum) {
        if (cci > 0) {
          cci--;
        } else {
          cci = blendTable.size - 1;
        }
        lensum = blendTable.entries[cci].lensum;
      }
      while (blend_index_current >= lensum + blendTable.entries[cci].len) {
        cci++;
        if (cci >= blendTable.size) {
          cci = 0;
        }
        lensum = blendTable.entries[cci].lensum;
      }
      // relative position inside the current color-blend blendTable entry
      uint16_t pos = blend_index_current - blendTable.entries[cci].lensum;

      // cache some values
      if (cached_cci != cci) { // cache still valid?
        cached_cci = cci;
        struct dancer_blendentry entry = blendTable.entries[cci];
        struct dancer_blendentry next_entry = blendTable.entries[(cci+1) % blendTable.size];
        len = entry.len;
        r = entry.col.r;
        g = entry.col.g;
        b = entry.col.b;
        dr = next_entry.col.r - r;
        dg = next_entry.col.g - g;
        db = next_entry.col.b - b;
      }
  
      // calculate color
      c.r = map2(r, dr, len, pos);
      c.g = map2(g, dg, len, pos);
      c.b = map2(b, db, len, pos);
      if (curPxDimm < 255) {    
        c.r = (uint16_t(c.r) * curPxDimm) >> 8;
        c.g = (uint16_t(c.g) * curPxDimm) >> 8;
        c.b = (uint16_t(c.b) * curPxDimm) >> 8;
      }

      // set color
      uint16_t dst_idx = ballStartIdx + ((circle_offset_current + idxInBall) % DANCER_BALL_LENGTH);
      if (dst_idx < SEGMENT.length()) {
        SEGMENT.setPixelColor(dst_idx, c);
      }
  
      if (mirror) {
        // set mirrored pixel
        uint16_t dst_idx2 = ballStartIdx + ((DANCER_BALL_LENGTH + circle_offset_current - idxInBall) % DANCER_BALL_LENGTH);
        if (dst_idx2 != dst_idx && dst_idx2 < SEGMENT.length()) {
          SEGMENT.setPixelColor(dst_idx2, c);
        }
      }

      // calculate per-led (pixel) offset inside this segment for next loop iteration
      blend_index_current = (blendTable.totalLen + blend_index_current + (px_offset % blendTable.totalLen)) % blendTable.totalLen;
      curPxDimm = (curPxDimm * px_dimm) >> 8;
    }

    return FRAMETIME;
  }

  static uint8_t lastX;

  /** This effect helps to find the "circle offset". Change the intensitiy slider until the red dot is at the top.
   *  Green marks the currently configured position, blue is the physical first pixel of this ball. */
  static uint16_t fxDancerCircleOffsetHelper() {
    uint8_t offset = (SEGMENT.intensity / 10) % DANCER_BALL_LENGTH;
    if (DANCER_DEBUG_OUTPUT && SEGMENT.intensity != lastX) {
      Serial.println("Offset: " + String(offset) + " (" + String(SEGMENT.intensity) + ")");
      lastX = SEGMENT.intensity;
    }
    for(uint16_t pxIdx = 0; pxIdx < SEGMENT.length(); pxIdx++) {
      CRGB c = CRGB(0, 0, 0);
      uint16_t idxInBall = pxIdx % DANCER_BALL_LENGTH;
      uint16_t ballIdx = (pxIdx / DANCER_BALL_LENGTH) % DANCER_MAX_BALLS;
      if (idxInBall == 0) {
        c.b = 64;
      }
      if ((idxInBall + offset) % DANCER_BALL_LENGTH == 0) {
        c.r = 64;
      }
      if ((idxInBall + DANCER_CIRCLE_OFFSET[ballIdx]) % DANCER_BALL_LENGTH == 0) {
        c.g = 64;
      }      
      SEGMENT.setPixelColor(pxIdx, c);
    }
    return FRAMETIME;
  }


  // ---------------------------------

/*
  static uint16_t myOffsetEffect() {
    // Get the number of segments
    int num_segments = strip.getSegmentsNum();
    
    // Iterate over each segment
    for (int segmentIndex = 0; segmentIndex < num_segments; segmentIndex++) {
      // Get segment information (start and end LED indices)
      Segment& seg = strip.getSegment(segmentIndex);
      
      // Calculate offset based on segment index
      uint32_t segmentOffset = segmentIndex * 100;  // For example, 100ms per segment
      
      // Adjust the counter for this segment
      uint32_t segmentCounter = strip.now + segmentOffset;
  
      // Apply effect logic (e.g., color wipe or animation) for this segment
      for (int i = seg.start; i <= seg.stop; i++) {
        // This simple effect sets colors based on the segment's position
        if (segmentCounter % 2 == 0) {
          strip.setPixelColor(i, RGBW32(255, 0, 0, 0));  // Red
        } else {
          strip.setPixelColor(i, RGBW32(0, 255, 0, 0));  // Green
        }
      }
    }
 
    return 20; // Delay before next frame (ms)
  }

  static uint16_t sinusoidalEffectWithOffset() {
    int num_segments = strip.getSegmentsNum();
    
    for (int segmentIndex = 0; segmentIndex < num_segments; segmentIndex++) {
      Segment& seg = strip.getSegment(segmentIndex);
      
      // Calculate the offset for each segment
      uint32_t segmentOffset = segmentIndex * 10;  // A smaller offset for smoother effects
      
      // Apply a sinusoidal pattern with offset
      for (int i = seg.start; i <= seg.stop; i++) {
        uint32_t pixelOffset = segmentOffset + (i % 255);  // Individual pixel offset
        uint8_t brightness = (sin((strip.now + pixelOffset) * 0.1) * 127 + 128);  // Sin wave pattern
  
        // Set the color based on sine wave calculation
        strip.setPixelColor(i, RGBW32(brightness, 0, 0, 0));  // Red channel
      }
    }
    
    return 20;  // Frame delay (ms)
  }
//*/
    
};
uint8_t DragonDancerFx::lastX = 0;

