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
  } sfm3000_dev_t;

/* Constants ------------------------------------------------------------------------------------------------*/
#define SFM3000_I2C_ADDRESS         0x40       /* SFM3000 I2C address */

#define I2C_MASTER_SCL_IO           9        /* GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO           8        /* GPIO number for I2C master data */
#define I2C_MASTER_FREQ_HZ          400000    /* I2C master clock frequency */
#define I2C_MASTER_TIMEOUT_MS       1000      /* I2C master timeout in milliseconds */

#define SFM3000_READ_CMD            0x1000    /* SFM3000 read command */
#define SFM3000_OFFSET_FLOW         32000      /* Offset flow value */
#define SFM3000_SCALE_FACTOR_FLOW   140.0f     /* Scale factor for flow rate conversion */

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/

/* Functions ------------------------------------------------------------------------------------------------*/
  esp_err_t sfm3000_init(i2c_master_bus_handle_t i2c_bus_handle);
  esp_err_t sfm3000_read_flow_rate(float *flow_rate);
  esp_err_t initializeSFM3000Device(sfm3000_dev_t ** sfm3000, i2c_master_bus_handle_t i2c_bus_handle);

#ifdef __cplusplus
}
#endif

#endif
/* END OF FILE -------------------------------------------------------------------------------------------------------*/