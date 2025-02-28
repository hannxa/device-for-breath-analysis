/**
**********************************************************************************************************************
  * @file    data_storage.c
  * @brief   This file is the Data Storage module implementation
  * @authors juliahrycyna
  * @date    February 28, 2025
**********************************************************************************************************************
  */

/* Includes -------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "esp_log.h"
#include "data_storage.h"

/* Private typedef ---------------------------------------------------------------------------------------------------*/

/* Private define ----------------------------------------------------------------------------------------------------*/

/* Private macros ----------------------------------------------------------------------------------------------------*/

/* Private variables -------------------------------------------------------------------------------------------------*/
static const char *TAG = "DATA_STORAGE";

static void saveFloatAsUint(float value, uint8_t table[4]);
static float readUintAsFloat(uint8_t table[4]);

float *temperature_data;
float *humidity_data;
float *pressure_data;
float *audio_data;

int temperature_read_index = 0, temperature_save_index = 0;
int humidity_read_index = 0, humidity_save_index = 0;
int pressure_read_index = 0, pressure_save_index = 0;
int audio_read_index = 0, audio_save_index = 0;
int temperature_data_count = 0, humidity_data_count = 0, pressure_data_count = 0, audio_data_count = 0;

/* Private function declarations -------------------------------------------------------------------------------------*/
/*
 * @function initialize_memory
 *
 * @abstract Allocates memory for storing sensor data
 *
 * @return None
 */
void initialize_memory() {
    temperature_data = (float *)malloc(MAX_DATA_POINTS * sizeof(float));
    humidity_data = (float *)malloc(MAX_DATA_POINTS * sizeof(float));
    pressure_data = (float *)malloc(MAX_DATA_POINTS * sizeof(float));
    audio_data = (float *)malloc(MAX_DATA_POINTS * sizeof(float));

    if (!temperature_data || !humidity_data || !pressure_data || !audio_data) {
        ESP_LOGE(TAG, "Memory allocation failed!");
        assert(0);
    }
    ESP_LOGI(TAG, "Memory initialized successfully.");
}

/*
 * @function purge_memory
 *
 * @abstract Releases allocated memory
 *
 * @return None
 */
void purge_memory() {
    free(temperature_data);
    free(humidity_data);
    free(pressure_data);
    free(audio_data);
    ESP_LOGI(TAG, "Memory purged successfully.");
}

/*
 * @function save_measurement
 *
 * @abstract Saves a measurement in the corresponding storage array
 *
 * @param[in] save_index: Index at which to store the measurement
 * @param[in] data_count: Counter for stored data points
 * @param[in] data_array: Pointer to the data array
 * @param[in] value: Measurement value to store
 *
 * @return None
 */
static void save_measurement(int *save_index, int *data_count, float *data_array, float value) {
    if (*data_count >= MAX_DATA_POINTS) {
        ESP_LOGE(TAG, "Error: Memory full. Cannot overwrite unread data.");
        return;
    }
    data_array[*save_index] = value;
    *save_index = (*save_index + 1) % MAX_DATA_POINTS;
    (*data_count)++;
    ESP_LOGI(TAG, "Measurement saved: %.2f", value);
}

/*
 * @function read_measurement
 *
 * @abstract Reads a measurement from the storage array
 *
 * @param[in] read_index: Index from which to read the measurement
 * @param[in] data_count: Counter for stored data points
 * @param[in] data_array: Pointer to the data array
 *
 * @return float: Measurement value read
 */
static float read_measurement(int *read_index, int *data_count, float *data_array) {
    if (*data_count <= 0) {
        ESP_LOGE(TAG, "Error: No data to read.");
        return -1.0f;
    }
    float value = data_array[*read_index];
    *read_index = (*read_index + 1) % MAX_DATA_POINTS;
    (*data_count)--;
    ESP_LOGI(TAG, "Measurement read: %.2f", value);
    return value;
}

/* Exported function definitions -------------------------------------------------------------------------------------*/
void save_temperature(float temperature) {
    save_measurement(&temperature_save_index, &temperature_data_count, temperature_data, temperature);
}

float read_temperature() {
    return read_measurement(&temperature_read_index, &temperature_data_count, temperature_data);
}

void save_humidity(float humidity) {
    save_measurement(&humidity_save_index, &humidity_data_count, humidity_data, humidity);
}

float read_humidity() {
    return read_measurement(&humidity_read_index, &humidity_data_count, humidity_data);
}

void save_pressure(float pressure) {
    save_measurement(&pressure_save_index, &pressure_data_count, pressure_data, pressure);
}

float read_pressure() {
    return read_measurement(&pressure_read_index, &pressure_data_count, pressure_data);
}

void save_audio(float audio) {
    save_measurement(&audio_save_index, &audio_data_count, audio_data, audio);
}

float read_audio() {
    return read_measurement(&audio_read_index, &audio_data_count, audio_data);
}

/*
 * @function saveFloatAsUint
 *
 * @abstract Converts a float value into a 4-byte array representation
 *
 * @param[in] value: Float value to convert
 * @param[out] table: 4-byte array storing the converted value
 *
 * @return None
 */
static void saveFloatAsUint(float value, uint8_t table[4]) {
    uint8_t *bytePointer = (uint8_t *)&value;
    for (int i = 0; i < 4; i++) {
        table[i] = bytePointer[i];
    }
}

/*
 * @function readUintAsFloat
 *
 * @abstract Converts a 4-byte array representation back into a float value
 *
 * @param[in] table: 4-byte array containing the stored float value
 *
 * @return float: Converted float value
 */
static float readUintAsFloat(uint8_t table[4]) {
    float value;
    uint8_t *bytePointer = (uint8_t *)&value;
    for (int i = 0; i < 4; i++) {
        bytePointer[i] = table[i];
    }
    return value;
}

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
