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

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN PV */
int prev_x, prev_y = -1; // no prev location on start up for IR light
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_LPUART1_UART_Init(void);
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

void LCD_DrawCircle(int cx, int cy, int r, uint16_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                LCD_DrawPixel(cx + x, cy + y, color);
            }
        }
    }
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
    static int prev_x = -1, prev_y = -1;
    static int prev_r = 3;

    int size = 2 * r + 1;
    uint8_t buf[size * size * 2]; // RGB565 buffer

    // Fill buffer with white pixels (0xFFFF) to erase previous pointer
    for (int i = 0; i < size * size; i++) {
        buf[2*i] = 0xFF;      // upper byte
        buf[2*i+1] = 0xFF;    // lower byte
    }

    // Erase previous pointer by drawing white over it
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

        // 4. Oversampling/Averaging
        uint32_t sum_x = 0;
        uint32_t sum_y = 0;
        const uint8_t NUM_SAMPLES = 10;

        for (int i = 0; i < NUM_SAMPLES; i++) {
            sum_x += Touch_Read(0xD0);
            sum_y += Touch_Read(0x90);
        }

        uint16_t raw_x = sum_x / NUM_SAMPLES;
        uint16_t raw_y = sum_y / NUM_SAMPLES;

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

        LCD_DrawPen(pixel_x, pixel_y, 0xF800);
        // Print calculated pixels, not raw values
       // printf("Pix X: %d, Pix Y: %d\r\n", pixel_x, pixel_y);
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
  LCD_BlackScreen(); // Default begin black screen
  HAL_Delay(100);

  LCD_ClearScreen(); // Default begin clear screen
  HAL_Delay(100);


////  // DRAWING CIRCLE
//  int centerX = 160; // screen center X
//  int centerY = 240; // screen center Y
//  int radius = 3;    // radius
//  int pathRadius = 100; // radius of circular path
//
//  for (int angle = 0; angle < 360; angle++) {
//      // Convert angle to radians
//      float rad = angle * 3.14159f / 180.0f;
//
//      // Compute new pointer position
//      int x = centerX + (int)(pathRadius * cosf(rad));
//      int y = centerY + (int)(pathRadius * sinf(rad));
//
//      // Draw the pointer (erases previous automatically)
//      LCD_DrawingPointerCircle(x, y, radius);
//
//      HAL_Delay(10); // adjust speed
//  }
//
//  LCD_ClearScreen();
//  HAL_Delay(100);
//
//  for (int angle = 0; angle < 360; angle++) {
//        // Convert angle to radians
//        float rad = angle * 3.14159f / 180.0f;
//
//        // Compute new pointer position
//        int x = centerX + (int)(pathRadius * cosf(rad));
//        int y = centerY + (int)(pathRadius * sinf(rad));
//
//        // Draw the pointer (erases previous automatically)
//        LCD_IRPointerCircle(x, y, radius);
//
//        HAL_Delay(10); // adjust speed
//    }
//
//  LCD_ClearScreen();
//  HAL_Delay(100);
//
//  // SQUARE TEST
//  int squareSize = 50; // Side length of square
//  int startX = centerX - squareSize / 2;
//  int startY = centerY - squareSize / 2;
//
//  // Draw Square (Move along the edges step by step)
//  // Start at top-left corner
//  for (int i = 0; i < squareSize; i++) {
//	  // Top side: Draw right
//	  LCD_DrawingPointerCircle(startX + i, startY, radius);
//	  HAL_Delay(5);
//  }
//  for (int i = 0; i < squareSize; i++) {
//	  // Right side: Draw down
//	  LCD_DrawingPointerCircle(startX + squareSize - 1, startY + i, radius);
//	  HAL_Delay(5);
//  }
//  for (int i = 0; i < squareSize; i++) {
//	  // Bottom side: Draw left
//	  LCD_DrawingPointerCircle(startX + squareSize - 1 - i, startY + squareSize - 1, radius);
//	  HAL_Delay(5);
//  }
//  for (int i = 0; i < squareSize; i++) {
//	  // Left side: Draw up
//	  LCD_DrawingPointerCircle(startX, startY + squareSize - 1 - i, radius);
//	  HAL_Delay(5);
//  }
//
//  HAL_Delay(500); // pause to view the square
//
//  LCD_ClearScreen(); // Default begin clear screen
//  HAL_Delay(100);
//
//  // Draw Square (Move along the edges step by step)
//  // Start at top-left corner
//  for (int i = 0; i < squareSize; i++) {
//  // Top side: Draw right
//	  LCD_IRPointerCircle(startX + i, startY, radius);
//	  HAL_Delay(5);
//  }
//  for (int i = 0; i < squareSize; i++) {
//  // Right side: Draw down
//	LCD_IRPointerCircle(startX + squareSize - 1, startY + i, radius);
//	HAL_Delay(5);
//  }
//  for (int i = 0; i < squareSize; i++) {
//	  // Bottom side: Draw left
//	  LCD_IRPointerCircle(startX + squareSize - 1 - i, startY + squareSize - 1, radius);
//	  HAL_Delay(5);
//  }
//  for (int i = 0; i < squareSize; i++) {
//	// Left side: Draw up
//	LCD_IRPointerCircle(startX, startY + squareSize - 1 - i, radius);
//	HAL_Delay(5);
//  }
//
//    HAL_Delay(500); // pause to view the square
//
//    LCD_ClearScreen(); // Default begin clear screen
//    HAL_Delay(100);
//
//  // BLOCK M TEST
//  int triangleHeight = 100;
//  int triangleBase = 100;
//  int triangleX = centerX - triangleBase / 2;
//
//  for (int i = triangleHeight; i >= 0; i--) {
//	  int x = triangleX;
//	  int y = centerY - i;
//	  LCD_DrawingPointerCircle(x, y, radius);
//	  HAL_Delay(5);
//  }
//
//  for (int i = 0; i <= triangleHeight / 2; i++) {
//	  int x = triangleX + i;
//	  int y = centerY - i;
//	  LCD_DrawingPointerCircle(x, y, radius);
//	  HAL_Delay(5);
//  }
//
//  for (int i = triangleHeight / 2; i >= 0; i--) {
//	  int x = triangleX + triangleBase - i;
//	  int y = centerY - i;
//	  LCD_DrawingPointerCircle(x, y, radius);
//	  HAL_Delay(5);
//  }
//
//  for (int i = 0; i < triangleHeight; i++) {
//	  int x = triangleX + triangleHeight;
//	  int y = centerY - i;
//	  LCD_DrawingPointerCircle(x, y, radius);
//	  HAL_Delay(5);
//  }
//
//  HAL_Delay(500);

  // LCD_ClearScreen(); // Default begin clear screen

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

  setvbuf(stdout, NULL, _IONBF, 0);
  printf("hello");
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
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
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
