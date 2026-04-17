/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define LCD_CS_Pin GPIO_PIN_5
#define LCD_CS_GPIO_Port GPIOA

#define TCH_CS_Pin GPIO_PIN_5
#define TCH_CS_GPIO_Port GPIOD

#define RESET_Pin GPIO_PIN_8
#define RESET_GPIO_Port GPIOB

#define DC_RS_Pin GPIO_PIN_9
#define DC_RS_GPIO_Port GPIOB

#define Touch_IRQ_Pin GPIO_PIN_6
#define Touch_IRQ_GPIO_Port GPIOD
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

typedef enum {
    STATE_BOOT_HOMING,      // 0: System booting, homing the gantry
    STATE_AUTO_DRAW,        // 1: Drawing default CSV patterns
    STATE_JOYSTICK_MODE,    // 2: Manual control via physical joystick
    STATE_LCD_MODE,         // 3: User is drawing on the touchscreen
	 STATE_LCD_INTER,
    STATE_IR_MODE,          // 4: User is drawing with Wii remote
    STATE_CLEARING          // 5: Mechanical wiping routine
} SystemState;


/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_CS_Pin GPIO_PIN_5
#define LCD_CS_GPIO_Port GPIOA
#define Y_axis_Direction_Pin GPIO_PIN_0
#define Y_axis_Direction_GPIO_Port GPIOB
#define X_axis_Limit_Pin GPIO_PIN_10
#define X_axis_Limit_GPIO_Port GPIOE
#define X_axis_Limit_EXTI_IRQn EXTI15_10_IRQn
#define Y_axis_Limit_Pin GPIO_PIN_14
#define Y_axis_Limit_GPIO_Port GPIOE
#define Y_axis_Limit_EXTI_IRQn EXTI15_10_IRQn
#define X_axis_Enable_Pin GPIO_PIN_11
#define X_axis_Enable_GPIO_Port GPIOD
#define X_axis_Direction_Pin GPIO_PIN_13
#define X_axis_Direction_GPIO_Port GPIOD
#define TCH_CS_Pin GPIO_PIN_5
#define TCH_CS_GPIO_Port GPIOD
#define Touch_IRQ_Pin GPIO_PIN_6
#define Touch_IRQ_GPIO_Port GPIOD
#define Touch_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define RESET_Pin GPIO_PIN_8
#define RESET_GPIO_Port GPIOB
#define DC_RS_Pin GPIO_PIN_9
#define DC_RS_GPIO_Port GPIOB
#define Y_axis_Enable_Pin GPIO_PIN_0
#define Y_axis_Enable_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */
extern volatile uint8_t manual_mode;
extern volatile SystemState current_state;
#define MAX_LCD_POINTS 300 // How many points the user can draw
extern volatile int user_lcd_path[MAX_LCD_POINTS][2];
extern volatile int user_path_length;
extern volatile int sendingflag;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
