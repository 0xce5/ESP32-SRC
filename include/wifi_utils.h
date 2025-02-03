#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <WiFi.h>

class WifiUtils {
public:
  // List of predefined SSIDs and passwords
  static const char *ssidList[];
  static const char *passwordList[];

  // Function to scan for available networks and attempt to connect to them
  static bool connectToAvailableNetworks();

private:
  // Function to scan for available networks
  static void scanAvailableNetworks();

  // Function to attempt to connect to a specific network
  static bool attemptToConnect(const char *ssid, const char *password);
};

#endif
