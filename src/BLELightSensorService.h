#ifndef BLE_LIGHT_SENSOR_SERVICE_H
#define BLE_LIGHT_SENSOR_SERVICE_H

#include <algorithm>
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

struct CharactersticWriteCallback {
    const char* uuid;
    void (*callback)(BleLightSensorService*, NimBLECharacteristic*);
};



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

    // Write callback map for when characteristics are written to by the central
    static constexpr size_t callbacksLen = 6;
    CharactersticWriteCallback writeCallbacks[callbacksLen] = {
        { UUID_SENSOR_NAME_CHAR, &BleLightSensorService::onWriteSensorName    },
        { UUID_SCAN_INTERVAL_CHAR, &BleLightSensorService::onWriteScanInterval },
        { UUID_WIFI_SSID_CHAR, &BleLightSensorService::onWriteWifiSSID         },
        { UUID_WIFI_PASSWORD_CHAR, &BleLightSensorService::onWriteWifiPassword },
        { UUID_WIFI_ENABLED_CHAR, &BleLightSensorService::onWriteWifiEnabled   },
        { UUID_WIFI_SCAN_CMD_CHAR, &BleLightSensorService::onWriteWifiScanCmd  }
    };  

    static void onWriteSensorName(BleLightSensorService* bleSvcInst, NimBLECharacteristic* c) {
        String newName = c->getValue().c_str();
        Serial.print("Received new sensor name: "); Serial.println(newName);
        if (bleSvcInst) bleSvcInst->saveSensorName(newName);
    }

    static void onWriteScanInterval(BleLightSensorService* bleSvcInst, NimBLECharacteristic* c) {
        std::string value = c->getValue();
        int interval = 0;
        if(!value.empty()) interval = atoi(value.c_str());
        Serial.print("Received new scan interval: "); Serial.println(interval);
        if (bleSvcInst) bleSvcInst->saveScanInterval(interval);
    }

    static void onWriteWifiSSID(BleLightSensorService* bleSvcInst, NimBLECharacteristic* c) {
        String ssid = c->getValue().c_str();
        Serial.print("Received new WiFi SSID: "); Serial.println(ssid);
        SettingsManager settings; settings.begin(); settings.loadSettings();
        String pwd = bleSvcInst ? bleSvcInst->pWifiPasswordChar->getValue().c_str() : "";
        settings.setWiFiCredentials(ssid, pwd); settings.end();
        Serial.println("Wi-Fi SSID saved.");
    }

    static void onWriteWifiPassword(BleLightSensorService* bleSvcInst, NimBLECharacteristic* c) {
        std::string value = c->getValue();
        String pwd = value.c_str();
        Serial.print("Received new WiFi Password: "); Serial.println(pwd);
        SettingsManager settings; settings.begin(); settings.loadSettings();
        String ssid = bleSvcInst ? bleSvcInst->pWifiSSIDChar->getValue().c_str() : "";
        settings.setWiFiCredentials(ssid, pwd); settings.end();
        Serial.println("Wi-Fi password saved.");
    }

    static void onWriteWifiEnabled(BleLightSensorService* bleSvcInst, NimBLECharacteristic* c) {
        std::string value = c->getValue();
        bool enabled = (!value.empty() && (value[0] == '1' || value[0] == 't' || value[0] == 'T'));
        Serial.print("Received Wi-Fi Enabled state: "); Serial.println(enabled ? "Enabled" : "Disabled");
        SettingsManager settings; settings.begin(); settings.loadSettings(); 
        settings.setWifiEnabled(enabled); settings.end();
        Serial.println("Wi-Fi enabled state saved.");
    }

    static void onWriteWifiScanCmd(BleLightSensorService* bleSvcInst, NimBLECharacteristic* c) {
        Serial.println("=== onWriteWifiScanCmd called ===");
        std::string value = c->getValue();
        Serial.print("Received value length: "); Serial.println(value.length());
        
        if (!value.empty()) {
            Serial.print("First byte (decimal): "); Serial.println((int)value[0]);
            Serial.print("First byte (hex): 0x"); Serial.println((int)value[0], HEX);
        }
        
        // iOS sends a UInt8 with value 1 (not ASCII '1' which is 49)
        bool doScan = (!value.empty() && (value[0] == 1 || value[0] == '1'));
        Serial.print("doScan: "); Serial.println(doScan ? "true" : "false");
        Serial.print("bleSvcInst: "); Serial.println(bleSvcInst ? "valid" : "NULL");
        
        if(doScan && bleSvcInst) {
            Serial.println("Start scanning for Wi-Fi SSIDs...");
            int n = WiFi.scanNetworks();
            Serial.println("Scan complete.");
            String ssidList;
            for(int i=0; i<n; i++) {
                if(i>0) ssidList += ", ";
                ssidList += WiFi.SSID(i);
            }
            bleSvcInst->pWifiSSIDsChar->setValue(ssidList.c_str());
            bleSvcInst->pWifiSSIDsChar->notify();
            
            // Reset to 0 (raw byte, not ASCII)
            uint8_t zero = 0;
            c->setValue(&zero, 1);
            Serial.print("Found SSIDs: "); Serial.println(ssidList);
        }
    }

    // --- Callback classes ---
    class GenericWriteCallback : public NimBLECharacteristicCallbacks {

        // Note: add your characteristic callbacks to the writeCallbacks array in BleLightSensorService
        // if you want to handle more writable characteristics.
        
        std::string toLower(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return result;
        }
        
        void onRead(NimBLECharacteristic* c, NimBLEConnInfo& connInfo) override {
            Serial.println("GenericWriteCallback::onRead called");
        }
        
        void onWrite(NimBLECharacteristic* c, NimBLEConnInfo& connInfo) override {
            Serial.println("===== GenericWriteCallback::onWrite called =====");
            if(!gBleInstance) {
                Serial.println("ERROR: onWrite called but gBleInstance is null!");
                return;
            }
            std::string uuid = toLower(c->getUUID().toString());
            Serial.print("UUID: "); 
            Serial.println(uuid.c_str());
            
            size_t callbacksLen = sizeof(gBleInstance->writeCallbacks)/sizeof(CharactersticWriteCallback);
            for (size_t i = 0; i< callbacksLen; i++){
                std::string storedUuid = toLower(gBleInstance->writeCallbacks[i].uuid);
                
                if (uuid == storedUuid){
                    Serial.print("Found matching callback at index "); Serial.println(i);
                    gBleInstance->writeCallbacks[i].callback(gBleInstance, c);
                    return;
                }
            }
            Serial.println("WARNING: No matching callback found for this UUID");
        }
        
        void onStatus(NimBLECharacteristic* c, int code) override {
            Serial.print("GenericWriteCallback::onStatus called, code: "); Serial.println(code);
        }
        
        void onSubscribe(NimBLECharacteristic* c, NimBLEConnInfo& connInfo, uint16_t subValue) override {
            Serial.print("GenericWriteCallback::onSubscribe called, subValue: "); Serial.println(subValue);
        }
    };

    GenericWriteCallback genericCallback; // Single instance reused for all writable characteristics

