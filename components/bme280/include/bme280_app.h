/**
  **********************************************************************************************************************
  * @file    bme280_app.h
  * @brief   This file is the header file for BME280 sensor API
  * @authors patrykmonarcha
  * @date Dec 3, 2024
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef _BME280_APP_H_
#define _BME280_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -------------------------------------------------------------------------------------------------*/
#include "driver/i2c_types.h"
#include "bme280_driver.h"

/* Types ----------------------------------------------------------------------------------------------------*/

/* Constants ------------------------------------------------------------------------------------------------*/

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/

/* Functions ------------------------------------------------------------------------------------------------*/
/*
 * @function initializeBME280Device
 *
 * @abstract This function initializes BME280 sensor
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] i2c_bus_handle: I2C bus handle
 *
 * @return None
 */
esp_err_t initializeBME280Device(bme280_t ** bme280, i2c_master_bus_handle_t i2c_bus_handle);

#ifdef __cplusplus
}
#endif

#endif // _BME280_APP_H_

/*
 * @function getBME280Humidity
 *
 * @abstract This function reads BME280 sensor humidity
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[out] pressure: humidity value
 *
 * @return
 *    - esp_err_t status code
*/
esp_err_t getBME280Humidity(bme280_t * bme280, float *humidity);

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
