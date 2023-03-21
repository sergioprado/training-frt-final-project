#ifndef __VEHICLE_H_
#define __VEHICLE_H_

/* initialize vehicle */
void vehicle_init();

/* notify collision */
void vehicle_stop_request();

/* return door status - 0=CLOSED 1=OPENED */
uint8_t vehicle_door_status_get();

/* return ignition status - 0=OFF 1=ON*/
uint8_t vehicle_ignition_status_get();

#endif /* __VEHICLE_H_ */
