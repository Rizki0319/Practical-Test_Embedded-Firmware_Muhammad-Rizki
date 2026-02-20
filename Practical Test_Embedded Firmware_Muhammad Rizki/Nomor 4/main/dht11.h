#ifndef DHT11_H
#define DHT11_H

#include "driver/gpio.h"
#include "esp_err.h"

/*
Struct untuk menyimpan data pembacaan DHT11
*/
typedef struct {
    float temperature;   // suhu dalam Celsius
    float humidity;      // kelembaban dalam %
} dht11_data_t;

/*
Function membaca data dari DHT11
gpio_num = pin yang digunakan
result = tempat menyimpan hasil pembacaan
*/
esp_err_t dht11_read(gpio_num_t gpio_num, dht11_data_t *result);

#endif