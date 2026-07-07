/**
 * wokwi_stubs.h - Wokwi Simulation Stubs for ESP32_Lora
 * 
 * Include this file when building with -D WOKWI flag.
 * Wokwi lacks: SX1278 LoRa, INA226, DS18B20 (physical sensors).
 * We stub these to test: scheduler, RTC, relay sequencing, LCD.
 *
 * Usage in platformio.ini:
 *   [env:slave]
 *   build_flags = -D WOKWI   (add before building for Wokwi)
 */

#ifndef WOKWI_STUBS_H
#define WOKWI_STUBS_H

#ifdef WOKWI

// ============================================================
// LORA SERVICE STUBS (SX1278 not available in Wokwi)
// ============================================================
// Turn the real LoraService into no-ops.
// The scheduler runs autonomously from RTC — no LoRa needed.

#define LORA_STUB_INIT      0  // LoraService::init() always succeeds
#define LORA_STUB_SEND      0  // LoraService::sendPacket() no-op success
#define LORA_STUB_RECV      0  // LoraService::receivePacket() always empty

// ============================================================
// INA226 STUBS (no real INA226 chip in Wokwi)
// ============================================================
// Use a potentiometer on ADC pin to simulate Vbus voltage.
// GPIO34 → potentiometer wiper, mapped as 0-15V range.

#define WOKWI_SOLAR_ADC_PIN  34   // ADC1_CH6

// Default: solarOK = true (pump runs)
// Turn pot to 0V to simulate night (pump pauses)

// ============================================================
// SERIAL COMMAND INJECTION (simulate Master broadcasts)
// ============================================================
//
// Type these commands in Wokwi Serial Monitor on slave ESP32s:
//   sched 09:00,5,14:00,5      → Set R1=09:00/5min, R2=14:00/5min
//   manual 4,10                → Manual relay 4, 10 minutes
//   stop                       → STOP ALL
//   time 2026-06-07 09:00:00   → Set RTC time
//   status                     → Print current relay/schedule state

#define WOKWI_SERIAL_CMDS    1  // Enable serial command injection

#endif // WOKWI
#endif // WOKWI_STUBS_H
