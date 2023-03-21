#ifndef __COLLISION_H_
#define __COLLISION_H_

#include <stdint.h>

/* init collision management */
void collision_init(uint8_t priority, uint32_t stack);

/* stop collision management */
void collision_stop();

/* start collision management */
void collision_start();

/* reset collision status */
void collision_reset();

#endif /* __COLLISION_H_ */
