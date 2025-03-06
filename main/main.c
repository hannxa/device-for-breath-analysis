/**
  **********************************************************************************************************************
  * @file    main.c
  * @brief   This file is the entry file for the device's firmware
  * @authors patrykmonarcha
  * @date Dec 1, 2024
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bme280_app.h"
#include "bme280_driver.h"
#include "driver/i2c_master.h"
#include "i2c_interface.h"
#include "ble_gap.h"
#include "nvs_flash.h"
#include "rtc_driver.h"
#include "ble_gatt.h"
#include "data_storage.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/
#define MEASUREMENTS_FREQUENCY 10 // [Hz]
#define MEASUREMENTS_DELAY_MS (1000 / MEASUREMENTS_FREQUENCY) // [ms]
#define BLE_STREAM_DELAY_MS 500 // [ms]

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "MAIN";

/* External variables ------------------------------------------------------------------------------------------------*/
TaskHandle_t xBME280Handle = NULL;
TaskHandle_t xBLEStreamHandle = NULL;

/* Private function declarations -------------------------------------------------------------------------------------*/
static float temperatureReading = 0;
static float humidityReading = 0;
static float pressureReading = 0;

/* Private function definitions --------------------------------------------------------------------------------------*/

/* Exported function definitions -------------------------------------------------------------------------------------*/
void vBME280Task(void * pvParameters) {

    i2c_master_bus_handle_t i2c_bus_handle = initializeI2CBus(BME280_SDA_PIN, BME280_SCL_PIN);
    bme280_t * bme280 = NULL;

    ESP_ERROR_CHECK(initializeBME280Device(&bme280, i2c_bus_handle));

    ESP_ERROR_CHECK(setBME280Mode(bme280, BME280_MODE_CYCLE));

    while (1) {

        do {
            getBME280Temperature(bme280, &temperatureReading);
            save_temperature(temperatureReading);

            getBME280Humidity(bme280, &humidityReading);
            save_humidity(humidityReading);

            getBME280Pressure(bme280, &pressureReading);
            save_pressure(pressureReading);

        } while(isBME280Sampling(bme280));

        vTaskDelay(MEASUREMENTS_DELAY_MS / portTICK_PERIOD_MS);
    }

    removeBME280(bme280);
    i2c_del_master_bus(i2c_bus_handle);
}

void vBLEStreamTask(void * pvParameters) {

    while (1) {
        send_temperature_notification();
        send_humidity_notification();
        send_pressure_notification();
        send_audio_notification();
        vTaskDelay(BLE_STREAM_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void app_main(void) {

    ESP_LOGI(TAG, "Starting app");

    ESP_LOGI(TAG, "Initializing memory");
    initialize_memory();

    ESP_LOGI(TAG, "Initializing BLE");
    ble_init();

    ESP_LOGI(TAG, "Starting BME280 task");
    xTaskCreate(vBME280Task, "BME280", 8192, NULL, tskIDLE_PRIORITY + 1, &xBME280Handle);
    ESP_LOGI(TAG, "Starting BLE stream task");
    xTaskCreate(vBLEStreamTask, "BLESTREAM", 4096, NULL, tskIDLE_PRIORITY, &xBLEStreamHandle);

}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
