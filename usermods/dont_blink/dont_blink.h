#pragma once

// WLED main loop calls button.cpp's handleIO() which switches off the relay (if configured) when brightness is zero

#define DONTBLINK_DEBUG_OUTPUT true
#include "wled.h"
#include "time.h"
#include "ESP8266WiFi.h"

#define DONT_BLINK_FADE_DURATION_MS 1500
#define DONT_BLINK_OFF_DURATION_MS 2000

class DontBlink : public Usermod
{
public:
    bool blinkingEnabled = true;
    int blinkingPeriod = 60;
    bool doBlink = false;
    unsigned long blinkStart = 0;
    bool initDone = false;

    void setup()
    {
        if (DONTBLINK_DEBUG_OUTPUT)
            Serial.println("Don't Blink mod setup()!");
        initDone = true;
    }

    void addToConfig(JsonObject &root) override
    {
        JsonObject top = root.createNestedObject(FPSTR(_name));
        top[FPSTR(_nameConfigEnabled)] = blinkingEnabled;
        top[FPSTR(_nameConfigPeriod)] = blinkingPeriod;
    }

    bool readFromConfig(JsonObject &root) override
    {
        JsonObject top = root[FPSTR(_name)];
        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top[FPSTR(_nameConfigEnabled)], blinkingEnabled, true);
        configComplete &= getJsonValue(top[FPSTR(_nameConfigPeriod)], blinkingPeriod, 42);
        if (DONTBLINK_DEBUG_OUTPUT)
        {
            Serial.print("DontBlink enabled: " + String(blinkingEnabled ? "true" : "false") + ", Period: " + String(blinkingPeriod) + " min\n");
            if (year(localTime) <= 2000) {
                Serial.print("Current time is not known yet. ");
            }
            Serial.printf("Time: %04d-%02d-%02d %02d:%02d:%02d\n",
                            year(localTime), month(localTime), day(localTime),
                            hour(localTime), minute(localTime), second(localTime));
            if (WiFi.isConnected()) {
                Serial.printf("Wifi is connected at: %s\n", WiFi.localIP().toString().c_str());
                WiFi.printDiag(Serial1);
            }
            if (!WiFi.isConnected()) {
                Serial.println("Wifi is disconnected.");
            }
        }
        return configComplete;
    }
/* Seems we don't need this at all (Analog_Clock_h didn't have those), unsure what they do and why I had it in the first place

    void addToJsonState(JsonObject &root) override
    {
        if (!initDone || !blinkingEnabled)
            return; // prevent crash on boot applyPreset()

        JsonObject top = root[FPSTR(_name)];
        if (top.isNull())
            top = root.createNestedObject(FPSTR(_name));

        top[FPSTR(_nameConfigEnabled)] = blinkingEnabled;
        top[FPSTR(_nameConfigPeriod)] = blinkingPeriod;
        top[FPSTR("Test_A")] = 42;
    }

    void readFromJsonState(JsonObject &root) override
    {
        if (!initDone)
            return; // prevent crash on boot applyPreset()

        JsonObject top = root[FPSTR(_name)];
        if (!top.isNull())
        {
            blinkingEnabled = top[FPSTR(_nameConfigEnabled)] | blinkingEnabled;
            blinkingPeriod = top[FPSTR(_nameConfigPeriod)];
        }
    }
*/
    void appendConfigData() override
    {
/* This seems to be unecessary and not do anything anyway
        oappend(SET_F("addField('dont_blink','TEST_C',"));
        oappend(blinkingEnabled ? SET_F("true") : SET_F("false"));
        oappend(SET_F(");"));
//*/
    }

    bool wasConnected = false;

    void loop()
    {
        if (initDone && blinkingEnabled && doBlink) {
            // TODO this is now called twice (in this loop and in handleOverlayDraw), not good - however if the strip is off, the other method seems to not be called anymore, preventing us to come out of blinking
            // can't remember why it's not enough to only do it here, not in handleOverlayDraw()? perhaps ordering issue?
            handleBlinking(millis() - blinkStart);
        }

        if (WiFi.isConnected() && !wasConnected) {
            Serial.printf("Connected to Wifi. IP address: %s\n", WiFi.localIP().toString().c_str());
            WiFi.printDiag(Serial1);
            wasConnected = true;
        }
        if (!WiFi.isConnected() && wasConnected) {
            Serial.println("Wifi disconnected.");
            wasConnected = false;
        }
    }

    static uint16_t lastTriggeredMinOfDay;

    void handleOverlayDraw() override
    {
        if (!blinkingEnabled)
            return;
        if (doBlink)
        {
            unsigned long elapsed = millis() - blinkStart;
            doBlink = handleBlinking(elapsed);
        }
        else
        {
            uint16_t minOfDay = minute(localTime) + hour(localTime) * 60;
            bool isBlinkTime = (blinkingPeriod > 0) && (minOfDay % blinkingPeriod == 0);
            if (isBlinkTime && minOfDay != lastTriggeredMinOfDay)
            {
                origBri = strip.getBrightness();
                doBlink = true;
                blinkStart = millis();
                Serial.println("Blinking every " + String(blinkingPeriod) + " min: triggered at " + String(hour(localTime)) + ":" + String(minute(localTime)) + ", last was " + String(lastTriggeredMinOfDay) + ", origBri=" + String(origBri));
                lastTriggeredMinOfDay = minOfDay;
            }
        }
    }

#define DONT_BLINK_FADE_DURATION_MS 1500
#define DONT_BLINK_OFF_DURATION_MS 2000
private:
    uint8_t origBri;
    static const char _name[];
    static const char _nameConfigEnabled[];
    static const char _nameConfigPeriod[];

    bool handleBlinking(unsigned long elapsed)
    {
        bool continueBlinking = true;
        float brightness = 0.0f;
        if (elapsed < DONT_BLINK_FADE_DURATION_MS)
        {
            brightness = 1.0f - (float(elapsed) / DONT_BLINK_FADE_DURATION_MS); // fade out
        }
        else if (elapsed < DONT_BLINK_FADE_DURATION_MS + DONT_BLINK_OFF_DURATION_MS)
        {
            brightness = 0.0f; // implicitely switches off the relay (if configured)
        }
        else if (elapsed < DONT_BLINK_FADE_DURATION_MS + DONT_BLINK_OFF_DURATION_MS + DONT_BLINK_FADE_DURATION_MS)
        {
            brightness = float(elapsed - (DONT_BLINK_FADE_DURATION_MS + DONT_BLINK_OFF_DURATION_MS)) / DONT_BLINK_FADE_DURATION_MS; // fade in
        }
        else
        {
            brightness = 1.0f;
            continueBlinking = false;
        }
        strip.setBrightness(origBri * brightness);
        return continueBlinking;
    }
};
uint16_t DontBlink::lastTriggeredMinOfDay = 0;
const char DontBlink::_name[] PROGMEM = "Don't Blink";
const char DontBlink::_nameConfigEnabled[] PROGMEM = "Blinking enabled";
const char DontBlink::_nameConfigPeriod[] PROGMEM = "Blinking Period (min)";
