#include "wifi_utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <esp_sleep.h>

const char *serverURL = "https://mad-fak.space/api/data";
const char *host = "mad-fak.space";
const int httpPort = 443;  // HTTPS port

const char* ntpServer = "time.google.com";
const long gmtOffset_sec = 8 * 3600;  // UTC+8 offset
const int daylightOffset_sec = 0;     // No daylight savings in most UTC+8 regions

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX\n" \
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n" \
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx\n" \
"NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n" \
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0\n" \
"MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube\n" \
"Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e\n" \
"WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd\n" \
"BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd\n" \
"BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN\n" \
"l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw\n" \
"Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v\n" \
"Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG\n" \
"SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ\n" \
"odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY\n" \
"+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs\n" \
"kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep\n" \
"8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1\n" \
"vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl\n" \
"-----END CERTIFICATE-----\n";

int lcdColumns = 20;
int lcdRows = 4;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

WiFiClientSecure client;

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.print("ESP32 Time: ");
    Serial.print(asctime(&timeinfo));  // Print human-readable time
}

byte heart[8] = {0b00000, 0b01010, 0b11111, 0b11111,
                 0b01110, 0b00100, 0b00000, 0b00000};

byte cautionSign[8] = {B11111, B10001, B10101, B10101,
                       B10101, B10001, B10101, B11111};

bool isAuthenticated = false;
#define BUTTON_1 34 
#define BUTTON_2 33 
#define BUTTON_3 32 

#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

HardwareSerial SIM900A(2);

int countdown = 45;
String phoneNumbers[] = {"+639663071486"};

void sendATCommand(String command) {
  Serial.println("Sending command...");
  SIM900A.println(command);
  while (SIM900A.available()) {
    String response = SIM900A.readString();
    Serial.println("Response: " + response);
  }
}

void notify(String text) {
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  delay(1500);
  lcd.setCursor(0, 2);
  lcd.print(text);
}

void throwErr(String text, int customChar) {
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.write(customChar);
  lcd.print(text);
}

void sendPostRequest() {
  Serial.println("Connecting to server...");
  client.setCACert(root_ca);

  Serial.print("Free heap before connect: ");
  Serial.println(ESP.getFreeHeap());

  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    Serial.print("Error code: ");
    Serial.println(client.getWriteError());
    return;
  }

  Serial.println("Connected! Sending POST request...");
  String jsonData = "{\"data\": \"test\"}";

  client.print("POST /api/data HTTP/1.1\r\n");
  Serial.println("Sent request line");
  client.print("Host: " + String(host) + "\r\n");
  Serial.println("Sent Host header");
  client.print("User-Agent: ESP32\r\n");
  Serial.println("Sent User-Agent");
  client.print("Content-Type: application/json\r\n");
  Serial.println("Sent Content-Type");
  client.print("Content-Length: " + String(jsonData.length()) + "\r\n");
  Serial.println("Sent Content-Length");
  client.print("Connection: close\r\n\r\n");
  Serial.println("Sent Connection header and blank line");
  client.print(jsonData);
  Serial.println("Request sent. Reading response...");

  String responseHeaders = "";
  String line;
  while (client.connected()) {
    line = client.readStringUntil('\n');
    responseHeaders += line + "\n";
    if (line == "\r") {
      break;
    }
  }

  int contentLength = 0;
  size_t headerStart = responseHeaders.indexOf("Content-Length: ");
  if (headerStart != -1) {
    size_t headerEnd = responseHeaders.indexOf("\r\n", headerStart);
    contentLength = responseHeaders.substring(headerStart + 16, headerEnd).toInt();
  }

  String responseBody = "";
  while (contentLength > 0 && client.available()) {
    char c = client.read();
    responseBody += c;
    contentLength--;
  }

  client.stop();

  Serial.println("Response headers: ");
  Serial.println(responseHeaders);
  Serial.println("Response body: ");
  Serial.println(responseBody);
}

void sendMessage(int button, String buildingId, int floor) {
  String notifyMsg = String(button) + " pressed.";

  if (button == 1) {
    notifyMsg += " Low";
  }

  if (button == 2) {
    notifyMsg += " Medium";
  }

  if (button == 3) {
    notifyMsg += " High";
  }

  Serial.println(notifyMsg);
  lcd.setCursor(0, 1);
  lcd.print("         ");
  notify(notifyMsg);
  for (int i = 0; i < sizeof(phoneNumbers) / sizeof(phoneNumbers[0]); i++) {
    String phoneNumber = String(phoneNumbers[i]);
    String finalMessage = "ALERT! Medical Attention required at: Building " + buildingId + ", Floor" + floor;
    if (button == 1) {
      finalMessage += " Low Severity";
    }

    if (button == 2) {
      notifyMsg += " Medium Severity";
    }

    if (button == 3) {
      notifyMsg += " High Severity";
    }
    String notifyMessage = "SMS Sent: " + phoneNumbers[i];

    sendATCommand("AT+CMGS=\"" + phoneNumber + "\"");
    SIM900A.print(finalMessage);
    SIM900A.write(26);
    delay(1000);
    notify(notifyMessage);
    sendPostRequest();
    notify("DATABASE UPDATED");
  }
}

