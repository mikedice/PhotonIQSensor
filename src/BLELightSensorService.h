#ifndef BLE_LIGHT_SENSOR_SERVICE_H
#define BLE_LIGHT_SENSOR_SERVICE_H

#include <NimBLEDevice.h>
#include "Settings.h"
// This class migrates the original ArduinoBLE-based implementation to NimBLE-Arduino.
// Key differences:
//  - Uses NimBLEServer/NimBLEService/NimBLECharacteristic.
//  - Characteristic callbacks implemented via NimBLECharacteristicCallbacks subclasses.
//  - Connection callbacks via NimBLEServerCallbacks.
//  - Notify performed explicitly after setting new light level value.

class BleLightSensorService;
static BleLightSensorService* gBleInstance = nullptr; // for static-style access inside callbacks.

class BleLightSensorService : public NimBLEServerCallbacks {
public:
    // UUID constants (same values as previous implementation to maintain compatibility)
    static constexpr const char* UUID_LIGHT_SERVICE              = "3d80c0aa-56b9-458f-82a1-12ce0310e076";
    static constexpr const char* UUID_LIGHT_CHARACTERISTIC       = "646bd4e2-0927-45ac-bf41-fd9c69aa31dd";
    static constexpr const char* UUID_WIFI_SERVICE               = "458800E6-FC10-46BD-8CDA-7F0F74BB1DBF";
    static constexpr const char* UUID_WIFI_SSIDS_CHAR            = "B30041A1-23DF-473A-AEEC-0C8514514B03";
    static constexpr const char* UUID_WIFI_SCAN_CMD_CHAR         = "5F8B1E42-1A56-4B5A-8026-8B15BC7EE5F3";
    static constexpr const char* UUID_SETTINGS_SERVICE           = "C1D5A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F";
    static constexpr const char* UUID_SENSOR_NAME_CHAR           = "D2C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F";
    static constexpr const char* UUID_SCAN_INTERVAL_CHAR         = "E3F4B5C6-8D9E-4F0A-B1C2-D3E4F5A6B7C8";
    static constexpr const char* UUID_WIFI_SSID_CHAR             = "B2C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F";
    static constexpr const char* UUID_WIFI_PASSWORD_CHAR         = "C2C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F";
    static constexpr const char* UUID_WIFI_ENABLED_CHAR          = "D3C1A3B2-7E2F-4F4C-9F1D-3A2B1C0D4E5F";

private:
    NimBLEServer*        pServer             = nullptr;
    NimBLEService*       pLightService       = nullptr;
    NimBLEService*       pWifiService        = nullptr;
    NimBLEService*       pSettingsService    = nullptr;

    NimBLECharacteristic* pLightLevelChar        = nullptr;
    NimBLECharacteristic* pWifiSSIDsChar         = nullptr;
    NimBLECharacteristic* pWifiScanCmdChar       = nullptr;
    NimBLECharacteristic* pSensorNameChar        = nullptr;
    NimBLECharacteristic* pScanIntervalChar      = nullptr;
    NimBLECharacteristic* pWifiSSIDChar          = nullptr;
    NimBLECharacteristic* pWifiPasswordChar      = nullptr;
    NimBLECharacteristic* pWifiEnabledChar       = nullptr;

    // --- Callback classes ---
    class GenericWriteCallback : public NimBLECharacteristicCallbacks {
        void onWrite(NimBLECharacteristic* c) {
            if(!gBleInstance) return;
            const std::string uuid = c->getUUID().toString();
            std::string value = c->getValue();

            if(uuid == UUID_SENSOR_NAME_CHAR) {
                String newName = value.c_str();
                Serial.print("Received new sensor name: "); Serial.println(newName);
                gBleInstance->saveSensorName(newName);
            } else if(uuid == UUID_SCAN_INTERVAL_CHAR) {
                int interval = 0;
                if(!value.empty()) interval = atoi(value.c_str());
                Serial.print("Received new scan interval: "); Serial.println(interval);
                gBleInstance->saveScanInterval(interval);
            } else if(uuid == UUID_WIFI_SSID_CHAR) {
                String ssid = value.c_str();
                Serial.print("Received new WiFi SSID: "); Serial.println(ssid);
                SettingsManager settings; settings.begin(); settings.loadSettings();
                String pwd = gBleInstance->pWifiPasswordChar->getValue().c_str();
                settings.setWiFiCredentials(ssid, pwd); settings.end();
                Serial.println("Wi-Fi SSID saved.");
            } else if(uuid == UUID_WIFI_PASSWORD_CHAR) {
                String pwd = value.c_str();
                Serial.print("Received new WiFi Password: "); Serial.println(pwd);
                SettingsManager settings; settings.begin(); settings.loadSettings();
                String ssid = gBleInstance->pWifiSSIDChar->getValue().c_str();
                settings.setWiFiCredentials(ssid, pwd); settings.end();
                Serial.println("Wi-Fi password saved.");
            } else if(uuid == UUID_WIFI_ENABLED_CHAR) {
                bool enabled = (!value.empty() && (value[0] == '1' || value[0] == 't' || value[0] == 'T')); // simple parse
                Serial.print("Received Wi-Fi Enabled state: "); Serial.println(enabled ? "Enabled" : "Disabled");
                SettingsManager settings; settings.begin(); settings.loadSettings(); settings.setWifiEnabled(enabled); settings.end();
                Serial.println("Wi-Fi enabled state saved.");
            } else if(uuid == UUID_WIFI_SCAN_CMD_CHAR) {
                // value of '1' indicates scan
                bool doScan = (!value.empty() && (value[0] == '1'));
                if(doScan) {
                    Serial.println("Start scanning for Wi-Fi SSIDs...");
                    int n = WiFi.scanNetworks();
                    Serial.println("Scan complete.");
                    String ssidList;
                    for(int i=0;i<n;i++) {
                        if(i>0) ssidList += ", ";
                        ssidList += WiFi.SSID(i);
                        // NimBLE uses its own task; no explicit poll required.
                    }
                    gBleInstance->pWifiSSIDsChar->setValue(ssidList.c_str());
                    gBleInstance->pWifiSSIDsChar->notify();
                    // reset command
                    c->setValue("0");
                    Serial.print("Found SSIDs: "); Serial.println(ssidList);
                }
            }
        }
    };

