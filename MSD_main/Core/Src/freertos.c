/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "joystick.h"
#include "gmove.h"
#include <stdlib.h>
#include "path_data.h"
#include <stdio.h>
#include "usart.h"
#include "lcd_touch.h"
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
/* USER CODE BEGIN Variables */

osMessageQueueId_t MQH;
volatile uint32_t last_activity_time = 0;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for motorTask */
osThreadId_t motorTaskHandle;
const osThreadAttr_t motorTask_attributes = {
  .name = "motorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for joyTask */
osThreadId_t joyTaskHandle;
const osThreadAttr_t joyTask_attributes = {
  .name = "joyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for inputTask */
osThreadId_t inputTaskHandle;
const osThreadAttr_t inputTask_attributes = {
  .name = "inputTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for idleTask */
osThreadId_t idleTaskHandle;
const osThreadAttr_t idleTask_attributes = {
  .name = "idleTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartMotorTask(void *argument);
void StartJoyTask(void *argument);
void StartInputTask(void *argument);
void StartIdleTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

	const osMessageQueueAttr_t MQ_attr = {
			.name = "Motor_Queue"
	};

	MQH = osMessageQueueNew(16, sizeof(MotorCmd_t), &MQ_attr);

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of motorTask */
  motorTaskHandle = osThreadNew(StartMotorTask, NULL, &motorTask_attributes);

  /* creation of joyTask */
  joyTaskHandle = osThreadNew(StartJoyTask, NULL, &joyTask_attributes);

  /* creation of inputTask */
  inputTaskHandle = osThreadNew(StartInputTask, NULL, &inputTask_attributes);

  /* creation of idleTask */
  idleTaskHandle = osThreadNew(StartIdleTask, NULL, &idleTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
* @brief Function implementing the motorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void *argument)
{
  /* USER CODE BEGIN StartMotorTask */
	MotorCmd_t cmd;
	  for(;;) {
	    if (osMessageQueueGet(MQH, &cmd, NULL, osWaitForever) == osOK) {
//	      switch(cmd.type) {
//	        case CMD_HOME:
//	          Gantry_Home();
//	          break;
//	        case CMD_JOYSTICK:
//	          joyMove(cmd.x, cmd.y);
//	          break;
//	        case CMD_LINE:
//	          lineMove(cmd.x, cmd.y, cmd.speed);
//	          break;
//	      }
	    	printf("Test\n");
	    	printf("Type: %d | X: %.2f | Y: %.2f\r\n", cmd.type, cmd.x, cmd.y);
	    	osDelay(500);

	    	//FIXXXXXXX
	    }
	  }
  /* USER CODE END StartMotorTask */
}

/* USER CODE BEGIN Header_StartJoyTask */
/**
* @brief Function implementing the joyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartJoyTask */
void StartJoyTask(void *argument)
{
  /* USER CODE BEGIN StartJoyTask */
	MotorCmd_t cmd;
	cmd.type = CMD_JOYSTICK;

	// Define your calibration struct here if needed
	JoyCal joy = {2000, 1900, 0.00096237, 0.00048359, 0.00001819, -0.00048837};

	for(;;) {
		int rawX = getX();
		int rawY = getY();

		// Deadzone check (adjust 2000/1900 to your center values)
		if (abs(rawX - 2000) > 150 || abs(rawY - 1900) > 150) {
			last_activity_time = osKernelGetTickCount(); // Reset idle timer

			float x_out, y_out, mag, angle;
			joystick_correct(&joy, rawX, rawY, &x_out, &y_out, &mag, &angle);

			cmd.x = x_out;
			cmd.y = y_out;
			osMessageQueuePut(MQH, &cmd, 0, 0);
		}

		osDelay(20); // Poll at 50Hz

	}
  /* USER CODE END StartJoyTask */
}

/* USER CODE BEGIN Header_StartInputTask */
/**
* @brief Function implementing the inputTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartInputTask */
void StartInputTask(void *argument)
{
  /* USER CODE BEGIN StartInputTask */
	MotorCmd_t cmd;
	  cmd.type = CMD_LINE;
	  cmd.speed = 100;

	  for(;;)
	  {
	      // 1. Check IR Camera (10ms timeout prevents blocking)
	      uint8_t start = 0;
	      if (HAL_UART_Receive(&huart1, &start, 1, 10) == HAL_OK)
	      {
	          if (start == 0xAA || start == 0xAB)
	          {
	              uint8_t raw[4];
	              if (HAL_UART_Receive(&huart1, raw, 4, 10) == HAL_OK)
	              {
	                  last_activity_time = osKernelGetTickCount();
	                  uint16_t x = ((uint16_t)raw[0] << 8) | raw[1];
	                  uint16_t y = ((uint16_t)raw[2] << 8) | raw[3];

	                  cmd.x = percentDist(percentIR(x));
	                  cmd.y = percentDist(percentIR(y));
	                  osMessageQueuePut(MQH, &cmd, 0, 0);

	                  if (start == 0xAA) LCD_IRPointerCircle(x, y, 3);
	                  else LCD_DrawingPointerCircle(x, y, 3);
	              }
	          }
	      }

	      // 2. Check Touch Screen (Polling is RTOS-safe)
	      if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_RESET)
	      {
	          uint16_t z1 = Touch_Read(0xB0);
	          if (z1 >= 50 && z1 <= 4000)
	          {
	              uint32_t sum_x = 0, sum_y = 0;
	              for (int i = 0; i < 5; i++) { sum_x += Touch_Read(0xD0); sum_y += Touch_Read(0x90); }
	              uint16_t raw_x = sum_x / 5; uint16_t raw_y = sum_y / 5;

	              if (raw_x >= 200 && raw_x <= 1811 && raw_y >= 200 && raw_y <= 1920)
	              {
	                  last_activity_time = osKernelGetTickCount();

	                  int pixel_x = ((int)raw_x - 200) * 320 / (1811 - 200);
	                  int pixel_y = 479 - (((int)raw_y - 200) * 480 / (1920 - 200));

	                  // Convert pixels directly to motor coordinates
	                  cmd.x = percentDist((pixel_x * 100) / 320);
	                  cmd.y = percentDist((pixel_y * 100) / 480);
	                  osMessageQueuePut(MQH, &cmd, 0, 0);

	                  LCD_DrawingPointerCircle(pixel_x, pixel_y, 3);
//	                  printf("%d,%d\n\r", pixel_x, pixel_y); // Send to Python script
	              }
	          }
	      }
	      osDelay(10); // Yield to motor/joystick tasks
	  }
  /* USER CODE END StartInputTask */
}

/* USER CODE BEGIN Header_StartIdleTask */
/**
* @brief Function implementing the idleTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIdleTask */
void StartIdleTask(void *argument)
{
  /* USER CODE BEGIN StartIdleTask */
	  const uint32_t IDLE_TIMEOUT = 10000; // 10 seconds (10,000 ticks)
	  int point_idx = 0;

	  MotorCmd_t cmd;
	  cmd.type = CMD_LINE;
	  cmd.speed = 100;

	  for(;;)
	  {
	    if ((osKernelGetTickCount() - last_activity_time) > IDLE_TIMEOUT) {
	        // System is idle! Stream pattern points.

	        cmd.x = path_data[point_idx][0];
	        cmd.y = path_data[point_idx][1];

	        // osWaitForever acts as our flow control.
	        // It waits until the motor task finishes a move and pulls a command out.
	        osMessageQueuePut(MQH, &cmd, 0, osWaitForever);

	        point_idx++;
	        // Replace PATH_LENGTH with whatever macro defines your array size in path_data.h
	        if (point_idx >= NUM_PATH_POINTS) {
	            point_idx = 0;
	        }
	    } else {
	        // System is active. Reset the pattern and sleep.
	        point_idx = 0;
	        osDelay(500);
	    }
	  }
  /* USER CODE END StartIdleTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    // Place a breakpoint here to catch the task that overflowed
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/* USER CODE END Application */

