/**
  **********************************************************************************************************************
  * @file    sfm3000_driver.h
  * @brief   This file is the header file for SFM3000 sensor driver
  * @authors liwiaflorkiewicz
  * @date Mar 10, 2025
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef SFM3000_DRIVER_H
#define SFM3000_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -------------------------------------------------------------------------------------------------*/
#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

/* Types ----------------------------------------------------------------------------------------------------*/
  typedef struct {
    i2c_master_dev_handle_t i2c_device;                      /* I2C port number */
    uint8_t i2c_id_address;                      /* I2C device address */
    i2c_master_bus_handle_t i2c_bus_handle;
    i2c_device_config_t device_config;
    uint8_t chip_id;
  } sfm3000_t;

/* Constants ------------------------------------------------------------------------------------------------*/
#define SFM3000_I2C_ADDRESS         0x40       /* SFM3000 I2C address */
#define SFM3000_SDA_PIN             8
#define SFM3000_SCL_PIN             9

#define I2C_MASTER_FREQ_HZ          400000    /* I2C master clock frequency */
#define SFM3000_TIMEOUT_MS          1000      /* I2C master timeout in milliseconds */

#define SFM3000_READ_CMD            0x1000    /* SFM3000 read command */
#define SFM3000_OFFSET_FLOW         32000      /* Offset flow value */
#define SFM3000_SCALE_FACTOR_FLOW   142.8f     /* Scale factor for flow rate conversion */
#define SFM3000_START_MEASUREMENT_CMD 0x1000

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/

/* Functions ------------------------------------------------------------------------------------------------*/
  esp_err_t sfm3000_init(sfm3000_t *dev, i2c_master_bus_handle_t bus_handle);
  esp_err_t sfm3000_start_measurement(sfm3000_t *dev);
  esp_err_t sfm3000_read_flow(sfm3000_t *dev, float *flow);

#ifdef __cplusplus
}
#endif

#endif
/* END OF FILE -------------------------------------------------------------------------------------------------------*/