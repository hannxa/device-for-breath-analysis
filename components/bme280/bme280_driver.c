/**
  **********************************************************************************************************************
  * @file    bme280_driver.c
  * @brief   This file is the BME280 sensor driver implementation
  * @authors patrykmonarcha
  * @date Dec 2, 2024
  **********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include "bme280_driver.h"
#include "esp_log.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Private typedef ---------------------------------------------------------------------------------------------------*/
/** @brief BME280 structure
 *
 * This structure is used to store all BME280 sensor-related data
 *
 */
struct bme280_t{
    i2c_master_dev_handle_t i2c_device;
    i2c_device_config_t device_config;
    i2c_master_bus_handle_t i2c_bus_handle;
    uint8_t chip_id;
    struct {
        uint16_t T1;
        int16_t T2;
        int16_t T3;
        uint16_t P1;
        int16_t P2;
        int16_t P3;
        int16_t P4;
        int16_t P5;
        int16_t P6;
        int16_t P7;
        int16_t P8;
        int16_t P9;
        uint8_t H1;
        int16_t H2;
        uint8_t H3;
        int16_t H4;
        int16_t H5;
        int8_t H6;
    } compensation_data;
    int32_t temperature_fine;
};

/* Private define ----------------------------------------------------------------------------------------------------*/
#define BME280_REGISTER_SENSOR_RESET 0xE0
#define BME280_RESET_VECTOR 0xB6
#define BME280_REGISTER_CHIP_ID 0xD0
#define BME280_CHIP_ID  0x60
#define BME280_REGISTER_TEMPERATURE_MSB 0xFA
#define BME280_REGISTER_PRESSURE_MSB 0xF7
#define BME280_REGISTER_HUMIDITY_MSB 0xFD
#define BME280_REGISTER_CONFIG 0xF5
#define BME280_REGISTER_MEASUREMENT_CONTROL 0xF4
#define BME280_REGISTER_STATUS 0xF3
#define BME280_REGISTER_HUMIDITY_CONTROL 0xF2
#define BME280_REGISTER_CALIBRATION_HIGH_BANK 0xE1
#define BME280_REGISTER_CALIBRATION_LOW_BANK 0x88

/* Private macros ----------------------------------------------------------------------------------------------------*/
#define isChipIDCorrect(chip_id) (((chip_id) == BME280_CHIP_ID))
#define validateSensor(bme280) (!(bme280->i2c_device == NULL && bme280->chip_id == 0xAD))

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char * TAG = "bme280_driver";

/* External variables ------------------------------------------------------------------------------------------------*/

/* Private function declarations -------------------------------------------------------------------------------------*/
/*
 * @function createDeviceBME280
 *
 * @abstract This function adds BME280 sensor to the I2C line
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] device_address: BME280's I2C address
 *
 * @return
 *      - esp_err_t status code
 */
static esp_err_t createDeviceBME280(bme280_t * bme280, const uint16_t device_address);

/*
 * @function readBME280
 *
 * @abstract This function reads data from BME280 sensor
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] address: Register address
 *
 * @param[out] data_out: Output data buffer
 *
 * @param[in] size: Number of bytes to read
 *
 * @return
 *      - esp_err_t status code
 */
static esp_err_t readBME280(bme280_t * bme280, uint8_t address, uint8_t * data_out, size_t size);

/*
 * @function writeBME280
 *
 * @abstract This function writes data to BME280 sensor
 *
 * @param[in] bme280: BME280 instance
 *
 * @param[in] address: Register address
 *
 * @param[in] data_in: Input data buffer
 *
 * @param[in] size: Number of bytes to write
 *
 * @return
 *      - esp_err_t status code
 */
static esp_err_t writeBME280(bme280_t * bme280, uint8_t address, const uint8_t * data_in, size_t size);

/*
 * @function checkForBME280ChipID
 *
 * @abstract This function checks if BME280 chip ID is correct
 *
 * @param[in] bme280: BME280 instance
 *
 * @return
 *      - esp_err_t status code
 */
static esp_err_t checkForBME280ChipID(bme280_t * bme280);

/*
 * @function lookForBME280Sensor
 *
 * @abstract This function checks if BME280 sensor is present on the I2C line
 *
 * @param[in] bme280: BME280 instance
 *
 * @return
 *      - esp_err_t status code
 */
static esp_err_t lookForBME280Sensor(bme280_t * bme280);

