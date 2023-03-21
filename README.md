# training-frt-final-project
FreeRTOS Training Final Project source code.

Instructions to integrate this source code with the development environment:

1. Copy or link the `vehicle` directory to the application's source directory.

2. Add the `vehicle` directory to the list of include paths of the compiler.

3. Enable the following macros in `FreeRTOSConfig.h`:

```
#define INCLUDE_vTaskPrioritySet              1
#define configUSE_MALLOC_FAILED_HOOK          1
#define configCHECK_FOR_STACK_OVERFLOW        2
#define configUSE_TRACE_FACILITY              1
#define configUSE_STATS_FORMATTING_FUNCTIONS  1
#define configGENERATE_RUN_TIME_STATS         1

void timer_runtime_stats_configure();
uint32_t timer_runtime_stats_get_counter();
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() timer_runtime_stats_configure()
#define portGET_RUN_TIME_COUNTER_VALUE() timer_runtime_stats_get_counter()
```

4. Add the following code to the `main()` function:

```
    /* initialize hal */
    elw_init_hal();

    /* initialize vehicle logic */
    vehicle_init();

    /* start the scheduler */
    vTaskStartScheduler();
```
