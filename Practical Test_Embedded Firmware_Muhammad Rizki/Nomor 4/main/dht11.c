#include "dht11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"

/*
DHT11 menggunakan timing sangat cepat (microsecond),
maka kita pakai delay microsecond dari esp_rom_delay_us()
*/

/*
Function untuk set pin sebagai OUTPUT
*/
static void set_output(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
}

/*
Function untuk set pin sebagai INPUT
*/
static void set_input(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}

/*
Function menunggu pin berubah state dengan timeout
return = durasi waktu
*/
static int wait_for_level(gpio_num_t pin, int level, int timeout_us)
{
    int count = 0;

    while (gpio_get_level(pin) == level) {
        esp_rom_delay_us(1);
        count++;

        if (count > timeout_us)
            return -1; // timeout
    }

    return count;
}

/*
Function utama membaca data dari DHT11
*/
esp_err_t dht11_read(gpio_num_t pin, dht11_data_t *result)
{
    uint8_t data[5] = {0};

    /*
    ==========================
    STEP 1 — START SIGNAL
    ==========================
    MCU tarik LOW minimal 18ms
    */
    set_output(pin);
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    /*
    MCU tarik HIGH 20-40us
    */
    gpio_set_level(pin, 1);
    esp_rom_delay_us(30);

    /*
    Ubah pin jadi input untuk baca respon sensor
    */
    set_input(pin);

    /*
    ==========================
    STEP 2 — RESPONSE SENSOR
    ==========================
    Sensor LOW 80us lalu HIGH 80us
    */

    if (wait_for_level(pin, 0, 100) < 0)
        return ESP_FAIL;

    if (wait_for_level(pin, 1, 100) < 0)
        return ESP_FAIL;

    if (wait_for_level(pin, 0, 100) < 0)
        return ESP_FAIL;

    /*
    ==========================
    STEP 3 — READ 40 BIT DATA
    ==========================
    Format data:
    8 bit humidity int
    8 bit humidity decimal
    8 bit temperature int
    8 bit temperature decimal
    8 bit checksum
    */

    for (int i = 0; i < 40; i++) {

        /* Tunggu sinyal LOW awal bit */
        if (wait_for_level(pin, 1, 70) < 0)
            return ESP_FAIL;

        /* Ukur durasi HIGH */
        int duration = wait_for_level(pin, 0, 100);

        if (duration < 0)
            return ESP_FAIL;

        /*
        Jika HIGH > 40us → bit = 1
        Jika HIGH < 40us → bit = 0
        */
        int bit = (duration > 40);

        data[i / 8] <<= 1;
        data[i / 8] |= bit;
    }

    /*
    ==========================
    STEP 4 — CHECKSUM VALIDATION
    ==========================
    */
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];

    if (checksum != data[4])
        return ESP_FAIL;

    /*
    Simpan hasil
    */
    result->humidity = data[0];
    result->temperature = data[2];

    return ESP_OK;
}