/*
 * @function resetBME280
 *
 * @abstract This function resets BME280 sensor
 *
 * @param[in] bme280: BME280 instance
 *
 * @return
 *      - esp_err_t status code
 */
static esp_err_t resetBME280(bme280_t * bme280);

/*
 * @function calibrateBME280
 *
 * @abstract This function reads BME280 sensor's calibration data
 *
 * @param[in] bme280: BME280 instance
 *
 * @return
 *      - esp_err_t status code
 */
static esp_err_t calibrateBME280(bme280_t * bme280);

/* Private function definitions --------------------------------------------------------------------------------------*/
static esp_err_t createDeviceBME280(bme280_t * bme280, const uint16_t device_address) {
    ESP_LOGD(TAG, "Creating BME280 device at address 0x%2X", device_address);
    bme280->device_config.device_address = device_address;

    esp_err_t error = i2c_master_bus_add_device(bme280->i2c_bus_handle, &bme280->device_config, &bme280->i2c_device);
    if (error == ESP_OK) {
        ESP_LOGD(TAG, "Device BME280 successfully created at address 0x%2X", device_address);
        return error;
    } else {
        ESP_LOGE(TAG, "Error creating device BME280 at address 0x%2X", device_address);
        return error;
    }
}

static esp_err_t readBME280(bme280_t * bme280, uint8_t address, uint8_t * data_out, size_t size) {

    return i2c_master_transmit_receive(bme280->i2c_device, &address, sizeof(address), data_out, size, BME280_TIMEOUT);
}

static esp_err_t writeBME280(bme280_t * bme280, uint8_t address, const uint8_t * data_in, size_t size) {
    esp_err_t error;

    for (uint8_t i = 0; i < size; i++) {
        uint8_t data[2] = {(address + i), data_in[i]};
        error = i2c_master_transmit(bme280->i2c_device, data, 2, BME280_TIMEOUT);

        if (error != ESP_OK) {
            return error;
        }

    }

    return ESP_OK;
}

static esp_err_t checkForBME280ChipID(bme280_t * bme280) {
    ESP_LOGD(TAG, "Looking for BME280 sensors on I2C line");

    esp_err_t error = readBME280(bme280, BME280_REGISTER_CHIP_ID, &bme280->chip_id, sizeof bme280->chip_id);
    if (error == ESP_OK) {
        if (isChipIDCorrect(bme280->chip_id)) {
            ESP_LOGD(TAG, "BME280 device detected at address 0x%2X with id 0x%2X", bme280->device_config.device_address, bme280->chip_id);
            return ESP_OK;
        } else {
            ESP_LOGE(TAG, "Connected sensor model might be wrong. Expected chip id 0x%2X, but got 0x%2X instead", BME280_CHIP_ID, bme280->chip_id);            error = ESP_ERR_NOT_FOUND;
        }
    }

    ESP_LOGW(TAG, "Failed getting address: address 0x%hx, chip id=%2X. Error: %s", bme280->device_config.device_address, bme280->chip_id, esp_err_to_name(error));

    return error;
}

static esp_err_t lookForBME280Sensor(bme280_t * bme280) {
    ESP_LOGD(TAG, "Looking for BME280 sensors on I2C line");

    esp_err_t error;
    error = createDeviceBME280(bme280, BME280_DEVICE_ADDRESS);

    if (error != ESP_OK) {
        return error;
    }

    error = checkForBME280ChipID(bme280);

    if (error != ESP_OK) {
        error = createDeviceBME280(bme280, BME280_DEVICE_ALTERNATIVE_ADDRESS);
        if (error != ESP_OK) {
            ESP_LOGE(TAG, "Error creating BME280 device on address 0x%2X. Error: %s", BME280_DEVICE_ALTERNATIVE_ADDRESS, esp_err_to_name(error));
            return error;
        }

        error = checkForBME280ChipID(bme280);

        if (error != ESP_OK) {
            ESP_LOGE(TAG, "BME280 not found.");
            bme280->i2c_device = NULL;
            bme280->chip_id = 0xAD;
        }
    }

    return error;
}

static esp_err_t resetBME280(bme280_t * bme280) {
    const static uint8_t data_in[] = {BME280_RESET_VECTOR};

    return writeBME280(bme280, BME280_REGISTER_SENSOR_RESET, data_in, sizeof data_in);
}

