#ifndef _FILE_LOGGER_H_
#define _FILE_LOGGER_H_
#include <SD.h>

class FileLogger
{
    
private:
    uint8_t csPin; // Chip Select pin for the SD card

public:
    FileLogger(uint8_t csPin) {
        this->csPin = csPin;
    }

    void begin()
    {
        if (!SD.begin(csPin)) // Assuming CS pin is 4
        {
            Serial.println("SD card initialization failed!");
            return;
        }
        Serial.println("SD card initialized successfully.");
    }
};

#endif // _FILE_LOGGER_H_