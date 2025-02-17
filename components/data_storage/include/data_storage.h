#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <stdint.h>

#define MAX_PSRAM_MEMORY 8388608  // w bajtach
#define MAX_MEMORY_FOR_ONE_MEASUREMENT (MAX_PSRAM_MEMORY / 4)
#define DATA_POINT_SIZE sizeof(float) // w bajtach
#define MAX_DATA_POINTS (MAX_MEMORY_FOR_ONE_MEASUREMENT / DATA_POINT_SIZE)
#define DATA_POINTS 2000

// Deklaracje zmiennych globalnych
extern float *temperature_data;
extern float *humidity_data;
extern float *pressure_data;
extern float *audio_data;

extern int temperature_read_index, temperature_save_index;
extern int humidity_read_index, humidity_save_index;
extern int pressure_read_index, pressure_save_index;
extern int audio_read_index, audio_save_index;
extern int temperature_data_count, humidity_data_count, pressure_data_count, audio_data_count;

// Deklaracje funkcji
void initialize_memory();
void purge_memory();

void save_temperature(float temperature);
float read_temperature();

void save_humidity(float humidity);
float read_humidity();

void save_pressure(float pressure);
float read_pressure();

void save_audio(float audio);
float read_audio();

void saveFloatAsUint(float value, uint8_t table[4]);
float readUintAsFloat(uint8_t table[4]);

#endif // DATA_STORAGE_H
