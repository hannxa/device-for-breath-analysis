#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <stdint.h>

// Macros
#define MAX_FLASH_MEMORY 3774874
#define DATA_POINT_SIZE 14
#define DATA_POINTS 200000
#define MAX_DATA_POINTS (MAX_FLASH_MEMORY / DATA_POINT_SIZE)

#if DATA_POINTS > MAX_DATA_POINTS
#error "DATA_POINTS size exceeds the maximum allowed value."
#endif

// Structs
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    int16_t audio;
} MeasurementData;

typedef struct {
    float data[DATA_POINTS];
    int flags[DATA_POINTS];
    int read_index;
    int save_index;
} MeasurementArray;

// Function Prototypes
void save_temperature(float temperature);
float read_temperature(void);

void save_humidity(float humidity);
float read_humidity(void);

void save_pressure(float pressure);
float read_pressure(void);

void save_audio(int16_t audio);
int16_t read_audio(void);

float get_memory_usage(MeasurementArray *measurement);

#endif // DATA_STORAGE_H
