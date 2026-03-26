#include "main.h"
#include "tim.h"
#include "gpio.h"

volatile int32_t current_x_steps = 0;
volatile int32_t current_y_steps = 0;

volatile uint8_t x_homing = 0;
volatile uint8_t y_homing = 0;

volatile uint8_t x_is_homed = 0;
volatile uint8_t y_is_homed = 0;

volatile int32_t targetX = 0;
volatile int32_t targetY = 0;
volatile double currX = 0.0;
volatile double currY = 0.0;
volatile int8_t dirX = 1;
volatile int8_t dirY = 1;

volatile int32_t debugsteps = 0;
volatile double debugdist = 0;
volatile double tmm = 0;

#define STEPS_PER_MM (800.0 / 41.0)
#define ldX 1
#define ldY 0
#define aX 0
#define aY 1

//2ax movement stuff
volatile int dX = 0.0;
volatile int dY = 0.0;
volatile int bErr = 0;
volatile int remSteps = 0;
volatile int ms = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // X-Axis Limit Switch Hit
    if (GPIO_Pin == GPIO_PIN_10) {
    	if (x_homing == 1) {
    		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1); // Kill X motor
    		x_is_homed = 1; // Tell the system X is done
    	}
    }

    // Y-Axis Limit Switch Hit
    if (GPIO_Pin == GPIO_PIN_14) {
    	if (y_homing == 1) {
    		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1); // Kill Y motor
    		y_is_homed = 1;// Tell the system Y is done
    	}
    }

}


void Gantry_Home(void) {

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Switch X (PD12) back to PWM (TIM4)
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	// Switch Y (PA0) back to PWM (TIM2)
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM2; // TIM2 uses AF1 on PA0
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	x_homing = 1;
	y_homing = 1;

    //Enable the motor drivers (Active Low)
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); // X EN
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);  // Y EN

    //Set direction towards the limit switches
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   // X DIR
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);    // Y DIR

    // slow speed
    __HAL_TIM_SET_AUTORELOAD(&htim4, 3000);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 1500);

    __HAL_TIM_SET_AUTORELOAD(&htim2, 3000);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1500);

    //Reset flags in case they got triggered by noise on boot
    x_is_homed = 0;
    y_is_homed = 0;

    // Start both motors!
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);


    //Trap CPU until EXTI fires
    while (x_is_homed == 0) {//add y later
        // Wait for the crash...
    }

    x_homing = 0; //this is done to prevent more interruptions (debouncing method)

    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);  // Y EN

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

   while(y_is_homed == 0) {}

    y_homing = 0;

    HAL_Delay(500);

    //Reverse direction (Away from switch)
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);


    //Turn motor back on
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    // Drive more for clearance
    HAL_Delay(200);

    //Stop motor- homing is finished
    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);



    //recalibrate
    current_x_steps = 0;
    current_y_steps = 0;
    currX = 0;
    currY = 0;
}

//void Gantry_Home(void) {
//    x_homing = 1;
//    y_homing = 1;
//    x_is_homed = 0;
//    y_is_homed = 0;
//
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); //X EN
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   //X DIR
//
//    while (x_is_homed == 0) {
//            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
//            HAL_Delay(1);
//            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
//            HAL_Delay(2);
//    }
//    x_homing = 0;
//
//    HAL_Delay(50);
//
//    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
//
//    y_is_homed = 0;
//
//    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);  //Y EN
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);  //Y DIR
//
//    HAL_Delay(5);
//
//    while (y_is_homed == 0) {
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//        HAL_Delay(1);
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//        HAL_Delay(2);
//    }
//    y_homing = 0;
//
//    HAL_Delay(500);
//
//    //Reverse direction
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
//
//    //go forward a bit
//    for(int i = 0; i < 200; i++) {
//        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
//        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
//        HAL_Delay(1);
//    }
//
//    //en off
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);
//
//    current_x_steps = 0;
//    current_y_steps = 0;
//    currX = 0;
//    currY = 0;
//}

//void mMove(uint8_t axis, double target_mm) {
//	tmm = target_mm;
//    double dist = target_mm - (axis == aX ? currX : currY);
//    debugdist = dist;
//    int8_t dir = (dist >= 0) ? 1 : -1;
//    if (dist < 0) dist = -dist;
//
//    int32_t steps = (int32_t)(dist * STEPS_PER_MM);
//    debugsteps = steps;
//
//    if (axis == aX) {
//        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, (dir == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
//        dirX = dir;
//        targetX = steps;
//        currX = target_mm;
//        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); // Enable X
//        HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1);
//    } else if (axis == aY) {
//        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (dir == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
//        dirY = dir;
//        targetY = steps;
//        currY = target_mm;
//        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET); // Enable Y
//        HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_1);
//    }
//}
//
//void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
//    if (htim->Instance == TIM4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
//        if (targetX > 0) {
//            targetX--;
//        } else {
//            HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);
//        }
//    }
//    else if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
//        if (targetY > 0) {
//            targetY--;
//        } else {
//            HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_1);
//        }
//    }
//}

void mLine(double targetXl, double targetYl) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Switch X (PD12) to GPIO
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	// Switch Y (PA0) to GPIO
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	int32_t tx = (int32_t)(targetXl * STEPS_PER_MM);
	int32_t ty = (int32_t)(targetYl * STEPS_PER_MM);

	int32_t cx = (int32_t)(currX * STEPS_PER_MM);
	int32_t cy = (int32_t)(currY * STEPS_PER_MM);

	dX = tx - cx;
	dY = ty - cy;

	dirX = (dX >= 0) ? 1 : -1;
	dirY = (dY >= 0) ? 1 : -1;

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, (dirX == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (dirY == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);

	dX = (dX > 0) ? dX : -dX;
	dY = (dY > 0) ? dY : -dY;

	if (dX > dY) {
		ms = 1;
		bErr = dX / 2;
		remSteps = dX;
	} else {
		ms = 0;
		bErr = dY / 2;
		remSteps = dY;
	}

	currX = targetXl;
	currY = targetYl;

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);

	HAL_TIM_Base_Start_IT(&htim1);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {

        if (remSteps > 0) {
            if (ms == 1) {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);

                bErr -= dY;
                if (bErr < 0) {
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
                    bErr += dX;
                }
            } else {
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

                bErr -= dX;
                if (bErr < 0) {
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
                    bErr += dY;
                }
            }
            remSteps--;
        } else {
            HAL_TIM_Base_Stop_IT(&htim1);
        }
    }
}
