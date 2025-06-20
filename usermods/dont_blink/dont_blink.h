#pragma once

// TODO Set strip brightness to zero -> this should trigger a relay output (if configured)
// TODO change fading speed
// TODO find out why the config page doesn't work

#define DONTBLINK_DEBUG_OUTPUT true
#include "wled.h"
#include "time.h"

class DontBlink : public Usermod
{
public:
    bool blinkingEnabled = true;
    bool doBlink = false;
    unsigned long blinkStart = 0;
    bool initDone = false;

    void setup()
    {
        if (DONTBLINK_DEBUG_OUTPUT)
            Serial.println("Don't Blink mod setup()!");
        initDone = true;
    }

    void addToJsonState(JsonObject &root) override
    {
        if (!initDone || !blinkingEnabled)
            return; // prevent crash on boot applyPreset()

        JsonObject usermod = root[FPSTR(_name)];
        if (usermod.isNull())
            usermod = root.createNestedObject(FPSTR(_name));

        usermod[FPSTR(_nameConfig)] = blinkingEnabled;
    }

    void readFromJsonState(JsonObject &root) override
    {
        if (!initDone)
            return; // prevent crash on boot applyPreset()

        JsonObject usermod = root[FPSTR(_name)];
        if (!usermod.isNull())
        {
            blinkingEnabled = usermod[FPSTR(_nameConfig)] | blinkingEnabled;
        }
    }

    bool readFromConfig(JsonObject &root) override
    {
        JsonObject top = root[FPSTR(_name)];
        bool configComplete = !top.isNull();

        configComplete &= getJsonValue(top[FPSTR(_nameConfig)], blinkingEnabled, true);
        if (DONTBLINK_DEBUG_OUTPUT)
        {
            Serial.print("DontBlink readFromConfig: ");
            Serial.println(blinkingEnabled ? "true" : "false");
        }
        return configComplete;
    }

    void addToConfig(JsonObject &root) override
    {
        JsonObject top = root.createNestedObject(FPSTR(_name));
        top[FPSTR(_nameConfig)] = blinkingEnabled;
    }

    void appendConfigData() override
    {
        oappend(SET_F("addField('dont_blink','blinkingEnabled',"));
        oappend(blinkingEnabled ? SET_F("true") : SET_F("false"));
        oappend(SET_F(");"));
    }

    void loop()
    {
        // Not needed unless your effect needs background processing
        if (initDone && blinkingEnabled && doBlink) {
            // TODO this is now called twice, not good - however if the strip is off, the other method seems to not be called anymore, leading to a dead program
            handleBlinking(millis() - blinkStart);
        }
    }

    static uint8_t lastTriggeredHour;

    void handleOverlayDraw() override
    {
        if (!blinkingEnabled)
            return;
        if (doBlink)
        {
            unsigned long elapsed = millis() - blinkStart;
            if (elapsed < 5000)
            {
                handleBlinking(elapsed);
            }
            else
            {
                doBlink = false;
            }
        }
        else
        {
            if (second(localTime) % 15 == 0 /* minute(localTime)*0 == 0 && hour(localTime) != lastTriggeredHour */)
            {
                doBlink = true;
                blinkStart = millis();
                lastTriggeredHour = hour(localTime);
                origBri = strip.getBrightness();
            }
        }
    }

private:
    uint8_t origBri;
    static const char _name[];
    static const char _nameConfig[];

    void handleBlinking(unsigned long elapsed)
    {
        float brightness = 0.0f;
        if (elapsed < 1500)
        {
            brightness = 1.0f - (elapsed / 1500.0f); // fade out
        }
        else if (elapsed > 3500)
        {
            brightness = (elapsed - 3500) / 1500.0f; // fade in
        }
        // uint16_t len = strip.getLengthTotal();
        // for (uint16_t i = 0; i < len; i++)
        // {
        //     CRGB color = strip.getPixelColor(i);
        //     color.nscale8_video((uint8_t)(brightness * 255));
        //     strip.setPixelColor(i, color);
        // }
        strip.setBrightness(origBri * brightness);
    }
};
uint8_t DontBlink::lastTriggeredHour = -1;
const char DontBlink::_name[] PROGMEM = "dont_blink";
const char DontBlink::_nameConfig[] PROGMEM = "blinkingEnabled";
