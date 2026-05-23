/**
 * Time_Manager.h - DS3231 RTC Time Management
 */

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>

class TimeManager {
public:
    static bool init();
    static DateTime now();
    static void updateRTC(DateTime dt);
    static void setEpoch(uint32_t epoch);   // Set RTC from Unix epoch
    static String getFormattedTime();
    static bool isRTCValid();
private:
    static RTC_DS3231 rtc;
};

#endif
