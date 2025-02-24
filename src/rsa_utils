#include "rsa_utils.h"
#include <mbedtls/base64.h>

// Random generator context
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_entropy_context entropy;

// Initialize the random number generator
void initRandom() {
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
}

// Initialize an RSA key context
mbedtls_pk_context initPkContext(const char *key) {
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    int ret = mbedtls_pk_parse_public_key(&pk, (const unsigned char *)key, strlen(key) + 1);
    if (ret != 0) {
        ret = mbedtls_pk_parse_key(&pk, (const unsigned char *)key, strlen(key) + 1, nullptr, 0);
        if (ret != 0) {
            Serial.printf("Failed to parse key: %d\n", ret);
            mbedtls_pk_free(&pk);
            return pk;
        }
    }
    return pk;
}

// Base64 encoding function
String base64_encode(const unsigned char *data, size_t len) {
    size_t olen;
    unsigned char output[MBEDTLS_MPI_MAX_SIZE * 2]; // Ensure enough space

    mbedtls_base64_encode(output, sizeof(output), &olen, data, len);
    return String((char *)output);
}

// Base64 decoding function
std::vector<uint8_t> base64_decode(const char *encoded) {
    size_t olen;
    std::vector<uint8_t> output(strlen(encoded)); // Allocate enough space

    mbedtls_base64_decode(output.data(), output.size(), &olen,
                          (const unsigned char *)encoded, strlen(encoded));
    output.resize(olen); // Resize to actual length
    return output;
}

// RSA Encryption
String rsaEncrypt(const String &plaintext, const char *publicKey) {
    mbedtls_pk_context pk = initPkContext(publicKey);
    if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_RSA)) {
        Serial.println("Key is not an RSA key!");
        mbedtls_pk_free(&pk);
        return "";
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

    size_t olen;
    unsigned char output[MBEDTLS_MPI_MAX_SIZE]; // Adjust buffer for key size

    int ret = mbedtls_rsa_pkcs1_encrypt(
        rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PUBLIC,
        plaintext.length(), (const unsigned char *)plaintext.c_str(), output);

    if (ret != 0) {
        Serial.printf("Encryption failed: %d\n", ret);
        mbedtls_pk_free(&pk);
        return "";
    }

    mbedtls_pk_free(&pk);

    // Encode output to Base64 for safe transport
    return base64_encode(output, rsa->len);
}

// RSA Decryption
String rsaDecrypt(const String &ciphertext, const char *privateKey) {
    mbedtls_pk_context pk = initPkContext(privateKey);
    if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_RSA)) {
        Serial.println("Key is not an RSA key!");
        mbedtls_pk_free(&pk);
        return "";
    }

    mbedtls_rsa_context *rsa = mbedtls_pk_rsa(pk);
    mbedtls_rsa_set_padding(rsa, MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);

    size_t olen;
    unsigned char output[MBEDTLS_MPI_MAX_SIZE]; // Adjust buffer size

    // Decode Base64 before decryption
    std::vector<uint8_t> decoded = base64_decode(ciphertext.c_str());

    int ret = mbedtls_rsa_pkcs1_decrypt(
        rsa, mbedtls_ctr_drbg_random, &ctr_drbg, MBEDTLS_RSA_PRIVATE, &olen,
        decoded.data(), output, sizeof(output));

    if (ret != 0) {
        Serial.printf("Decryption failed: %d\n", ret);
        mbedtls_pk_free(&pk);
        return "";
    }

    mbedtls_pk_free(&pk);
    return String((char *)output, olen);
}
