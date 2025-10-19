#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

struct SensorSettings {
    String sensorName;
    int updateInterval;
    String wifiSSID;
    String wifiPassword;
    bool wifiEnabled;
};

class SettingsManager {
public:
    SettingsManager()
    {
        // Set default settings
        settings.sensorName = "PhotonIQSensor";
        settings.updateInterval = 60; // Default to 60 seconds
        settings.wifiEnabled = false;
        settings.wifiSSID = "";
        settings.wifiPassword = "";
    }

    void begin()
    {
        preferences.begin("sensorSettings", false);
    }

    void end()
    {
        preferences.end();
    }

    void loadSettings()
    {
        settings.sensorName = preferences.getString("sensorName", "PhotonIQSensor");
        settings.updateInterval = preferences.getInt("updateInterval", 60);
        settings.wifiEnabled = preferences.getBool("wifiEnabled", true);
    }

    void saveSettings()
    {
        preferences.putString("sensorName", settings.sensorName);
        preferences.putInt("updateInterval", settings.updateInterval);
        preferences.putBool("wifiEnabled", settings.wifiEnabled);
    }

    void setSensorName(const String &name)
    {
        settings.sensorName = name;
        saveSettings();
    }

    void setScanInterval(int interval)
    {
        settings.updateInterval = interval;
        saveSettings();
    }

    void setWiFiCredentials(const String &ssid, const String &password)
    {
        // For simplicity, we just enable or disable Wi-Fi based on whether SSID is empty
        settings.wifiSSID = ssid;
        settings.wifiPassword = password;
        saveSettings();
    }

    void setWifiEnabled(bool enabled)
    {
        settings.wifiEnabled = enabled;
        saveSettings();
    }

    SensorSettings getSettings()
    {
        return settings;
    }

private:
    Preferences preferences;
    SensorSettings settings;
};

#endif // SETTINGS_H