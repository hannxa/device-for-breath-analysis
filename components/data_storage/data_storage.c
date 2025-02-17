#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "esp_log.h"

// Maksymalna liczba punktów pomiarowych (dla jednego typu pomiaru)
#define MAX_PSRAM_MEMORY 8388608  // w bajtach
#define MAX_MEMORY_FOR_ONE_MEASUREMENT (MAX_PSRAM_MEMORY / 4)
#define DATA_POINT_SIZE sizeof(float) // w bajtach
#define MAX_DATA_POINTS (MAX_MEMORY_FOR_ONE_MEASUREMENT / DATA_POINT_SIZE)

#define DATA_POINTS 2000

static const char *TAG = "MEASUREMENTS";

// Dynamiczne tablice dla każdego typu pomiarów
float *temperature_data;
float *humidity_data;
float *pressure_data;
float *audio_data;

// Indeksy odczytu i zapisu dla każdego typu pomiaru
int temperature_read_index = 0, temperature_save_index = 0;
int humidity_read_index = 0, humidity_save_index = 0;
int pressure_read_index = 0, pressure_save_index = 0;
int audio_read_index = 0, audio_save_index = 0;
int temperature_data_count = 0, humidity_data_count = 0, pressure_data_count = 0, audio_data_count = 0;

// Funkcja inicjalizująca pamięć
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

// Funkcja zwalniająca pamięć
void purge_memory() {
    free(temperature_data);
    free(humidity_data);
    free(pressure_data);
    free(audio_data);
    ESP_LOGI(TAG, "Memory purged successfully.");
}

// Funkcja do zapisywania pomiaru
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

// Funkcja do odczytu pomiaru
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

// Funkcje specyficzne dla typów pomiarów
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

static void saveFloatAsUint(float value, uint8_t table[4]) {
    uint8_t *bytePointer = (uint8_t *)&value;
    for (int i = 0; i < 4; i++) {
        table[i] = bytePointer[i];
    }
}

// Funkcja do odczytu uint8_t jako float
static float readUintAsFloat(uint8_t table[4]) {
    float value;
    uint8_t *bytePointer = (uint8_t *)&value;
    for (int i = 0; i < 4; i++) {
        bytePointer[i] = table[i];
    }
    return value;
}
