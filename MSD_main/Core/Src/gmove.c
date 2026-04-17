#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "path_data.h"

extern volatile int user_lcd_path[MAX_LCD_POINTS][2];
extern volatile int user_path_length;
extern volatile int sendingflag;

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


volatile int32_t debugStepsX = 0;
volatile int32_t debugStepsY = 0;
volatile uint32_t debugArrX = 0;
volatile uint32_t debugArrY = 0;
volatile int32_t debugYCB = 0;

#define percentDist(p) ((p) * 5.03238)

volatile double MAX_ARR = 8000;


//void GMove_EXTI_Callback(uint16_t GPIO_Pin) {
    // X-Axis Limit Switch Hit

//}


void Gantry_Home(void) {

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

void mMove(uint8_t axis, double target_mm) {
	tmm = target_mm;
    double dist = target_mm - (axis == aX ? currX : currY);
    debugdist = dist;
    int8_t dir = (dist >= 0) ? 1 : -1;
    if (dist < 0) dist = -dist;

    int32_t steps = (int32_t)(dist * STEPS_PER_MM);
    debugsteps = steps;

    if (axis == aX) {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, (dir == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
        dirX = dir;
        targetX = steps;
        currX = target_mm;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); // Enable X
        HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1);
    } else if (axis == aY) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (dir == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        dirY = dir;
        targetY = steps;
        currY = target_mm;
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET); // Enable Y
        HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_1);
    }
}

void lineMove(double target_x_mm, double target_y_mm, double speed) {
	//while (xMoving || yMoving) {}
    double distX = target_x_mm - currX;
    double distY = target_y_mm - currY;

    int8_t dX = (distX >= 0) ? 1 : -1;
    int8_t dY = (distY >= 0) ? 1 : -1;

    if (distX < 0) distX = -distX;
    if (distY < 0) distY = -distY;

    int32_t stepsX = (int32_t)(distX * STEPS_PER_MM);
    int32_t stepsY = (int32_t)(distY * STEPS_PER_MM);

    if (stepsX == 0 && stepsY == 0) return;

    // Set directions
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, (dX == 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0,  (dY == 1) ? GPIO_PIN_SET   : GPIO_PIN_RESET);

    uint32_t base_arr = -70.01 * speed + MAX_ARR;  // your existing top speed
    uint32_t arrX = base_arr;
    uint32_t arrY = base_arr;

    // Scale the slower axis so both finish at the same time
    if (stepsX > 0 && stepsY > 0) {
        if (stepsX >= stepsY) {
            arrY = (uint32_t)((double)base_arr * (double)stepsX / (double)stepsY);
        } else {
            arrX = (uint32_t)((double)base_arr * (double)stepsY / (double)stepsX);
        }
    }

    dirX = dX;
    dirY = dY;
    targetX = stepsX;
    targetY = stepsY;

    currX = target_x_mm;
    currY = target_y_mm;

    targetX = (stepsX > 0) ? stepsX : -1;
    targetY = (stepsY > 0) ? stepsY : -1;

//    if (stepsX > 0) {
//    	xMoving = 1;
//        __HAL_TIM_SET_AUTORELOAD(&htim4, arrX);
//        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, arrX / 2);
//        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); // Enable X
//        HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1);
//    }
//
//    debugStepsX = stepsX; debugStepsY = stepsY; debugArrX = arrX; debugArrY = arrY;
//
//
//    if (stepsY > 0) {
//    	yMoving = 1;
//        __HAL_TIM_SET_AUTORELOAD(&htim2, arrY);
//        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, arrY / 2);
//        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET); // Enable Y
//        HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_1);
//    }

    if (stepsX > 0) {

	   // Stop timer fully, clear everything, then restart
	   HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);
	   __HAL_TIM_SET_COUNTER(&htim4, 0);
	   __HAL_TIM_SET_AUTORELOAD(&htim4, arrX);
	   __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, arrX / 2);
	   __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_CC1);
	   __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
	   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
	   HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1);
     }

     if (stepsY > 0) {
	   HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_1);
	   __HAL_TIM_SET_COUNTER(&htim2, 0);
	   __HAL_TIM_SET_AUTORELOAD(&htim2, arrY);
	   __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, arrY / 2);
	   __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
	   __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
	   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);
	   HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_1);
     }

     while(targetX >= 0 || targetY >= 0){
    	 if ((current_state == STATE_LCD_MODE) && (sendingflag == 1)) {
    	     		 return;
		 }
    	 if(manual_mode == 1) {
    		 targetX = -1;
    		 targetY = -1;
    		 break;
    	 }

     }
  }





void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM4 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        if (targetX > 0) {
            targetX--;
        }

        if(targetX == 0) {
            HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);
            targetX = -1;
        }
    }
    else if (htim->Instance == TIM2 ){//&& htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    	debugYCB++;
        if (targetY > 0) {
            targetY--;
        }

        if(targetY == 0) {
            HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_1);
            targetY = -1;
        }
    }
}

void procCSV(const double path[][2], int length)
{
    for (int i = 0; i < length; i++) {
        if (manual_mode == 1) break;

        double target_x = path[i][0];
        double target_y = path[i][1];

        lineMove(percentDist(target_x), percentDist(target_y), 100);
    }
}

void procUserDrawing() {
	double tx;
	double ty;
	lineMove(percentDist(100 - user_lcd_path[0][0]), percentDist(user_lcd_path[0][1]), 100);
	//CLEAR HERE

	for(int i = 1; i < user_path_length; i++) {
		if(manual_mode == 1) break;

		tx = 100 - user_lcd_path[i][0];
		ty = user_lcd_path[i][1];

		lineMove(percentDist(tx), percentDist(ty), 100);
	}
}