static esp_err_t calibrateBME280(bme280_t * bme280) {
    ESP_LOGD(TAG, "Getting BME280 calibration values");

    esp_err_t error;
    uint8_t data_buffer[26];

    error = readBME280(bme280, BME280_REGISTER_CALIBRATION_LOW_BANK, data_buffer, sizeof data_buffer);

    if (error != ESP_OK) {
        return error;
    }

    bme280->compensation_data.T1 = data_buffer[0] | (data_buffer[1] << 8);
    bme280->compensation_data.T2 = data_buffer[2] | (data_buffer[3] << 8);
    bme280->compensation_data.T3 = data_buffer[4] | (data_buffer[5] << 8);
    bme280->compensation_data.P1 = data_buffer[6] | (data_buffer[7] << 8);
    bme280->compensation_data.P2 = data_buffer[8] | (data_buffer[9] << 8);
    bme280->compensation_data.P3 = data_buffer[10] | (data_buffer[11] << 8);
    bme280->compensation_data.P4 = data_buffer[12] | (data_buffer[13] << 8);
    bme280->compensation_data.P5 = data_buffer[14] | (data_buffer[15] << 8);
    bme280->compensation_data.P6 = data_buffer[16] | (data_buffer[17] << 8);
    bme280->compensation_data.P7 = data_buffer[18] | (data_buffer[19] << 8);
    bme280->compensation_data.P8 = data_buffer[20] | (data_buffer[21] << 8);
    bme280->compensation_data.P9 = data_buffer[22] | (data_buffer[23] << 8);

    bme280->compensation_data.H1 = data_buffer[23];

    error = readBME280(bme280, BME280_REGISTER_CALIBRATION_HIGH_BANK, data_buffer, 7);

    if (error != ESP_OK) {
        return error;
    }

    bme280->compensation_data.H2 = data_buffer[0] | (data_buffer[1] << 8);
    bme280->compensation_data.H3 = data_buffer[2];
    bme280->compensation_data.H4 = (data_buffer[3] << 4) | (data_buffer[4] & 0x0F);
    bme280->compensation_data.H5 = (data_buffer[4] >> 4) | (data_buffer[5] << 4);
    bme280->compensation_data.H6 = data_buffer[6];

    return ESP_OK;
}

/* Exported function definitions -------------------------------------------------------------------------------------*/
bme280_t * createBME280Instance(i2c_master_bus_handle_t i2c_bus_handle) {
    bme280_t * bme280 = malloc(sizeof(bme280_t));

    if (bme280) {
        memset(bme280, 0, sizeof(bme280_t));
        bme280->i2c_bus_handle = i2c_bus_handle;
        bme280->device_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        bme280->device_config.device_address = 0xDE;
        bme280->device_config.scl_speed_hz = BME280_I2C_CLK_SPEED_HZ;
        bme280->i2c_device = NULL;
        bme280->chip_id = 0xAD;
    } else {
        ESP_LOGE(TAG, "Failed allocating memory for BME280 instance");
        removeBME280(bme280);
        return NULL;
    }

    return bme280;
}

void removeBME280(bme280_t * bme280) {
    if (bme280 != NULL && bme280->i2c_device != NULL) {
        i2c_master_bus_rm_device(bme280->i2c_device);
    }

    free(bme280);
}

esp_err_t initializeBME280(bme280_t * bme280) {
    if (bme280 == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t error = lookForBME280Sensor(bme280) || resetBME280(bme280);

    if (error == ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(10));

        calibrateBME280(bme280);

        ESP_LOGD(TAG, "BME280 sensor calibration data");
        ESP_LOG_BUFFER_HEXDUMP(TAG, &bme280->compensation_data, sizeof(bme280->compensation_data), ESP_LOG_DEBUG);
    }

    return error;
}

esp_err_t configureBME280(bme280_t * bme280, bme280_config_t * config) {
    if (bme280 == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!validateSensor(bme280)) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t num = (config->temperature_sampling << 5) | (config->pressure_sampling << 2) | BME280_MODE_SLEEP;
    esp_err_t error = writeBME280(bme280, BME280_REGISTER_MEASUREMENT_CONTROL, &num, sizeof num);

    if (error) {
        return error;
    }

    num = (config->standby << 5) | (config->iir_filter << 2);
    error = writeBME280(bme280, BME280_REGISTER_CONFIG, &num, sizeof num);

    if (error) {
        return error;
    }

    num = config->humidity_sampling;
    error = writeBME280(bme280, BME280_REGISTER_HUMIDITY_CONTROL, &num, sizeof(num));

    if (error) {
        return error;
    }

    return ESP_OK;
}