int getPressedButton() {
  int button = -1;  // -1 means no button or multiple buttons pressed

  if (digitalRead(BUTTON_1) == HIGH) {
      if (button != -1) return -1; // More than one button is pressed
      button = 1;
  }
  if (digitalRead(BUTTON_2) == HIGH) {
      if (button != -1) return -1;
      button = 2;
  }
  if (digitalRead(BUTTON_3) == HIGH) {
      if (button != -1) return -1;
      button = 3;
  }

  return button;  // 1, 2, or 3 if exactly one is pressed; -1 otherwise
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.createChar(0, cautionSign);

  while (!Serial) {
  };
  Serial.println("SPI and Serial initialized.");
  notify("CONNECTING");

  if (WifiUtils::connectToAvailableNetworks()) {
    Serial.println("Successfully connected to a network.");
    notify("WIFI OK");
  } else {
    Serial.println("Failed to connect to any available network.");
    throwErr("CONNECT FAIL!", 0);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(2000);  // Give time to sync

  printLocalTime();  // Show current ESP32 time

  IPAddress resolvedIP;
  if (WiFi.hostByName(host, resolvedIP)) {
    Serial.print("Resolved IP: ");
    Serial.println(resolvedIP);
  } else {
    Serial.println("Failed to resolve hostname.");
  }

  Serial.print("Free heap before connect: ");
  Serial.println(ESP.getFreeHeap());
  client.setCACert(root_ca);  // Use the provided certificate
  if (client.connect(host, httpPort)) {  // Updated to use httpPort
    Serial.println("Connected to server.");
  } else {
    Serial.println("Connection failed.");
  }

  SIM900A.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Initializing SIM900A...");
  Serial.println("Check");
  notify("GSM OK");
  lcd.setCursor(0, 0);
  lcd.print("MAD_FAK");
  notify("READY TO SCAN.");
}
void loop() {
  if (rfid.PICC_IsNewCardPresent() && !isAuthenticated) {
    isAuthenticated = true;
    notify("Authenticated.");
  }

  int button = getPressedButton();

  if (isAuthenticated && button == 1 ) {
    sendMessage(1, "Grade 9", 1);
    isAuthenticated = false;
    // do nothing 
  }

  else if (isAuthenticated && button == 2 ) {
    sendMessage(2, "Grade 9", 2);
    isAuthenticated = false;
    // do nothing 
  }

  else if (isAuthenticated && button == 3 ) {
    sendMessage(3, "Grade 9", 3);
    isAuthenticated = false;
    // do nothing 
  }
  delay(100);

/*#include <WiFi.h>*/
/*#include <HTTPClient.h>*/
/**/
/*const char* ssid = "SimNet";  // Replace with your WiFi SSID*/
/*const char* password = "test1234";  // Replace with your WiFi password*/
/**/
/*const char* serverURL = "https://mad-fak.space/api/data";*/
/**/
/*#define NUM_REQUESTS 50  // Run 50 trials*/
/**/
/*// Python-style array output*/
/*String latency = "latency = [";*/
/*String bandwidth = "bandwidth = [";*/
/*String packet_loss = "packet_loss = [";*/
/*String rssi_values = "rssi = [";*/
/**/
/*void measureLatencyBandwidthPacketLoss();*/
/**/
/*void setup() {*/
/*    Serial.begin(9600);*/
/*    delay(1000);*/
/**/
/*    // Connect to WiFi*/
/*    WiFi.begin(ssid, password);*/
/*    Serial.print("Connecting to WiFi...");*/
/*    while (WiFi.status() != WL_CONNECTED) {*/
/*       delay(500);*/
/*       Serial.print(".");*/
/*    }*/
/*    Serial.println("\nConnected!");*/
/*    Serial.println(WiFi.RSSI());*/
/**/
/*    // Perform measurements*/
/*    measureLatencyBandwidthPacketLoss();*/
/**/
/*    // Print Python-style arrays*/
/*    Serial.println(latency + "]");*/
/*    Serial.println(bandwidth + "]");*/
/*    Serial.println(packet_loss + "]");*/
/*    Serial.println(rssi_values + "]");*/
/*}*/
/**/
/*void measureLatencyBandwidthPacketLoss() {*/
/*    int failedRequests = 0;*/
/**/
/*    for (int i = 0; i < NUM_REQUESTS; i++) {*/
/*        if (WiFi.status() != WL_CONNECTED) {*/
/*            Serial.println("WiFi Disconnected!");*/
/*            return;*/
/*        }*/
/**/
/*        HTTPClient http;*/
/*        http.begin(serverURL);*/
/*        http.addHeader("Content-Type", "application/json");*/
/**/
/*        String payload = "{\"data\": \"test-packet-" + String(i) + "\"}";*/
/**/
/*        unsigned long startTime = millis();  // Start time*/
/*        int httpResponseCode = http.POST(payload);*/
/*        unsigned long endTime = millis();  // End time*/
/**/
/*        int rssi = WiFi.RSSI();  // Get current RSSI*/
/**/
/*        if (httpResponseCode > 0) {*/
/*            unsigned long responseTime = endTime - startTime;*/
/*            int dataSize = payload.length() + 100;  // Approximate size with headers (bytes)*/
/*            float speed = (dataSize * 8.0) / (responseTime / 1000.0);  // Bits per second*/
/**/
/*            // Store results*/
/*            latency += String(responseTime) + (i < NUM_REQUESTS - 1 ? ", " : "");*/
/*            bandwidth += String(speed / 1000.0) + (i < NUM_REQUESTS - 1 ? ", " : "");  // Convert to Kbps*/
/*        } else {*/
/*            failedRequests++;*/
/*        }*/
/**/
/*        rssi_values += String(rssi) + (i < NUM_REQUESTS - 1 ? ", " : "");*/
/**/
/*        http.end();*/
/*        delay(500);  // Short delay between requests*/
/*    }*/
/**/
/*    float packetLossPercent = ((float)failedRequests / NUM_REQUESTS) * 100.0;*/
/*    packet_loss += String(packetLossPercent);*/
/*}*/
/**/
/*void loop() {*/
/*    // Nothing here*/
/*}*/
