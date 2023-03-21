#include "FreeRTOS.h"
#include "task.h"
#include "elw_hal.h"

void error(char *msg)
{
    elw_uart_tx_string(msg);
}

void panic(char *msg)
{
    error(msg);
    for(;;);
}

void timer_runtime_stats_configure()
{
    elw_timer_init();
    elw_timer_start(100);
}

uint32_t timer_runtime_stats_get_counter()
{
    return elw_timer_counter_get();
}

void vApplicationMallocFailedHook() 
{ 
    error("Malloc failed");
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    error("Stack overflow at task: ");
    error((char *)pcTaskName);
}
