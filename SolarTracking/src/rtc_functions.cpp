// --- rtc_functions.cpp --- 
#include "rtc_functions.h" 
#include <Arduino.h> 
#include "lcd_functions.h" 
#include "config.h" 
#include "freertos/semphr.h" 
#include <Wire.h> 
#include "RTClib.h"

// CRITICAL FIX: Array for Day of Week string lookup (Defined here)
const char* dow_str[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

RTC_DS3231 rtc_module;

/**
 * @brief Initializes the DS3231 RTC module.
 */
void initRTC() { 
  Serial.println("[RTC] Attempting to begin...");
  Serial.flush();
  
  // Wokwi simulation bug: sometimes DS1307 takes a moment to respond on I2C bus
  delay(10); 

  if (!rtc_module.begin()) {
    Serial.println("Couldn't find RTC");
    showLCDMessage("RTC Error", "Check Wiring");
    return;
  }

  Serial.println("[RTC] Connected, checking power loss flag...");
  Serial.flush();

  if (rtc_module.lostPower()) {
    showLCDMessage("RTC Lost power", "Setting time..."); 
    Serial.println("RTC lost power, let's set the time!");
    rtc_module.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // RTC is operational — mark valid in system state
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    sysState.rtc_valid = true;
    xSemaphoreGive(stateMutex);
  }
  Serial.println("[RTC] Initialized OK, rtc_valid=TRUE");
} 

// --- MUTEX PROTECTED FUNCTIONS --- 

/**
 * @brief Safely reads the current date and time from the RTC (MUTEX PROTECTED).
 */
RtcDateTime getRtcDateTime() { 
  // We mock the RtcDateTime object to keep compatibility with the old Zoo Code astro math
  // but we pull the real data from the DS3231.
  RtcDateTime safeTime = RtcDateTime(2000, 1, 1, 0, 0, 0); 
  if (stateMutex == NULL) { return safeTime; } 

  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) { 
    DateTime now = rtc_module.now();
    xSemaphoreGive(stateMutex); 
    
    if (now.year() < 2024 || now.day() == 0 || now.month() == 0) { 
        return safeTime; 
    } 

    return RtcDateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()); 
  } else { 
    return safeTime; 
  } 
} 

/**
 * @brief Safely sets the current date and time on the RTC (MUTEX PROTECTED).
 */
void setRtcDateTime(RtcDateTime dateTime) { 
  if (stateMutex == NULL) return; 

  if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) { 
    rtc_module.adjust(DateTime(dateTime.Year(), dateTime.Month(), dateTime.Day(), dateTime.Hour(), dateTime.Minute(), dateTime.Second()));
    xSemaphoreGive(stateMutex); 
  } 
} 

/**
 * @brief Formats the current time for display (HH:MM:SS).
 */
String getDateTimeString() { 
  RtcDateTime now = getRtcDateTime(); 
  char buf[20]; 
  snprintf(buf, sizeof(buf), "%02u:%02u:%02u", now.Hour(), now.Minute(), now.Second()); 
  return String(buf); 
}