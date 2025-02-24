#include "wifi_utils.h"

// Predefined SSIDs and passwords
const char *WifiUtils::ssidList[] = {"5G", "2.4", "AndroidAP_2408"};

const char *WifiUtils::passwordList[] = {"VaughnBlake123!", "VaughnBlake123!",
                                         "angelkawaii02"};

// Function to scan for available networks
void WifiUtils::scanAvailableNetworks() {
  int networkCount = WiFi.scanNetworks(); // Scan for networks
  Serial.print("Scan complete. Found ");
  Serial.print(networkCount);
  Serial.println(" networks.");

  // Print the list of available SSIDs
  for (int i = 0; i < networkCount; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (Signal strength: ");
    Serial.print(WiFi.RSSI(i));
    Serial.println("dBm)");
  }
}

// Function to attempt to connect to a specific network
bool WifiUtils::attemptToConnect(const char *ssid, const char *password) {
  Serial.print("Attempting to connect to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  // Wait until connected or timeout
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime >= 10000) {
      Serial.println("Connection failed!");
      return false;
    }
    delay(100);
  }

  Serial.println("Connected to Wi-Fi!");
  return true;
}

// Function to scan available networks and try connecting to each
bool WifiUtils::connectToAvailableNetworks() {
  /*scanAvailableNetworks();*/

  int networkCount = WiFi.scanNetworks();

  for (int i = 0; i < networkCount; i++) {
    for (int j = 0; j < sizeof(ssidList) / sizeof(ssidList[0]); j++) {
      if (WiFi.SSID(i) == ssidList[j]) { // Match SSID with available network
        if (attemptToConnect(ssidList[j], passwordList[j])) {
          return true; // Successfully connected
        }
      }
    }
  }

  return false; // No SSID matched or no successful connection
}
