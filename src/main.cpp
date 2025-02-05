#include "keys.h"
#include "rsa_utils.h"
#include "wifi_utils.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <esp_sleep.h>

// If this does not work, ig ill just shrivel up and die
const char *serverURL = "http://mad-fak.local/api/data";
// const char *server = "mad-fak.local"; // Only the domain or IP
// const uint16_t port = 3000;           // Port number

// const char *PRIVATE_KEY = getPrivateKey();
// const char *PUBLIC_KEY = getPublicKey();
// const char *DEVICE_PUBLIC_KEY = getDevicePublicKey();

// Set the number of columns and rows for the LCD
int lcdColumns = 20;
int lcdRows = 4;

// Create an LCD object with the I2C address, columns, and rows
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

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
String phoneNumbers[] = {"+639125057697"}; // Array of phone numbers

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
  lcd.print("Hello");
  notify("Key load OK");
  // Configure SIM900A serial communication
  SIM900A.begin(9600, SERIAL_8N1, 16, 17); // RX=GPIO16, TX=GPIO17
  Serial.println("Initializing SIM900A...");
  // Test communication with AT command

  // sendATCommand("AT");
  Serial.println("Check");
  notify("GSM OK");

  // Card detection logic
  while (countdown > 0) {
    if (rfid.PICC_IsNewCardPresent()) {
      Serial.println("Card Detected. Proceeding.");
      notify("CARD DETECTED!");
      for (int i = 0; i < sizeof(phoneNumbers) / sizeof(phoneNumbers[0]); i++) {
        String phoneNumber = String(phoneNumbers[i]);
        String message = "ALERT! "; // Example message
        String notifyMessage = "SMS Sent to " + phoneNumbers[i];

        // Send AT command to initiate SMS to each phone number
        sendATCommand("AT+CMGS=\"" + phoneNumber + "\"");
        // Send the SMS content and end with Ctrl+Z (ASCII 26)
        SIM900A.print(message); // Send the message
        SIM900A.write(26);      // Send Ctrl+Z to indicate the end of the
        delay(1000);            // Wait for the SMS to be sent
        notify(notifyMessage);
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
/*
void sendPostRequest(const String &encryptedPayload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    const char *serverURL =
        "http://192.168.1.113:3000/api/data"; // Update with your local IP

    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    // Send encrypted POST request
    int httpResponseCode = http.POST(encryptedPayload);

    // Read response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Wi-Fi Disconnected");
  }
}

String encryptJson(const char *buildingId, int floor, const char *command,
                   const char *publicKey) {
  // Create JSON object
  StaticJsonDocument<256> doc;
  doc["buildingId"] = buildingId;
  doc["floor"] = floor;
  doc["command"] = command;

  // Convert JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);

  // Encrypt the JSON string
  String encryptedJson = rsaEncrypt(jsonString.c_str(), publicKey);

  // Create the final JSON with the encrypted data
  StaticJsonDocument<256> finalDoc;
  finalDoc["data"] = encryptedJson;

  // Convert final JSON object to string
  String finalJsonString;
  serializeJson(finalDoc, finalJsonString);

  return finalJsonString;
}

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  initRandom(); // Initialize random generator
  // Start WiFi connection process
  if (WifiUtils::connectToAvailableNetworks()) {
    Serial.println("Successfully connected to a network.");
  } else {
    Serial.println("Failed to connect to any available network.");
  }

  /*
 WiFiClient client;
 if (!client.connect(server, port)) {
   Serial.println("Connection failed");
   return;
 }
 Serial.println("Connected!");

 String url = "/"; // The path you want to access
 client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + server +
              "\r\n" + "Connection: close\r\n\r\n");

 // Wait for server response
 while (client.connected()) {
   while (client.available()) {
     char c = client.read();
     Serial.write(c);
   }
 }

 client.stop();

}

void loop() {
  // Example: call sendPostRequest with some encrypted payload
  String encryptedPayload =
      encryptJson("Grade 9", 3, "CHOKING", DEVICE_PUBLIC_KEY);
  sendPostRequest(encryptedPayload);

  delay(5000); // Call every 5 seconds for testing
}*/
