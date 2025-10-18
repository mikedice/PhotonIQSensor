#ifndef BLE_LIGHT_SENSOR_SERVICE_H
#define BLE_LIGHT_SENSOR_SERVICE_H
#include <ArduinoBLE.h>
class BleLightSensorService;
BleLightSensorService* globalInstance = nullptr;

class BleLightSensorService
{
    BLEService lightLevelService;
    BLEStringCharacteristic lightLevelCharacteristic;
    BLEService wifiService;
    BLEStringCharacteristic wifiSSIDsCharacteristic;
    BLEBooleanCharacteristic wifiSSIDScanCommandCharacteristic;
public:
    BleLightSensorService()
        : lightLevelService("3d80c0aa-56b9-458f-82a1-12ce0310e076"),                                     // UUID for the Light Sensor Service
          lightLevelCharacteristic("646bd4e2-0927-45ac-bf41-fd9c69aa31dd", BLERead | BLENotify, 20), // UUID for the Light Characteristic
          wifiService("458800E6-FC10-46BD-8CDA-7F0F74BB1DBF"),                                   // UUID for the Wi-Fi Service
          wifiSSIDsCharacteristic("B30041A1-23DF-473A-AEEC-0C8514514B03", BLERead | BLENotify, 100),
          wifiSSIDScanCommandCharacteristic("5F8B1E42-1A56-4B5A-8026-8B15BC7EE5F3",BLEWrite)
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
        }
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
        BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
        BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

        BLE.setLocalName("LightSensor");
        lightLevelService.addCharacteristic(lightLevelCharacteristic);

        wifiService.addCharacteristic(wifiSSIDsCharacteristic);
        wifiService.addCharacteristic(wifiSSIDScanCommandCharacteristic);
        wifiSSIDScanCommandCharacteristic.setEventHandler(BLEWritten, wifiScanCommandHandler);

        BLE.addService(wifiService);
        BLE.addService(lightLevelService);
        
        lightLevelCharacteristic.writeValue("-1"); // Initial value
        wifiSSIDsCharacteristic.writeValue(""); // Initial value
        wifiSSIDScanCommandCharacteristic.writeValue(false); // Initial value

        BLE.setAdvertisedService(lightLevelService);
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