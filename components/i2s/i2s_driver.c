/**
**********************************************************************************************************************
  * @file    i2s_driver.c
  * @brief   This file is the I2S API implementation
  * @authors liwiaflorkiewicz
  * @date Jan 4, 2025
  **********************************************************************************************************************
*/

/* Includes -------------------------------------------------------------------------------------------------*/
#include "i2s_driver.h"

#include "driver/i2s_types_legacy.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "esp_log.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "i2s_driver";

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/

/* Private function definitions --------------------------------------------------------------------------------------*/
static esp_err_t configure_i2s_pins(void) {
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_DATA_PIN
    };

    return i2s_set_pin(I2S_NUM, &pin_config);
}
/* Exported function definitions -------------------------------------------------------------------------------------*/
void init_i2s() {
    i2s_config_t i2s_config = {
      .mode = I2S_MODE_RX,
      .sample_rate = I2S_SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S,
      .dma_buf_count = I2S_DMA_BUF_COUNT,
      .dma_buf_len = I2S_DMA_BUF_LEN,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0
    };

    esp_err_t err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE("I2S", "Failed installing driver: %s", esp_err_to_name(err));
        return;
    }

    err = configure_i2s_pins();
    if (err != ESP_OK) {
        ESP_LOGE("I2S", "Failed setting pins: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI("I2S", "I2S initialized successfully");

}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
