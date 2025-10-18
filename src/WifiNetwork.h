#ifndef __WIFI_NETWORK_H__
#define __WIFI_NETWORK_H__

#include <Arduino.h>
#include <WiFi.h>

class WifiNetwork {
public:
    WifiNetwork(const char* ssid_, const char* password_) {
        ssid = ssid_;
        password = password_;
    }

    void connect() {
        Serial.print("Connecting to Wi-Fi on ");
        Serial.print(ssid);
        Serial.print(" ...");
        WiFi.begin(ssid, password);
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(CONNECT_WAIT_DELAY);
            Serial.print(".");
            attempts++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected!");
        } else {
            Serial.println("\nFailed to connect.");
        }
    }

    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }

    void disconnect() {
        WiFi.disconnect();
        Serial.println("Disconnected from Wi-Fi.");
    }

    void printStatus() {
        if (isConnected()) {
            Serial.println("Wi-Fi is connected.");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("Wi-Fi is not connected.");
        }
    }
    void printSSID() {
        Serial.print("Current SSID: ");
        Serial.println(WiFi.SSID());
    }

    void printSignalStrength() {
        long rssi = WiFi.RSSI();
        Serial.print("Signal Strength (RSSI): ");
        Serial.println(rssi);
    }

    void printBSSID() {
        Serial.print("BSSID: ");
        Serial.println(WiFi.BSSIDstr());
    }

    void printEncryptionType() {
        Serial.print("Encryption Type: ");
        // ESP32 does not provide encryption type for the connected network directly
        // So we scan and match SSID
        int n = WiFi.scanNetworks();
        for (int i = 0; i < n; ++i) {
            if (WiFi.SSID(i) == WiFi.SSID()) {
                uint8_t encType = WiFi.encryptionType(i);
                switch (encType) {
                    case WIFI_AUTH_OPEN:
                        Serial.println("Open");
                        break;
                    case WIFI_AUTH_WEP:
                        Serial.println("WEP");
                        break;
                    case WIFI_AUTH_WPA_PSK:
                        Serial.println("WPA_PSK");
                        break;
                    case WIFI_AUTH_WPA2_PSK:
                        Serial.println("WPA2_PSK");
                        break;
                    case WIFI_AUTH_WPA_WPA2_PSK:
                        Serial.println("WPA_WPA2_PSK");
                        break;
                    case WIFI_AUTH_WPA2_ENTERPRISE:
                        Serial.println("WPA2_ENTERPRISE");
                        break;
                    default:
                        Serial.println("Unknown");
                }
                return;
            }
        }
        Serial.println("Unknown");
    }
    void printNetworkDetails() {
        if (!isConnected()) {
            Serial.println("Not connected to any Wi-Fi network.");
            return;
        }
        printStatus();
        printSSID();
        printSignalStrength();
        printBSSID();
        printEncryptionType();
    }

private:
    const char* ssid;
    const char* password;
    static const int CONNECT_WAIT_DELAY = 1000;
};

#endif // __WIFI_NETWORK_H__