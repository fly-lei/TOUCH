/*
 * ILI9341.h
 *
 *  Created on: Mar 14, 2026
 *      Author: 14019
 */

#ifndef INC_ILI9341_H_
#define INC_ILI9341_H_
#include "main.h"

/* 1. 您的专属 FSMC 绝对物理地址 */
#define LCD_REG  (*((volatile uint16_t *)0x60000000)) // RS=0, 发送命令
#define LCD_DATA (*((volatile uint16_t *)0x60020000)) // RS=1, 发送/读取数据

/* ========================================================= */
/* 常用颜色定义 (16-bit RGB565 格式)                         */
/* ========================================================= */

/* 1. 基础三原色及黑白 */
#define WHITE           0xFFFF  /* 纯白 */
#define BLACK           0x0000  /* 纯黑 */
#define RED             0xF800  /* 纯红 */
#define GREEN           0x07E0  /* 纯绿 */
#define BLUE            0x001F  /* 纯蓝 */

/* 2. 二次混合色 (高饱和度，适合做警示或高亮) */
#define YELLOW          0xFFE0  /* 黄色 (红+绿) */
#define CYAN            0x07FF  /* 青色 (绿+蓝) */
#define MAGENTA         0xF81F  /* 品红/紫红 (红+蓝) */

/* 3. 护眼深色系 (非常适合做高级 UI 的背景色) */
#define DARKBLUE        0x01CF  /* 深蓝色 (比纯蓝暗一点，更稳重) */
#define NAVY            0x000F  /* 海军蓝 */
#define DARKGREEN       0x03E0  /* 深绿色 */
#define DARKCYAN        0x03EF  /* 深青色 */
#define MAROON          0x7800  /* 栗色/深红色 */
#define OLIVE           0x7BE0  /* 橄榄绿 */

/* 4. 现代 UI 常用过渡色系 (适合做菜单栏、边框、阴影) */
#define GRAY            0x8410  /* 标准灰 */
#define LIGHTGRAY       0xD69A  /* 浅灰色 (适合做未选中项的字体颜色) */
#define DARKGRAY        0x4208  /* 深灰色 (适合做次要背景色) */
#define SILVER          0xC618  /* 银色 */

/* 5. 个性化点缀色 (适合做进度条、特殊按钮) */
#define ORANGE          0xFD20  /* 橙色 */
#define PURPLE          0x780F  /* 紫色 */
#define PINK            0xF81F  /* 粉色 */
#define BROWN           0xBC40  /* 棕色 */
#define GOLD            0xFEA0  /* 金色 */

/* ========================================================= */
/* USER CODE END PTD */
void LCD_Fill(uint16_t , uint16_t , uint16_t , uint16_t , uint16_t );
void LCD_Clear(uint16_t color);
void ILI9341_Init(void);
uint16_t Read_ILI9341_ID(void);
void LCD_WriteReg(uint16_t reg);
void LCD_WriteData(uint16_t data);
//void LCD_SetWindow(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd);
void LCD_SetWindow(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd);
void LCD_ShowChar(uint16_t x, uint16_t y, char chr, uint16_t fg_color, uint16_t bg_color) ;
//void LCD_ShowString(uint16_t x, uint16_t y, char *str, uint8_t zoom, uint16_t fg, uint16_t bg)
void LCD_ShowString(uint16_t x, uint16_t y, char *str, uint16_t fg_color, uint16_t bg_color);
//static uint32_t LCD_Pow(uint8_t m, uint8_t n);
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint16_t fg_color, uint16_t bg_color);

uint16_t XPT2046_Read_Adc_Smooth(uint8_t cmd);
uint8_t TP_Read_XY(uint16_t *x, uint16_t *y);
void Touch_Process(void);
void LCD_Backlight_Init(void);
void LCD_SetBrightness(uint8_t percent);
/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#endif /* INC_ILI9341_H_ */
