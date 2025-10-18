#ifndef __REALTIME_CLOCK_H__
#define __REALTIME_CLOCK_H__
#include <Arduino.h>
#include <RTClib.h>
#include <NTPClient.h>

#include "WifiNetwork.h"
#include <WiFiUdp.h>

class RealtimeClock {
public:
    RealtimeClock(WifiNetwork &wifiNetwork_) : rtc(RTC_DS3231()), wifiNetwork(wifiNetwork_) {}

    bool begin() {
        bool result = rtc.begin();
        if (!result) {
            Serial.println("RealtimeClock initialization failed!");
        } 
        else {
            Serial.println("RealtimeClock initialized successfully.");
            adjustClockIfPowerLost();
        }
        return result;
    }

    bool lostPower() {
        return rtc.lostPower();
    }

    void adjust(const DateTime& dt) {
        rtc.adjust(dt);
    }

    DateTime now() {
        return rtc.now();
    }
    String timestamp(DateTime::timestampOpt opt = DateTime::TIMESTAMP_FULL) {
        return rtc.now().timestamp(opt);
    }
    
    String nowAs12HourString() {
        DateTime now = rtc.now();
        int hour12 = now.hour() % 12;
        if (hour12 == 0) hour12 = 12; // handle midnight/noon as 12
        String meridian = (now.hour() < 12) ? "AM" : "PM";

        // Pad minutes with leading zero if needed
        String minuteStr = (now.minute() < 10) ? "0" + String(now.minute()) : String(now.minute());

        return String(hour12) + ":" + minuteStr + " " + meridian;
    }


private:
    void adjustClockIfPowerLost() {
        if (lostPower()) {
            Serial.println("RTC lost power, adjusting time from NTP...");
            if (wifiNetwork.isConnected()) {
                // Use NTP to get the current time
                WiFiUDP ntpUDP;
                NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
                timeClient.begin();
                if (timeClient.forceUpdate()) {
                    DateTime ntpTime = DateTime(timeClient.getEpochTime());
                    adjust(ntpTime);
                    Serial.print("RTC adjusted to: ");
                    Serial.println(ntpTime.timestamp());
                } else {
                    Serial.println("Failed to update RTC from NTP.");
                }
            } else {
                Serial.println("Wi-Fi not connected, cannot adjust RTC.");
            }
        }
    }
    RTC_DS3231 rtc;
    WifiNetwork &wifiNetwork;

};

#endif // __REALTIME_CLOCK_H__