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
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "bme280_app.h"
#include "bme280_driver.h"
#include "driver/i2c_master.h"
#include "i2c_interface.h"
#include "inmp441_driver.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "app_main";

/* External variables ------------------------------------------------------------------------------------------------*/
TaskHandle_t xChipInfoHandle = NULL;
TaskHandle_t xBME280Handle = NULL;
TaskHandle_t xINMP441Handle = NULL;

/* Private function declarations -------------------------------------------------------------------------------------*/

/* Private function definitions --------------------------------------------------------------------------------------*/

/* Exported function definitions -------------------------------------------------------------------------------------*/
void vChipInfoTask(void * pvParameters) {
    while(1) {
        /* Print chip information */
        esp_chip_info_t chip_info;
        uint32_t flash_size;
        esp_chip_info(&chip_info);
        printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
               CONFIG_IDF_TARGET,
               chip_info.cores,
               (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
               (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
               (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
               (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

        unsigned major_rev = chip_info.revision / 100;
        unsigned minor_rev = chip_info.revision % 100;
        printf("silicon revision v%d.%d, ", major_rev, minor_rev);
        if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
            printf("Get flash size failed");
            return;
        }

        printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
               (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void vBME280Task(void * pvParameters) {

    i2c_master_bus_handle_t i2c_bus_handle = initializeI2CBus(BME280_SDA_PIN, BME280_SCL_PIN);
    bme280_t * bme280 = NULL;

    ESP_ERROR_CHECK(initializeBME280Device(&bme280, i2c_bus_handle));

    ESP_ERROR_CHECK(setBME280Mode(bme280, BME280_MODE_CYCLE));

    while (1) {
        do {
            vTaskDelay(pdMS_TO_TICKS(1));
        } while(isBME280Sampling(bme280));
        
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }

    removeBME280(bme280);
    i2c_del_master_bus(i2c_bus_handle);
}

void vInitINMP441(void * pvParameters) {
    ESP_LOGI(TAG, "vInitINMP441 task started");
    esp_err_t ret = initINMP441();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "INMP441 initialized successfully in vInitINMP441");
    } else {
        ESP_LOGE(TAG, "Failed to initialize INMP441 in vInitINMP441");
    }
    vTaskDelete(NULL);
}

void app_main(void) {

    ESP_LOGI(TAG, "Starting app");

    ESP_LOGI(TAG, "Creating CHIPINFO task");
    xTaskCreate(vChipInfoTask, "CHIPINFO", 2048, NULL, tskIDLE_PRIORITY + 1, &xChipInfoHandle);

    ESP_LOGI(TAG, "Creating BME280 task");
    xTaskCreate(vBME280Task, "BME280", 8192, NULL, tskIDLE_PRIORITY + 2, &xBME280Handle);

    //ESP_LOGI(TAG, "Creating INMP441 task");
    //xTaskCreate(vInitINMP441, "INMP441", 8192, NULL, tskIDLE_PRIORITY + 3, &xINMP441Handle);

    ESP_LOGI(TAG, "All tasks created");

}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
