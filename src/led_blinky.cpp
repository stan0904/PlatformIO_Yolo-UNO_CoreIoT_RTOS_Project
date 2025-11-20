#include "led_blinky.h"
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// These are defined in another module (e.g. main.cpp)
extern float g_temperatureC;              // Last measured temperature in °C
extern SemaphoreHandle_t xTempSemaphore;  // "New temperature ready" notification

// Private helper functions for different blink patterns
static void blinkCool();
static void blinkComfort();
static void blinkHot();

void led_blinky(void *pvParameters)
{
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LOW);

    // Max time we wait for a new temperature before reusing old value
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(3000);

    for (;;)
    {
        /*
         * Wait (block) until:
         *  - The temperature task "gives" the semaphore (xSemaphoreGive)
         *    => new temperature is available in g_temperatureC
         *  - OR the timeout expires (no new update, still use last value)
         */
        if (xSemaphoreTake(xTempSemaphore, xMaxBlockTime) == pdTRUE)
        {
            // Case 1: semaphore taken successfully
            // => temperature has just been updated by the sensor task
        }
        else
        {
            // Case 2: timeout (no xSemaphoreGive() during xMaxBlockTime)
            // => continue using the last temperature value
        }

        float temp = g_temperatureC;  // Read the shared temperature value

        // ----- Condition handling: 3 different LED behaviors -----
        if (temp < 25.0f)
        {
            // COOL RANGE: T < 25°C
            // Slow, calm blink pattern
            blinkCool();
        }
        else if (temp < 35.0f)
        {
            // COMFORT RANGE: 25°C ≤ T < 35°C
            // Medium speed blink pattern
            blinkComfort();
        }
        else
        {
            // HOT RANGE: T ≥ 35°C
            // Fast "alarm" blink pattern to warn about overheating
            blinkHot();
        }
    }
}

// --------------------- Blink pattern implementations ---------------------

// COOL: 2 slow pulses (1 s ON, 1 s OFF)
static void blinkCool()
{
    for (int i = 0; i < 2; ++i)
    {
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 s ON
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 s OFF
    }
}

// COMFORT: 4 medium pulses (400 ms ON, 400 ms OFF)
static void blinkComfort()
{
    for (int i = 0; i < 4; ++i)
    {
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(400));   // 0.4 s ON
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(400));   // 0.4 s OFF
    }
}

// HOT: 10 fast pulses (100 ms ON, 100 ms OFF) as alarm
static void blinkHot()
{
    for (int i = 0; i < 10; ++i)
    {
        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));   // 0.1 s ON
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));   // 0.1 s OFF
    }

    // Optional pause to make the "alarm burst" recognizable
    vTaskDelay(pdMS_TO_TICKS(500));
}
