/*
 *  This creates sound tones by outputting a square wave on a DAC pin. The
 *  volume of the tone is the level of the DAC pin.
 *  
 *  The square wave is created by a timer. The timer runs at 2x the freq, because
 *  it needs to transition high and then low.
 * 
 *   
 */
#ifndef SOUND_H
  #define SOUND_H

#include "esp32-hal-timer.h";

//#define AUDIO_PIN 25
#define MIN_FREQ 20
#define MAX_FREQ 16000

hw_timer_t * sndTimer = NULL;

volatile bool sound_on = true;
volatile bool sound_wave_high = true;  // this toggles to create the high/low transitions of the wave
volatile uint8_t sound_volume = 0;
// in ESP 2.x this used to be a prescaler of 80 on 80Mhz (ESP32_F_CPU/AUDIO_INTERRUPT_PRESCALER), now it's a frequency of 1MHz in timerBegin()
#define TIMER_TICKS 1000000

void sound_init(int pin);
bool sound(uint16_t freq, uint8_t volume);
void soundOff();

int dac_pin;



void IRAM_ATTR onSoundTimer() 
{    
  if (sound_on) {    
			dacWrite(dac_pin, (sound_wave_high?sound_volume:0));
      sound_wave_high = ! sound_wave_high;
   
  }
  else
    dacWrite(dac_pin, 0);
}

void sound_init(int pin){  // pin must be a DAC pin number !! (typically 25 or 26)
  dac_pin = pin;
	sound_on = false;
	pinMode(dac_pin, OUTPUT);
	sound_volume = 0;
	
	sndTimer = timerBegin(1000000);
  timerAttachInterrupt(sndTimer, &onSoundTimer);          
  timerAlarm(sndTimer, TIMER_TICKS/MIN_FREQ, true, 0); // lower timer freq
}

void sound_pause() // this prevents the interrupt from firing ... use during eeprom write
{
	if (sndTimer != NULL)
		timerStop(sndTimer);	
}

void sound_resume() // resume from pause ... after eeprom write
{
	if (sndTimer != NULL) {
// 2024-08-29 Zefiro: seems to not work. Instead do a full re-init --- still doesn't work :(
    timerStop(sndTimer);
    timerRestart(sndTimer);
    sound_init(dac_pin);
	}
}

bool sound(uint16_t freq, uint8_t volume){
	if (volume == 0) {
		soundOff();
		return false;
	}
	if (freq < MIN_FREQ || freq > MAX_FREQ) {
		return false;
	}
	sound_on = true;
	sound_volume = volume;
  timerAlarm(sndTimer, TIMER_TICKS/(freq * 2), true, 0);
	return true;	
}

void soundOff(){  
 sound_on = false;
 sound_volume = 0;
 timerAlarm(sndTimer, TIMER_TICKS/(MIN_FREQ), true, 0);  // lower timer freq
}



#endif
