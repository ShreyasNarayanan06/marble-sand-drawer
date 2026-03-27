#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "main.h"
#include <math.h>

// The calibration matrix struct
typedef struct {
    float x0, y0;      // center offset
    float a11, a12;    // calibration matrix A
    float a21, a22;
} JoyCal;

// Function prototypes
int getX(void);
int getY(void);
void joystick_correct(const JoyCal *cal, float x_raw, float y_raw, float *x_out, float *y_out, float *mag, float *angle);

// Let's wrap your massive calibration script into one clean function!
JoyCal joystick_calibrate(void);

#endif // JOYSTICK_H


