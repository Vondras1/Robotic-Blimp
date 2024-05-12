#ifndef DIODS_H
#define DIODS_H

#include "GeneralLib.h"

const unsigned int LAND_LED = 9;
const unsigned int UP_LED = 19;
const unsigned int DOWN_LED = 18;
const unsigned int FLY_FORWARD_LED = 5;

void control_diodes(command& cmd, bool neww);
void update_LAND_LED(void);
void update_FORWARD_FLY_LED(void);
void turn_off_LEDS_after_interval(void);
void light_up_after_new_cmd(const command& cmd, bool neww);


#endif // DIODS_H