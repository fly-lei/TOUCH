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
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "qpc.h"
#include "blinky.h"
#include <stdarg.h>
#include <string.h>
#define Q_SPY
//#ifdef Q_SPY
//#error "Q_SPY IS ALIVE!!! 如果您在控制台看到这行报错，说明宏配置成功了！"
//#endif
void UART_Log(const char *fmt, ...) {
/* 如果开启了 QS 透视功能，就让传统的纯文本打印自动静音，防止污染 QS 二进制数据流！ */
#ifndef Q_SPY

    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
    va_end(args);

#endif
}
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* T_CS: PD13, T_CLK: PE0, T_MOSI: PE2, T_MISO: PE3, T_IRQ: PE4 */
#define T_CS_LOW()    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET)
#define T_CS_HIGH()   HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET)

#define T_CLK_LOW()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET)
#define T_CLK_HIGH()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET)

#define T_MOSI_LOW()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET)
#define T_MOSI_HIGH() HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET)

#define T_MISO_READ() HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)
#define T_IRQ_READ()  HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)

/* 软件模拟 SPI 发送并接收一个字节 */
uint8_t XPT2046_WriteRead(uint8_t data) {
    uint8_t i, res = 0;
    for (i = 0; i < 8; i++) {
        if (data & 0x80) T_MOSI_HIGH(); else T_MOSI_LOW();
        data <<= 1;
        T_CLK_LOW();
        T_CLK_HIGH(); // 上升沿采样
        res <<= 1;
        if (T_MISO_READ()) res |= 0x01;
    }
    return res;
}

/* 读取指定的 ADC 通道（X 或 Y） */
uint16_t XPT2046_Read_Adc(uint8_t cmd) {
    uint16_t res = 0;
    T_CS_LOW();
    XPT2046_WriteRead(cmd);
    // XPT2046 需要一点时间转换，这里读回 16 位，取高 12 位
    res = XPT2046_WriteRead(0x00) << 8;
    res |= XPT2046_WriteRead(0x00);
    T_CS_HIGH();
    return res >> 4; // 12 位有效数据
}













/* 1. 您的专属 FSMC 绝对物理地址 */
#define LCD_REG  (*((volatile uint16_t *)0x60000000)) // RS=0, 发送命令
#define LCD_DATA (*((volatile uint16_t *)0x60020000)) // RS=1, 发送/读取数据

/* 2. 读取 ILI9341 屏幕 ID 的函数 */
uint16_t Read_ILI9341_ID(void) {
    uint16_t dummy, id_00, id_93, id_41;

    LCD_REG = 0xD3;       // 发送读取 ID 命令

    dummy = LCD_DATA;     // 第 1 次读：假读 (Dummy Read，丢弃)
    id_00 = LCD_DATA;     // 第 2 次读：通常是 0x00
    id_93 = LCD_DATA;     // 第 3 次读：应该是 0x93
    id_41 = LCD_DATA;     // 第 4 次读：应该是 0x41

    // 将 0x93 和 0x41 拼起来返回
    return (id_93 << 8) | id_41;
}

/* 基础底层函数 */
void LCD_WriteReg(uint16_t reg) {
    LCD_REG = reg;
}

void LCD_WriteData(uint16_t data) {
    LCD_DATA = data;
}

/* 常用颜色定义 (RGB565格式) */
#define WHITE   0xFFFF
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void ILI9341_Init(void) {
    // 1. 退出休眠模式
    LCD_WriteReg(0x11);
    HAL_Delay(120); // 必须等待 120ms 让屏幕内部电源稳定

    // 2. 内存访问控制 (设置屏幕方向)
    LCD_WriteReg(0x36);
    LCD_WriteData(0x48); // 0x48 为默认竖屏

    // 3. 像素格式设置 (16-bit RGB565)
    LCD_WriteReg(0x3A);
    LCD_WriteData(0x55);

    // 4. 开启显示
    LCD_WriteReg(0x29);
}
void LCD_Clear(uint16_t color) {
    uint32_t i;

    // 设置列地址 (0 到 239)
    LCD_WriteReg(0x2A);
    LCD_WriteData(0x00); LCD_WriteData(0x00);
    LCD_WriteData(0x00); LCD_WriteData(0xEF); // 239

    // 设置页/行地址 (0 到 319)
    LCD_WriteReg(0x2B);
    LCD_WriteData(0x00); LCD_WriteData(0x00);
    LCD_WriteData(0x01); LCD_WriteData(0x3F); // 319

    // 开始写入内存
    LCD_WriteReg(0x2C);

    // 连续写入 240 * 320 个像素点
    for(i = 0; i < 76800; i++) {
        LCD_DATA = color; // FSMC 会自动生成写时序，极速！
    }
}
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint8_t g_qf_ready = 0;           // 1. 我们加的安全阀
static QEvt const *l_blinkyQSto[10];       // 2. Blinky 的信箱 (队列)
static QF_MPOOL_EL(QEvt) l_smlPoolSto[20]; // 3. 框架的备用内存池 (保留这一个即可)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
extern void BSP_init(void);    // 板级初始化声明
extern void GuiAO_ctor(void);
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
  MX_USART1_UART_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE BEGIN 2 */
  /* USER CODE BEGIN 2 */

    /* 刚进 main，先发个开机信号 */
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n\r\n>>> SYSTEM RESTART <<<\r\n", 28, 100);

    HAL_Delay(100);
    ILI9341_Init();
    LCD_Clear(RED);
    HAL_UART_Transmit(&huart1, (uint8_t *)"> LCD Init OK\r\n", 15, 100);

    /* 测试 QF_init */
    QF_init();
    HAL_UART_Transmit(&huart1, (uint8_t *)"> QF_init OK\r\n", 14, 100);

  #ifdef Q_SPY
    /* 测试 QS_INIT */
    HAL_UART_Transmit(&huart1, (uint8_t *)"> Ready for QS_INIT\r\n", 21, 100);
    if (!QS_INIT((void *)0)) {
        HAL_UART_Transmit(&huart1, (uint8_t *)"!!! QS_INIT FAILED !!!\r\n", 24, 100);
        Error_Handler();
    }
    HAL_UART_Transmit(&huart1, (uint8_t *)"> QS_INIT OK\r\n", 14, 100);
  #endif

    /* 测试内存池和构造 */
    QF_poolInit(l_smlPoolSto, sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));
    Blinky_ctor();
    HAL_UART_Transmit(&huart1, (uint8_t *)"> Ctor OK\r\n", 11, 100);

    /* 测试状态机启动 */
    QACTIVE_START(AO_Blinky, 1U, l_blinkyQSto, Q_DIM(l_blinkyQSto), (void *)0, 0U, (void *)0);
    HAL_UART_Transmit(&huart1, (uint8_t *)"> Active Start OK\r\n", 19, 100);

    HAL_UART_Transmit(&huart1, (uint8_t *)"> Entering QF_run...\r\n", 22, 100);
    return QF_run();

    /* USER CODE END 2 */
    /* USER CODE END 2 */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
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
