#ifndef __LOG_H_
#define __LOG_H_

#include <stdint.h>

/* messages IDs */
typedef enum
{
    LOG_EVENT_VEHICLE_ON,
    LOG_EVENT_VEHICLE_OFF,
    LOG_EVENT_DOOR_OPEN,
    LOG_EVENT_DOOR_CLOSE,
    LOG_EVENT_COLLISION_DETECTED,
    LOG_EVENT_OIL_LEVEL_WARNING,
    LOG_EVENT_NO_FUEL,
    LOG_EVENT_FUELING,
} log_events_t;

/* initialize log */
void log_init(uint8_t priority, uint32_t stack);

/* log an event */
void log_do(log_events_t event);

/* get logs in formatted string */
void log_get(char *buf, size_t size);

#endif /* __LOG_H_ */
