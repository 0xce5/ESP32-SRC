#include <WiFi.h>
#include <HTTPClient.h>

const char* ssidToConnect = "AndroidAP_5614";  // Replace with your target SSID
const char* password = "skibidisigma";   // Replace with your network password
const char* serverAddress = "http://192.168.252.205"; // Replace with your server's IP address
const int serverPort = 8000; // Replace with your server's port number

#define RFID_INTERRUPT_PIN 2

void setup() {
  Serial.begin(9600);
  pinMode(RFID_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(RFID_INTERRUPT_PIN, handleRFIDInterrupt, FALLING);
  goToSleep();
  delay(1000); // Allow time for Serial Monitor to open

  // Start Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Ensure we're disconnected before scanning

  // Scan for networks
  Serial.println("Scanning for networks...");
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");

  bool found = false;

  // Check for the specific SSID
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    Serial.print("Found SSID: ");
    Serial.println(ssid);

    if (ssid.equals(ssidToConnect)) {
      found = true;
      Serial.println("Target SSID found. Attempting to connect...");
      WiFi.begin(ssidToConnect, password);


      unsigned long startTime = millis();
      const unsigned long timeout = 10000; // 10 seconds timeout

      // Wait for connection
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        if (millis() - startTime >= timeout) {
          Serial.println("\nConnection attempt timed out.");
          return; // Exit setup if timeout occurs
        }
      }

      Serial.println("\nConnected to Wi-Fi!");
      break; // Exit the loop if connected
    }
  }

  if (!found) {
    Serial.println("Target SSID not found.");
  }
    //placeholder
  }

void loop() {

  if (shouldUpdateDatabase()) {

    updateDatabase();
    Serial.println("ESP32 is sleeping");
  }

  // Other code here
}

void goToSleep() {
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RFID_INTERRUPT_PIN, 0);
  esp_deep_sleep_start();
}
bool shouldUpdateDatabase() {
  // Code to check if it's time to update the database
  // Return true if it's time to update, false otherwise
}

void updateDatabase() {
  HTTPClient http;
  int failloop = 0;

  while (true) {
    String data = "key1=value1&key2=value2"; // Replace with your data

    http.begin(serverAddress);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(data);

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("HTTP Response: " + payload);
      break;
    } else {
      Serial.println("HTTP REQ Error: " + String(httpResponseCode));
      Serial.println("Fail Count: " + String(failloop + 1));
      failloop++;
      delay(3000);
    }

    http.end();
  }
}