public:
    BleLightSensorService() = default;

    // NimBLEServerCallbacks overrides
    void onConnect(NimBLEServer* pServer) {
        Serial.println("===================================");
        Serial.println("Central CONNECTED!");
        Serial.println("===================================");
    }
    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("===================================");
        Serial.println("Central DISCONNECTED!");
        Serial.println("===================================");
        Serial.println("Restarting advertising...");
        NimBLEDevice::getAdvertising()->start();
    }

    void scanForPeers(){
        Serial.println("Scanning for connected peers...");
        std::vector<uint16_t> peerDevices = pServer->getPeerDevices();
        Serial.print("Currently connected peers: "); Serial.println(peerDevices.size());

        if (peerDevices.size() > 0){
            for (uint16_t i = 0; i < peerDevices.size(); i++){
                NimBLEConnInfo info = pServer->getPeerInfo(peerDevices[i]);
                Serial.print("Peer ID: "); Serial.println(peerDevices[i]);
                Serial.print(" Address: "); Serial.println(info.getAddress().toString().c_str());
                Serial.print(" ID Address: "); Serial.println(info.getIdAddress().toString().c_str());
                Serial.print(" Conn Handle: "); Serial.println(String(info.getConnHandle()).c_str());
            }
        }
        else
        {
            Serial.println("No connected peers found.");
            NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
            if (!pAdvertising->isAdvertising()) {
                Serial.println("No connected peers; restarting advertising.");
                // In void begin() I am setting pServer->advertiseOnDisconnect(true); so advertising should automatically
                // restart on disconnect. Theoretically this is redundant. Maybe can remove it later because it may
                // never get called.
                pAdvertising->start();
            } else {
                Serial.println("No connected peers; already advertising.");
            }
        }
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
        Serial.println("Creating WiFi Service...");
        pWifiService = pServer->createService(UUID_WIFI_SERVICE);
    pWifiSSIDsChar    = pWifiService->createCharacteristic(UUID_WIFI_SSIDS_CHAR,    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pWifiScanCmdChar  = pWifiService->createCharacteristic(UUID_WIFI_SCAN_CMD_CHAR, NIMBLE_PROPERTY::WRITE);
        Serial.print("WiFi Scan CMD Char UUID: "); Serial.println(UUID_WIFI_SCAN_CMD_CHAR);
        Serial.print("WiFi Scan CMD Char created at: "); Serial.println((unsigned long)pWifiScanCmdChar, HEX);
        Serial.print("WiFi Scan CMD Char properties: "); Serial.println(pWifiScanCmdChar->getProperties());
        pWifiSSIDsChar->setValue("");
        pWifiScanCmdChar->setValue("0");
        Serial.print("WiFi Scan CMD initial value: "); Serial.println(pWifiScanCmdChar->getValue().c_str());

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

        // Start services
        pLightService->start();
        pWifiService->start();
        pSettingsService->start();

        // Attach callbacks to writable characteristics (AFTER services are started)
        Serial.println("Setting callbacks...");
        Serial.print("genericCallback address: "); Serial.println((unsigned long)&genericCallback, HEX);
        
        Serial.println("Setting WiFi Scan CMD callback...");
        pWifiScanCmdChar->setCallbacks(&genericCallback);
        Serial.print("  Callback object address: "); Serial.println((unsigned long)pWifiScanCmdChar->getCallbacks(), HEX);
        
        pSensorNameChar->setCallbacks(&genericCallback);
        pScanIntervalChar->setCallbacks(&genericCallback);
        pWifiSSIDChar->setCallbacks(&genericCallback);
        pWifiPasswordChar->setCallbacks(&genericCallback);
        pWifiEnabledChar->setCallbacks(&genericCallback);
        Serial.println("All callbacks set.");

        // Setup advertising
        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(UUID_LIGHT_SERVICE);
        pAdvertising->addServiceUUID(UUID_WIFI_SERVICE);
        pAdvertising->addServiceUUID(UUID_SETTINGS_SERVICE);
        //pAdvertising->setScanResponse(true);
        pAdvertising->start();
        pServer->advertiseOnDisconnect(true); // Takes care of dead connections such as when you stop the debugger on the IOS app in XCode :)
        Serial.println("NimBLE Light Sensor Service started & advertising.");
    }

    void updateLightValue(const String& value) {
        if(!pLightLevelChar) return;
        pLightLevelChar->setValue(value.c_str());
        // Notify only if there are subscribed clients

            // Serial.print("Notifying light value: "); Serial.println(value);
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