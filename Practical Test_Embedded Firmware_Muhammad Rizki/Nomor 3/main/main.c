#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/*
    ===============================
    KONFIGURASI SISTEM
    ===============================
*/

#define NUM_SENSORS 5

/*
    ===============================
    SHARED DATA BUFFER
    ===============================
*/

typedef struct {
    int sensorValue[NUM_SENSORS];
} SharedDataBuffer;

SharedDataBuffer sharedDataBuffer;

SemaphoreHandle_t dataMutex;

/*
    ===============================
    TASK SENSOR
    ===============================
*/

void sensorTask(void *pvParameters)
{
    while (1)
    {
        // __Ambil__ __mutex__ __sebelum__ __menulis__ data
        if (xSemaphoreTake(dataMutex, portMAX_DELAY))
        {
            printf("\n[Sensor Task] Reading sensors...\n");

            for (int i = 0; i < NUM_SENSORS; i++)
            {
                // __Simulasi__ __pembacaan__ sensor (0–100)
                sharedDataBuffer.sensorValue[i] = rand() % 101;

                printf("[Sensor Task] Sensor %d = %d\n",
                       i + 1,
                       sharedDataBuffer.sensorValue[i]);
            }

            // __Lepas__ __mutex__ __setelah__ __selesai__
            xSemaphoreGive(dataMutex);
        }

        // Delay 2 __detik__
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/*
    ===============================
    TASK COMMUNICATION
    ===============================
*/

void communicationTask(void *pvParameters)
{
    while (1)
    {
        // __Ambil__ __mutex__ __sebelum__ __membaca__ data
        if (xSemaphoreTake(dataMutex, portMAX_DELAY))
        {
            printf("\n[Communication Task] Sending data to server...\n");

            for (int i = 0; i < NUM_SENSORS; i++)
            {
                printf("[Communication Task] Sensor %d data sent = %d\n",
                       i + 1,
                       sharedDataBuffer.sensorValue[i]);
            }

            printf("[Communication Task] Data sent successfully\n");

            // __Lepas__ __mutex__
            xSemaphoreGive(dataMutex);
        }

        // __Kirim__ data __tiap__ 5 __detik__
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/*
    ===============================
    APP MAIN (ENTRY POINT ESP-IDF)
    ===============================
*/

void app_main(void)
{
    printf("System Started\n");

    // Seed random number
    srand(time(NULL));

    // __Membuat__ __mutex__
    dataMutex = xSemaphoreCreateMutex();

    if (dataMutex == NULL)
    {
        printf("Failed to create __mutex__!\n");
        return;
    }

    /*
        __Membuat__ __kedua__ task __berjalan__ __bersamaan__
    */
    xTaskCreate(
        sensorTask,        	// __fungsi__ task
        "Sensor Task",     		// __nama__
        2048,             // stack size
        NULL,             // parameter
        1,                 	// priority
        NULL
    );

    xTaskCreate(
        communicationTask,
        "Communication Task",
        2048,
        NULL,
        1,
        NULL
    );
}