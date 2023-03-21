#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "elw_hal.h"
#include "check.h"
#include "log.h"
#include "serial.h"
#include "fuel.h"
#include "oil.h"
#include "vehicle.h"

/* states of the state machines */
typedef enum {
    SERIAL_STATE_CMD,
    SERIAL_STATE_DATA
} serial_state_t;

/* available commands */
#define SERIAL_CMD_FUEL		'0'
#define SERIAL_CMD_LOG		'1'
#define SERIAL_CMD_STATUS	'2'
#define SERIAL_CMD_STAT		'3'
#define SERIAL_CMD_STACK	'4'

/* queue to update display */
static QueueHandle_t serial_queue;

/* buffer to format messages */
static char serial_buf[1024];

static void serial_msg_send(char *msg, int newline)
{
    elw_uart_tx_string(msg);
    while (newline) {
        elw_uart_tx_string("\r\n");
        newline--;
    }
}

static void serial_cmd_handle_fuel(unsigned char *data)
{
    uint8_t fuel;

    fuel = (data[0] - 0x30) * 10 + (data[1] - 0x30);

    if (fuel)
        fuel_set(fuel);
}

static void serial_cmd_handle_log()
{
    log_get(serial_buf, sizeof(serial_buf));
    serial_msg_send(serial_buf, 1);
}

static void serial_cmd_handle_status()
{
    snprintf(serial_buf, sizeof(serial_buf), "Veiculo................: %s",
            vehicle_ignition_status_get() ? "ligado" : "desligado");
    serial_msg_send(serial_buf, 1);

    snprintf(serial_buf, sizeof(serial_buf), "Porta..................: %s",
            vehicle_door_status_get() ? "aberta" : "fechada");
    serial_msg_send(serial_buf, 1);

    snprintf(serial_buf, sizeof(serial_buf), "Nivel de oleo..........: %02d%%",
            oil_get());
    serial_msg_send(serial_buf, 1);

    snprintf(serial_buf, sizeof(serial_buf), "Nivel de combustivel...: %02d%%",
            fuel_get());
    serial_msg_send(serial_buf, 2);
}

static void serial_cmd_handle_stat()
{
    serial_msg_send("Task            Abs Time        % Time", 1);
    serial_msg_send("****************************************", 1);

    vTaskGetRunTimeStats(serial_buf);
    serial_msg_send(serial_buf, 1);
}

static void serial_cmd_handle_stack()
{
    serial_msg_send("Task           State  Priority  Stack   #", 1);
    serial_msg_send("************************************************", 1);

    vTaskList(serial_buf);
    serial_msg_send(serial_buf, 1);
}

static void serial_cmd_handle(uint8_t cmd, uint8_t *data)
{
    switch (cmd) {

    case SERIAL_CMD_FUEL:
        serial_cmd_handle_fuel(data);
        break;

    case SERIAL_CMD_LOG:
        serial_cmd_handle_log();
        break;

    case SERIAL_CMD_STATUS:
        serial_cmd_handle_status();
        break;

    case SERIAL_CMD_STAT:
        serial_cmd_handle_stat();
        break;

    case SERIAL_CMD_STACK:
        serial_cmd_handle_stack();
        break;
    }
}

static void serial_data_handle(uint8_t data)
{
    static serial_state_t state = SERIAL_STATE_CMD;
    static uint32_t ind = 0;
    static uint8_t buf[2] = { 0 };

    switch (state) {

    case SERIAL_STATE_CMD:
        if (data == SERIAL_CMD_FUEL) {
            state = SERIAL_STATE_DATA;
        }
        else
        {
            serial_cmd_handle(data, buf);
        }
        break;

    case SERIAL_STATE_DATA:
        buf[ind++] = data;
        if (ind == 2) {
            serial_cmd_handle(SERIAL_CMD_FUEL, buf);
            state = SERIAL_STATE_CMD;
            ind = 0;
        }
        break;
    }
}

static void serial_task(void *pvParameters)
{
    uint8_t data;

    for (;;) {
        xQueueReceive(serial_queue, &data, portMAX_DELAY);
        serial_data_handle(data);
    }
}

static void uart_isr(uint8_t data)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xQueueSendFromISR(serial_queue, &data, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void serial_init(uint8_t priority, uint32_t stack)
{
    if ((serial_queue = xQueueCreate(8, sizeof(uint8_t))) == NULL) {
        error("Could not create serial queue!");
        return;
    }

    if (xTaskCreate(serial_task, "serial", stack, NULL, priority, NULL) != pdPASS) {
        error("Could not create serial task");
        return;
    }

    elw_uart_rx_irq_enable(uart_isr);
}
