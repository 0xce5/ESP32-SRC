#include "cert.h"
#include "wifi_utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <esp_sleep.h>

// If this does not work, ig ill just shrivel up and die
const char *serverURL = "http://mad-fak.local/api/data";
const char *host = "mad-fak.local";
const int httpsPort = 443;

// const char *PRIVATE_KEY = getPrivateKey();
// const char *PUBLIC_KEY = getPublicKey();
// const char *DEVICE_PUBLIC_KEY = getDevicePublicKey();

// Set the number of columns and rows for the LCD
int lcdColumns = 20;
int lcdRows = 4;

// Create an LCD object with the I2C address, columns, and rows
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//
WiFiClientSecure client;

// Define the heart shape in an 8x5 pixel grid
byte heart[8] = {0b00000, 0b01010, 0b11111, 0b11111,
                 0b01110, 0b00100, 0b00000, 0b00000};

byte cautionSign[8] = {B11111, B10001, B10101, B10101,
                       B10101, B10001, B10101, B11111};
// byte invCaution[8] = {B11111, B11111, B11011, B11011,

#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// Initialize Serial2 for SIM900A communication
HardwareSerial SIM900A(2); // UART2 on ESP32\

int countdown = 45;
String phoneNumbers[] = {"+639663071486"}; // Array of phone numbers

// Send message to SIM900A
void sendATCommand(String command) {
  Serial.println("Sending command...");
  SIM900A.println(command);
  while (SIM900A.available()) {
    String response = SIM900A.readString();
    Serial.println("Response: " + response);
  }
}

// Send message to LCD
void notify(String text) {
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  delay(1500);
  lcd.setCursor(0, 2);
  lcd.print(text);
}

// Send an error message to LCD
void throwErr(String text, int customChar) {
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.write(customChar);
  lcd.print(text);
}

void sendPostRequest() {
  Serial.println("Connecting to server...");

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  Serial.println("Connected! Sending POST request...");

  // JSON payload
  String jsonData =
      "{\"data\": "
      "\"hi13456789ijhgfckgcgy\", \"gamer\": \"gamergamernddwqhu\"}";

  // HTTP request
  client.print("POST /api/data HTTP/1.1\r\n");
  client.print("Host: " + String(host) + "\r\n");
  client.print("User-Agent: ESP32\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(jsonData.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(jsonData);

  // Read response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r")
      break; // End of headers
  }

  Serial.println("Response:");
  Serial.println(client.readString());
  client.stop();
}

// -----------------------------------------------------------------------------------
void setup() {
  // Start Serial Monitor
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();      // Initialize the LCD
  lcd.backlight(); // Turn on the backlight
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
  // Start mDNS
  if (!MDNS.begin("esp32")) {
    Serial.println("Error starting mDNS");
    return;
  }

  IPAddress resolvedIP;
  if (WiFi.hostByName(host, resolvedIP)) {
    Serial.print("Resolved IP: ");
    Serial.println(resolvedIP);
  } else {
    Serial.println("Failed to resolve hostname.");
  }

  // Set cert
  /*client.setInsecure(); // Bypasses SSL certificate validation*/
  /*client.setCACert(ssl_cert);*/
  if (client.connect(host, httpsPort)) {
    Serial.println("Connected to server with SSL certificate.");
  } else {
    Serial.println("Connection failed.");
  }

  // Configure SIM900A serial communication
  SIM900A.begin(9600, SERIAL_8N1, 16, 17); // RX=GPIO16, TX=GPIO17
  Serial.println("Initializing SIM900A...");
  // Test communication with AT command

  // sendATCommand("AT");
  Serial.println("Check");
  notify("GSM OK");
  lcd.setCursor(0, 0);
  lcd.print("MAD_FAK");
  notify("READY TO SCAN.");

  // Card detection logic
  while (countdown > 0) {
    if (rfid.PICC_IsNewCardPresent()) {
      Serial.println("Card Detected. Proceeding.");
      lcd.setCursor(0, 1);
      lcd.print("         ");
      notify("CARD DETECTED!");
      for (int i = 0; i < sizeof(phoneNumbers) / sizeof(phoneNumbers[0]); i++) {
        String phoneNumber = String(phoneNumbers[i]);
        String message = "ALERT! "; // Example message
        String notifyMessage = "SMS Sent: " + phoneNumbers[i];

        // Send AT command to initiate SMS to each phone number
        sendATCommand("AT+CMGS=\"" + phoneNumber + "\"");
        // Send the SMS content and end with Ctrl+Z (ASCII 26)
        SIM900A.print(message); // Send the message
        SIM900A.write(26);      // Send Ctrl+Z to indicate the end of the
        delay(1000);            // Wait for the SMS to be sent
        notify(notifyMessage);
        sendPostRequest();
        notify("DATABASE UPDATED");
      }

      if (false) {
        // something something gamer
      }
      countdown--;
      delay(1000);
    }
  }
  Serial.println("FUNCTION END");
  delay(4000);
  Serial.println("SLEEP STARTS IN 5 SECONDS");
  delay(1000);
  Serial.println("SLEEP IN ONE SECOND");
}
void loop() {}


