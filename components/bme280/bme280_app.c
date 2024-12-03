/**
  **********************************************************************************************************************
  * @file    bme280_app.c
  * @brief   This file is the BME280 sensor API implementation
  * @authors patrykmonarcha
  * @date Dec 3, 2024
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include "bme280_driver.h"
#include "bme280_app.h"
#include "esp_log.h"
#include "driver/i2c_types.h"
#include "driver/i2c_master.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "bme280_app";

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/
/*
 * @function convertReadTemperatureToFloat
 *
 * @abstract This function converts received sensor temperature to float value in degree Celcius
 *
 * @param[in] temperature_in: Input temperature
 *
 * @param[out] temperature_out: Temperature in degree Celcius
 *
 * @return None
 */
static void convertReadTemperatureToFloat(int32_t * temperature_in, float * temperature_out);

/*
 * @function convertReadPressureToFloat
 *
 * @abstract This function converts received sensor pressure to float value in Pa
 *
 * @param[in] pressure_in: Input pressure
 *
 * @param[out] pressure_out: Pressure in Pa
 *
 * @return None
 */
static void convertReadPressureToFloat(uint32_t * pressure_in, float * pressure_out);

/*
 * @function convertReadHumidityToFloat
 *
 * @abstract This function converts received sensor humidity to float value in %RH
 *
 * @param[in] humidity_in: Input humidity
 *
 * @param[out] humidity_out: Humidity in %RH
 *
 * @return None
 */
static void convertReadHumidityToFloat(uint32_t * humidity_in, float * humidity_out);

/* Private function definitions --------------------------------------------------------------------------------------*/
static void convertReadTemperatureToFloat(int32_t * temperature_in, float * temperature_out) {
    *temperature_out = (float)*temperature_in * 0.01f;
}

static void convertReadPressureToFloat(uint32_t * pressure_in, float * pressure_out) {
    *pressure_out = (float)*pressure_in * (1.0f/256.0f);}

static void convertReadHumidityToFloat(uint32_t * humidity_in, float * humidity_out) {
    *humidity_out = (*humidity_in == UINT32_MAX) ? -1.0f : (float)*humidity_in * (1.0f/1024.0f);
}

/* Exported function definitions -------------------------------------------------------------------------------------*/
esp_err_t initializeBME280Device(bme280_t ** bme280, i2c_master_bus_handle_t i2c_bus_handle) {
    *bme280 = createBME280Instance(i2c_bus_handle);
    if (!*bme280) {
        ESP_LOGE(TAG, "Creating BME280 driver failed");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(initializeBME280(*bme280));
    bme280_config_t bme_cfg = BME280_DEFAULT_CONFIG;
    ESP_ERROR_CHECK(configureBME280(*bme280, &bme_cfg));

    return ESP_OK;
}


/* END OF FILE -------------------------------------------------------------------------------------------------------*/
