#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <stdint.h>

// Funkcje publiczne do zarządzania pomiarami
void save_temperature(float temperature);
float read_temperature(void);

void save_humidity(float humidity);
float read_humidity(void);

void save_pressure(float pressure);
float read_pressure(void);

void save_audio(float audio);
float read_audio(void);

float get_memory_usage_percentage(void);

#endif // DATA_STORAGE_H
