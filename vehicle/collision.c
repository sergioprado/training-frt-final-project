#include "elw_hal.h"
#include "vehicle.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
#include "check.h"
#include "collision.h"

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} acc_data_t;

/* airbag led */
#define LED_AIRBAG  ELW_LED_RED

/* collision task handler */
static xTaskHandle collision_task_handler;

static int collision_check(acc_data_t *acc)
{
    static uint8_t collided = 0;

    if (acc->y < -30)
        collided++;
    else
        collided = 0;

    if (collided >= 3)
        return 1;
    else
        return 0;
}

static void collision_airbag_on()
{
    elw_led_set(LED_AIRBAG, ELW_LED_ON);
}

static void collision_airbag_off()
{
    elw_led_set(LED_AIRBAG, ELW_LED_OFF);
}

static void collision_task(void *pvParameters)
{
    acc_data_t acc;

    for (;;) {

        vTaskDelay(30/portTICK_PERIOD_MS);

        elw_accel_read(&acc.x, &acc.y, &acc.z);

        if (collision_check(&acc)) {
            collision_airbag_on();
            vehicle_stop_request();
            log_do(LOG_EVENT_COLLISION_DETECTED);
        }
    }
}

void collision_reset()
{
    collision_airbag_off();
}

void collision_start()
{
    vTaskResume(collision_task_handler);
}

void collision_stop()
{
    vTaskSuspend(collision_task_handler);
}

void collision_init(uint8_t priority, uint32_t stack)
{
    if (xTaskCreate(collision_task, "collision", stack, NULL, priority,
            &collision_task_handler) != pdPASS) {
        panic("Could not create collision task");
    }
}
