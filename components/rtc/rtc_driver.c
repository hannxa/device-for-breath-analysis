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

    rtc_time->year = t.tm_year + 1900;  // struct tm stores year since 1900
    rtc_time->month = t.tm_mon + 1;     // struct tm stores months 0-11
    rtc_time->day = t.tm_mday;
    rtc_time->hour = t.tm_hour;
    rtc_time->minute = t.tm_min;
    rtc_time->second = t.tm_sec;
    rtc_time->milliseconds = tv->tv_usec / 1000;  // Convert microseconds to milliseconds
}

static void initialize_sntp(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // Czekamy na synchronizację czasu
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    if (retry == retry_count) {
        ESP_LOGI(TAG,"Failed to update system time\n");
    } else {
        ESP_LOGI(TAG,"System time synchronized successfully\n");
    }
}

bool set_current_time(void) {
    initialize_sntp();

    setenv("TZ", "UTC", 1);
    tzset();

    struct timeval tv;
    gettimeofday(&tv, NULL);  // Get the current time
    settimeofday(&tv, NULL);  // Set the system time

    timeval_to_rtc(&tv, &rtc_register);  // Update the rtc_register with the new time
    ESP_LOGI(TAG, "Current time set to system time");

    return true;
}

void get_current_time(void) {
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
    ESP_LOGI(TAG,"Current time in microseconds since the Epoch: %lld\n", time_us);
}