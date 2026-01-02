// TODO
// simplify directional toggle and check if it works
// use SEGENV.step (and possibly SEGENV.aux0) for current index instead of basing blend_index_current on strip.now
// look again into palettes
// try WLED OTA update


#pragma once

#include "wled.h"
#include "../wled00/FX.h"

#define DANCER_DEBUG_OUTPUT false // uses Serial.println for debugging
#define FX_MODE_DANCER 200
/*
Name@ sliders ; colors ; palette ; flags ; commands

sliders: 2..5 + 0..3 buttons. buttons begin after 5 entries in the CVS, which can be left empty if not needed
  SEGMENT.speed/intensity/custom1/custom2/custom3 (0..255 except custom3: 0..31)
  SEGMENT.check1/2/3
colors: names of used colors (0..3), "fg/bg", should be 1-2 letters, shown in the colored dot
palette: optional, treated as cvs, if anything other than a number (e.g. "!"): "palette" icon shown
flags: string, optional, a number, defined in index.js/populateEffects() as "m", default 1=1D effect
commands:
  CSV of key=val
  sx=default value for speed
  ix=default value for intensity
  c1/c2/c3=default value for the custom sliders
  o1/o2/o3=default value for the checkboxes (0/1)
  pal=set a specific palette (palette and effect IDs are shown in tooltip)
  m12=2D matrix related number, defined in index.js populateSegments map2D
  si= sound sim (shown in segment config)
  col1/col2/col3=Custom command to set segment colors. As only int16_t is available, use 5-6-5 format: RRRRR GGGGGG BBBBB
  
Commands are set in the Backend, FX_fcn.cpp Segment::setMode() using extractModeDefaults().
Only those keys which are explicitely listed there can be set.

Frontend in index.js
on load, /json/effects (names of all effects) and /json/fxdata (effect data, part after the @) is loaded.
In populateEffects() the HTML is generated (list with names, icons, and the commands in data-opt html attribute)
Upon selecting an effect (or any data change), setEffectParameters() displays the controls (sliders, checkboxes, etc.)

JSON as sniffed from the network tab:
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
static const char _data_FX_MODE_DANCER[] PROGMEM = "Dancer@Color speed,Circle speed,Circle Dimming,Tail Length,,Mirrored,Flipped,Reverse;;;;sx=73,ix=135,c1=50,c2=0,o1=1,o2=1,o3=1";
//colorful: static const char _data_FX_MODE_DANCER[] PROGMEM = "Dancer@Color speed,Circle speed,Circle Dimming,Tail Length,,Mirrored,Flipped,Reverse;;;;sx=250,ix=128,c1=50,c2=63,o1=0,o2=1,o3=0";
static const char _data_FX_MODE_DANCER_HELPER[] PROGMEM = "(Dancer Helper)@Offset x10,,,,,Current,Saved,Origin;Current,Saved,Origin;;1;sx=0,bri=128,o1=1,o2=1,o3=1,col1=63488,col2=2016,col3=31";
// static const char _data_FX_MODE_EXAMPLE[] PROGMEM = "Effect Name@Slider 1,Slider 2,Slider 3,Slider 4,Slider 5,Checkbox 1,Checkbox 1,Checkbox 1;col1,c2lor,3col,ccl4?;!;012vf;sx=1,ix=2,c1=3,c2=4,c3=5,o1=1,o2=1,o3=1,pal=50,mi=true,bri=128,m12=2,si=1";

#define MAX_BLEND_ENTRIES 20

struct dancer_blendentry {
  CRGB col;
  uint16_t len; // length of this entry, in time units
  uint16_t lensum; // accumulated length at the start of this entry
};

struct dancer_blendTable {
  uint8_t size; // number of entries in this table
  uint16_t totalLen; // total length of this table, in time units
  struct dancer_blendentry entries[MAX_BLEND_ENTRIES];
};

#define DANCER_BLENDTABLE

#ifdef DANCER_BLENDTABLE
#define DANCER_BALL_LENGTH 16 // number of LEDs in a ball
#define DANCER_BALL_OFFSET 200 // offset for the ball color in the blend table
// note: lensum and totalLen are not set in this struct, but calculated in initBlendTable()
struct dancer_blendTable blendTable = {
  16, 0,
  { { { 0xFF, 0x00, 0x00 }, 150, 0 }, //    0: red
    { { 0xFF, 0x00, 0x00 },  25, 0 },
    { { 0xFF, 0xFF, 0x00 },  25, 0 },
    { { 0x00, 0xFF, 0xFF }, 150, 0 }, //  200: cyan
    { { 0x00, 0xFF, 0xFF },  50, 0 },
    { { 0x00, 0xFF, 0x00 }, 150, 0 }, //  400: green
    { { 0x00, 0xFF, 0x00 },  25, 0 },
    { { 0xFF, 0xFF, 0x00 },  25, 0 },
    { { 0xFF, 0x00, 0xFF }, 150, 0 }, //  600: violet
    { { 0xFF, 0x00, 0xFF },  25, 0 },
    { { 0xFF, 0x00, 0x00 },  25, 0 },
    { { 0xFF, 0xFF, 0x00 }, 150, 0 }, //  800: yellow
    { { 0xFF, 0xFF, 0x00 },  25, 0 },
    { { 0x00, 0xFF, 0x00 },  25, 0 },
    { { 0x00, 0x00, 0xFF }, 150, 0 }, // 1000: blue
    { { 0x00, 0x00, 0xFF },  50, 0 } }// total 1200
};
#else // DANCER_BLENDTABLE
#define DANCER_BALL_LENGTH 16 // number of LEDs in a ball
#define DANCER_BALL_OFFSET 200 // offset for the ball color in the blend table
#define dragon_flex_a 5    // skew linear: value to set at specified position
#define dragon_flex_b 170  // skew linear: position where to set the specified value
#define  dragon_flex_c 50   // time (in ticks) to show pure red

// note: lensum and totalLen are not set in this struct, but calculated in initBlendTable()
struct dancer_blendTable blendTable = { // full rainbow (red-enhanced) in 3650 ticks
  9, 0, {
    { { 0x00, 0xFF, 0x00 }, 600, 0 },
    { { 0x00, 0xFF, 0xFF }, 600, 0 },
    { { 0x00, 0x00, 0xFF }, 600, 0 },
    { { 0xFF, 0x00, 0xFF }, 600 - dragon_flex_b, 0 },
    { { 0xFF, 0x00, dragon_flex_a }, dragon_flex_b, 0 },
    { { 0xFF, 0x00, 0x00 }, dragon_flex_c, 0 },
    { { 0xFF, 0x00, 0x00 }, dragon_flex_b, 0 },
    { { 0xFF, dragon_flex_a, 0x00 }, 600 - dragon_flex_b, 0 },
    { { 0xFF, 0xFF, 0x00 }, 600, 0 },
  }};
#endif // DANCER_BLENDTABLE

#define DANCER_MAX_BALLS 6
// how many leds offset we need to get from led index 0 to the "led on the upper side of the circle" (based on measurement of Dancer hardware)
uint8_t DANCER_CIRCLE_OFFSET[DANCER_MAX_BALLS] = { 9, 10, 7, 16, 15, 1 };

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
    for (uint8_t j = 0; j < blendTable.size; j++) {
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
    uint8_t speed = SEGMENT.speed;
    uint16_t px_dimm = 256 - SEGMENT.custom1; // dimm strength, 0=no dimming, 255=full dimming (inverted because we multiply and divide by 256)
    bool mirror = SEGMENT.check1; // off = hard edge but more noticeable effect, on = more subtle but smooth
    bool toggleCircleDirection = SEGMENT.check2; // toggle circle movement direction for each ball
    uint16_t curPxDimm = 0;

    CRGB c;
    // current ball index, 0..DANCER_MAX_BALLS-1
    uint8_t ballIdx = DANCER_MAX_BALLS;
    // current pixel index in the ball, 0..DANCER_BALL_LENGTH-1
    uint16_t idxInBall = DANCER_BALL_LENGTH;
    // segment pixel index of first pixel in the current ball
    uint16_t ballStartIdx = 0;
    // current color index in blendTable
    uint8_t cci = 0;
    // optimization: if the same as cci, we don't need to recalculate rgb
    uint8_t cached_cci = -1; // cache starts invalid
    // current color in blendTable
    uint8_t r = 0, g = 0, b = 0;
    // delta to next color in blendTable
    int16_t dr = 0, dg = 0, db = 0;
    // current length to next color in blendTable
    uint16_t len = 0;
    // color offset in blendTable for this ball
    uint32_t color_offset;
    // per-pixel color offset (16.16 fixed point)
    uint32_t px_offset_fp = 0;
    // number of leds to be computed in a ball
    uint16_t px_count = mirror ? (DANCER_BALL_LENGTH >> 1) + 1 : DANCER_BALL_LENGTH;
    // index in blendTable: for the first led of a ball, for the current led in the loop
    uint32_t blend_index_current_fp = 0; // 16.16 fixed point
    uint32_t blend_index_current = 0;
    // speed (and direction) to rotate the circle
    uint16_t circleSpeed = SEGMENT.intensity - 128; // == 0 ? 0 : 256-SEGMENT.intensity;
    // current position in the circle
    int32_t circle_offset_current = 0;
    // if it's an even ball and toggle direction is activated
    bool isToggledDirection = false;

    int32_t nowUp = millis(); // we can't use strip.now, as this is millis() at start of the effect offset by an unknown timebase, whereas strip.getLastShow() is millis() at the end(!) of last show(), without offset
    int32_t nowDiff = nowUp - strip.getLastShow();

    double desiredDuration_s = -1.0;
    double inc = 0;
    uint32_t inc2 = 0;
    if (speed != 0) {
//      double k = powf(10, 1.0/64); // 64 values per decade, ~1...~9600
//      double desiredDuration_s = powf(k, 256-speed); // desired duration -> about 1s to 2h40m speed slider range
      desiredDuration_s = powf(10, (256-speed)/52.0) / 10; // desired duration -> about 0.1s to 2h13 speed slider range
      inc = (blendTable.totalLen << 16) / desiredDuration_s / 1000; // inc per millisecond
      inc2 = static_cast<uint32_t>(inc * nowDiff);
      SEGMENT.step = (SEGMENT.step + inc2) % (blendTable.totalLen << 16);
    }

    uint16_t virtualLength = 0;
    if (SEGMENT.custom2 > 0) {
      virtualLength = powf(10, SEGMENT.custom2/52.0); // full length of BlendTable would need this many LEDs (magic value 52 to get a reasonable slider range)
      double inc = (blendTable.totalLen << 16) / virtualLength; // inc per LED
      if (SEGMENT.check3) inc = (blendTable.totalLen << 16) - inc; // reverse direction
      px_offset_fp = static_cast<uint32_t>(inc);
    } else {
      px_offset_fp = 0;
    }

    if (circleSpeed != 0) {
      // circleSpeed multiplicator is somewhat arbitrary to get a good effect in a reasonable speed slider range
      SEGMENT.aux0 = (SEGMENT.aux0 + (DANCER_BALL_LENGTH << 10) + (nowDiff * circleSpeed)) % (DANCER_BALL_LENGTH << 10);
    }
    uint16_t circle_time_offset = SEGMENT.aux0 >> 10;

    
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

        blend_index_current_fp = SEGMENT.step + (color_offset << 16);

        circle_offset_current = DANCER_CIRCLE_OFFSET[ballIdx % DANCER_MAX_BALLS];
        
        isToggledDirection = toggleCircleDirection && (ballIdx & 1);
        if (isToggledDirection) { // even ball
          circle_offset_current += circle_time_offset;
        } else { // odd ball
          circle_offset_current += DANCER_BALL_LENGTH - circle_time_offset;
        }
        curPxDimm = SEGMENT.currentBri();
        if (DANCER_DEBUG_OUTPUT && (strip.now % 1000 < FRAMETIME) && ballIdx == 0) {
          Serial.print("ballIdx=" + String(ballIdx) + ", blend_index_current=" + String(blend_index_current_fp >> 16) +
          ", circleSpeed=" + String(circleSpeed) + ", circle_offset=" + String(circle_offset_current) + ", speed=" + String(speed) + ", desiredDuration_s=" + String(desiredDuration_s) + ", virtualLength=" + String(virtualLength) +
          ", curPxDimm=" + String(curPxDimm) + ", px_dimm=" + String(px_dimm) + ", pxIdx=" + String(pxIdx) + ", idxInBall=" + String(idxInBall) + 
          ", ballStartIdx=" + String(ballStartIdx) + ", nowDiff=" + nowDiff + ", inc=" + String(inc) + ", inc2=" + String(inc2) + ", step=");
          Serial.println(SEGMENT.step, HEX);
        }
      }
      if (idxInBall >= px_count) {
        // skip this pixel, as it's already set by the mirrored pixel
        continue;
      }

      blend_index_current = blend_index_current_fp >> 16;
    
      // limit ball color offset to total range of color-blend table
      while (blend_index_current >= blendTable.totalLen) { blend_index_current -= blendTable.totalLen; }
      while (blend_index_current < 0) { blend_index_current += blendTable.totalLen ; }
      
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
        uint16_t pos = blend_index_current - lensum;

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
        if (((circleSpeed >= 0) != isToggledDirection) || mirror) { // there's no XOR in C++, so we use != on two booleans instead
          uint16_t dst_idx = ballStartIdx + ((circle_offset_current + idxInBall) % DANCER_BALL_LENGTH);
          if (dst_idx < SEGMENT.length()) {
            SEGMENT.setPixelColor(dst_idx, c);
          }
        }
    
        if (((circleSpeed < 0) != isToggledDirection) || mirror) {
          // set mirrored pixel
          uint16_t dst_idx2 = ballStartIdx + ((DANCER_BALL_LENGTH + circle_offset_current - idxInBall) % DANCER_BALL_LENGTH);
          if (dst_idx2 < SEGMENT.length()) {
            SEGMENT.setPixelColor(dst_idx2, c);
          }
        }

        // calculate per-led (pixel) offset inside this segment for next loop iteration
        blend_index_current_fp = (blend_index_current_fp + px_offset_fp) % (blendTable.totalLen << 16);
        curPxDimm = (curPxDimm * px_dimm) >> 8;
    }

    return FRAMETIME;
  }

  static uint8_t lastX;

  /** This effect helps to find the "circle offset". Change the offset (speed) slider until the red dot (color1) is at the top.
   *  Green (color2) marks the currently configured (saved) position, blue (color3) is the physical first pixel (origin) of this ball. */
  static uint16_t fxDancerCircleOffsetHelper() {
    uint8_t offset = (SEGMENT.speed/ 10) % DANCER_BALL_LENGTH;
    if (DANCER_DEBUG_OUTPUT && SEGMENT.speed != lastX) {
      Serial.println("Offset: " + String(offset) + " (" + String(SEGMENT.speed) + ")");
      lastX = SEGMENT.speed;
    }
    for(uint16_t pxIdx = 0; pxIdx < SEGMENT.length(); pxIdx++) {
      uint32_t c = BLACK;
      uint16_t idxInBall = pxIdx % DANCER_BALL_LENGTH;
      uint16_t ballIdx = (pxIdx / DANCER_BALL_LENGTH) % DANCER_MAX_BALLS;
      if (idxInBall == 0 && SEGMENT.check3) {
        c = color_add(c, SEGMENT.colors[2], true);
      }
      if ((DANCER_CIRCLE_OFFSET[ballIdx] % DANCER_BALL_LENGTH == idxInBall) && SEGMENT.check2) {
        c = color_add(c, SEGMENT.colors[1], true);
      }      
      if ((offset % DANCER_BALL_LENGTH == idxInBall) && SEGMENT.check1) {
        c = color_add(c, SEGMENT.colors[0], true);
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

