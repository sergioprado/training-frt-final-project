#include "oil.h"
#include "elw_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
#include "check.h"
#include "display.h"

/* default oil level */
static unsigned int oil_level = 100;

/* oil task handler */
static xTaskHandle oil_task_handler;

static int oil_check(unsigned int oil)
{
    static unsigned int notify = 0;

    oil_level = oil * 100 / 4096;

    if ((oil_level < 5) || (oil_level > 95))
        notify++;
    else
        notify = 0;

    if (notify >= 3)
        return 1;
    else
        return 0;
}

static void oil_task(void *pvParameters)
{
    unsigned int oil, log = 0;

    for (;;) {

        vTaskDelay(300/portTICK_PERIOD_MS);

        oil = elw_light_read();

        if (oil_check(oil)) {
            display_update(DISPLAY_EVENT_OIL_LEVEL_WARNING, 0);
            if (!log) {
                log_do(LOG_EVENT_OIL_LEVEL_WARNING);
                log = 1;
            }
        }
        else {
            display_update(DISPLAY_EVENT_VEHICLE_ON, 0);
            log = 0;
        }
    }
}

uint32_t oil_get()
{
    return oil_level;
}

void oil_start()
{
    vTaskResume(oil_task_handler);
}

void oil_stop()
{
    vTaskSuspend(oil_task_handler);
}

void oil_init(uint8_t priority, uint32_t stack)
{
    if (xTaskCreate(oil_task, "oil", stack, NULL, priority, &oil_task_handler) != pdPASS) {
        panic("Could not create oil task");
    }
}
