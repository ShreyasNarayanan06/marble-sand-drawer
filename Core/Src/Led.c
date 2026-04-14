/*
 * Led.c
 *
 *  Created on: Apr 14, 2026
 *      Author: bljiang
 */


#include "Led.h"
#include <math.h>

#define PI 3.14159265


/*Data*/
uint8_t LED_Data[MAX_LED][4];
uint8_t LED_Mod[MAX_LED][4];

/* 24 bits per LED + reset pulse buffer*/
uint16_t pwmData[(24 * MAX_LED) + 50];

volatile uint8_t datasentflag = 0;
extern TIM_HandleTypeDef htim1; // TODO: change this to the tim we use

/*Set everything up to zero*/
void WS2812_Init(void)
{
    for (int i = 0; i < MAX_LED; i++)
    {
        Set_LED(i, 0, 0, 0);
    }
    WS2812_Send();
}


/*
 * 0 < LEDnum <= MAX_LED
 * */
void Set_LED (int LEDnum, int Red, int Green, int Blue)
{
	if (LEDnum < 0 || LEDnum >= MAX_LED) return;
    LED_Data[LEDnum][0] = LEDnum;
    LED_Data[LEDnum][1] = Green;
    LED_Data[LEDnum][2] = Red;
    LED_Data[LEDnum][3] = Blue;
}

/*
 * Set up brightness
 *
 **/

void Set_Brightness (int brightness)
{
#if USE_BRIGHTNESS

    if (brightness > 45) brightness = 45;
    if (brightness < 0) brightness = 0;

    for (int i = 0; i < MAX_LED; i++)
    {
        LED_Mod[i][0] = LED_Data[i][0];

        for (int j = 1; j < 4; j++)
        {
//        	float angle = (90 - brightness) * PI / 180;
//          LED_Mod[i][j] = (LED_Data[i][j]) / tan(angle);
        	float angle = (90.0f - brightness) * PI / 180.0f;
        	LED_Mod[i][j] = (uint8_t)((float)LED_Data[i][j] / tanf(angle));

        }
    }

#endif
}

/*
 * SEND DATA
 * Added green, red, and blue for debugging help
 * we have a 4 MHz clock so to step down to 800 kHz we need to
 * PSC = 0, ARR = 4 (rem STM does +1)
 * */
void WS2812_Send(void)
{
    uint32_t indx = 0;

    for (int i = 0; i < MAX_LED; i++)
    {

#if USE_BRIGHTNESS
        uint8_t green = LED_Mod[i][1];
        uint8_t red   = LED_Mod[i][2];
        uint8_t blue  = LED_Mod[i][3];
#else
        uint8_t green = LED_Data[i][1];
        uint8_t red   = LED_Data[i][2];
        uint8_t blue  = LED_Data[i][3];
#endif

        uint32_t color = (green << 16) | (red << 8) | blue; /*build color from green red blue*/

        for (int i = 23; i >= 0; i--)
        {
            if (color & (1 << i))
            {
                pwmData[indx] = 13; /* HIGH longer → 1 */
            }
            else
            {
                pwmData[indx] = 7; /* HIGH shorter → 0 */
            }
            indx++;
        }
    }

    /*
     * reset pulse (LOW for >50us)
     * Calibration might be needed
     * */
    for (int i = 0; i < 50; i++)
    {
        pwmData[indx++] = 0;
    }

    // start DMA
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)pwmData, indx); // TODO: tim we are using
    while(!datasentflag){};
    datasentflag = 0;
}

/*Call Back*/
/*For milestone one we use PC8*/
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) // TODO: tim we are using
    {
        HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1); // TODO: tim we are using
        datasentflag = 1;
    }
}
