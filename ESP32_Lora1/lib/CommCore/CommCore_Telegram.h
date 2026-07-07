/**
 * CommCore_Telegram.h - Telegram Bot Setup + Send
 * 
 * Requires TELEGRAM_CERTIFICATE_ROOT defined in User_config.h
 * Requires Bot_Token_1[60] and Bot_Group[20] from CommCore_Params.h
 */

#ifndef COMMCORE_TELEGRAM_H
#define COMMCORE_TELEGRAM_H

#include "CommCore_Params.h"

// ============================================================
// TELEGRAM SSL CERTIFICATE (Must be in User_config.h)
// ============================================================
#ifndef TELEGRAM_CERTIFICATE_ROOT
#define TELEGRAM_CERTIFICATE_ROOT \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
  "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
  "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
  "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
  "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
  "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
  "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
  "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
  "PNt0OKRKzE0Fvd86iuYRN9oMQKdsh8hQAkWMm+oxDiUWf6h4QBz0BB8qnFu8LF8C\n" \
  "MmGEFaQ7F2W1s6oKQxwz1aB1uX2e3k0hvqNV+8RjMNB6G9YJ7jKlYpR1KGqvE5Hs\n" \
  "3L0m+Ey1HkWB9dV1qY2EwGfNsjUY7B2t0R9McDsASZNAIU3BUBT1ASCB6RJX8mhH\n" \
  "++7+7x5kQxvEQlbXmL1pTG+ZEu5B0mBB5F4hXw0XwKMC9UJxEKFB3BQx4CmRXYB\n" \
  "4jqRUhPiCAc1kNcCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\n" \
  "MAMBAf8wHQYDVR0OBBYEFKrx5M5HjLqPRLY++fUJqR4aCRPmMB8GA1UdIwQYMBaA\n" \
  "FKrx5M5HjLqPRLY++fUJqR4aCRPmMA0GCSqGSIb3DQEBBQUAA4IBAQAcXgFiRhPq\n" \
  "VUE/1q1bqTfFj5zHk5Uq4MOMQ6BKMN5PqTPpJpJ4Zx6EAlD4k2OK4Y+YsL2kDGW\n" \
  "KX5gWrP4bMxHJARQ4wR8W+1pF1YqD8H5Oe5vKU1OaT7x/2H0U8H8LqX+3wCp1R\n" \
  "0d2L5Q9s3K3s6+4dZ3K0c0MqH8f6M7Sq+2Q8t+0r6d3a4L5N3wKt5uFbOJ2j2e\n" \
  "z3j7vR8R8e7Qo2F3Uq3Ld2nDz8v8wOq6s4M3P6bO7xMn3t0mK4e9s2q5u0K4yG\n" \
  "L5H4yG7V6r7o1J9H7i8P2a9T4w6r3dV2s1L9zF5k3g0s2o8j1v6s9=\n" \
  "-----END CERTIFICATE-----\n"
#endif

// ============================================================
// TELEGRAM SETUP
// ============================================================
void CommCore_telegramSetup() {
  bot1 = UniversalTelegramBot(String(Bot_Token_1), secured_client1);
  secured_client1.setCACert(TELEGRAM_CERTIFICATE_ROOT);
}

// ============================================================
// SEND TELEGRAM MESSAGE (simple wrapper)
// ============================================================
bool CommCore_sendTelegram(const char* msg) {
  if (Internet != 1) return false;
  if (strlen(Bot_Token_1) < 20 || strlen(Bot_Group) < 3) return false;
  return bot1.sendMessage(Bot_Group, msg, "");
}

#endif // COMMCORE_TELEGRAM_H
