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
#include "sfm3000_driver.h"

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
TaskHandle_t xSFM3000Handle = NULL;

/* Private function declarations -------------------------------------------------------------------------------------*/
static float temperatureReading = 0;
static float humidityReading = 0;
static float pressureReading = 0;
static float flowRateReading = 0;

/* Private function definitions --------------------------------------------------------------------------------------*/

/* Exported function definitions -------------------------------------------------------------------------------------*/
void vBME280Task(void * pvParameters) {

    //i2c_master_bus_handle_t i2c_bus_handle = initializeI2CBus(BME280_SDA_PIN, BME280_SCL_PIN);
    i2c_master_bus_handle_t i2c_bus_handle = (i2c_master_bus_handle_t)pvParameters;
    bme280_t * bme280 = NULL;

    // Initialize the BME280 sensor
    esp_err_t ret = initializeBME280Device(&bme280, i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BME280 sensor");
        vTaskDelete(NULL);  // Delete the task if initialization fails
    }
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

/**
  * @brief Task to read data from the SFM3000 sensor.
  */
void vSFM3000Task(void * pvParameters) {
    /*i2c_master_bus_handle_t i2c_bus_handle = (i2c_master_bus_handle_t)pvParameters;

    // Initialize the SFM3000 sensor
    esp_err_t ret = sfm3000_init(i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SFM3000 sensor");
        vTaskDelete(NULL);  // Delete the task if initialization fails
    }

    while (1) {
        // Read the flow rate from the SFM3000 sensor
        ret = sfm3000_read_flow_rate(&flowRateReading);
        if (ret == ESP_OK) {
            printf("Flow rate: %.2f slm\n", flowRateReading);
            ESP_LOGI(TAG, "Flow rate: %.2f slm", flowRateReading);
        } else {
            ESP_LOGE(TAG, "Failed to read flow rate from SFM3000");
        }

        vTaskDelay(MEASUREMENTS_DELAY_MS / portTICK_PERIOD_MS);
    }*/

    i2c_master_bus_handle_t i2c_bus_handle = (i2c_master_bus_handle_t)pvParameters;
    sfm3000_dev_t * sfm3000 = NULL;

    // Initialize the SFM3000 sensor
    esp_err_t ret = initializeSFM3000Device(&sfm3000, i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SFM3000 sensor");
        vTaskDelete(NULL);  // Delete the task if initialization fails
    }

    while (1) {
        // Read the flow rate from the SFM3000 sensor
        ret = sfm3000_read_flow_rate(&flowRateReading);
        if (ret == ESP_OK) {
            printf("Flow rate: %.2f slm\n", flowRateReading);
            ESP_LOGI(TAG, "Flow rate: %.2f slm", flowRateReading);
        } else {
            ESP_LOGE(TAG, "Failed to read flow rate from SFM3000");
        }

        vTaskDelay(MEASUREMENTS_DELAY_MS / portTICK_PERIOD_MS);
    }

}

void app_main(void) {

    ESP_LOGI(TAG, "Starting app");

    ESP_LOGI(TAG, "Initializing memory");
    initialize_memory();

    ESP_LOGI(TAG, "Initializing BLE");
    ble_init();

    // Initialize I2C bus
    i2c_master_bus_handle_t i2c_bus_handle = initializeI2CBus(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    if (i2c_bus_handle == NULL) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus");
        return;
    }
    ESP_LOGI(TAG, "I2C bus initialized");

    ESP_LOGI(TAG, "Starting BME280 task");
    xTaskCreate(vBME280Task, "BME280", 8192, i2c_bus_handle, tskIDLE_PRIORITY + 1, &xBME280Handle);

    ESP_LOGI(TAG, "Starting BLE stream task");
    xTaskCreate(vBLEStreamTask, "BLESTREAM", 4096, NULL, tskIDLE_PRIORITY, &xBLEStreamHandle);

    // Start the SFM3000 task
    ESP_LOGI(TAG, "Starting SFM3000 task");
    //xTaskCreate(vSFM3000Task, "SFM3000", 4096, i2c_bus_handle, tskIDLE_PRIORITY + 2, &xSFM3000Handle);
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
