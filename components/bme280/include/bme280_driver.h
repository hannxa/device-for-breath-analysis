/**
  **********************************************************************************************************************
  * @file    bme280_driver.h
  * @brief   This file is the header file for BME280 sensor driver
  * @authors patrykmonarcha
  * @date Dec 2, 2024
  **********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef _BME280_DRIVER_H_
#define _BME280_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes -------------------------------------------------------------------------------------------------*/
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include "sdkconfig.h"
#include "bme280_bits.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Types ----------------------------------------------------------------------------------------------------*/

/* Constants ------------------------------------------------------------------------------------------------*/
#define BME280_I2C_CLK_SPEED_HZ 1000000
#define BME280_TIMEOUT 5000
#define BME280_DEVICE_ADDRESS 0x76
#define BME280_DEVICE_ALTERNATIVE_ADDRESS 0x77
#define BME280_SDA_PIN 8
#define BME280_SCL_PIN 9

/* Macros ---------------------------------------------------------------------------------------------------*/

/* Variables ------------------------------------------------------------------------------------------------*/
typedef struct bme280_t bme280_t;

/* Functions ------------------------------------------------------------------------------------------------*/
/*
 * @function createBME280Instance
 *
 * @abstract This function creates BME280 sensor instance
 *
 * @param[in] i2c_bus_handle: I2C bus handle
 *
 * @return BME280 instance
 */
bme280_t * createBME280Instance(i2c_master_bus_handle_t i2c_bus_handle);

/*
 * @function removeBME280
 *
 * @abstract This function removes BME280 sensor instance
 *
 * @param[in] bme280: BME280 instance
 *
 * @return None
 */
void removeBME280(bme280_t * bme280);

/*
 * @function initializeBME280
 *
 * @abstract This function initializes BME280 sensor and reads it calibration data
 *
 * @param[in] bme280: BME280 instance
 *
 * @return
 *      - esp_err_t status code
 */
esp_err_t initializeBME280(bme280_t * bme280);

/*
 * @function configureBME280
 *
 * @abstract This function configures BME280 sensor
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] config: BME280 configuration
 *
 * @return
 *      - esp_err_t status code
 */
esp_err_t configureBME280(bme280_t * bme280, bme280_config_t * config);
 /*
 * @function readBME280Humidity
 *
 * @abstract This function reads BME280 sensor raw humidity value
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[out] humidity: raw humidity value
 *
 * @return
 *    - esp_err_t status code
 */
 esp_err_t readBME280Humidity(bme280_t *bme280, uint32_t *humidity);

/*
 * @function setBME280Mode
 *
 * @abstract This function sets BME280 sensor mode
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] mode: Chosen operation mode
 *
 * @return
 *      - esp_err_t status code
 */
esp_err_t setBME280Mode(bme280_t * bme280, bme280_mode_t mode);

/*
 * @function isBME280Sampling
 *
 * @abstract This function checks if BME280 sensor is sampling environment conditions
 *
 * @param[in] bme280: BME280 instance
 *
 * @return
 *      - true: On success
 *      - false: On fail
 */
bool isBME280Sampling(bme280_t * bme280);

/*
 * @function compensateBME280Temperature
 *
 * @abstract This function calculates final temperature value
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] input_temperature: Raw temperature value
 *
 * @return Temperature value
 */
int32_t compensateBME280Temperature(bme280_t * bme280, int32_t input_temperature);

/*
 * @function compensateBME280Pressure
 *
 * @abstract This function calculates final pressure value
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] input_pressure: Raw pressure value
 *
 * @return Pressure value
 */
uint32_t compensateBME280Pressure(bme280_t * bme280, int32_t input_pressure);

/*
 * @function compensateBME280Humidity
 *
 * @abstract This function calculates final humidity value
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] input_humidity: Raw humidity value
 *
 * @return Humidity value
 */
uint32_t compensateBME280Humidity(bme280_t * bme280, int32_t input_humidity);

#ifdef __cplusplus
}
#endif

#endif // _BME280_DRIVER_H_

/* END OF FILE -------------------------------------------------------------------------------------------------------*/