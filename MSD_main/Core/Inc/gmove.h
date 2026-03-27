#ifndef GMOVE_H
#define GMOVE_H

#include "main.h"

#define STEPS_PER_MM (800.0 / 41.0)
#define AX 0
#define AY 1

extern volatile int32_t current_x_steps;
extern volatile int32_t current_y_steps;
extern volatile uint8_t x_homing;
extern volatile uint8_t y_homing;
extern volatile uint8_t x_is_homed;
extern volatile uint8_t y_is_homed;
extern volatile int32_t target_x;
extern volatile int32_t target_y;
extern volatile double curr_x;
extern volatile double curr_y;
extern volatile int8_t dir_x;
extern volatile int8_t dir_y;

void Gantry_Home(void);
void mMove(uint8_t axis, double target_mm);
void lineMove(double target_x_mm, double target_y_mm, double speed);
void procCSV(void);

#endif /* GMOVE_H */