    GenericWriteCallback genericCallback; // Single instance reused for all writable characteristics

public:
    BleLightSensorService() = default;

    // NimBLEServerCallbacks overrides
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Central connected.");
    }
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Central disconnected; restarting advertising.");
        NimBLEDevice::getAdvertising()->start();
    }

    void begin() {
        gBleInstance = this;
        NimBLEDevice::init("LightSensor");
    // Optional: adjust TX power if desired (constant may differ per core)

        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(this);

        // Load settings for initial values
        SettingsManager settingsManager; settingsManager.begin(); settingsManager.loadSettings();
        SensorSettings currentSettings = settingsManager.getSettings(); settingsManager.end();

        // --- Light Level Service ---
        pLightService = pServer->createService(UUID_LIGHT_SERVICE);
    pLightLevelChar = pLightService->createCharacteristic(UUID_LIGHT_CHARACTERISTIC, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
        pLightLevelChar->setValue("-1");

        // --- WiFi Service ---
        pWifiService = pServer->createService(UUID_WIFI_SERVICE);
    pWifiSSIDsChar    = pWifiService->createCharacteristic(UUID_WIFI_SSIDS_CHAR,    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pWifiScanCmdChar  = pWifiService->createCharacteristic(UUID_WIFI_SCAN_CMD_CHAR, NIMBLE_PROPERTY::WRITE);
        pWifiSSIDsChar->setValue("");
        pWifiScanCmdChar->setValue("0");
        pWifiScanCmdChar->setCallbacks(&genericCallback);

        // --- Settings Service ---
        pSettingsService = pServer->createService(UUID_SETTINGS_SERVICE);
    pSensorNameChar   = pSettingsService->createCharacteristic(UUID_SENSOR_NAME_CHAR,  NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pScanIntervalChar = pSettingsService->createCharacteristic(UUID_SCAN_INTERVAL_CHAR, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pWifiSSIDChar     = pSettingsService->createCharacteristic(UUID_WIFI_SSID_CHAR,     NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pWifiPasswordChar = pSettingsService->createCharacteristic(UUID_WIFI_PASSWORD_CHAR, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
    pWifiEnabledChar  = pSettingsService->createCharacteristic(UUID_WIFI_ENABLED_CHAR,  NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);

        // Initial values
        pSensorNameChar->setValue(currentSettings.sensorName.c_str());
        pScanIntervalChar->setValue(String(currentSettings.updateInterval).c_str());
        pWifiSSIDChar->setValue(currentSettings.wifiSSID.c_str());
        pWifiPasswordChar->setValue(currentSettings.wifiPassword.c_str());
        pWifiEnabledChar->setValue(currentSettings.wifiEnabled ? "1" : "0");

        // Attach callbacks to writable characteristics
        pSensorNameChar->setCallbacks(&genericCallback);
        pScanIntervalChar->setCallbacks(&genericCallback);
        pWifiSSIDChar->setCallbacks(&genericCallback);
        pWifiPasswordChar->setCallbacks(&genericCallback);
        pWifiEnabledChar->setCallbacks(&genericCallback);

        // Start services
        pLightService->start();
        pWifiService->start();
        pSettingsService->start();

        // Setup advertising
        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(UUID_LIGHT_SERVICE);
        pAdvertising->addServiceUUID(UUID_WIFI_SERVICE);
        pAdvertising->addServiceUUID(UUID_SETTINGS_SERVICE);
        //pAdvertising->setScanResponse(true);
        pAdvertising->start();

        Serial.println("NimBLE Light Sensor Service started & advertising.");
    }

    void updateLightValue(const String& value) {
        if(!pLightLevelChar) return;
        pLightLevelChar->setValue(value.c_str());
        // Notify only if there are subscribed clients

            Serial.print("Notifying light value: "); Serial.println(value);
            pLightLevelChar->notify();
    
    }

    // Persistence helpers (wrap SettingsManager so callback class can reuse)
    void saveSensorName(const String& name) {
        SettingsManager settings; settings.begin(); settings.loadSettings(); settings.setSensorName(name); settings.end();
        Serial.println("Sensor name saved to settings.");
    }
    void saveScanInterval(int interval) {
        SettingsManager settings; settings.begin(); settings.loadSettings(); settings.setScanInterval(interval); settings.end();
        Serial.println("Scan interval saved to settings.");
    }
};

#endif // BLE_LIGHT_SENSOR_SERVICE_H