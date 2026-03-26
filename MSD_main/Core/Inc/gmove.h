#ifndef GMOVE_H
#define GMOVE_H

#include "main.h"

#define STEPS_PER_MM (800.0 / 41.0)
#define ldX 1
#define ldY 0
#define aX 0
#define aY 1

// Expose variables to main.c
extern volatile int remSteps;
extern volatile double currX;
extern volatile double currY;

// Function prototypes
void Gantry_Home(void);
void mLine(double targetXl, double targetYl);

#endif /* GMOVE_H */