esp_err_t setBME280Mode(bme280_t * bme280, bme280_mode_t mode) {
    uint8_t control_measurement;
    esp_err_t error;

    error = readBME280(bme280, BME280_REGISTER_MEASUREMENT_CONTROL, &control_measurement, 1);

    if (error != ESP_OK) {
        return error;
    }

    control_measurement = (control_measurement & (~3)) | mode;

    return writeBME280(bme280, BME280_REGISTER_MEASUREMENT_CONTROL, &control_measurement, 1);
}

bool isBME280Sampling(bme280_t * bme280) {
    uint8_t status;

    esp_err_t error;

    error = readBME280(bme280, BME280_REGISTER_STATUS, &status, 1);

    if (error == ESP_OK) {
        return (status & (1 << 3)) != 0;
    } else {
        return false;
    }

}

int32_t compensateBME280Temperature(bme280_t * bme280, int32_t input_temperature) {
    int32_t var1, var2, temperature;
    var1 = ((((input_temperature >> 3) - ((int32_t)bme280->compensation_data.T1 << 1))) * ((int32_t)bme280->compensation_data.T2)) >> 11;
    var2 = (((((input_temperature >> 4) - ((int32_t)bme280->compensation_data.T1)) * ((input_temperature >> 4) - ((int32_t)bme280->compensation_data.T1))) >> 12) * ((int32_t)bme280->compensation_data.T3)) >> 14;
    bme280->temperature_fine = var1 + var2;
    temperature  = (bme280->temperature_fine * 5 + 128) >> 8;

    return temperature;
}

uint32_t compensateBME280Pressure(bme280_t * bme280, int32_t input_pressure) {
    int64_t var1, var2, pressure;
    var1 = ((int64_t)bme280->temperature_fine) - 128000;
    var2 = var1 * var1 * (int64_t)bme280->compensation_data.P6;
    var2 = var2 + ((var1*(int64_t)bme280->compensation_data.P5) << 17);
    var2 = var2 + (((int64_t)bme280->compensation_data.P4) << 35);
    var1 = ((var1 * var1 * (int64_t)bme280->compensation_data.P3) >> 8) + ((var1 * (int64_t)bme280->compensation_data.P2) << 12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)bme280->compensation_data.P1) >> 33;
    if(var1 == 0){
        return 0;
    }
    pressure = 1048576 - input_pressure;
    pressure = (((pressure << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)bme280->compensation_data.P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
    var2 =(((int64_t)bme280->compensation_data.P8) * pressure) >> 19;
    pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)bme280->compensation_data.P7) << 4);

    return (uint32_t)pressure;
}

uint32_t compensateBME280Humidity(bme280_t * bme280, int32_t input_humidity) {
    int32_t humdidity;
    humdidity = (bme280->temperature_fine - ((int32_t)76800));
    humdidity = (((((input_humidity << 14) - (((int32_t)bme280->compensation_data.H4) << 20) - (((int32_t)bme280->compensation_data.H5) * humdidity)) + ((int32_t)16384)) >> 15) * (((((((humdidity * ((int32_t)bme280->compensation_data.H6)) >> 10) * (((humdidity * ((int32_t)bme280->compensation_data.H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)bme280->compensation_data.H2) + 8192) >> 14));
    humdidity = (humdidity - (((((humdidity >> 15) * (humdidity >> 15)) >> 7) * ((int32_t)bme280->compensation_data.H1)) >> 4));
    humdidity = (humdidity < 0 ? 0 : humdidity);
    humdidity = (humdidity > 419430400 ? 419430400 : humdidity);

    return(uint32_t)(humdidity >> 12);
}

esp_err_t readBME280Temperature(bme280_t * bme280, int32_t * int32_temp) {
    if (bme280 == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!validateSensor(bme280)) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t data_buffer[3];

    esp_err_t error = readBME280(bme280, BME280_REGISTER_TEMPERATURE_MSB, data_buffer, sizeof(data_buffer));
    if (error != ESP_OK) {
        return error;
    }

    *int32_temp = compensateBME280Temperature(bme280, (data_buffer[0] << 12) | (data_buffer[1] << 4) | (data_buffer[2] >> 4));

    return ESP_OK;
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
