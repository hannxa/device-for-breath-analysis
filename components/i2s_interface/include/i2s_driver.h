/**
  *********************************************************************************************************************
  * @file    i2s_driver.h
  * @brief   This file is the I2S API implementation
  * @authors liwiaflorkiewicz
  * @date Jan 4, 2025
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef I2S_DRIVER_H
#define I2S_DRIVER_H

/* Includes -------------------------------------------------------------------------------------------------*/
#include "driver/i2s_std.h"
#include "esp_log.h"

/* External variables ------------------------------------------------------------------------------------------------*/
#define I2S_NUM         (0)
#define I2S_SAMPLE_RATE (8000)
#define I2S_CHANNEL_NUM (2)
#define I2S_BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define I2S_DMA_BUF_COUNT (8)
#define I2S_DMA_BUF_LEN   (1024)

#define I2S_BCK_PIN          GPIO_NUM_5          // Pin BCK (Bit Clock)
#define I2S_WS_PIN           GPIO_NUM_4          // Pin WS (Word Select)
#define I2S_DATA_IN_PIN         GPIO_NUM_19          // Pin DATA in

extern i2s_chan_handle_t tx_channel;
extern i2s_chan_handle_t rx_channel;

/* Types ----------------------------------------------------------------------------------------------------*/

/* Constants ------------------------------------------------------------------------------------------------*/

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/

/* Functions ------------------------------------------------------------------------------------------------*/
/**
 * @function init_i2s
 *
 * @abstract This function initializes I2S interface
 *
 * @return None
 */
esp_err_t init_i2s(void);

#endif //I2S_DRIVER_H

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
