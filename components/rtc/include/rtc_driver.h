//
// Created by Hanna Baranowska on 10.01.2025.
//

#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

//UUID for Current Time Service and Characteristics
#define CURRENT_TIME_SERVICE_UUID 0x1805
#define CURRENT_TIME_CHARACTERISTIC_UUID 0x2A2B

#define ADJUST_REASON 0xE0

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day_of_week;
    uint8_t milliseconds;
    uint8_t adjust_reason;
} rtc_time_t;

bool set_current_time();
void get_current_time();
#endif //RTC_DRIVER_H
