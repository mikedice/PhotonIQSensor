
#include <SPI.h>
#include <Wire.h>
#include "RealtimeClock.h"
#include "WifiNetwork.h"
#include "LightSensor.h"
// Removed LightDisplay.h include
#include "FileLogger.h"
#include "BLELightSensorService.h"


// Helper functions
// Removed GetDisplayValues callback and related display code
void waitForSerial(uint16_t timeout = 2000);

// Wi-Fi Network
char ssid[] = "a-shortfall-of-gravitas";
char pass[] = "kona1234";
WifiNetwork wifiNetwork(ssid, pass);

RealtimeClock realtimeClock(wifiNetwork); // Create an instance of the RealtimeClock class. Uses WifiNetwork to adjust clock if clock's power was lost.

LightSensor lightSensor; // Create an instance of the LightSensor class.


FileLogger fileLogger(6); // Create my file system wrapper.

BleLightSensorService bleLightSensorService; // Create an instance of the BLE Light Sensor Service.

unsigned long lastPeerScan = 0;
const unsigned long peerScanPeriod = 5000; // 5 seconds


// Arduino Setup function
void setup()
{
  Serial.begin(115200);
  waitForSerial();

  // Initialize Wi-Fi
  wifiNetwork.connect();
  wifiNetwork.printNetworkDetails();

  // Initialize RTC
  realtimeClock.begin();

  // Initialize light sensor
  lightSensor.begin();

  // Initialize file system (SD card)
  fileLogger.begin();

  // Removed e-Paper display initialization

  Serial.println("BLE Initiailization...");

  bleLightSensorService.begin(); // Initialize BLE Light Sensor Service

  // Removed initial display update code
  Serial.println("Setup completed successfully.");
}

// Arduino Loop function
void loop()
{
  // Run scanForPeers every 5 seconds
  if (millis() - lastPeerScan >= peerScanPeriod) {
    bleLightSensorService.scanForPeers();
    lastPeerScan = millis();
  }

  // lightSensor.printLightLevel(); // Print light level to Serial Monitor
  String lightValue = lightSensor.getLightAsString();
  bleLightSensorService.updateLightValue(lightValue); // Update BLE service with light value
  // Serial.println("updated light value over BLE");
}

// Wait for a PC to connect to the Serial port
void waitForSerial(uint16_t timeout)
{
  unsigned long start = millis();
  while (!Serial && (millis() - start < timeout))
  {
    ; // spin briefly
  }
}

// Removed GetDisplayValues function

/*
 Serial.print("Initializing SD card...");


if (!SD.begin(6)) {
 Serial.println("SD init failed!");
 return;
}

Serial.println("SD init succeeded.");

File testFile = SD.open("test.txt", FILE_WRITE);
if (testFile) {
 testFile.println("Hello from Nano 33!");
 testFile.close();
 Serial.println("Wrote to test.txt");
} else {
 Serial.println("Failed to open file.");
}
 */

// fileSystem.begin();
