/**
  **********************************************************************************************************************
  * @file rtc_driver.h
  * @brief This file is the header file for RTC driver
  * @author hannabaranowska
  * @date January 06, 2025
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#ifdef _cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------------------------------------------------*/
#include <stdint.h>

/* Types -------------------------------------------------------------------------------------------------------------*/
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

/* Constants ---------------------------------------------------------------------------------------------------------*/
#define ADJUST_REASON 0xE0

/* Macros ------------------------------------------------------------------------------------------------------------*/

/* Variables ---------------------------------------------------------------------------------------------------------*/

/* Functions ---------------------------------------------------------------------------------------------------------*/
/*
 * @function set_time
 *
 * @abstract Sets the system time to the time provided in the 10-byte payload.
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
 * @return None
 */
void set_time(const uint8_t payload[10]);

/*
 * @function get_time
 *
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
uint8_t* get_time();

#endif //RTC_DRIVER_H

/* END OF FILE -------------------------------------------------------------------------------------------------------*/

