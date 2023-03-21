#ifndef __TASKS_INFO_H_
#define __TASKS_INFO_H_

/* priority */
#define CONTROL_TASK_INIT_PRIORITY     5
#define COLLISION_TASK_PRIORITY        4
#define OIL_TASK_PRIORITY              3
#define FUEL_TASK_PRIORITY             3
#define REFUELING_TASK_PRIORITY        3
#define SERIAL_TASK_PRIORITY           3
#define CONTROL_TASK_RUNTIME_PRIORITY  2
#define DISPLAY_TASK_PRIORITY          1
#define LOG_TASK_PRIORITY              1

/* stack */
#define CONTROL_TASK_STACK_SIZE     128
#define COLLISION_TASK_STACK_SIZE   128
#define OIL_TASK_STACK_SIZE         96
#define FUEL_TASK_STACK_SIZE        128
#define REFUELING_TASK_STACK_SIZE   96
#define LOG_TASK_STACK_SIZE         128
#define DISPLAY_TASK_STACK_SIZE     96
#define SERIAL_TASK_STACK_SIZE      256

#endif /* __TASKS_INFO_H_ */
