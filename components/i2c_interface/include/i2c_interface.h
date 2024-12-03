/**
  **********************************************************************************************************************
  * @file    i2c_interface.h
  * @brief   This file is the header file for I2C API
  * @authors patrykmonarcha
  * @date Dec 3, 2024
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef _I2C_INTERFACE_H_
#define _I2C_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -------------------------------------------------------------------------------------------------*/
#include "driver/i2c_types.h"

/* Types ----------------------------------------------------------------------------------------------------*/

/* Constants ------------------------------------------------------------------------------------------------*/

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/

/* Functions ------------------------------------------------------------------------------------------------*/
/*
 * @function initializeI2CBus
 *
 * @abstract This function Initialized I2C bus with given parameters
 *
 * @param[in] sda_pin: SDA GPIO pin number
 *
 * @param[in] scl_pin: SCL GPIO pin number
 *
 * @return i2c bus handle
 */
i2c_master_bus_handle_t initializeI2CBus(uint8_t sda_pin, uint8_t scl_pin);

#ifdef __cplusplus
}
#endif

#endif // _I2C_INTERFACE_H_

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
