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

#include <string.h>

#include "esp_log.h"
#include "driver/i2c_master.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char *TAG = "sfm3000_driver";
static i2c_master_dev_handle_t dev_handle;

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/
static esp_err_t createDeviceSFM3000(sfm3000_dev_t *sfm3000, const uint16_t device_address);
static esp_err_t sfm3000_read_raw_data(uint16_t *raw_data);
static esp_err_t checkForSFM3000ChipID(sfm3000_dev_t *sfm3000);
static esp_err_t lookForSFM3000Sensor(sfm3000_dev_t *sfm3000);

/* Private function definitions --------------------------------------------------------------------------------------*/
static esp_err_t createDeviceSFM3000(sfm3000_dev_t *sfm3000, const uint16_t device_address) {
  ESP_LOGD(TAG, "Creating SFM3000 device at address 0x%2X", device_address);
  sfm3000->i2c_id_address = device_address;

  i2c_device_config_t dev_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = device_address,
    .scl_speed_hz = I2C_MASTER_FREQ_HZ,
  };

  esp_err_t error = i2c_master_bus_add_device(sfm3000->i2c_bus_handle, &dev_config, &sfm3000->i2c_device);
  if (error == ESP_OK) {
    ESP_LOGD(TAG, "Device SFM3000 successfully created at address 0x%2X", device_address);
    return error;
  }
  else {
    ESP_LOGE(TAG, "Error creating device SFM3000 at address 0x%2X", device_address);
    return error;
  }
}

static esp_err_t sfm3000_read_raw_data(uint16_t *raw_data) {
  uint8_t data[2];
  uint8_t read_cmd[2] = {
    (SFM3000_READ_CMD >> 8) & 0xFF,
    SFM3000_READ_CMD & 0xFF
};

  esp_err_t ret = i2c_master_transmit(dev_handle, read_cmd, sizeof(read_cmd), -1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to send read command: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = i2c_master_receive(dev_handle, data, sizeof(data), -1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read data: %s", esp_err_to_name(ret));
    return ret;
  }

  *raw_data = (data[0] << 8) | data[1];
  ESP_LOGI(TAG, "Raw data read: 0x%04X", *raw_data);

  return ESP_OK;
}

static esp_err_t checkForSFM3000ChipID(sfm3000_dev_t *sfm3000) {
  uint16_t chip_id;
  esp_err_t error = sfm3000_read_raw_data(&chip_id);
  if (error != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read chip ID: %s", esp_err_to_name(error));
    return error;
  }

  if (chip_id != SFM3000_I2C_ADDRESS) {
    ESP_LOGE(TAG, "Invalid chip ID: 0x%02X", chip_id);
    return ESP_ERR_INVALID_RESPONSE;
  }

  ESP_LOGI(TAG, "SFM3000 chip ID: 0x%02X", chip_id);
  return ESP_OK;
}

static esp_err_t lookForSFM3000Sensor(sfm3000_dev_t *sfm3000) {
  ESP_LOGD(TAG, "Looking for SFM3000 sensor on I2C line");

  esp_err_t error = createDeviceSFM3000(sfm3000, SFM3000_I2C_ADDRESS);
  if (error != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create SFM3000 device: %s", esp_err_to_name(error));
    return error;
  }

  error = checkForSFM3000ChipID(sfm3000);
  if (error != ESP_OK) {
    ESP_LOGE(TAG, "Failed to check SFM3000 chip ID: %s", esp_err_to_name(error));
    return error;
  }

  return ESP_OK;
}

/* Exported function definitions -------------------------------------------------------------------------------------*/
esp_err_t sfm3000_read_flow_rate(float *flow_rate) {
  uint16_t raw_data;

  esp_err_t ret = sfm3000_read_raw_data(&raw_data);
  if (ret != ESP_OK) {
    return ret;
  }

  // Convert raw data to flow rate
  *flow_rate = ((float)raw_data - SFM3000_OFFSET_FLOW) / SFM3000_SCALE_FACTOR_FLOW;

  ESP_LOGI(TAG, "Flow rate: %.2f slm", *flow_rate);
  return ESP_OK;
}

sfm3000_dev_t * createSFM3000Instance(i2c_master_bus_handle_t i2c_bus_handle) {
  sfm3000_dev_t * sfm3000 = malloc(sizeof(sfm3000_dev_t));

  if (sfm3000) {
    memset(sfm3000, 0, sizeof(sfm3000_dev_t));
    sfm3000->i2c_bus_handle = i2c_bus_handle;
    sfm3000->device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    sfm3000->device_config.device_address = SFM3000_I2C_ADDRESS;
    sfm3000->device_config.scl_speed_hz = I2C_MASTER_FREQ_HZ;
    sfm3000->i2c_device = NULL;
  } else {
    ESP_LOGE(TAG, "Failed allocating memory for SFM3000 instance");
    free(sfm3000);
    return NULL;
  }

  return sfm3000;
}

esp_err_t initializeSFM3000Device(sfm3000_dev_t ** sfm3000, i2c_master_bus_handle_t i2c_bus_handle) {
  *sfm3000 = createSFM3000Instance(i2c_bus_handle);
  if (!*sfm3000) {
    ESP_LOGE(TAG, "Creating SFM3000 driver failed");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "SFM3000 driver created successfully");

  esp_err_t error = lookForSFM3000Sensor(*sfm3000);
  if (error != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize SFM3000 sensor");
    return error;
  }

  return ESP_OK;
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
