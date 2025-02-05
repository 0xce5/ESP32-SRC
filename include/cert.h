#ifndef CERT_H
#define CERT_H

#include <WiFiClientSecure.h>

// Declare the certificate as a constant string
extern const char *ssl_cert;

// Declare a global X509List object for the certificate
extern X509List cert;

#endif // CERT_H
