#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <stdint.h>

/* display events */
typedef enum
{
    DISPLAY_EVENT_VEHICLE_OFF = 0,
    DISPLAY_EVENT_VEHICLE_ON,
    DISPLAY_EVENT_DOOR_OPEN,
    DISPLAY_EVENT_OIL_LEVEL_WARNING,
    DISPLAY_EVENT_FUEL_LEVEL,
} display_events_t;

/* initialize display */
void display_init(uint8_t priority, uint32_t stack);

/* update display */
void display_update(display_events_t event, uint32_t value);

#endif /* __DISPLAY_H_ */
