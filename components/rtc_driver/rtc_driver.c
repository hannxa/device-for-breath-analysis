#include "rtc_driver.h"
#include <esp_log.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "esp_sntp.h"

static const char *TAG = "RTC_DRIVER";
static rtc_time_t rtc_register;

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

bool set_current_time(int year, int month, int day, int hour, int minute, int second, int milliseconds) {
    struct tm t;
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    t.tm_isdst = -1;  // Not considering daylight saving time

    time_t time = mktime(&t);
    if (time == -1) {
        ESP_LOGE(TAG, "Failed to convert time");
        return false;
    }

    struct timeval tv = { .tv_sec = time, .tv_usec = milliseconds * 1000 }; // Convert milliseconds to microseconds
    if (settimeofday(&tv, NULL) != 0) {
        ESP_LOGE(TAG, "Failed to set system time");
        return false;
    }

    timeval_to_rtc(&tv, &rtc_register);  // Update the rtc_register with the new time
    ESP_LOGI(TAG, "Time set to: %04d-%02d-%02d %02d:%02d:%02d.%03d", year, month, day, hour, minute, second, milliseconds);

    return true;
}

void get_current_time(void) {
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    struct tm t;
    localtime_r(&tv_now.tv_sec, &t);
    int milliseconds = tv_now.tv_usec / 1000;

    ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d.%03d",
              t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, milliseconds);
}