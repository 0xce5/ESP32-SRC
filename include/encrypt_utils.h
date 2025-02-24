
#ifndef ENCRYPT_UTILS_H
#define ENCRYPT_UTILS_H

#include <Arduino.h>
#include <mbedtls/aes.h>
#include <mbedtls/md.h>
#include <mbedtls/base64.h>
#include <ArduinoJson.h>

// Configuration constants
// AES Key Size (256 bits)
#define AES_KEY_SIZE 32  // 32 bytes = 256 bits

// HMAC Key Size (256 bits)
#define HMAC_KEY_SIZE 32 // 32 bytes = 256 bits

// IV Size (128 bits, same as AES block size)
#define IV_SIZE 16       // 16 bytes = 128 bits

// HMAC Size (SHA256 output size)
#define HMAC_SIZE 32     // 32 bytes = 256 bits

// Block size for AES CBC mode (in bytes)
#define BLOCK_SIZE 16    // AES block size (16 bytes)

// Function prototypes
void deriveKeys();
String encryptToJson(const char* plaintext);
String decryptFromJson(const String& json);

// AES Encryption
void aesEncrypt(const uint8_t* plaintext, uint8_t* ciphertext, uint8_t* iv, size_t length);
void aesDecrypt(const uint8_t* ciphertext, uint8_t* plaintext, uint8_t* iv, size_t length);

// HMAC Generation
void generateHMAC(const uint8_t* data, size_t length, uint8_t* hmac_output);

// Base64 Encoding and Decoding
String base64Encode(const uint8_t* data, size_t length);
int base64Decode(const String& encoded, uint8_t* decoded, size_t maxLength);

#endif // ENCRYPT_UTILS_H