/*#include <WiFi.h>*/
/*#include <HTTPClient.h>*/
/**/
/*const char* ssid = "2.4";  */
/*const char* password = "VaughnBlake123!";  */
/*const char* url = "http://192.168.1.113";  */
/**/
/*const int dataSize = 100;  // Adjust the size as needed*/
/*unsigned long responseTimes[dataSize];  */
/*int responseIndex = 0;  // Index for storing response times*/
/*int responseCount = 0;   // Counter for successful responses*/
/**/
/*void setup() {*/
/*  Serial.begin(9600);*/
/*  WiFi.begin(ssid, password);*/
/*  Serial.print("Connecting to WiFi");*/
/**/
/*  while (WiFi.status() != WL_CONNECTED) {*/
/*    delay(500);*/
/*    Serial.print(".");*/
/*  }*/
/**/
/*  Serial.println("\nConnected to WiFi!");*/
/*}*/
/**/
/*void loop() {*/
/*  if (WiFi.status() == WL_CONNECTED && responseCount < dataSize) {*/
/*    HTTPClient http;*/
/*    Serial.println("Starting request...");*/
/**/
/*    unsigned long startRequest = millis();  // Start time for the request*/
/*    http.begin(url);*/
/*    int httpCode = http.GET();*/
/*    unsigned long requestTime = millis() - startRequest;  // Time taken for the request*/
/**/
/*    if (httpCode > 0) {*/
/*      Serial.printf("Response code: %d\n", httpCode);*/
/**/
/*      unsigned long startResponse = millis();  // Start time for the response*/
/*      String payload = http.getString();  // Get response content*/
/*      unsigned long responseTime = millis() - startResponse;  // Time taken for the response*/
/**/
/*      // Calculate total time (request + response)*/
/*      unsigned long totalTime = requestTime + responseTime;*/
/**/
/*      Serial.printf("Request Time: %lu ms\n", requestTime);*/
/*      Serial.printf("Response Time: %lu ms\n", responseTime);*/
/*      Serial.printf("Total Time (Request + Response): %lu ms\n", totalTime);*/
/**/
/*      // Store the total time in the array*/
/*      responseTimes[responseIndex] = totalTime;*/
/*      responseIndex = (responseIndex + 1) % dataSize;  // Loop back if array is full*/
/**/
/*      // Increment response count*/
/*      responseCount++;*/
/*      Serial.printf("Successful responses: %d\n", responseCount);*/
/**/
/*    } else {*/
/*      Serial.printf("Request failed, error: %s\n", http.errorToString(httpCode).c_str());*/
/*    }*/
/**/
/*    http.end();*/
/*  } else if (responseCount >= dataSize) {*/
/*    // When dataSize is reached, print the array as a single line*/
/*    Serial.print("\nResponse Times Array: [");*/
/*    for (int i = 0; i < dataSize; i++) {*/
/*      Serial.print(responseTimes[i]);*/
/*      if (i < dataSize - 1) {*/
/*        Serial.print(", ");  // Separate values with commas*/
/*      }*/
/*    }*/
/*    Serial.println("]");*/
/**/
/*    // Stop further execution or reset*/
/*    while (1);  // Halts the loop once the data is printed*/
/*  } else {*/
/*    Serial.println("WiFi not connected!");*/
/*  }*/
/**/
/*  delay(100);  // Wait before the next request*/
/*}*/

