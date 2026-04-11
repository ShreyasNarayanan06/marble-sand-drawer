/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gmove.h"
#include "stdio.h"
#include "joystick.h"
#include "lcd_touch.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define percentDist(p) ((p) * 5.03238)

#define percentIR(p) (uint16_t)(((uint32_t)(p) * 100) / 1023)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */






/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint8_t manual_mode = 0;
volatile uint8_t ir_mode = 1;


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_LPUART1_UART_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  //JoyCal joy = joystick_calibrate();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 // joyMove(1.0f, 0.0f); // Force full speed X
//    JoyCal joy = {
//    		  2000, 1900,
//          0.00096237, 0.00048359,
//          0.00001819, -0.00048837
//    };

//  float x_out, y_out, mag, angle;
//  uint8_t raw[4];
//
//  if (ir_mode) Gantry_Home();
//
//
//  while (1)
//  {
//	  if(manual_mode) {
//		  int rawX = getX();
//		  int rawY = getY();
//		  //joystick_correct(&joy, rawX, rawY, &x_out, &y_out, &mag, &angle);
//		  joyMove(x_out, y_out);
//		  HAL_Delay(20);
//	  }
//	  else if (ir_mode) {
//		  uint8_t start = 0;
//		      HAL_StatusTypeDef status;
//
//		       status = HAL_UART_Receive(&huart1, &start, 1, 1000);
//		       if (status != HAL_OK)
//		       {
//		           printf("Timeout or RX fail waiting for start\r\n");
//		           continue;
//		       }
//
//
//		       if (start != 0xAA)
//		       {
//		           continue;
//		       }
//
//		       uint8_t raw[4] = {0};
//		       status = HAL_UART_Receive(&huart1, raw, 4, 1);
//		       if (status != HAL_OK)
//		       {
//		           printf("Payload RX fail\r\n");
//		           continue;
//		       }
//
//		       uint16_t x = ((uint16_t)raw[0] << 8) | raw[1];
//		       uint16_t y = ((uint16_t)raw[2] << 8) | raw[3];
//
//		       uint16_t irx = percentIR(x);
//		       uint16_t iry = percentIR(y);
//
//		       printf("raw = %02X %02X %02X %02X | x = %u, y = %u\r\n",
//		              raw[0], raw[1], raw[2], raw[3], irx, iry);
//
//		       if (irx == 100 && iry == 100) {
//		    	   continue;
//		       }
//
//		       else {
//		    	  lineMove(percentDist(irx), percentDist(iry), 100);
//		       }
//
//		       HAL_Delay(10);
//	  }
//
//	  else {
//		  printf("auto mode\r\n");
//		  Gantry_Home();
//		  procCSV();
//		  manual_mode = 1;
//	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
 // }
  LCD_Init();
    LCD_ClearScreen();
    LCD_DrawBorder();
    while(1) {}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&hlpuart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    Touch_EXTI_Callback(GPIO_Pin);
    GMove_EXTI_Callback(GPIO_Pin);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
