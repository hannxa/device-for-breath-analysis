#include <stdio.h>
#include <stdint.h>

// Maksymalna dostępna pamięć Flash dla danych (w bajtach)
#define MAX_FLASH_MEMORY 1887436 // 2 MB - 10% na przyszłe funkcje. (jednostka-bajt, czyli to 2*1024*1024*0.9) (te 2 MB to mi wyszły z funkcji sprawdzającej ilość wolnej pamięcie FLASH)
#define MAX_MEMORY_FOR_ONE_MEASUREMENT (MAX_FLASH_MEMORY / 4)
#define DATA_POINT_SIZE sizeof(float) // Rozmiar jednego pomiaru (w bajtach)
#define DATA_POINTS 2000 // Liczba punktów dla jednego pomiaru?? Tak to rozumiem, nie jestem pewna
#define MAX_DATA_POINTS (MAX_MEMORY_FOR_ONE_MEASUREMENT/DATA_POINT_SIZE) // Maksymalna liczba punktów pomiarowych (dla jednego typu pomiaru)

#define TEMP_START_ADDR 0
#define PRESS_START_ADDR (1 * MAX_DATA_POINTS * DATA_POINT_SIZE)
#define HUMID_START_ADDR (2 * MAX_DATA_POINTS * DATA_POINT_SIZE)
#define AUDIO_START_ADDR (3 * MAX_DATA_POINTS * DATA_POINT_SIZE) //

// Wspólna tablica dla wszystkich pomiarów
uint8_t measurements[4 * MAX_DATA_POINTS * DATA_POINT_SIZE];

// Indeksy odczytu i zapisu dla każdego typu pomiaru
static int read_index[4] = {0, 0, 0, 0};
static int save_index[4] = {0, 0, 0, 0};
static int data_count[4] = {0, 0, 0, 0}; // Liczba dostępnych danych w buforze

// Funkcja do zapisywania floata jako uint8_t
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

// Funkcja do zapisywania pomiaru
static void save_measurement(int type, float value) {
    if (data_count[type] >= MAX_DATA_POINTS) {
        printf("Error: Memory full for type %d. Cannot overwrite unread data.\n", type);
        return;
    }

    int base_addr = type * MAX_DATA_POINTS * DATA_POINT_SIZE;

    // Konwersja float na uint8_t
    uint8_t float_as_uint[DATA_POINT_SIZE];
    saveFloatAsUint(value, float_as_uint);

    // Zapis danych w tablicy
    for (int i = 0; i < DATA_POINT_SIZE; i++) {
        measurements[base_addr + save_index[type] * DATA_POINT_SIZE + i] = float_as_uint[i];
    }

    // Przesunięcie indeksu zapisu
    save_index[type] = (save_index[type] + 1) % MAX_DATA_POINTS;

    // Zwiększenie liczby dostępnych danych
    data_count[type]++;
}

// Funkcja do odczytu pomiaru
static float read_measurement(int type) {
    if (data_count[type] <= 0) {
        printf("Error: No data to read for type %d.\n", type);
        return -1.0f; // Wartość błędu
    }

    int base_addr = type * MAX_DATA_POINTS * DATA_POINT_SIZE;

    // Odczyt danych z tablicy
    uint8_t float_as_uint[DATA_POINT_SIZE];
    for (int i = 0; i < DATA_POINT_SIZE; i++) {
        float_as_uint[i] = measurements[base_addr + read_index[type] * DATA_POINT_SIZE + i];
    }

    // Przesunięcie indeksu odczytu
    read_index[type] = (read_index[type] + 1) % MAX_DATA_POINTS;

    // Zmniejszenie liczby dostępnych danych
    data_count[type]--;

    // Konwersja uint8_t na float
    return readUintAsFloat(float_as_uint);
}

// Funkcje specyficzne dla typów pomiarów
void save_temperature(float temperature) {
    save_measurement(0, temperature);
}

float read_temperature() {
    return read_measurement(0);
}

void save_humidity(float humidity) {
    save_measurement(1, humidity);
}

float read_humidity() {
    return read_measurement(1);
}

void save_pressure(float pressure) {
    save_measurement(2, pressure);
}

float read_pressure() {
    return read_measurement(2);
}

void save_audio(float audio) {
    save_measurement(3, audio);
}

float read_audio() {
    return read_measurement(3);
}

// Funkcja do obliczenia procentu zajętej pamięci
float get_memory_usage_percentage() {
    int total_data_points = 0;
    for (int i = 0; i < 4; i++) {
        total_data_points += data_count[i];
    }

    int used_memory = total_data_points * DATA_POINT_SIZE;
    return ((float)used_memory / (4 * MAX_DATA_POINTS * DATA_POINT_SIZE)) * 100.0f;
}
