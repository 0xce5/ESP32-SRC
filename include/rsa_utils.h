#ifndef RSA_UTILS_H
#define RSA_UTILS_H

#include <Arduino.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>
#include <vector>

// Initialize random generator (call once)
void initRandom();

// Encrypt a plaintext string using a public key
String rsaEncrypt(const String &plaintext, const char *publicKey);

// Decrypt a ciphertext string using a private key
String rsaDecrypt(const String &ciphertext, const char *privateKey);

#endif // RSA_UTILS_H

