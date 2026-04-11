#ifndef LCD_TOUCH_H
#define LCD_TOUCH_H

#include "main.h"
#include "spi.h"

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

void LCD_Init(void);
void LCD_ClearScreen(void);
void LCD_BlackScreen(void);
void LCD_DrawBorder(void);
void LCD_DrawingPointerCircle(int x, int y, int r);
void LCD_DrawPen(int x, int y, uint16_t color);
void Touch_EXTI_Callback(uint16_t GPIO_Pin);
#endif
