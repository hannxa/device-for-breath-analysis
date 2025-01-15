#include <stdio.h>
#include <stdint.h>

#define MAX_FLASH_MEMORY 3774874
#define DATA_POINT_SIZE 14

#define DATA_POINTS 200000

#define MAX_DATA_POINTS (MAX_FLASH_MEMORY / DATA_POINT_SIZE)

#if DATA_POINTS > MAX_DATA_POINTS
#error "DATA_POINTS size exceeds the maximum allowed value."
#endif

typedef struct {
    float temperature;
    float humidity;
    float pressure;
    int16_t audio;
} MeasurementData;

MeasurementData measurements[DATA_POINTS];

typedef struct {
    float data[DATA_POINTS];
    int flags[DATA_POINTS];
    int read_index;
    int save_index;
} MeasurementArray;

MeasurementArray temperature_data = { .read_index = 0, .save_index = 0 };
MeasurementArray humidity_data = { .read_index = 0, .save_index = 0 };
MeasurementArray pressure_data = { .read_index = 0, .save_index = 0 };
MeasurementArray audio_data = { .read_index = 0, .save_index = 0 };

static void reset_save_index_if_needed(MeasurementArray *measurement) {
    if (measurement->save_index >= DATA_POINTS) {
        measurement->save_index = 0;
    }
}

static int is_memory_full(MeasurementArray *measurement) {
    return measurement->flags[measurement->save_index] == 1;
}

static void store_measurement(float value, MeasurementArray *measurement) {
    measurement->data[measurement->save_index] = value;
    measurement->flags[measurement->save_index] = 1;
    measurement->save_index++;
}

static void save_measurement(float value, MeasurementArray *measurement) {
    reset_save_index_if_needed(measurement);
    if (is_memory_full(measurement)) {
        printf("Error: Memory full.\n");
        return;
    } else {
        store_measurement(value, measurement);
    }
}

static void reset_read_index_if_needed(MeasurementArray *measurement) {
    if (measurement->read_index >= DATA_POINTS) {
        measurement->read_index = 0;
    }
}

static int is_data_available(MeasurementArray *measurement) {
    return measurement->flags[measurement->read_index] != 0;
}

static float fetch_measurement(MeasurementArray *measurement) {
    float value = measurement->data[measurement->read_index];
    measurement->flags[measurement->read_index] = 0; // Reset flag after reading
    measurement->read_index++;
    return value;
}

float read_measurement(MeasurementArray *measurement) {
    reset_read_index_if_needed(measurement);
    if (!is_data_available(measurement)) {
        printf("Error: No data to read.\n");
        return -1; // Return -1 in case of error
    }
    return fetch_measurement(measurement);
}

void save_temperature(float temperature) {
    save_measurement(temperature, &temperature_data);
}

float read_temperature() {
    return read_measurement(&temperature_data);
}

void save_humidity(float humidity) {
    save_measurement(humidity, &humidity_data);
}

float read_humidity() {
    return read_measurement(&humidity_data);
}

void save_pressure(float pressure) {
    save_measurement(pressure, &pressure_data);
}

float read_pressure() {
    return read_measurement(&pressure_data);
}

void save_audio(int16_t audio) {
    save_measurement((float)audio, &audio_data);
}

int16_t read_audio() {
    return (int16_t)read_measurement(&audio_data);
}

float get_memory_usage(MeasurementArray *measurement) {
    int used_memory = 0;
    for (int i = 0; i < DATA_POINTS; i++) {
        if (measurement->flags[i] == 1) {
            used_memory++;
        }
    }
    return (used_memory / (float)DATA_POINTS) * 100;
}