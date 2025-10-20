#ifndef BLE_LIGHT_SENSOR_SERVICE_H
#define BLE_LIGHT_SENSOR_SERVICE_H
#include <ArduinoBLE.h>
class BleLightSensorService;
BleLightSensorService* globalInstance = nullptr;
#include "Settings.h"

class BleLightSensorService
{
    BLEService lightLevelService;
    BLEStringCharacteristic lightLevelCharacteristic;
    BLEService wifiService;
    BLEStringCharacteristic wifiSSIDsCharacteristic;
    BLEBooleanCharacteristic wifiSSIDScanCommandCharacteristic;
    BLEService settingsService;
    BLEStringCharacteristic sensorNameCharacteristic;
    BLEIntCharacteristic scanIntervalCharacteristic;
    BLEStringCharacteristic wifiSSIDCharacteristic;
    BLEStringCharacteristic wifiPasswordCharacteristic;
    BLEBoolCharacteristic wifiEnabledCharacteristic;

public:
    BleLightSensorService()
        : lightLevelService("3d80c0aa-56b9-458f-82a1-12ce0310e076"),                                     // UUID for the Light Sensor Service
          lightLevelCharacteristic("646bd4e2-0927-45ac-bf41-fd9c69aa31dd", BLERead | BLENotify, 20), // UUID for the Light Characteristic
          wifiService("458800E6-FC10-46BD-8CDA-7F0F74BB1DBF"),                                   // UUID for the Wi-Fi Service
          wifiSSIDsCharacteristic("B30041A1-23DF-473A-AEEC-0C8514514B03", BLERead | BLENotify, 100),
          wifiSSIDScanCommandCharacteristic("5F8B1E42-1A56-4B5A-8026-8B15BC7EE5F3",BLEWrite),
          settingsService("C1D5A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F"),                                   // UUID for the Settings Service
          sensorNameCharacteristic("D2C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F", BLERead | BLEWrite | BLENotify, 100),      // UUID for the Sensor Name Characteristic
          scanIntervalCharacteristic("E3F4B5C6-8D9E-4F0A-B1C2-D3E4F5A6B7C8", BLERead | BLEWrite | BLENotify),
          wifiSSIDCharacteristic("B2C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F", BLERead | BLEWrite | BLENotify, 100),
          wifiPasswordCharacteristic("C2C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F", BLERead | BLEWrite | BLENotify, 100),
          wifiEnabledCharacteristic("D3C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F", BLERead | BLEWrite | BLENotify)
    {
    }

    static void blePeripheralConnectHandler(BLEDevice central)
    {
        // central connected event handler
        Serial.print("Connected event, central: ");
        Serial.println(central.address());
    }

    static void blePeripheralDisconnectHandler(BLEDevice central)
    {
        // central disconnected event handler
        Serial.print("Disconnected event, central: ");
        Serial.println(central.address());
    }

    static void wifiScanCommandHandler(BLEDevice central, BLECharacteristic characteristic)
    {
        // This callback is invoked when the central device writes to the characteristic
        bool scan = characteristic.value();
        Serial.println("Received command to scan Wi-Fi SSIDs: ");

        if (scan) {
            Serial.println("Start scanning for Wi-Fi SSIDs...");
            // Perform Wi-Fi scan and update the wifiSSIDsCharacteristic
            int n = WiFi.scanNetworks();
            Serial.println("Scan complete.");
            
            String ssidList = "";
            for (int i = 0; i < n; ++i) {
                if (i > 0) ssidList += ", ";
                ssidList += WiFi.SSID(i);
                BLE.poll(); // Keep BLE stack running during the scan
            }
            globalInstance->wifiSSIDsCharacteristic.writeValue(ssidList);
            Serial.print("Found SSIDs: ");
            Serial.println(ssidList);
            characteristic.writeValue(false); // Reset command characteristic
        }
    }

    static void sensorNameWriteHandler(BLEDevice central, BLECharacteristic characteristic)
    {
        BLEStringCharacteristic& stringValue = static_cast<BLEStringCharacteristic&>(characteristic);
        String newName = stringValue.value();
        Serial.print("Received new sensor name: ");
        Serial.println(newName);
        globalInstance->sensorNameCharacteristic.writeValue(newName);
        globalInstance->saveSensorName(newName);
    }

    static void scanIntervalWriteHandler(BLEDevice central, BLECharacteristic characteristic)
    {
        BLEIntCharacteristic& intValue = static_cast<BLEIntCharacteristic&>(characteristic);
        int newInterval = intValue.value();
        Serial.print("Received new scan interval: ");
        Serial.println(newInterval);
        globalInstance->scanIntervalCharacteristic.writeValue(newInterval);
        globalInstance->saveScanInterval(newInterval);
    }

