/**
 **********************************************************************************************************************
  * @file    inmp441_driver.c
  * @brief
  * @authors liwiaflorkiewicz
  * @date Jan 8, 2025
  **********************************************************************************************************************
*/

/* Includes -------------------------------------------------------------------------------------------------*/
#include "i2s_interface.h"
#include "inmp441_driver.h"
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
static esp_err_t initINMP441(void);
static esp_err_t readINMP441Data(uint8_t *data_out, size_t size);

/* Private function definitions --------------------------------------------------------------------------------------*/
static esp_err_t initINMP441(void) {
    ESP_LOGI(TAG, "Initializing INMP441 sensor");
    init_i2s();
    ESP_LOGI(TAG, "INMP441 microphone initialized successfully");
    return ESP_OK;
}

static esp_err_t readINMP441Data(uint8_t *data_out, size_t size)
{
    size_t bytes_read = 0;
    int16_t temp_buffer[size];

    ESP_ERROR_CHECK(i2s_channel_enable(rx_channel));

    while (1) {
        if (i2s_channel_read(rx_channel, temp_buffer, I2S_DMA_BUF_LEN, &bytes_read, portMAX_DELAY) == ESP_OK) {
            ESP_LOGI(TAG, "Read %d bytes", bytes_read);
            printf("Read %d bytes\n", bytes_read);
        }
        else {
            ESP_LOGE(TAG, "Error reading data");
        }
    }

    ESP_LOGI(TAG, "Reading INMP441 sensor data");
    return ESP_OK;
}


/* Exported function definitions -------------------------------------------------------------------------------------*/


/* END OF FILE -------------------------------------------------------------------------------------------------------*/
