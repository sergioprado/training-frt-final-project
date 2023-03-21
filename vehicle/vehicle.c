#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "elw_hal.h"
#include "check.h"
#include "collision.h"
#include "oil.h"
#include "fuel.h"
#include "log.h"
#include "display.h"
#include "serial.h"
#include "vehicle.h"
#include "tasks_info.h"

/* ignition LED definition */
#define VEHICLE_LED_IGNITION  ELW_LED_GREEN

/* vehicle control id */
typedef enum {
    CONTROL_ID_IGNITION,
    CONTROL_ID_DOOR
} vehicle_control_id_t;

/* vehicle control status */
typedef enum {
    CONTROL_STATUS_ON,
    CONTROL_STATUS_OFF,
    CONTROL_STATUS_TOGGLE,
    CONTROL_STATUS_INVALID,
} vehicle_control_status_t;

/* vehicle control structure */
typedef struct {
    vehicle_control_id_t id;
    vehicle_control_status_t status;
} vehicle_control_t;

/* queue to communicate with control task */
static xQueueHandle vehicle_queue = NULL;

/* vehicle current status */
static vehicle_control_status_t vehicle_ignition_status;
static vehicle_control_status_t vehicle_door_status;

static void vehicle_system_init()
{
    collision_init(COLLISION_TASK_PRIORITY, COLLISION_TASK_STACK_SIZE);
    oil_init(OIL_TASK_PRIORITY, OIL_TASK_STACK_SIZE);
    fuel_init(FUEL_TASK_PRIORITY, FUEL_TASK_STACK_SIZE,
            REFUELING_TASK_PRIORITY, REFUELING_TASK_STACK_SIZE);
    log_init(LOG_TASK_PRIORITY, LOG_TASK_STACK_SIZE);
    display_init(DISPLAY_TASK_PRIORITY, DISPLAY_TASK_STACK_SIZE);
    serial_init(SERIAL_TASK_PRIORITY, SERIAL_TASK_STACK_SIZE);
}

static void vehicle_stop()
{
    collision_stop();
    oil_stop();
    fuel_stop();
}

static void vehicle_start()
{
    collision_start();
    oil_start();
    fuel_start();
}

static void vehicle_ignition_on(void)
{
    if (vehicle_door_status == CONTROL_STATUS_OFF &&
            vehicle_ignition_status != CONTROL_STATUS_ON) {
        vehicle_ignition_status = CONTROL_STATUS_ON;
        elw_led_set(VEHICLE_LED_IGNITION, ELW_LED_ON);
        collision_reset();
        vehicle_start();
        display_update(DISPLAY_EVENT_VEHICLE_ON, 0);
        log_do(LOG_EVENT_VEHICLE_ON);
    }
}

static void vehicle_ignition_off(void)
{
    if (vehicle_ignition_status != CONTROL_STATUS_OFF) {
        vehicle_stop();
        vehicle_ignition_status = CONTROL_STATUS_OFF;
        elw_led_set(VEHICLE_LED_IGNITION, ELW_LED_OFF);
        display_update(DISPLAY_EVENT_VEHICLE_OFF, 0);
        log_do(LOG_EVENT_VEHICLE_OFF);
    }
}

static void vehicle_door_open(void)
{
    if (vehicle_ignition_status == CONTROL_STATUS_OFF &&
            vehicle_door_status == CONTROL_STATUS_OFF) {
        vehicle_door_status = CONTROL_STATUS_ON;
        display_update(DISPLAY_EVENT_DOOR_OPEN, 0);
        log_do(LOG_EVENT_DOOR_OPEN);
    }
}

static void vehicle_door_close(void)
{
    if (vehicle_door_status != CONTROL_STATUS_OFF) {
        vehicle_door_status = CONTROL_STATUS_OFF;
        display_update(DISPLAY_EVENT_VEHICLE_OFF, 0);
        log_do(LOG_EVENT_DOOR_CLOSE);
    }
}

static void vehicle_ignition_control(vehicle_control_status_t status)
{
    if (status == CONTROL_STATUS_TOGGLE) {
        if (vehicle_ignition_status == CONTROL_STATUS_ON)
            status = CONTROL_STATUS_OFF;
        else
            status = CONTROL_STATUS_ON;
    }

    switch (status) {

    case CONTROL_STATUS_ON:
        vehicle_ignition_on();
        break;

    case CONTROL_STATUS_OFF:
        vehicle_ignition_off();
        break;

    case CONTROL_STATUS_TOGGLE:
    default:
        break;
    }
}

static void vehicle_door_control(vehicle_control_status_t status)
{
    if (status == CONTROL_STATUS_TOGGLE) {
        if (vehicle_door_status == CONTROL_STATUS_ON)
            status = CONTROL_STATUS_OFF;
        else
            status = CONTROL_STATUS_ON;
    }

    switch (status) {

    case CONTROL_STATUS_ON:
        vehicle_door_open();
        break;

    case CONTROL_STATUS_OFF:
        vehicle_door_close();
        break;

    case CONTROL_STATUS_TOGGLE:
    default:
        break;
    }
}

static void vehicle_status_init()
{
    vehicle_ignition_status = CONTROL_STATUS_INVALID;
    vehicle_door_status     = CONTROL_STATUS_INVALID;

    vehicle_door_close();
    vehicle_ignition_off();
}

static void btn_isr(uint8_t btn)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vehicle_control_t control;

    switch (btn) {

    case ELW_BTN_SW1:
        control.id = CONTROL_ID_IGNITION;
        control.status = CONTROL_STATUS_TOGGLE;
        xQueueSendFromISR(vehicle_queue, &control, &xHigherPriorityTaskWoken);
        break;

    case ELW_BTN_SW3:
        control.id = CONTROL_ID_DOOR;
        control.status = CONTROL_STATUS_TOGGLE;
        xQueueSendFromISR(vehicle_queue, &control, &xHigherPriorityTaskWoken);
        break;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void vehicle_task(void *pvParameters)
{
    vehicle_control_t control;

    vehicle_system_init();

    vehicle_status_init();

    elw_btn_irq_enable(btn_isr);

    vTaskPrioritySet(NULL, CONTROL_TASK_RUNTIME_PRIORITY);

    for (;;) {

        xQueueReceive(vehicle_queue, &control, portMAX_DELAY);

        switch (control.id) {

        case CONTROL_ID_IGNITION:
            vehicle_ignition_control(control.status);
            break;

        case CONTROL_ID_DOOR:
            vehicle_door_control(control.status);
            break;
        }
    }
}

uint8_t vehicle_door_status_get()
{
    if (vehicle_door_status == CONTROL_STATUS_ON)
        return 1;
    else
        return 0;
}

uint8_t vehicle_ignition_status_get()
{
    if (vehicle_ignition_status == CONTROL_STATUS_ON)
        return 1;
    else
        return 0;
}

void vehicle_stop_request()
{
    vehicle_control_t control;

    control.id     = CONTROL_ID_IGNITION;
    control.status = CONTROL_STATUS_OFF;

    xQueueSend(vehicle_queue, &control, portMAX_DELAY);
}

void vehicle_init()
{
    if ((vehicle_queue = xQueueCreate(4, sizeof(vehicle_control_t))) == NULL) {
        panic("Could not create vehicle queue");
    }

    if (xTaskCreate(vehicle_task, "vehicle", CONTROL_TASK_STACK_SIZE,
            NULL, CONTROL_TASK_INIT_PRIORITY, NULL) != pdPASS) {
        panic("Could not create vehicle task");
    }
}
