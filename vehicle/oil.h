#ifndef __OIL_H_
#define __OIL_H_

#include <stdint.h>

/* init oil management */
void oil_init(uint8_t priority, uint32_t stack);

/* stop oil management */
void oil_stop();

/* start oil management */
void oil_start();

/* get oil level */
uint32_t oil_get();

#endif /* __OIL_H_ */
