/**
 * Time_Manager.cpp - DS3231 RTC Implementation
 */

#include "Time_Manager.h"

RTC_DS3231 TimeManager::rtc;

bool TimeManager::init() {
    if (!rtc.begin()) {
        Serial.println(F("[RTC] DS3231 NOT FOUND!"));
        return false;
    }
    if (rtc.lostPower()) {
        Serial.println(F("[RTC] Lost power - setting compile time"));
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    Serial.printf("[RTC] Time: %s\n", getFormattedTime().c_str());
    return true;
}

DateTime TimeManager::now() {
    return rtc.now();
}

void TimeManager::updateRTC(DateTime dt) {
    rtc.adjust(dt);
}

void TimeManager::setEpoch(uint32_t epoch) {
    rtc.adjust(DateTime(epoch));
}

String TimeManager::getFormattedTime() {
    DateTime n = rtc.now();
    char buf[20];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            n.year(), n.month(), n.day(),
            n.hour(), n.minute(), n.second());
    return String(buf);
}

bool TimeManager::isRTCValid() {
    DateTime n = rtc.now();
    return (n.year() >= 2024);
}
