#ifndef __FUEL_H_
#define __FUEL_H_

#include <stdint.h>

/* init fuel management */
void fuel_init(uint8_t prio1, uint32_t stack1, uint8_t prio2, uint32_t stack2);

/* stop fuel management */
void fuel_stop();

/* start fuel management */
void fuel_start();

/* get fuel level */
uint8_t fuel_get();

/* set fuel level */
void fuel_set(uint8_t fuel);

#endif /* __FUEL_H_ */
