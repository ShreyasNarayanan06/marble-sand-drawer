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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gmove.h"
#include "stdio.h"
#include "joystick.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define percentDist(p) ((p) * 5.03238)
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
  /* USER CODE BEGIN 2 */
  /*
  //enable the drivers
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET); // X-axis EN (PD11)
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);  // Y-axis EN (PE0)

  //Set direction
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);   // X-axis DIR (PD13)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);    // Y-axis DIR (PB0)

  //test PWM signal driving
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);


//	*/
//  Gantry_Home();
//  //mMove(0, percentDist(50));
//  HAL_Delay(1000);
////
////// // lineMove(percentDist(50), percentDist(50));
//  lineMove(percentDist(50), percentDist(50), 100);
//  HAL_Delay(500);
//  lineMove(percentDist(10), percentDist(10), 50);
////  HAL_Delay(500);
//  lineMove(percentDist(80), percentDist(10));
//  HAL_Delay(500);
//  lineMove(percentDist(10), percentDist(10));

//  procCSV();



  //mMove(1, percentDist(50));
//
//  setvbuf(stdout, NULL, _IONBF, 0);
//
  // Joystick code
  int CALIBRATE_TIME_DELAY = 2000; // Time for calibration
  // TODO: MEASURE CENTER
  printf("Keep the stick at neutral (center)\r\n");
  HAL_Delay(CALIBRATE_TIME_DELAY);
  printf("Measuring Center!\r\n");
  int centerX = getX(); // 2000
  int centerY = getY(); // 1900

  // TODO: MEASURE X_NEG
  printf("Move the stick to X_NEG (left)\r\n");
  HAL_Delay(CALIBRATE_TIME_DELAY);
  printf("Measuring X_NEG!\r\n");
  int leftX= getX();
  int leftY= getY();

  // TODO: MEASURE X_POS
  printf("Move the stick to X_POS (right)\r\n");
  HAL_Delay(CALIBRATE_TIME_DELAY);
  printf("Measuring X_POS!\r\n");
  int rightX= getX();
  int rightY= getY();

  // TODO: MEASURE Y_NEG
  printf("Move the stick to Y_NEG (down)\r\n");
  HAL_Delay(CALIBRATE_TIME_DELAY);
  printf("Measuring Y_NEG!\r\n");
  int downX= getX();
  int downY= getY();

  // TODO: MEASURE Y_POS
  printf("Move the stick to Y_POS (up)\r\n");
  HAL_Delay(CALIBRATE_TIME_DELAY);
  printf("Measuring Y_POS!\r\n");
  int upX= getX();
  int upY= getY();

  // TODO: RUN CALIBRATE FUNCTION TO GENERATE COEFFFS
  // Step 1: subtract center from each extreme point
  float relLeftX = (float)leftX - centerX;
  float relLeftY = (float)leftY - centerY;

  float relRightX = (float)rightX - centerX;
  float relRightY = (float)rightY - centerY;

  float relDownX = (float)downX - centerX;
  float relDownY = (float)downY - centerY;

  float relUpX = (float)upX - centerX;
  float relUpY = (float)upY - centerY;

  // Step 2: compute basis vectors
  // ex = (pR - pL)/2
  float eXX = (relRightX - relLeftX) * 0.5f;
  float eXY = (relRightY - relLeftY) * 0.5f;

  // ey = (pU - pD)/2
  float eYX = (relUpX - relDownX) * 0.5f;
  float eYY = (relUpY - relDownY) * 0.5f;

  // Step 3: build B = [ ex.x  ey.x
  //                     ex.y  ey.y ]
//  cal.B.m11 = cal.ex.x;
//  cal.B.m12 = cal.ey.x;
//  cal.B.m21 = cal.ex.y;
//  cal.B.m22 = cal.ey.y;

 // Step 4: invert B to get A
 float det = eXX * eYY - eYX * eXY;

 float a11 =  eYY / det;
 float a12 = -eYX / det;
 float a21 = -eXY / det;
 float a22 =  eXX / det;
//  // 0.00096237
//  // 0.00048359
//  // 0.00001819
//  // -0.00048837
//
////   // TODO: INSTANTIATE JOYCAL
//
  JoyCal joy = {
		  centerX, centerY,
      a11, a12,
      a21, a22
  };

//  JoyCal joy = {
//  		  2000, 1900,
//        0.00096237, 0.00048359,
//        0.00001819, -0.00048837
//  };

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  int X_RAW = getX();
	  int Y_RAW = getY();

	// printf("raw: %d,%d\r\n", ADC_VAL1, ADC_VAL2);

	  // Process ADC values
	  float X_POS = 0;
	  float Y_POS = 0;
	  float MAG = 0;
	  float ANGLE = 0;
	  // void joystick_correct(const JoyCal *cal, float x_raw, float y_raw,
  //      float *x_out, float *y_out,
  //      float *mag, float *angle)
	  joystick_correct(&joy, X_RAW, Y_RAW, &X_POS, &Y_POS, &MAG, &ANGLE);
	  printf("%d,%d\r\n", X_RAW, Y_RAW);
	  // printf("%f,%f\r\n", X_POS, Y_POS);

	  // TODO: this is the polar coordinates (hopefully the ai cooked)
	  // printf("mag=%f, angle=%f\r\n", MAG, ANGLE);

	  // TODO: figure out sampling rate
	  HAL_Delay(100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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
