// --- rtc_functions.h --- 
#ifndef RTC_FUNCTIONS_H 
#define RTC_FUNCTIONS_H 
#include <Arduino.h> 

// We provide a mock RtcDateTime struct here so we don't have to rewrite 
// the massive NOAA solar math algorithm that Zoo Code wrote using it.
class RtcDateTime {
public:
    uint16_t year;
    uint8_t month, day, hour, minute, second;

    RtcDateTime() : year(2000), month(1), day(1), hour(0), minute(0), second(0) {}
    RtcDateTime(uint16_t y, uint8_t m, uint8_t d, uint8_t h, uint8_t min, uint8_t s)
        : year(y), month(m), day(d), hour(h), minute(min), second(s) {}

    uint16_t Year() const { return year; }
    uint8_t Month() const { return month; }
    uint8_t Day() const { return day; }
    uint8_t Hour() const { return hour; }
    uint8_t Minute() const { return minute; }
    uint8_t Second() const { return second; }
};

// 1. EXTERN THE DAY OF WEEK ARRAY (Defined in rtc_functions.cpp)
extern const char* dow_str[]; 

// 2. PROTOTYPE THE FUNCTIONS (Critical: ensures initRTC is declared)
void initRTC(); 
RtcDateTime getRtcDateTime(); 
void setRtcDateTime(RtcDateTime dateTime); 
void updateRTCfromNTP(); 
String getDateTimeString();

#endif