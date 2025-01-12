
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
#define I2S_DUPLEX_MODE CONFIG_USE_DUPLEX

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "i2s_driver";

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/
static void i2s_init_std_double(void);
static void i2s_init_std_simple(void);

/* Private function definitions --------------------------------------------------------------------------------------*/
static void i2s_init_std_double(void) {
    i2s_chan_config_t chan_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_config, &tx_channel, &rx_channel));

    i2s_std_config_t std_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_DATA_OUT_PIN,
            .din = I2S_DATA_IN_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_channel, &std_config));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_channel, &std_config));
}

static void i2s_init_std_simple(void) {
    i2s_chan_config_t tx_chan_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_config, &tx_channel, NULL));

    i2s_std_config_t tx_std_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_DATA_OUT_PIN,
            .din = I2S_DATA_IN_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_channel, &tx_std_config));


    i2s_chan_config_t rx_chan_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_config, &rx_channel, NULL));

    i2s_std_config_t rx_std_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_WS_PIN,
            .dout = I2S_DATA_OUT_PIN,
            .din = I2S_DATA_IN_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_channel, &rx_std_config));

    rx_std_config.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;

}


/* Exported function definitions -------------------------------------------------------------------------------------*/
void init_i2s(void)
{
    #if I2S_DUPLEX_MODE
    i2s_init_std_double();
    #else
    i2s_init_std_simple();
    #endif

    ESP_LOGI(TAG, "I2S initialized successfully");
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
