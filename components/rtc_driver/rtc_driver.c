/**
**********************************************************************************************************************
  * @file    rtc_driver.c
  * @brief   This file is the RTC driver API implementation
  * @authors hannabaranowska
  * @date    January 06, 2025
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include <esp_log.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "rtc_driver.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char *TAG = "RTC_DRIVER";
static rtc_time_t rtc_register;

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/
/*
 * @function timeval_to_rtc
 *
 * @abstract Converts struct timeval to rtc_time_t structure
 *
 * @param[in] tv: Input timeval structure
 *
 * @param[out] rtc_time: Output RTC time structure
 *
 * @return None
 */
static void timeval_to_rtc(const struct timeval *tv, rtc_time_t *rtc_time);

/*
 * @function set_current_time
 *
 * @abstract Sets the current RTC time
 *
 * @param[in] year, month, day, hour, minute, second, milliseconds: Date and time values
 *
 * @return None
 */
static void set_current_time(int year, int month, int day, int hour, int minute, int second, int milliseconds);

/*
 * @function get_current_time
 *
 * @abstract Retrieves the current RTC time
 *
 * @param[out] year, month, day, hour, minute, second, milliseconds, weekday: Date and time values
 *
 * @return None
 */
static void get_current_time(int *year, int *month, int *day, int *hour, int *minute, int *second, int *milliseconds, int *weekday);

/* Private function definitions --------------------------------------------------------------------------------------*/
static void timeval_to_rtc(const struct timeval *tv, rtc_time_t *rtc_time) {
    struct tm t;
    localtime_r(&tv->tv_sec, &t);

    rtc_time->year = t.tm_year + 1900;
    rtc_time->month = t.tm_mon + 1;
    rtc_time->day = t.tm_mday;
    rtc_time->hour = t.tm_hour;
    rtc_time->minute = t.tm_min;
    rtc_time->second = t.tm_sec;
    rtc_time->milliseconds = tv->tv_usec / 1000;
}

static void set_current_time(int year, int month, int day, int hour, int minute, int second, int milliseconds) {
    struct tm t;
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    t.tm_isdst = -1;  // Not considering daylight saving time

    ESP_LOGI(TAG, "Setting time: %d-%d-%d %d:%d:%d.%d", year, month, day, hour, minute, second, milliseconds);

    time_t time = mktime(&t);
    if (time == -1) {
        ESP_LOGE(TAG, "Failed to convert time");
    }

    struct timeval tv = { .tv_sec = time, .tv_usec = milliseconds * 1000 };
    if (settimeofday(&tv, NULL) != 0) {
        ESP_LOGE(TAG, "Failed to set system time");
    }

    timeval_to_rtc(&tv, &rtc_register);
}

static void get_current_time(int *year, int *month, int *day, int *hour, int *minute, int *second, int *milliseconds, int *weekday) {
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    struct tm t;
    localtime_r(&tv_now.tv_sec, &t);
    *milliseconds = tv_now.tv_usec / 1000;

    *year = t.tm_year + 1900;
    *month = t.tm_mon + 1;
    *day = t.tm_mday;
    *hour = t.tm_hour;
    *minute = t.tm_min;
    *second = t.tm_sec;
    *weekday = t.tm_wday;
}
void set_time(const uint8_t payload[10]) {
    int year = payload[0] | (payload[1] << 8);
    int month = payload[2];
    int day = payload[3];
    int hour = payload[4];
    int minute = payload[5];
    int second = payload[6];
    int milliseconds = payload[8];

    set_current_time(year, month, day, hour, minute, second, milliseconds);
}


uint8_t* get_time(void) {
    static uint8_t payload[10];
    int year, month, day, hour, minute, second, milliseconds, weekday;
    get_current_time(&year, &month, &day, &hour, &minute, &second, &milliseconds, &weekday);

    payload[0] = year & 0xFF;
    payload[1] = (year >> 8) & 0xFF;
    payload[2] = month;
    payload[3] = day;
    payload[4] = hour;
    payload[5] = minute;
    payload[6] = second;
    payload[7] = weekday;
    payload[8] = milliseconds;
    payload[9] = ADJUST_REASON;

    return payload;
}
