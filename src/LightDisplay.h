#ifndef __LIGHT_DISPLAY_H__
#define __LIGHT_DISPLAY_H__
#include <GxEPD2_3C.h>

class LightDisplay
{
    const uint16_t UPDATE_INTERVAL = 60000 / 2; // 30 seconds

public:
    LightDisplay(uint8_t chipSelectPin) : display(GxEPD2_213_Z19c(/*CS=D8*/ chipSelectPin, /*DC=D3*/ 9, /*RST=D4*/ 8, /*BUSY=D2*/ 7)),
                     lastUpdate(0),
                     displayValueCapture(nullptr)
    {
    }

    struct DisplayValues
    {
        String timeString;
        String lightString;
        bool wifiConnected;
    };
    using GetDisplayValues = void (*)(DisplayValues *values);

    void begin(GetDisplayValues captureFunction)
    {
        Serial.println("Initializing tri-color e-Paper...");
        display.init(1152000); // you can try a slower SPI speed if needed
        display.setRotation(1);
        display.setFont(&FreeMonoBold9pt7b);
        displayValueCapture = captureFunction;
    }

    void update(bool updateNow = false)
    {
        // LightDisplay removed: no display code needed for this project