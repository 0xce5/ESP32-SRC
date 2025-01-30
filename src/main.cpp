#include "encryptAES.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <cstdint>
#include <esp_sleep.h>

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

// Initialize Private key
uint8_t key[32];

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
  lcd.print("                   ");
  delay(1500);
  lcd.setCursor(0, 2);
  lcd.print(text)
}

// Send an error message to LCD
void throwErr(String text, int customChar) {
  lcd setCursor(0, 3);
  lcd.print("                   ");
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
  lcd.setCursor(0, 0);
  lcd.createChar(0, cautionSign);
  lcd.print("MAD-FAK");
  while (!Serial) {
  };
  Serial.println("SPI and Serial initialized.");
  notify("SPI & Serial init");
  initNVS(); // Initialize Non Volatile Storage

  // Check key
  if (!loadKeyFromNVS(key)) {
    Serial.println("Key not found.");
    throwErr("KEY NOT FOUND!", 0)
  }
  Serial.println("Key load OK");
  notify("Key load OK");
  // Configure SIM900A serial communication
  SIM900A.begin(9600, SERIAL_8N1, 16, 17); // RX=GPIO16, TX=GPIO17
  Serial.println("Initializing SIM900A...");
  // Test communication with AT command
  sendATCommand("AT");
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
        notify()
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
  esp_deep_sleep_start();
}
void loop() {}
