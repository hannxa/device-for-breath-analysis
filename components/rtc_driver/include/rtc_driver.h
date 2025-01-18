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

/*
 * @function set_current_time
 *
 * @abstract This function sets the current system time to the specified date and time.
 *
 * @param[in] year: The year to set (e.g., 2025)
 *
 * @param[in] month: The month to set (1-12)
 *
 * @param[in] day: The day to set (1-31)
 *
 * @param[in] hour: The hour to set (0-23)
 *
 * @param[in] minute: The minute to set (0-59)
 *
 * @param[in] second: The second to set (0-59)
 *
 * @param[in] milliseconds The milliseconds to set (0-999)
 *
 * @return true if the time was successfully set, false otherwise
 */
bool set_current_time();

/*
 * @function get_current_time
 *
 * @abstract This function gets the current system time.
 *
 * @return void
 */
void get_current_time();
#endif //RTC_DRIVER_H
