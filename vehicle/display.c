#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "elw_hal.h"
#include "check.h"
#include "display.h"

/* structure to update display */
typedef struct {
    display_events_t event;
    uint32_t value;
} display_info_t;

/* queue to update display */
static QueueHandle_t display_queue = NULL;

static void display_format_str(display_info_t *info, char *msg)
{
    if (info->event == DISPLAY_EVENT_FUEL_LEVEL) {
        if (info->value > 50)
            info->value = 50;
        msg[0] = (info->value / 10) + 0x30;
        msg[1] = (info->value % 10) + 0x30;
    }
    else {
        msg[3] = info->event + 0x30;
    }
}

static void display_task(void *pvParameters)
{
    display_info_t info;
    char msg[5] = "50 0";

    for (;;) {
        elw_lcd_clear();
        elw_lcd_write_string(msg);
        xQueueReceive(display_queue, &info, portMAX_DELAY);
        display_format_str(&info, msg);
    }
}

void display_update(display_events_t event, uint32_t value)
{
    display_info_t info;

    if (display_queue != NULL) {

        info.event = event;
        info.value = value;

        if (xQueueSend(display_queue, &info, 0) != pdTRUE) {
            error("Could not update display");
        }
    }
}

void display_init(uint8_t priority, uint32_t stack)
{
    if ((display_queue = xQueueCreate(5, sizeof(display_info_t))) != NULL) {
        if (xTaskCreate(display_task, "display", stack, NULL, priority, NULL) != pdPASS) {
            error("Could not create display task");
        }
    }
    else {
        error("Could not create display queue");
    }
}
