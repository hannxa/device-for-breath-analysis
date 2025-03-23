/**
  **********************************************************************************************************************
  * @file    sfm3000_driver.c
  * @brief   This file is the SFM3000 sensor driver implementation
  * @authors liwiaflorkiewicz
  * @date Mar 10, 2025
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include "sfm3000_driver.h"

#include <FreeRTOSConfig.h>
#include <portmacro.h>
#include <string.h>
#include <freertos/projdefs.h>
#include <rom/ets_sys.h>

#include "esp_log.h"
#include "driver/i2c_master.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char *TAG = "sfm3000_driver";

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/

/* Private function definitions --------------------------------------------------------------------------------------*/

/* Exported function definitions -------------------------------------------------------------------------------------*/
/**
  * @brief Initialize the SFM3000 sensor
  * @param dev Pointer to the SFM3000 device structure
  * @param bus_handle Handle to the I2C bus
  * @return esp_err_t ESP_OK on success, otherwise an error code
  */
esp_err_t sfm3000_init(sfm3000_t *dev, i2c_master_bus_handle_t bus_handle) {
  if (dev == NULL || bus_handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  dev->i2c_bus_handle = bus_handle;

  // Configure I2C device
  dev->device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev->device_config.device_address = SFM3000_I2C_ADDRESS;
  dev->device_config.scl_speed_hz = I2C_MASTER_FREQ_HZ;

  // Add device to the I2C bus
  esp_err_t ret = i2c_master_bus_add_device(dev->i2c_bus_handle, &dev->device_config, &dev->i2c_device);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "SFM3000 initialized successfully");
  return ESP_OK;
}

/**
  * @brief Start the measurement process on the SFM3000 sensor
  * @param dev Pointer to the SFM3000 device structure
  * @return esp_err_t ESP_OK on success, otherwise an error code
  */
esp_err_t sfm3000_start_measurement(sfm3000_t *dev) {
  if (dev == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t cmd[2] = {(SFM3000_START_MEASUREMENT_CMD >> 8) & 0xFF, SFM3000_START_MEASUREMENT_CMD & 0xFF};
  esp_err_t ret = i2c_master_transmit(dev->i2c_device, cmd, sizeof(cmd), SFM3000_TIMEOUT_MS / portTICK_PERIOD_MS);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start measurement: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG, "Measurement started successfully");
  return ESP_OK;
}

/**
  * @brief Read the flow rate from the SFM3000 sensor
  * @param dev Pointer to the SFM3000 device structure
  * @param flow Pointer to store the flow rate value
  * @return esp_err_t ESP_OK on success, otherwise an error code
  */
esp_err_t sfm3000_read_flow(sfm3000_t *dev, float *flow) {
  if (dev == NULL || flow == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t buffer[3] = {0};
  esp_err_t ret = i2c_master_receive(dev->i2c_device, buffer, sizeof(buffer), SFM3000_TIMEOUT_MS / portTICK_PERIOD_MS);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read flow: %s", esp_err_to_name(ret));
    return ret;
  }

  // Combine the received bytes into a 16-bit value
  uint16_t raw_flow = (buffer[0] << 8) | buffer[1];

  // Convert raw value to flow rate using the formula: Flow = (raw_flow - OFFSET) / SCALE_FACTOR
  *flow = ((float)raw_flow - SFM3000_OFFSET_FLOW) / SFM3000_SCALE_FACTOR_FLOW;

  ESP_LOGI(TAG, "Flow rate: %.2f", *flow);
  return ESP_OK;
}


/* END OF FILE -------------------------------------------------------------------------------------------------------*/
