#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"

#define NUM_SENSOR 5

// waktu dalam ms
#define READ_TIME_MS 40
#define SEND_TIME_MS 40
#define PERIOD_MS 200
#define TOLERANCE_MS 5

static const char *TAG = "SCHEDULER";

// mutex bus karena sensor share bus
SemaphoreHandle_t bus_mutex;

/*
====================================================
STRUCT SENSOR
====================================================
*/
typedef struct
{
    int id;
    int64_t next_deadline_us;
} sensor_t;

sensor_t sensors[NUM_SENSOR];

/*
====================================================
SIMULASI READ SENSOR (40 ms)
====================================================
*/
void read_sensor(int id)
{
    ESP_LOGI(TAG, "Sensor %d START READ", id);

    // simulasi waktu baca 40 ms
    vTaskDelay(pdMS_TO_TICKS(READ_TIME_MS));

    ESP_LOGI(TAG, "Sensor %d DONE READ", id);
}

/*
====================================================
SIMULASI SEND DATA (40 ms)
====================================================
*/
void send_sensor(int id)
{
    ESP_LOGI(TAG, "Sensor %d START SEND", id);

    // simulasi waktu kirim 40 ms
    vTaskDelay(pdMS_TO_TICKS(SEND_TIME_MS));

    ESP_LOGI(TAG, "Sensor %d DONE SEND", id);
}

/*
====================================================
PROSES SENSOR (READ + SEND)
====================================================
*/
void process_sensor(sensor_t *sensor)
{
    int64_t start_time = esp_timer_get_time();

    // lock bus
    xSemaphoreTake(bus_mutex, portMAX_DELAY);

    read_sensor(sensor->id);
    send_sensor(sensor->id);

    // release bus
    xSemaphoreGive(bus_mutex);

    int64_t end_time = esp_timer_get_time();
    int64_t duration_ms = (end_time - start_time) / 1000;

    ESP_LOGI(TAG,
             "Sensor %d TOTAL TIME = %" PRId64 " ms",
             sensor->id,
             duration_ms);
}

/*
====================================================
EARLIEST DEADLINE FIRST SCHEDULER
====================================================
Menjamin sensor yang paling dekat deadline jalan dulu.
====================================================
*/
int get_earliest_deadline_sensor()
{
    int index = 0;
    int64_t min_deadline = sensors[0].next_deadline_us;

    for (int i = 1; i < NUM_SENSOR; i++)
    {
        if (sensors[i].next_deadline_us < min_deadline)
        {
            min_deadline = sensors[i].next_deadline_us;
            index = i;
        }
    }

    return index;
}

/*
====================================================
SCHEDULER TASK
====================================================
*/
void scheduler_task(void *arg)
{
    int64_t now;

    while (1)
    {
        now = esp_timer_get_time();

        int idx = get_earliest_deadline_sensor();

        // cek apakah sudah waktunya dijalankan
        if (now >= sensors[idx].next_deadline_us)
        {
            int64_t diff_ms =
                (now - sensors[idx].next_deadline_us) / 1000;

            // cek toleransi
            if (diff_ms > TOLERANCE_MS)
            {
                ESP_LOGW(TAG,
                         "Sensor %d MISSED DEADLINE by %" PRId64 " ms",
                         sensors[idx].id,
                         diff_ms);
            }

            ESP_LOGI(TAG,
                     "EXECUTE Sensor %d at %lld ms",
                     sensors[idx].id,
                     now / 1000);

            process_sensor(&sensors[idx]);

            // schedule period berikutnya
            sensors[idx].next_deadline_us += PERIOD_MS * 1000;
        }

        // delay kecil supaya CPU tidak full
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/*
====================================================
INIT
====================================================
*/
void app_main(void)
{
    bus_mutex = xSemaphoreCreateMutex();

    int64_t start_time = esp_timer_get_time();

    // init sensor deadline awal
    for (int i = 0; i < NUM_SENSOR; i++)
    {
        sensors[i].id = i + 1;

        // semua sensor mulai sekarang + period
        sensors[i].next_deadline_us =
            start_time + (PERIOD_MS * 1000);

        ESP_LOGI(TAG,
                 "Sensor %d first deadline %lld ms",
                 sensors[i].id,
                 sensors[i].next_deadline_us / 1000);
    }

    xTaskCreate(
        scheduler_task,
        "scheduler_task",
        4096,
        NULL,
        5,
        NULL);
}