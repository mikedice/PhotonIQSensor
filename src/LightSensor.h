#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

class LightSensor {
public:
    LightSensor() : tsl(2591) {}

    bool begin() {
        if (tsl.begin()) {
            Serial.println("TSL2591 sensor found.");
            tsl.setGain(TSL2591_GAIN_MED); // Options: LOW, MED, HIGH, MAX
            tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
            return true;
        } else {
            Serial.println("Could not find TSL2591. Check wiring.");
            return false;
        }
    }

    void printLightLevel() {
        sensors_event_t event;
        tsl.getEvent(&event);
        if (event.light) {
            Serial.print("Light: ");
            Serial.print(event.light);
            Serial.println(" lux");
        } 
        else 
        {
            Serial.println("Sensor overload or no signal.");
        }
    }

    String getLightAsString() {
        sensors_event_t event;
        tsl.getEvent(&event);

        if (event.light) {
            return String(event.light, 2) + " lux"; // two decimal places
        } 
        else 
        {
            return "-- lux";
        }
    }

private:
    Adafruit_TSL2591 tsl; ///< TSL2591 light sensor instance
};

#endif // __LIGHT_SENSOR_H__