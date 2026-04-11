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
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
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
UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN PV */
int prev_x, prev_y = -1; // no prev location on start up for IR light
volatile uint8_t touch_flag = 0; // NEW: Tells main loop a touch happened
int drawmode = 1;
int irmode = 0;

volatile int last_pixel_x = -1;
volatile int last_pixel_y = -1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t *data, int size);
void LCD_WriteByte(uint8_t data);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Sends a command byte to the ST7796S

void LCD_ClearScreen() {
    // 1. Set column address (0 to 319)
    LCD_WriteCommand(0x2A);
    LCD_WriteByte(0x00); LCD_WriteByte(0x00); // start = 0
    LCD_WriteByte(0x01); LCD_WriteByte(0x3F); // end = 319

    // 2. Set row address (0 to 479)
    LCD_WriteCommand(0x2B);
    LCD_WriteByte(0x00); LCD_WriteByte(0x00); // start = 0
    LCD_WriteByte(0x01); LCD_WriteByte(0xDF); // end = 479

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

// for debugging
void LCD_BlackScreen() {
    // 1. Set column address (0 to 319)
    LCD_WriteCommand(0x2A);
    LCD_WriteByte(0x00); LCD_WriteByte(0x00); // start = 0
    LCD_WriteByte(0x01); LCD_WriteByte(0x3F); // end = 319

    // 2. Set row address (0 to 479)
    LCD_WriteCommand(0x2B);
    LCD_WriteByte(0x00); LCD_WriteByte(0x00); // start = 0
    LCD_WriteByte(0x01); LCD_WriteByte(0xDF); // end = 479

    // 3. Send Memory Write command
    LCD_WriteCommand(0x2C);

    // 4. Create a safe-sized buffer for exactly ONE row (320 pixels * 2 bytes = 640 bytes)
    uint8_t buf[640];
    for (int i = 0; i < 640; i++) {
        buf[i] = 0x00; // 0x000 is Black in RGB565
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
void LCD_WriteData(uint8_t *data, int size) {
	// dc high for data
    HAL_GPIO_WritePin(DC_RS_GPIO_Port, DC_RS_Pin, GPIO_PIN_SET);
    // select display with CS
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

void LCD_WriteByte(uint8_t data) {
    LCD_WriteData(&data, 1);
}

void LCD_DrawDot(int x, int y) {
    // 1. Set column address (0 to 319)

	int tempx = (int)((x/100.0) * 320);
	int tempy = (int)((y/100.0) * 480);

	int r = 5;

	if (tempx < r) tempx = r;
	if (tempx > (319 - r)) tempx = 319 - r;

	if (tempy < r) tempy = r;
	if (tempy > (479 - r)) tempy = 479 - r;

	uint8_t lower_x_left = (tempx - r) & 0xFF;
	uint8_t upper_x_left = (tempx - r) >> 8;

	uint8_t lower_x_right = (tempx + r) & 0xFF;
	uint8_t upper_x_right = (tempx + r) >> 8;

	uint8_t lower_y_down = (tempy - r) & 0xFF;
	uint8_t upper_y_down = (tempy - r) >> 8;

	uint8_t lower_y_up = (tempy + r) & 0xFF;
	uint8_t upper_y_up = (tempy + r) >> 8;

    LCD_WriteCommand(0x2A);
    LCD_WriteByte(upper_x_left); LCD_WriteByte(lower_x_left); // start = x - r
    LCD_WriteByte(upper_x_right); LCD_WriteByte(lower_x_right); // end = x + r

    // 2. Set row address (0 to 479)
    LCD_WriteCommand(0x2B);
    LCD_WriteByte(upper_y_down); LCD_WriteByte(lower_y_down); // start = y - r
    LCD_WriteByte(upper_y_up); LCD_WriteByte(lower_y_up); // end = y + r

    // 3. Send Memory Write command
    LCD_WriteCommand(0x2C);

    // 4. Create a safe-sized buffer
    int size = (2 * r + 1) * (2 * r + 1) * 2;
    uint8_t buf[size]; //
    for (int i = 0; i < size; i++) {
        buf[i] = 0x00; // 0x00 is Black in RGB565
    }

    LCD_WriteData(buf, size);

}

void LCD_DrawingPointer(int x, int y) {
    int r = 3; // radius of pointer square
    int size = (2*r + 1); // width/height of square
    uint8_t buf[(2*r+1)*(2*r+1)*2]; // RGB565 buffer

    // Erase previous pointer (we leave the content underneath intact, so skip if first)
    if (prev_x >= 0 && prev_y >= 0) {
        // In this case, we just "redraw" the previous content as black
        for (int i = 0; i < sizeof(buf); i++) buf[i] = 0x00; // black

        int x_start = prev_x - r;
        int y_start = prev_y - r;
        if (x_start < 0) x_start = 0;
        if (y_start < 0) y_start = 0;
        if (x_start + size > 319) x_start = 319 - size;
        if (y_start + size > 479) y_start = 479 - size;

        LCD_WriteCommand(0x2A);
        LCD_WriteByte(x_start >> 8); LCD_WriteByte(x_start & 0xFF);
        LCD_WriteByte((x_start + size - 1) >> 8); LCD_WriteByte((x_start + size - 1) & 0xFF);

        LCD_WriteCommand(0x2B);
        LCD_WriteByte(y_start >> 8); LCD_WriteByte(y_start & 0xFF);
        LCD_WriteByte((y_start + size - 1) >> 8); LCD_WriteByte((y_start + size - 1) & 0xFF);

        LCD_WriteCommand(0x2C);
        LCD_WriteData(buf, sizeof(buf));
    }

    // Draw new pointer as black
    for (int i = 0; i < sizeof(buf); i++) buf[i] = 0x00; // black
    int x_start = x - r;
    int y_start = y - r;
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_start + size > 319) x_start = 319 - size;
    if (y_start + size > 479) y_start = 479 - size;

    LCD_WriteCommand(0x2A);
    LCD_WriteByte(x_start >> 8); LCD_WriteByte(x_start & 0xFF);
    LCD_WriteByte((x_start + size - 1) >> 8); LCD_WriteByte((x_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2B);
    LCD_WriteByte(y_start >> 8); LCD_WriteByte(y_start & 0xFF);
    LCD_WriteByte((y_start + size - 1) >> 8); LCD_WriteByte((y_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2C);
    LCD_WriteData(buf, sizeof(buf));

    // Save position
    prev_x = x;
    prev_y = y;
}



// Draw a circular cursor at (x, y) with radius r
void LCD_DrawingPointerCircle(int x, int y, int r) {
    static int prev_x = -1, prev_y = -1;
    static int prev_r = 3;

    int size = 2 * r + 1;
    uint8_t buf[size * size * 2]; // RGB565 buffer

    // Fill buffer with black pixels (0x0000)
    for (int i = 0; i < size * size; i++) {
        buf[2*i] = 0x00;      // upper byte
        buf[2*i+1] = 0x00;    // lower byte
    }

    // Erase previous pointer by drawing black over it
    if (prev_x >= 0 && prev_y >= 0) {
        int x_start = prev_x - prev_r;
        int y_start = prev_y - prev_r;
        if (x_start < 0) x_start = 0;
        if (y_start < 0) y_start = 0;
        if (x_start + size > 319) x_start = 319 - size;
        if (y_start + size > 479) y_start = 479 - size;

        LCD_WriteCommand(0x2A);
        LCD_WriteByte(x_start >> 8); LCD_WriteByte(x_start & 0xFF);
        LCD_WriteByte((x_start + size - 1) >> 8); LCD_WriteByte((x_start + size - 1) & 0xFF);

        LCD_WriteCommand(0x2B);
        LCD_WriteByte(y_start >> 8); LCD_WriteByte(y_start & 0xFF);
        LCD_WriteByte((y_start + size - 1) >> 8); LCD_WriteByte((y_start + size - 1) & 0xFF);

        LCD_WriteCommand(0x2C);
        LCD_WriteData(buf, sizeof(buf));
    }

    // Draw new pointer (black circle) in the buffer
    // In this case, the buffer is already all black, so no need to fill again

    int x_start = x - r;
    int y_start = y - r;
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_start + size > 319) x_start = 319 - size;
    if (y_start + size > 479) y_start = 479 - size;

    LCD_WriteCommand(0x2A);
    LCD_WriteByte(x_start >> 8); LCD_WriteByte(x_start & 0xFF);
    LCD_WriteByte((x_start + size - 1) >> 8); LCD_WriteByte((x_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2B);
    LCD_WriteByte(y_start >> 8); LCD_WriteByte(y_start & 0xFF);
    LCD_WriteByte((y_start + size - 1) >> 8); LCD_WriteByte((y_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2C);
    LCD_WriteData(buf, sizeof(buf));

    // Save previous position
    prev_x = x;
    prev_y = y;
    prev_r = r;
}

// Draw a circular pointer like a laser pointer
void LCD_IRPointerCircle(int x, int y, int r) {
    //static int prev_x = -1, prev_y = -1;
    static int prev_r = 3;

    int size = 2 * r + 1;
    uint8_t buf[size * size * 2]; // RGB565 buffer

    // Fill buffer with white pixels (0xFFFF) to erase previous pointer
    for (int i = 0; i < size * size; i++) {
        buf[2*i] = 0xFF;      // upper byte
        buf[2*i+1] = 0xFF;    // lower byte
    }

    // Erase previous pointer by drawing white over it
    if ((prev_x >= 3) && (prev_x < 316) && (prev_y >= 123) && (prev_y < 437)) {
        int x_start = prev_x - prev_r;
        int y_start = prev_y - prev_r;
        if (x_start < 0) x_start = 0;
        if (y_start < 0) y_start = 0;
        if (x_start + size > 319) x_start = 319 - size;
        if (y_start + size > 479) y_start = 479 - size;

        LCD_WriteCommand(0x2A);
        LCD_WriteByte(x_start >> 8); LCD_WriteByte(x_start & 0xFF);
        LCD_WriteByte((x_start + size - 1) >> 8); LCD_WriteByte((x_start + size - 1) & 0xFF);

        LCD_WriteCommand(0x2B);
        LCD_WriteByte(y_start >> 8); LCD_WriteByte(y_start & 0xFF);
        LCD_WriteByte((y_start + size - 1) >> 8); LCD_WriteByte((y_start + size - 1) & 0xFF);

        LCD_WriteCommand(0x2C);
        LCD_WriteData(buf, sizeof(buf));
    }

    // Draw new pointer as black circle
    for (int i = 0; i < size * size; i++) {
        buf[2*i] = 0x00;      // upper byte
        buf[2*i+1] = 0x00;    // lower byte
    }

    int x_start = x - r;
    int y_start = y - r;
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_start + size > 319) x_start = 319 - size;
    if (y_start + size > 479) y_start = 479 - size;

    LCD_WriteCommand(0x2A);
    LCD_WriteByte(x_start >> 8); LCD_WriteByte(x_start & 0xFF);
    LCD_WriteByte((x_start + size - 1) >> 8); LCD_WriteByte((x_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2B);
    LCD_WriteByte(y_start >> 8); LCD_WriteByte(y_start & 0xFF);
    LCD_WriteByte((y_start + size - 1) >> 8); LCD_WriteByte((y_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2C);
    LCD_WriteData(buf, sizeof(buf));

    // Save current pointer position
    prev_x = x;
    prev_y = y;
    prev_r = r;
}

// Draw a single pixel (RGB565) on the LCD
void LCD_DrawPixel(int x, int y, uint16_t color) {
    if (x < 0 || x >= 320 || y < 0 || y >= 480) return; // bounds check
    uint8_t data[2] = { color >> 8, color & 0xFF };
    LCD_WriteCommand(0x2A);
    LCD_WriteByte(x >> 8); LCD_WriteByte(x & 0xFF); // col start
    LCD_WriteByte(x >> 8); LCD_WriteByte(x & 0xFF); // col end
    LCD_WriteCommand(0x2B);
    LCD_WriteByte(y >> 8); LCD_WriteByte(y & 0xFF); // row start
    LCD_WriteByte(y >> 8); LCD_WriteByte(y & 0xFF); // row end
    LCD_WriteCommand(0x2C);
    LCD_WriteData(data, 2);
}


void LCD_DrawCircle(int cx, int cy, int r, uint16_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                LCD_DrawPixel(cx + x, cy + y, color);
            }
        }
    }
}


void LCD_DrawPen(int x, int y, uint16_t color) {
    int r = 3; // Brush thickness
    int size = (2 * r + 1);
    uint8_t buf[size * size * 2];

    // Fill buffer with color
    for (int i = 0; i < size * size; i++) {
        buf[2 * i] = color >> 8;
        buf[2 * i + 1] = color & 0xFF;
    }

    int x_start = x - r;
    int y_start = y - r;
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_start + size > 319) x_start = 319 - size;
    if (y_start + size > 479) y_start = 479 - size;

    LCD_WriteCommand(0x2A);
    LCD_WriteByte(x_start >> 8); LCD_WriteByte(x_start & 0xFF);
    LCD_WriteByte((x_start + size - 1) >> 8); LCD_WriteByte((x_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2B);
    LCD_WriteByte(y_start >> 8); LCD_WriteByte(y_start & 0xFF);
    LCD_WriteByte((y_start + size - 1) >> 8); LCD_WriteByte((y_start + size - 1) & 0xFF);

    LCD_WriteCommand(0x2C);
    LCD_WriteData(buf, sizeof(buf));
}

uint16_t Touch_Read(uint8_t cmd) {
    uint8_t tx_data[3] = {cmd, 0x00, 0x00};
    uint8_t rx_data[3] = {0x00, 0x00, 0x00};

    HAL_GPIO_WritePin(TCH_CS_GPIO_Port, TCH_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi2, tx_data, rx_data, 3, 100);
    HAL_GPIO_WritePin(TCH_CS_GPIO_Port, TCH_CS_Pin, GPIO_PIN_SET);

    // Combine the 2 data bytes and shift right by 4 to get the 12-bit value
    return ((rx_data[1] << 8) | rx_data[2]) >> 4;
}

void LCD_DrawBorder(void) {
    uint16_t black = 0x0000; // RGB565 for Black

    // 1. Draw Top and Bottom edges (sweep across X)
    for (int x = 0; x < 320; x++) {
        LCD_DrawPen(x, 120, black);       // Top edge
        LCD_DrawPen(x, 440, black);     // Bottom edge
    }

    // 2. Draw Left and Right edges (sweep across Y)
    for (int y = 120; y < 440; y++) {
        LCD_DrawPen(0, y, black);       // Left edge
        LCD_DrawPen(319, y, black);     // Right edge
    }

    for (int y = 60; y < 120; y++) {
        LCD_DrawPen(160, y, black);
    }

    for (int x = 0; x < 320; x++) {
        LCD_DrawPen(x, 60, black);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_6) {
        // 1. Abort if finger is already lifted
        if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_SET) return;

        // 2. Read Z-axis (Pressure) to ensure valid physical contact
        uint16_t z1 = Touch_Read(0xB0);
        uint16_t z2 = Touch_Read(0xC0);

        // Z1 approaches 0 when there is no touch.
        if (z1 < 50 || z1 > 4000) return;

        // 3. Dummy reads to allow multiplexer voltage settling
        Touch_Read(0xD0);
        Touch_Read(0x90);

        const uint8_t NUM_SAMPLES = 9; // Odd numbers are slightly better for medians
                uint16_t samples_x[NUM_SAMPLES];
                uint16_t samples_y[NUM_SAMPLES];

                // A. Collect the raw data
                for (int i = 0; i < NUM_SAMPLES; i++) {
                    samples_x[i] = Touch_Read(0xD0);
                    samples_y[i] = Touch_Read(0x90);
                }

                // B. Sort X array (Insertion Sort - extremely fast for small N)
                for (int i = 1; i < NUM_SAMPLES; i++) {
                    uint16_t key = samples_x[i];
                    int j = i - 1;
                    while (j >= 0 && samples_x[j] > key) {
                        samples_x[j + 1] = samples_x[j];
                        j = j - 1;
                    }
                    samples_x[j + 1] = key;
                }

                // C. Sort Y array (Sort them independently to destroy independent noise!)
                for (int i = 1; i < NUM_SAMPLES; i++) {
                    uint16_t key = samples_y[i];
                    int j = i - 1;
                    while (j >= 0 && samples_y[j] > key) {
                        samples_y[j + 1] = samples_y[j];
                        j = j - 1;
                    }
                    samples_y[j + 1] = key;
                }

                // D. Average the middle values (Drop the lowest 2 and highest 2)
                uint32_t sum_x = 0;
                uint32_t sum_y = 0;
                const uint8_t TRIM_AMOUNT = 3; // Throw away 2 from top, 2 from bottom

                for (int i = TRIM_AMOUNT; i < (NUM_SAMPLES - TRIM_AMOUNT); i++) {
                    sum_x += samples_x[i];
                    sum_y += samples_y[i];
                }

                // Divide by the remaining samples (9 total - 4 dropped = 5)
                uint16_t raw_x = sum_x / (NUM_SAMPLES - (2 * TRIM_AMOUNT));
                uint16_t raw_y = sum_y / (NUM_SAMPLES - (2 * TRIM_AMOUNT));

        // 5. Abort if finger was lifted during the sampling loop
        if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_SET) return;

        // 6. Ignore garbage noise
        if (raw_x < 50 || raw_x > 4000 || raw_y < 50 || raw_y > 4000) {
            return;
        }

        // 7. Clamp raw values
        if (raw_x < 200) raw_x = 200;
        if (raw_x > 1811) raw_x = 1811;
        if (raw_y < 200) raw_y = 200;
        if (raw_y > 1920) raw_y = 1920;

        // 8. Map directly to pixels
        int pixel_x = ((int)raw_x - 200) * 320 / (1811 - 200);
        int pixel_y = ((int)raw_y - 200) * 480 / (1920 - 200);

        // Flip Y-axis (Invert over X-axis)
        pixel_y = 479 - pixel_y;

        // 9. Final safety clamp
        if (pixel_x < 0) pixel_x = 0; if (pixel_x > 319) pixel_x = 319;
        if (pixel_y < 0) pixel_y = 0; if (pixel_y > 479) pixel_y = 479;

                // ------------------------------------------

        	//NOTE X DIRECTION IS FLIPPED SO LEFT SECTION IS 160 to 320
        if (((160 <= pixel_x) && (pixel_x < 320)) && ((40 <= pixel_y) && (pixel_y< 100))) {
        	//clear button selected
        	LCD_BlackScreen(); // Default begin black screen

        	LCD_ClearScreen(); // Default begin clear screen
        	LCD_DrawBorder();
        	printf("CLEAR_LOG\n\r");

        	last_pixel_x = -1;
        	last_pixel_y = -1;
        	return;
        }
        if (((0 <= pixel_x) && (pixel_x < 160)) && ((40 <= pixel_y) && (pixel_y< 100))) {
        	//submit button selected
        	printf("SAVE_FILE\n\r");
        	return;
        }

		const int PIXEL_JUMP_THRESHOLD = 3000; // Maximum allowed pixel jump

		// 1. Accept the very first touch unconditionally to initialize
		if (last_pixel_x == -1 || last_pixel_y == -1) {
			last_pixel_x = pixel_x;
			last_pixel_y = pixel_y;
		}
		// 2. For all other touches, enforce the distance limit
		else {
			int diff_x = abs(pixel_x - last_pixel_x);
			int diff_y = abs(pixel_y - last_pixel_y);

			if (diff_x > PIXEL_JUMP_THRESHOLD || diff_y > PIXEL_JUMP_THRESHOLD) {
				// Jump is too large (phantom dot). Abort!
				return;
			}

			// Point is valid, update the tracker
			last_pixel_x = pixel_x;
			last_pixel_y = pixel_y;
		}

        // Print calculated pixels, not raw values
        int adjx = 319 - pixel_x;
        int adjy = pixel_y;

        int gantryx = (adjx/320.0)*100;
        int gantryy = ((adjy-120.0)/320.0)*100;

        if (gantryx > 100) return;
        if (gantryy > 100) return;
        if (gantryx < 0) return;
        if (gantryy < 0) return;
        LCD_DrawingPointerCircle(pixel_x, pixel_y, 3);


        printf("%d,%d\n\r", pixel_x, pixel_y);
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
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_LPUART1_UART_Init();
  MX_USART1_UART_Init();
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
  LCD_WriteByte(0x55); // 0x55 is 16 bit color

  LCD_WriteCommand(0x36); // Memory Data Access Control (Orientation)
  LCD_WriteByte(0x48); //default

  LCD_WriteCommand(0x29); // Display ON
  HAL_Delay(10);

  //SCREEN IS 320 by 480 PIXELS (resolution)

  //memory write command
  // LCD_BlackScreen(); // Default begin black screen
  HAL_Delay(100);

  LCD_ClearScreen(); // Default begin clear screen
  LCD_DrawBorder();
  HAL_Delay(100);

  setvbuf(stdout, NULL, _IONBF, 0);
  printf("hello");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  LCD_IRPointerCircle(160, 160, 3);
  HAL_Delay(100);
  LCD_IRPointerCircle(260, 260, 3);
  HAL_Delay(100);

  uint8_t raw[4];
  while (1)
  {
	  uint8_t start = 0;
	       HAL_StatusTypeDef status;

	       status = HAL_UART_Receive(&huart1, &start, 1, 1000);
	       if (status != HAL_OK)
	       {
	           printf("Timeout or RX fail waiting for start\r\n");
	           continue;
	       }

	       if (start != 0xAA && start != 0xAB)
	       {
	           continue;
	       }

	       uint8_t raw[4] = {0};
	       status = HAL_UART_Receive(&huart1, raw, 4, 1000);
	       if (status != HAL_OK)
	       {
	           printf("Payload RX fail\r\n");
	           continue;
	       }

	       uint16_t x = ((uint16_t)raw[0] << 8) | raw[1];
	       uint16_t y = ((uint16_t)raw[2] << 8) | raw[3];


	       printf("raw = %02X %02X %02X %02X | x = %u, y = %u\r\n",
	              raw[0], raw[1], raw[2], raw[3], x, y);
	       if (start == 0xAA) // cursor mode
	       LCD_IRPointerCircle(x, y, 3);

	       else // draw mode
	       LCD_DrawingPointerCircle(x, y, 3);
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
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  hlpuart1.FifoMode = UART_FIFOMODE_DISABLE;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&hlpuart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&hlpuart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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
  __HAL_RCC_GPIOG_CLK_ENABLE();
  HAL_PWREx_EnableVddIO2();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TCH_CS_GPIO_Port, TCH_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RESET_Pin|DC_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LCD_CS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TCH_CS_Pin */
  GPIO_InitStruct.Pin = TCH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TCH_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : RESET_Pin DC_RS_Pin */
  GPIO_InitStruct.Pin = RESET_Pin|DC_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