    static void wifiSSIDWriteHandler(BLEDevice central, BLECharacteristic characteristic)
    {
        BLEStringCharacteristic& stringValue = static_cast<BLEStringCharacteristic&>(characteristic);
        String newSSID = stringValue.value();
        Serial.print("Received new Wi-Fi SSID: ");
        Serial.println(newSSID);
        globalInstance->wifiSSIDCharacteristic.writeValue(newSSID);
        globalInstance->saveWiFiCredentials(newSSID, globalInstance->wifiPasswordCharacteristic.value());
    }

    static void wifiPasswordWriteHandler(BLEDevice central, BLECharacteristic characteristic)
    {
        BLEStringCharacteristic& stringValue = static_cast<BLEStringCharacteristic&>(characteristic);
        String newPassword = stringValue.value();
        Serial.print("Received new Wi-Fi Password: ");
        Serial.println(newPassword);
        globalInstance->wifiPasswordCharacteristic.writeValue(newPassword);
        globalInstance->saveWiFiCredentials(globalInstance->wifiSSIDCharacteristic.value(), newPassword);
    }

    static void wifiEnabledWriteHandler(BLEDevice central, BLECharacteristic characteristic)
    {
        BLEBoolCharacteristic& boolValue = static_cast<BLEBoolCharacteristic&>(characteristic);
        bool enabled = boolValue.value();
        Serial.print("Received Wi-Fi Enabled state: ");
        Serial.println(enabled ? "Enabled" : "Disabled");
        globalInstance->wifiEnabledCharacteristic.writeValue(enabled);
        SettingsManager settingsManager;
        settingsManager.begin();
        settingsManager.loadSettings();
        settingsManager.setWifiEnabled(enabled);
        settingsManager.end();
        Serial.println("Wi-Fi enabled state saved to settings.");
    }

    void saveWiFiCredentials(const String &ssid, const String &password)
    {
        SettingsManager settingsManager;
        settingsManager.begin();
        settingsManager.loadSettings();
        // Here you would add code to save the SSID and password to your settings structure
        // For example:
        settingsManager.setWiFiCredentials(ssid, password);
        settingsManager.end();
        Serial.println("Wi-Fi credentials saved to settings.");
    }

    void saveScanInterval(int interval)
    {
        SettingsManager settingsManager;
        settingsManager.begin();
        settingsManager.loadSettings();
        // Here you would add code to save the scan interval to your settings structure
        // For example:
        settingsManager.setScanInterval(interval);
        settingsManager.end();
        Serial.println("Scan interval saved to settings.");
    }

    void saveSensorName(const String &name)
    {
        SettingsManager settingsManager;
        settingsManager.begin();
        settingsManager.loadSettings();
        settingsManager.setSensorName(name);
        settingsManager.end();
        Serial.println("Sensor name saved to settings.");
    }

    void begin()
    {
        globalInstance = this;
        if (!BLE.begin())
        {
            Serial.println("starting BLE failed!");
            while (1)
                ;
        }
        SettingsManager settingsManager;
        settingsManager.begin();
        settingsManager.loadSettings();
        SensorSettings currentSettings = settingsManager.getSettings();
        settingsManager.end();

        BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
        BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

        BLE.setLocalName("LightSensor");


        
        lightLevelService.addCharacteristic(lightLevelCharacteristic);
        lightLevelCharacteristic.writeValue("-1"); // Initial value

        wifiService.addCharacteristic(wifiSSIDsCharacteristic);
        wifiService.addCharacteristic(wifiSSIDScanCommandCharacteristic);
        wifiSSIDScanCommandCharacteristic.setEventHandler(BLEWritten, wifiScanCommandHandler);
        wifiSSIDsCharacteristic.writeValue(""); // Initial value
        wifiSSIDScanCommandCharacteristic.writeValue(false); // Initial value

        settingsService.addCharacteristic(sensorNameCharacteristic);
        sensorNameCharacteristic.writeValue(currentSettings.sensorName); // Initial value

        BLE.addService(wifiService);
        BLE.addService(lightLevelService);
        BLE.addService(settingsService);
        BLE.setAdvertisedService(lightLevelService);
        BLE.setAdvertisedService(wifiService);
        BLE.setAdvertisedService(settingsService);

        
        BLE.advertise();
        Serial.println("BLE Light Sensor Service started");
    }

    void updateLightValue(const String &value)
    {
        if (lightLevelCharacteristic.subscribed())
        {
            Serial.print("Updating light value: ");
            Serial.println(value);
        }
        else
        {
            Serial.println("No subscribers for light characteristic, not updating.");
            return;
        }
        lightLevelCharacteristic.writeValue(value);
    }
};
#endif // BLE_LIGHT_SENSOR_SERVICE_H