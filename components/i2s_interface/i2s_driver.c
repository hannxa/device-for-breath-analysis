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
#include "driver/gpio.h"
#include "esp_log.h"
#include <driver/i2s_std.h>

#include <driver/i2s_types.h>
#include <driver/i2s_common.h>
#include <driver/i2s_types_legacy.h>

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/
#define I2S_PIN_NO_CHANGE (-1)
#define BUFFER_SIZE 1024
/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "i2s_driver";
i2s_chan_handle_t rx_channel = NULL;
static int16_t temp_buffer[BUFFER_SIZE];

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/
static void i2s_init_std_single(void);

/* Private function definitions --------------------------------------------------------------------------------------*/
static void i2s_init_std_single(void) {

    i2s_chan_config_t rx_chan_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_config, &rx_channel, NULL));

    i2s_std_config_t rx_std_config = {
        .clk_cfg = {
            .sample_rate_hz = 16000,//I2S_SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_384,
        },
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DATA_IN_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_channel, &rx_std_config));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_channel));

    ESP_LOGI(TAG, "I2S RX channel initialized and enabled");

}


/* Exported function definitions -------------------------------------------------------------------------------------*/
esp_err_t init_i2s_rx(void)
{
    i2s_chan_config_t rx_chan_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_SLAVE);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_config, &rx_channel, NULL));

    i2s_std_config_t rx_std_config = {
        .clk_cfg = {
            .sample_rate_hz = I2S_SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_384,
        },
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din = I2S_DATA_IN_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_channel, &rx_std_config));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_channel));

    ESP_LOGI(TAG, "I2S RX channel initialized and enabled");
    return ESP_OK;
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
