#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "elw_hal.h"
#include "check.h"
#include "log.h"

/* log time */
typedef struct {
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
} log_time_t;

/* log information */
typedef struct {
    log_events_t event;
    log_time_t time;
} log_info_t;

/* log constants */
#define MAX_LOG_MSG_SIZE	51
#define MAX_LOG_ITENS		10

/* log buffer */
static log_info_t log_info[MAX_LOG_ITENS];
static unsigned int log_index = 0, log_full = 0;

/* queue to log information */
static QueueHandle_t log_queue = NULL;

/* default messages */
static const char *log_messages[] = {
        "Veiculo ligado",									// 0
        "Veiculo desligado",								// 1
        "Porta aberta",										// 2
        "Porta fechada",									// 3
        "Colisao detectada",								// 4
        "Nivel de oleo do motor fora da faixa aceitavel",	// 5
        "Veiculo sem combustivel",							// 6
        "Abastecimento de combustivel realizado",			// 7
};

static void log_msg_append(char *buf, size_t size, int ind)
{
    snprintf(buf, size, "%s%02d:%02d:%02d - %s\r\n", buf,
            log_info[ind].time.hour, log_info[ind].time.min,
            log_info[ind].time.sec, log_messages[log_info[ind].event]);
}

static void log_save(log_events_t event)
{
    struct elw_rtc_datetime datetime;

    /* save message */
    log_info[log_index].event = event;

    /* save time */
    elw_rtc_datetime_get(&datetime);
    log_info[log_index].time.hour = datetime.hour;
    log_info[log_index].time.min  = datetime.minute;
    log_info[log_index].time.sec  = datetime.second;

    /* update index */
    log_index++;
    if (log_index == MAX_LOG_ITENS) {
        log_index = 0;
        log_full = 1;
    }
}

static void log_task(void *pvParameters)
{
    log_events_t event;

    for (;;) {
        xQueueReceive(log_queue, &event, portMAX_DELAY);
        log_save(event);
    }
}

void log_do(log_events_t event)
{
    if (log_queue != NULL) {
        if (xQueueSend(log_queue, &event, 0) != pdTRUE) {
            error("Could not log event because queue is full!");
        }
    }
    else {
        error("Cannot log event because log queue is not available!");
    }
}

void log_get(char *buf, size_t size)
{
    int i;

    buf[0] = 0;

    if (!log_full) {
        for (i = 0; i < log_index; i++)
            log_msg_append(buf, size, i);
    }
    else {

        for (i = log_index; i < MAX_LOG_ITENS; i++)
            log_msg_append(buf, size, i);

        for (i = 0; i < log_index; i++)
            log_msg_append(buf, size, i);
    }
}

void log_init(uint8_t priority, uint32_t stack)
{
    if ((log_queue = xQueueCreate(MAX_LOG_ITENS, sizeof(log_events_t))) != NULL) {
        if (xTaskCreate(log_task, "log", stack, NULL, priority, NULL) != pdPASS) {
            error("Could not create log task");
        }
    }
    else {
        error("Could not create log queue");
    }
}
