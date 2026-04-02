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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

/* USER CODE BEGIN PFP */
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t data, int size);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE BEGIN 0 */

// Sends a command byte to the ST7796S

void LCD_ClearScreen() {
    // 1. Set column address (0 to 319)
    LCD_WriteCommand(0x2A);
    LCD_WriteData(0x00, 1); LCD_WriteData(0x00, 1); // start = 0
    LCD_WriteData(0x01, 1); LCD_WriteData(0x3F, 1); // end = 319

    // 2. Set row address (0 to 479)
    LCD_WriteCommand(0x2B);
    LCD_WriteData(0x00, 1); LCD_WriteData(0x00, 1); // start = 0
    LCD_WriteData(0x01, 1); LCD_WriteData(0xDF, 1); // end = 479

    // 3. Send Memory Write command
    LCD_WriteCommand(0x2C);

    // 4. Create a safe-sized buffer for exactly ONE row (320 pixels * 2 bytes = 640 bytes)
    uint8_t buf[640];
    for (int i = 0; i < 640; i++) {
        buf[i] = 0xFF; // 0xFFFF is White in RGB565
    }

    // 5. Prepare pins for raw data transmission
    HAL_GPIO_WritePin(DC_RS_GPIO_Port, DC_RS_Pin, GPIO_PIN_SET);     // DC HIGH for Data
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); // CS LOW to select screen

    // 6. Transmit the 640-byte row 480 times to cover the whole screen
    for(uint32_t row = 0; row < 480; row++) {
        HAL_SPI_Transmit(&hspi1, buf, 640, HAL_MAX_DELAY);
    }

    // 7. Deselect the screen when finished
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void LCD_WriteCommand(uint8_t cmd) {
    //set DC low bc command
    HAL_GPIO_WritePin(DC_RS_GPIO_Port, DC_RS_Pin, GPIO_PIN_RESET);
    //select lcd
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    //send 1 byte
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    //deselect
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

// Sends a data byte to the ST7796S
void LCD_WriteData(uint8_t data, int size) {
    // dc high for data
    HAL_GPIO_WritePin(DC_RS_GPIO_Port, DC_RS_Pin, GPIO_PIN_SET);
    // select display with CS
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &data, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void LCD_DrawDot(int x, int y) {
    // 1. Set column address (0 to 319)

	int tempx = (int)((x/100.0) * 320);
	int tempy = (int)((y/100.0) * 480);

	uint8_t lower_x_left = (tempx - 5) & 0xFF;
	uint8_t upper_x_left = (tempx - 5) >> 8;

	uint8_t lower_x_right = (tempx + 5) & 0xFF;
	uint8_t upper_x_right = (tempx + 5) >> 8;

	uint8_t lower_y_down = (tempy - 5) & 0xFF;
	uint8_t upper_y_down = (tempy - 5) >> 8;

	uint8_t lower_y_up = (tempy + 5) & 0xFF;
	uint8_t upper_y_up = (tempy + 5) >> 8;

    LCD_WriteCommand(0x2A);
    LCD_WriteData(upper_x_left, 1); LCD_WriteData(lower_x_left, 1); // start = x - 5
    LCD_WriteData(upper_x_right, 1); LCD_WriteData(lower_x_right, 1); // end = x + 5

    // 2. Set row address (0 to 479)
    LCD_WriteCommand(0x2B);
    LCD_WriteData(upper_y_down, 1); LCD_WriteData(lower_y_down, 1); // start = y - 5
    LCD_WriteData(upper_y_up, 1); LCD_WriteData(lower_y_up, 1); // end = y + 5

    // 3. Send Memory Write command
    LCD_WriteCommand(0x2C);

    // 4. Create a safe-sized buffer for exactly ONE row (320 pixels * 2 bytes = 640 bytes)
    uint8_t buf[242]; //
    for (int i = 0; i < 242; i++) {
        buf[i] = 0x00; // 0x00 is Black in RGB565
    }

    LCD_WriteData(&buf, 242);

}

/* USER CODE END 0 */

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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  //hardware reset
  HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(50);
  HAL_GPIO_WritePin(RESET_GPIO_Port, RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(120);

  LCD_WriteCommand(0x01); //software reset
  HAL_Delay(120);

  LCD_WriteCommand(0x11); //wake up screen (sleep out)
  HAL_Delay(120);

  LCD_WriteCommand(0x3A);//pixel format
  LCD_WriteData(0x55, 1); // 0x55 is 16 bit color

  LCD_WriteCommand(0x36); // Memory Data Access Control (Orientation)
  LCD_WriteData(0x48, 1); //default

  LCD_WriteCommand(0x29); // Display ON
  HAL_Delay(10);

  //SCREEN IS 320 by 480 PIXELS (resolution)

  //memory write command

  LCD_ClearScreen();

  HAL_Delay(100);

  LCD_DrawDot(50, 50);

//  for(uint32_t i = 0; i < (480*320); i++) { // can use new WriteData func, no need for for loop
//
//
//      LCD_WriteData(0xFF, 1); //upper byte
//      LCD_WriteData(0xFF, 1); //lower byte
//  }
//
//  //SET COLUMN ADDRESS (from 0 to 319)
//  LCD_WriteCommand(0x2A);
//  LCD_WriteData(0x00); LCD_WriteData(0x64); //start = 0
//  LCD_WriteData(0x00); LCD_WriteData(0x6D); //end = 319
//
//  //SET ROW ADDRESS (from 0 to 479)
//  LCD_WriteCommand(0x2B);
//  LCD_WriteData(0x00); LCD_WriteData(0x64); // start = 0
//  LCD_WriteData(0x00); LCD_WriteData(0x6D); // end = 479
//
//  //memory write command
//  LCD_WriteCommand(0x2C);
//
//  //send data for every pixel
//  for(uint32_t i = 0; i < (100); i++) {
//      LCD_WriteData(0x00); //upper byte
//      LCD_WriteData(0x00); //lower byte
//  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RESET_Pin|DC_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RESET_Pin DC_RS_Pin */
  GPIO_InitStruct.Pin = RESET_Pin|DC_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
