/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "path_data.h"
#include<stdlib.h>
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


volatile uint8_t trigger_lcd_touch = 0;
volatile uint8_t trigger_ir_active = 0;
volatile uint8_t trigger_joystick = 0;
volatile uint8_t trigger_clear_btn = 0;

extern const int path_lengths[];
extern const double paths[][MAX_POINTS][COORDS];

// --- LCD Touch Tracking Variables ---

  uint8_t play_user_path = 0; // 0: Play default designs | 1: Play user design
  extern int prev_x;
  extern int prev_y;

  extern volatile int lpx;
  extern volatile int lpy;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint8_t manual_mode = 0;
volatile uint8_t ir_mode = 1;
volatile SystemState current_state = STATE_AUTO_DRAW;

 volatile int user_lcd_path[MAX_LCD_POINTS][2];
 volatile int user_path_length = 0;

volatile uint8_t touch_flag = 0;
volatile uint8_t uart_flag = 0;
uint8_t rx_byte;
uint8_t rx_buffer[5];
uint8_t rx_index = 0;
uint32_t last_activity = 0;

volatile int sendingflag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        if (rx_index == 0 && (rx_byte == 0xAA || rx_byte == 0xAB)) {
            rx_buffer[0] = rx_byte;
            rx_index++;
        } else if (rx_index > 0) {
            rx_buffer[rx_index++] = rx_byte;
            if (rx_index == 5) {
                uart_flag = 1;
                rx_index = 0;
            }
        }
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}
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
    LCD_Init();
    LCD_ClearScreen();
    LCD_DrawingInit();

    JoyCal joy = {2063.00, 2080.00, 0.00054599, -0.00006252, 0.00001426, -0.00100615};

    float x_out, y_out, mag, angle;

    printf("Homing Gantry...\r\n");
    Gantry_Home();
    printf("Homed!\r\n");

    uint32_t last_print = 0;
    int point_index = 0;

    uint32_t last_joy = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


    // FSM Tracking Variables
      uint8_t auto_draw_step = 0; // 0: Draw, 1: Wait, 2: Move to start, 3: Clear
      uint32_t wait_start_time = 0;
      uint8_t current_design_index = 0; // Tracks which of the 5 designs we are drawing

      setvbuf(stdout, NULL, _IONBF, 0);


      //lcd variables



    while (1)
      {
//          if (touch_flag) {
//              touch_flag = 0;
//              last_activity = HAL_GetTick();
//          }
//
//          if (uart_flag) {
//              uart_flag = 0;
//              last_activity = HAL_GetTick();
//
//              uint16_t x = ((uint16_t)rx_buffer[1] << 8) | rx_buffer[2];
//              uint16_t y = ((uint16_t)rx_buffer[3] << 8) | rx_buffer[4];
//
//              if (rx_buffer[0] == 0xAA) LCD_IRPointerCircle(x, y, 3);
//              else LCD_DrawingPointerCircle(x, y, 3);
//
//              uint16_t irx = percentIR(x);
//              uint16_t iry = percentIR(y);
//              if (!(irx == 100 && iry == 100)) {
//                  lineMove(percentDist(irx), percentDist(iry), 100);
//              }
//          }
//
//          if (manual_mode && (HAL_GetTick() - last_joy > 20)) {
//              last_joy = HAL_GetTick();
//
//              int rawX = getX();
//              int rawY = getY();
//
//              // Deadzone based on your new center values
//              if (abs(rawX - 2063) > 150 || abs(rawY - 2080) > 150) {
//                  last_activity = HAL_GetTick();
//                  joystick_correct(&joy, rawX, rawY, &x_out, &y_out, &mag, &angle);
//                  joyMove(x_out, y_out);
//              } else {
//                  joyMove(0, 0);
//              }
//          }
//
//          if (!manual_mode && (HAL_GetTick() - last_activity > 10000)) {
//              procCSV();
//              last_activity = HAL_GetTick();
//          }
    	//start new code

    	switch (current_state) {

    	          // ==========================================
    	          case STATE_BOOT_HOMING:
    	              // Homing was already done in setup, but we leave this here
    	              // in case you ever want to call a re-home during runtime.
    	              current_state = STATE_AUTO_DRAW;
    	              break;

    	          // ==========================================
    	              // ==========================================
    	                        case STATE_AUTO_DRAW:
    	                            switch(auto_draw_step) {

    	                                // STEP 0: Draw the design
    	                                case 0:
    	                                    // 1. DETERMINE WHICH ARRAY TO USE
    	                                    int current_length = (play_user_path) ? user_path_length : path_lengths[current_design_index];

    	                                    if (play_user_path) {
    	                                        printf("Drawing User LCD Path...\r\n");
    	                                    } else {
    	                                        printf("Drawing design %d...\r\n", current_design_index);
    	                                    }

    	                                    // Loop through the specific number of points for THIS design
    	                                    for (int i = 0; i < current_length; i++) {

    	                                        // 2. FETCH FROM THE CORRECT ARRAY
    	                                        double target_x = (play_user_path) ? user_lcd_path[i][0] : paths[current_design_index][i][0];
    	                                        double target_y = (play_user_path) ? user_lcd_path[i][1] : paths[current_design_index][i][1];

    	                                        // Use your existing lineMove function and percent macros
    	                                        lineMove(percentDist(target_x), percentDist(target_y), 100);

    	                                        // Quick safety break if user pressed the manual mode button OR touched the screen mid-draw
    	                                        if (manual_mode || current_state == STATE_LCD_MODE || current_state == STATE_LCD_INTER) break;
    	                                    }

    	                                    // Transition to Wait Step
    	                                    wait_start_time = HAL_GetTick();
    	                                    auto_draw_step = 1;
    	                                    break;

    	                                // STEP 1: Wait 10 seconds
    	                                case 1:
    	                                    // Only advance once 10,000 milliseconds have passed
    	                                    if ((HAL_GetTick() - wait_start_time) >= 10000) {
    	                                        // Advance to the next design index, looping back to 0 if we hit NUM_DESIGNS
    	                                        // ONLY do this if we are playing default paths!
    	                                        if (!play_user_path) {
    	                                            current_design_index = (current_design_index + 1) % NUM_DESIGNS;
    	                                        }
    	                                        auto_draw_step = 2;
    	                                    }
    	                                    break;

    	                                // STEP 2: Move to the first point of the NEXT design
    	                                case 2:
    	                                    printf("Moving to start of next design...\r\n");

    	                                    // 3. FETCH THE STARTING POINT FROM THE CORRECT ARRAY
    	                                    double start_x = (play_user_path) ? user_lcd_path[0][0] : paths[current_design_index][0][0];
    	                                    double start_y = (play_user_path) ? user_lcd_path[0][1] : paths[current_design_index][0][1];

    	                                    lineMove(percentDist(start_x), percentDist(start_y), 100);

    	                                    auto_draw_step = 3;
    	                                    break;

    	                                // STEP 3: Perform the mechanical clear
    	                                case 3:
    	                                    printf("Mechanical Clear sequence triggered...\r\n");

    	                                    // CALL EXTERNAL CLEARING FUNCTION HERE LATER
    	                                    // Mechanical_Clear_Sequence();

    	                                    // Loop back to Step 0 to draw the new design
    	                                    auto_draw_step = 0;
    	                                    break;
    	                            }

    	                            // --- Interrupt Checks for Auto Draw ---
    	                            // If the physical button changed manual_mode to 1, break out!
    	                            if (manual_mode) {
    	                                printf("Switching to Joystick Mode!\r\n");
    	                                current_state = STATE_JOYSTICK_MODE;
    	                            }
    	                            // The LCD transition is already handled safely by the Touch_EXTI_Callback!

    	                            break; // End of STATE_AUTO_DRAW

    	          // ==========================================
    	          case STATE_JOYSTICK_MODE:

    	              // Run joystick if we are past the 20ms debounce/refresh rate
    	              if (HAL_GetTick() - last_joy > 20) {
    	                  last_joy = HAL_GetTick();

    	                  int rawX = getX();
    	                  int rawY = getY();

    	                  // Deadzone check
    	                  if (abs(rawX - 2063) > 150 || abs(rawY - 2080) > 150) {
    	                      joystick_correct(&joy, rawX, rawY, &x_out, &y_out, &mag, &angle);
    	                      joyMove(x_out, y_out);
    	                  } else {
    	                      joyMove(0, 0); // Stop motors if stick is centered
    	                  }
    	              }

    	              // Exit condition: if the button is pressed again, return to Auto Draw
    	              if (!manual_mode) {
    	                  printf("Switching back to Auto Draw!\r\n");
    	                  joyMove(0, 0); // Ensure motors stop completely

						printf("Rehoming gantry to reset open-loop coordinates...\r\n");
						Gantry_Home();
						printf("Homed!\r\n");

						// Reset the Auto Draw state machine so it clears the board and starts fresh
						auto_draw_step = 3;
						current_state = STATE_AUTO_DRAW;
    	              }
    	              break;


    	              // ==========================================
    	          case STATE_LCD_INTER:
    	        	  break;
    	                        case STATE_LCD_MODE:
    	                        	//printf("stable LCD MODE REACHED ONCE SAVED");
    	                        	//HAL_Delay(1000);
    	                        	Gantry_Home();
    	                        	sendingflag = 0;
    	                        	procUserDrawing();
    	                        	HAL_Delay(5000);
    	                            break;

    	          //==========================================
    	          case STATE_IR_MODE:
    	        	  if (uart_flag) {
    	        	          uart_flag = 0;

    	        	          uint16_t raw_x = ((uint16_t)rx_buffer[1] << 8) | rx_buffer[2];
    	        	          uint16_t raw_y = ((uint16_t)rx_buffer[3] << 8) | rx_buffer[4];

    	        	          if (raw_x <= 1023 && raw_y <= 1023) {
    	        	              int gantryx = percentIR(raw_x);
    	        	              int gantryy = percentIR(raw_y);

    	        	              // Convert to LCD space
    	        	              int pixel_x = 319 - (gantryx * 320 / 100);
    	        	              int pixel_y = (gantryy * 320 / 100) + 120;

    	        	              // Clamp bounds
    	        	              if (pixel_x < 0) pixel_x = 0; if (pixel_x > 319) pixel_x = 319;
    	        	              if (pixel_y < 120) pixel_y = 120; if (pixel_y > 479) pixel_y = 479;

    	        	              // --- 1. Clear Button ---
    	        	              if (pixel_x >= 160 && pixel_y >= 40 && pixel_y < 100) {
    	        	                  LCD_ClearScreen();
    	        	                  LCD_DrawingInit();
    	        	                  user_path_length = 0;
    	        	                  printf("CLEAR_LOG\n\r");
    	        	                  lpx = -1; lpy = -1;
    	        	              }
    	        	              // --- 2. Save Button ---
    	        	              else if (pixel_x < 160 && pixel_y >= 40 && pixel_y < 100) {
    	        	                  sendingflag = 1;
    	        	                  play_user_path = 1; // Mark user path active
    	        	                  printf("SAVE_FILE\n\r");
    	        	                  for (int i = 0; i < user_path_length; i++) {
    	        	                      printf("x: %d, y: %d\n\r", 100 - user_lcd_path[i][0], user_lcd_path[i][1]);
    	        	                  }
    	        	                  current_state = STATE_LCD_MODE; // Transition to play
    	        	              }
    	        	              // --- 3. Draw & Record Point ---
    	        	              else {
    	        	                  if (rx_buffer[0] == 0xAA) LCD_IRPointerCircle(pixel_x, pixel_y, 3);
    	        	                  else LCD_DrawingPointerCircle(pixel_x, pixel_y, 3);

    	        	                  // Filter noise before saving
    	        	                  static int last_gx = -1, last_gy = -1;
    	        	                  if (abs(gantryx - last_gx) > 2 || abs(gantryy - last_gy) > 2) {
    	        	                      if (gantryx >= 0 && gantryx <= 100 && gantryy >= 0 && gantryy <= 100 && user_path_length < MAX_LCD_POINTS) {
    	        	                          user_lcd_path[user_path_length][0] = gantryx;
    	        	                          user_lcd_path[user_path_length][1] = gantryy;
    	        	                          user_path_length++;
    	        	                          last_gx = gantryx;
    	        	                          last_gy = gantryy;
    	        	                          printf("%d,%d\n\r", gantryx, gantryy);
    	        	                      }
    	        	                  }
    	        	              }
    	        	          }
    	        	      }
    	        	      break;

    	          // ==========================================
    	          case STATE_CLEARING:
    	              // Dedicated clearing state for manual triggers
    	              break;
          /* USER CODE END WHILE */
    	}
      }
    /* USER CODE BEGIN 3 */


    }
  /* USER CODE END 3 */


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
    if (GPIO_Pin == GPIO_PIN_1) {
        static uint32_t last_press = 0;
        // Ignore any presses within 200ms of the last one
        if (HAL_GetTick() - last_press > 200) {
            manual_mode = !manual_mode;
            last_press = HAL_GetTick();
        }
    }

    if (GPIO_Pin == GPIO_PIN_6) {
    	//current_state = STATE_LCD_INTER;
        if (current_state != STATE_LCD_MODE) {
            printf("Screen touched! Switching to LCD Mode...\r\n");
            joyMove(0, 0); // Hard stop the motors

            // printf("IRQ Fired!\r\n");

                    if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_SET) return;

                    uint16_t z1 = Touch_Read(0xB0);

                    if (z1 < 50 || z1 > 4000) return;

                    Touch_Read(0xD0);
                    Touch_Read(0x90);

                    uint32_t sum_x = 0;
                    uint32_t sum_y = 0;

                    for (int i = 0; i < 10; i++) {
                        sum_x += Touch_Read(0xD0);
                        sum_y += Touch_Read(0x90);
                    }

                    uint16_t raw_x = sum_x / 10;
                    uint16_t raw_y = sum_y / 10;

                    //printf("Raw: %d, %d\r\n", raw_x, raw_y);

                    if (raw_x < 50 || raw_x > 4000 || raw_y < 50 || raw_y > 4000) return;

                    if (raw_x < 200) raw_x = 200;
                    if (raw_x > 1811) raw_x = 1811;
                    if (raw_y < 200) raw_y = 200;
                    if (raw_y > 1920) raw_y = 1920;

                    int pixel_x = ((int)raw_x - 200) * 320 / (1811 - 200);
                    int pixel_y = 479 - (((int)raw_y - 200) * 480 / (1920 - 200));

                    if (pixel_y < 40) {

                    	 if (current_state == STATE_IR_MODE) {
                            printf("Switching back to Touch mode!\n\r");
                            current_state = STATE_LCD_MODE; // Or whatever your idle state is
                        } else {
                            printf("Switching to IR mode!\n\r");
                            current_state = STATE_IR_MODE;
                        }
//                        } else {
//                        	current_state = STATE_IR_MODE;
//                        }
//                        return;
                    }

                    if (current_state == STATE_IR_MODE) return;

                    if (pixel_x < 0) pixel_x = 0;
                    if (pixel_x > 319) pixel_x = 319;
                    if (pixel_y < 0) pixel_y = 0;
                    if (pixel_y > 479) pixel_y = 479;

                    if (pixel_x >= 160 && pixel_y >= 40 && pixel_y < 100) {
                        LCD_ClearScreen();
                        LCD_DrawingInit();
                        printf("CLEAR_LOG\n\r");
                        lpx = -1;
                        lpy = -1;
                        return;
                    }

                    if (pixel_x < 160 && pixel_y >= 40 && pixel_y < 100) {
                    	sendingflag = 1;
                        printf("SAVE_FILE\n\r");
                        for (int i = 0; i < user_path_length; i++) {
                        	printf("x: %d, y: %d\n\r", 100 - user_lcd_path[i][0], user_lcd_path[i][1]);
                        }
                    	current_state = STATE_LCD_MODE;
                        return;
                    }

                    if (lpx == -1) {
                        lpx = pixel_x;
                        lpy = pixel_y;
                    } else {
                        if (abs(pixel_x - lpx) > 3000 || abs(pixel_y - lpy) > 3000) return;
                        lpx = pixel_x;
                        lpy = pixel_y;
                    }

                    LCD_DrawingPointerCircle(pixel_x, pixel_y, 3);
                    //gantry conversions
                    int adjx = 319 - pixel_x;
                    int adjy = pixel_y;

                    int gantryx = (adjx/320.0)*100;
                    int gantryy = ((adjy-120.0)/320.0)*100;

                    if (gantryx > 100) return;
                    if (gantryy > 100) return;
                    if (gantryx < 0) return;
                    if (gantryy < 0) return;
                    user_lcd_path[user_path_length][0] = gantryx;
                    user_lcd_path[user_path_length][1] = gantryy;


                    user_path_length++;

                    printf("%d,%d\n\r", gantryx, gantryy);
        }
    }
    //x axis limit switch hit
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

    if (GPIO_Pin == GPIO_PIN_2) {
    	static uint32_t lpt = 0;
    	uint32_t tcurr = HAL_GetTick();

    	if(tcurr - lpt > 200) {
    		manual_mode = !manual_mode;

			HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
			HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

    		lpt = tcurr;
    	}
    }
}

//void Touch_EXTI_Callback(uint16_t GPIO_Pin) {

//}
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
