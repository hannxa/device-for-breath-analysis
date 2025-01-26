/**
 ****************************************************************************************
 *@file rtc_driver.h
 *@brief This file is the header file for RTC driver
 *@author Hanna Baranowska
 *@date January 06, 2025
 ****************************************************************************************
 */

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

/**
 * @function set_time
 * @abstract Sets the current system time based on the provided 10-byte payload.
 *
 * @param[in] payload A 10-byte array containing the time formatted as follows:
 *            - Byte 0: Lowest 8 bits of the year
 *            - Byte 1: Highest 8 bits of the year
 *            - Byte 2: Month
 *            - Byte 3: Day
 *            - Byte 4: Hour
 *            - Byte 5: Minute
 *            - Byte 6: Second
 *            - Byte 7: Day of the week
 *            - Byte 8: Milliseconds
 *            - Byte 9: Adjust reason
 *
 * @return void
 */
void set_time();

/**
 * @function get_time
 * @abstract Retrieves the current system time and stores it in a 10-byte payload.
 *
 * @return uint8_t* Pointer to a 10-byte array containing the time formatted as follows:
 *         - Byte 0: Lowest 8 bits of the year
 *         - Byte 1: Highest 8 bits of the year
 *         - Byte 2: Month
 *         - Byte 3: Day
 *         - Byte 4: Hour
 *         - Byte 5: Minute
 *         - Byte 6: Second
 *         - Byte 7: Day of the week
 *         - Byte 8: Milliseconds
 *         - Byte 9: Adjust reason
 */
uint8_t get_time();
#endif //RTC_DRIVER_H
