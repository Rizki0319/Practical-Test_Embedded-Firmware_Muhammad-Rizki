#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ENUM MODE LED

typedef enum {
    LED_OFF,
    LED_ON,
    LED_BLINK
} LedMode;

// STRUCT LED

typedef struct {
    const char* name;
    bool activeLevelHigh;
    float frequency;
    LedMode mode;
    bool state;
} LED;

// FUNCTION LED (method pengganti class method)

// Set mode
void LED_setMode(LED* led, LedMode newMode) {
    led->mode = newMode;
}

// Print state (simulasi output)
void LED_printState(LED* led) {

    bool outputLevel = led->activeLevelHigh ? led->state : !led->state;

    printf("%s -> %s (Output Level: %s)\n",
           led->name,
           led->state ? "ON" : "OFF",
           outputLevel ? "HIGH" : "LOW");
}

// ON
void LED_turnOn(LED* led) {
    led->state = true;
    LED_printState(led);
}

// OFF
void LED_turnOff(LED* led) {
    led->state = false;
    LED_printState(led);
}

// Toggle (blink)
void LED_toggle(LED* led) {
    led->state = !led->state;
    LED_printState(led);
}

// TASK LED (jalan independen)

void LED_run(void* param) {

    LED* led = (LED*)param;

    while (1) {

        switch(led->mode) {

            case LED_ON:
                LED_turnOn(led);
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;

            case LED_OFF:
                LED_turnOff(led);
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;

            case LED_BLINK:
                LED_toggle(led);

                float halfPeriod = (1.0 / led->frequency) * 500.0;
                vTaskDelay(pdMS_TO_TICKS((int)halfPeriod));
                break;
        }
    }
}

// MAIN ESP-IDF ENTRY POINT

void app_main() {

    // Membuat "object" LED (struct instance)
    // LED Red → Active LOW → 10Hz
    static LED ledRed = {
        .name = "LED RED",
        .activeLevelHigh = false,
        .frequency = 10,
        .mode = LED_OFF,
        .state = false
    };

    // LED Green → Active HIGH → 5Hz
    static LED ledGreen = {
        .name = "LED GREEN",
        .activeLevelHigh = true,
        .frequency = 5,
        .mode = LED_OFF,
        .state = false
    };

    // LED Blue → Active HIGH → 2Hz
    static LED ledBlue = {
        .name = "LED BLUE",
        .activeLevelHigh = true,
        .frequency = 2,
        .mode = LED_OFF,
        .state = false
    };

    // Set semua ke blink
    LED_setMode(&ledRed, LED_BLINK);
    LED_setMode(&ledGreen, LED_BLINK);
    LED_setMode(&ledBlue, LED_BLINK);

    // Jalankan task independen
    xTaskCreate(LED_run, "Red Task", 2048, &ledRed, 5, NULL);
    xTaskCreate(LED_run, "Green Task", 2048, &ledGreen, 5, NULL);
    xTaskCreate(LED_run, "Blue Task", 2048, &ledBlue, 5, NULL);
}