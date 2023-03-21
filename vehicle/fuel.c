#include "FreeRTOS.h"
#include "task.h"
#include "elw_hal.h"
#include "vehicle.h"
#include "semphr.h"
#include "log.h"
#include "check.h"
#include "display.h"
#include "fuel.h"

/* max fuel value */
#define MAX_FUEL_VALUE	50

/* fuel tasks handlers */
static xTaskHandle fuel_task_handler;
static xTaskHandle refueling_task_handler;

/* queue to update the fuel level */
static QueueHandle_t fuel_queue;

/* mutex to control access to fuel */
static SemaphoreHandle_t fuel_mutex;

/* fuel level */
static uint8_t fuel = MAX_FUEL_VALUE;

/* fuel percentage level */
static uint8_t fuel_percent_level = 100;

static uint8_t fuel_update(int8_t f)
{
    xSemaphoreTake(fuel_mutex, portMAX_DELAY);

    fuel += f;
    if (fuel > MAX_FUEL_VALUE) {
        fuel = MAX_FUEL_VALUE;
    }
    else if (fuel < 0) {
        fuel = 0;
    }
    f = fuel;

    xSemaphoreGive(fuel_mutex);

    fuel_percent_level = f * 100 / MAX_FUEL_VALUE;

    return f;
}

static void refueling_task(void *pvParameters)
{
    uint8_t refueling = 0;

    for (;;) {
        xQueueReceive(fuel_queue, &refueling, portMAX_DELAY);
        fuel_update(refueling);
        display_update(DISPLAY_EVENT_FUEL_LEVEL, fuel);
        log_do(LOG_EVENT_FUELING);
    }
}

static void fuel_task(void *pvParameters)
{
    display_update(DISPLAY_EVENT_FUEL_LEVEL, fuel);

    for (;;) {

        if (!fuel) {
            log_do(LOG_EVENT_NO_FUEL);
            vehicle_stop_request();
        }
        else {
            vTaskDelay(1000/portTICK_PERIOD_MS);
            fuel_update(-1);
            display_update(DISPLAY_EVENT_FUEL_LEVEL, fuel);
        }
    }
}

uint8_t fuel_get()
{
    return fuel_percent_level;
}

void fuel_set(uint8_t fuel)
{
    if (xQueueSend(fuel_queue, &fuel, 0) != pdTRUE) {
        error("Could not set fuel");
    }
}

void fuel_stop()
{
    vTaskSuspend(fuel_task_handler);
    vTaskResume(refueling_task_handler);
}

void fuel_start()
{
    vTaskResume(fuel_task_handler);
    vTaskSuspend(refueling_task_handler);
}

void fuel_init(uint8_t prio1, uint32_t stack1, uint8_t prio2, uint32_t stack2)
{
    if (xTaskCreate(fuel_task, "fuel", stack1, NULL, prio1,
            &fuel_task_handler) != pdPASS) {
        panic("Could not create fuel task");
    }

    if (xTaskCreate(refueling_task, "refueling", stack2, NULL, prio2,
            &refueling_task_handler) != pdPASS) {
        panic("Could not create refueling_task");
    }

    if ((fuel_queue = xQueueCreate(2, sizeof(uint8_t))) == 0) {
        panic("Could not create fuel_queue");
    }

    fuel_mutex = xSemaphoreCreateMutex();
}
