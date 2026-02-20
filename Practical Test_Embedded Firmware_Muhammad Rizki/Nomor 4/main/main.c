#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht11.h"

/*
Pilih GPIO untuk DHT11
Bisa diganti sesuai wiring
*/
#define DHT_PIN GPIO_NUM_4

void app_main(void)
{
    dht11_data_t sensor_data;

    while (1) {

        /*
        Panggil function baca sensor
        */
        if (dht11_read(DHT_PIN, &sensor_data) == ESP_OK) {

            printf("Temperature: %.1f C\n", (float)sensor_data.temperature);
			printf("Humidity: %.1f %%\n\n", (float)sensor_data.humidity);	

        } else {
            printf("Failed to read from DHT11\n");
        }

        /*
        DHT11 minimal interval pembacaan 1 detik
        */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}