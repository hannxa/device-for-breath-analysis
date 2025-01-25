/**
 **********************************************************************************************************************
  * @file    inmp441_driver.c
  * @brief
  * @authors liwiaflorkiewicz
  * @date Jan 8, 2025
  **********************************************************************************************************************
*/

/* Includes -------------------------------------------------------------------------------------------------*/
#include "inmp441_driver.h"
#include "i2s_driver.h"
#include <esp_log.h>
#include <portmacro.h>
#include <driver/i2s_common.h>

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "inmp441_driver";

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/

/* Private function definitions --------------------------------------------------------------------------------------*/

/* Exported function definitions -------------------------------------------------------------------------------------*/
esp_err_t initINMP441(void) {
    ESP_LOGI(TAG, "Initializing INMP441 sensor");
    esp_err_t ret = init_i2s();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "INMP441 microphone initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to initialize I2S in initINMP441");
    }
    return ret;
}

esp_err_t readINMP441Data(uint8_t *data_out, size_t size)
{
    size_t bytes_read = 0;
    int16_t *temp_buffer = malloc(size * sizeof(int16_t));
    if (!temp_buffer) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        return ESP_ERR_NO_MEM;
    }

    if (rx_channel == NULL) {
        ESP_LOGE(TAG, "RX channel is not initialized");
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(i2s_channel_enable(rx_channel));
    ESP_LOGI(TAG, "Reading INMP441 sensor data");

    for (int i = 0; i < 10; i++) {
        if (i2s_channel_read(rx_channel, temp_buffer, I2S_DMA_BUF_LEN, &bytes_read, portMAX_DELAY) == ESP_OK) {
            ESP_LOGI(TAG, "Read %d bytes", bytes_read);
            printf("Read %d bytes\n", bytes_read);
        }
        else {
            ESP_LOGE(TAG, "Error reading data");
        }
    }

    return ESP_OK;
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
