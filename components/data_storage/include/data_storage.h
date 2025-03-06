/**
**********************************************************************************************************************
  * @file    data_storage.h
  * @brief   This file is the header file for Data Storage module
  * @authors juliahrycyna
  * @date    February 28, 2025
**********************************************************************************************************************
  */

/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef _DATA_STORAGE_H_
#define _DATA_STORAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------------------------------------------------*/
#include <stdint.h>

/* Constants ---------------------------------------------------------------------------------------------------------*/
#define MAX_PSRAM_MEMORY (8388608 / 4) // Maximum available PSRAM memory divided by 4 (for float storage)
#define MAX_MEMORY_FOR_ONE_MEASUREMENT (MAX_PSRAM_MEMORY / 4) // Memory allocated per sensor type
#define DATA_POINT_SIZE sizeof(float) // Size of a single data point (float)
#define MAX_DATA_POINTS (MAX_MEMORY_FOR_ONE_MEASUREMENT / DATA_POINT_SIZE) // Maximum number of data points that can be stored

/* External Variables ------------------------------------------------------------------------------------------------*/
extern float *temperature_data;
extern float *humidity_data;
extern float *pressure_data;
extern float *audio_data;

extern int temperature_read_index, temperature_save_index;
extern int humidity_read_index, humidity_save_index;
extern int pressure_read_index, pressure_save_index;
extern int audio_read_index, audio_save_index;
extern int temperature_data_count, humidity_data_count, pressure_data_count, audio_data_count;

/* Function Declarations ---------------------------------------------------------------------------------------------*/
/*
 * @function initialize_memory
 *
 * @abstract Allocates memory for storing sensor data
 *
 * @return None
 */
void initialize_memory();

/*
 * @function purge_memory
 *
 * @abstract Releases allocated memory
 *
 * @return None
 */
void purge_memory();

/*
 * @function save_temperature
 *
 * @abstract Saves a temperature measurement
 *
 * @param[in] temperature: Temperature value to be stored
 *
 * @return None
 */
void save_temperature(float temperature);

/*
 * @function read_temperature
 *
 * @abstract Reads a stored temperature measurement
 *
 * @return float: Retrieved temperature value
 */
float read_temperature();

/*
 * @function save_humidity
 *
 * @abstract Saves a humidity measurement
 *
 * @param[in] humidity: Humidity value to be stored
 *
 * @return None
 */
void save_humidity(float humidity);

/*
 * @function read_humidity
 *
 * @abstract Reads a stored humidity measurement
 *
 * @return float: Retrieved humidity value
 */
float read_humidity();

/*
 * @function save_pressure
 *
 * @abstract Saves a pressure measurement
 *
 * @param[in] pressure: Pressure value to be stored
 *
 * @return None
 */
void save_pressure(float pressure);

/*
 * @function read_pressure
 *
 * @abstract Reads a stored pressure measurement
 *
 * @return float: Retrieved pressure value
 */
float read_pressure();

/*
 * @function save_audio
 *
 * @abstract Saves an audio level measurement
 *
 * @param[in] audio: Audio level value to be stored
 *
 * @return None
 */
void save_audio(float audio);

/*
 * @function read_audio
 *
 * @abstract Reads a stored audio level measurement
 *
 * @return float: Retrieved audio level value
 */
float read_audio();

#ifdef __cplusplus
}
#endif

#endif // _DATA_STORAGE_H_

/* END OF FILE -------------------------------------------------------------------------------------------------------*/
