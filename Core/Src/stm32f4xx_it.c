/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "qpc.h"
#include "signals.h"
#include <string.h>
extern UART_HandleTypeDef huart3;
extern QActive * const AO_Modbus;

/* 定义一个极其纯粹的底层 DMA 接收大水缸 (比如放 256 字节) */
uint8_t dma_rx_buf[256];

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern volatile uint8_t g_qf_ready;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* 引入外部函数声明 */
extern void Touch_Process(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_uart4_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
  __disable_irq(); /* 关门，封锁现场 */

  char const *msg = "\r\n\r\n!!! [HARD FAULT DETECTED] !!!\r\nMCU Crashed!\r\n";

  /* 用 USART1 强行把遗言砸出去！ */
  for (int i = 0; msg[i] != '\0'; i++) {
      while ((USART1->SR & (1 << 7)) == 0) { }
      USART1->DR = msg[i];
  }
  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

    /* * 直接调用 QTIMEEVT_TICK_X 是安全的。
     * 第二个参数 (void *)0 代表“任务级别”的心跳。
     * 如果你没定义自己的处理对象，传 (void *)0 是最标准的做法。
     */
    QTIMEEVT_TICK_X(0U, (void *)0);

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
//  static uint8_t touch_tick = 0;
//     if (++touch_tick >= 30) {
//         touch_tick = 0;
//         Touch_Process(); /* 呼叫咱们刚刚写的触摸翻译官！ */
//     }
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line1 interrupt.
  */
void EXTI1_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI1_IRQn 0 */

  /* USER CODE END EXTI1_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  /* USER CODE BEGIN EXTI1_IRQn 1 */

  /* USER CODE END EXTI1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_IRQn 0 */

  /* USER CODE END EXTI4_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
  /* USER CODE BEGIN EXTI4_IRQn 1 */

  /* USER CODE END EXTI4_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream1 global interrupt.
  */
void DMA1_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */

  /* USER CODE END DMA1_Stream1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
  /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

  /* USER CODE END DMA1_Stream1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream2 global interrupt.
  */
void DMA1_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream2_IRQn 0 */

  /* USER CODE END DMA1_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_uart4_rx);
  /* USER CODE BEGIN DMA1_Stream2_IRQn 1 */

  /* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream3 global interrupt.
  */
void DMA1_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream3_IRQn 0 */

  /* USER CODE END DMA1_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  /* USER CODE BEGIN DMA1_Stream3_IRQn 1 */

  /* USER CODE END DMA1_Stream3_IRQn 1 */
}

/**
  * @brief This function handles DMA1 stream4 global interrupt.
  */
void DMA1_Stream4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream4_IRQn 0 */

  /* USER CODE END DMA1_Stream4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_uart4_tx);
  /* USER CODE BEGIN DMA1_Stream4_IRQn 1 */

  /* USER CODE END DMA1_Stream4_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */
  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET) {

          /* 3. 必须的常规操作：清除 IDLE 标志位 (先读 SR 再读 DR 即可清除) */
          __HAL_UART_CLEAR_IDLEFLAG(&huart3);

          /* 4. 强行停止 DMA，防止新数据冲刷我们的缓冲缸 */
          HAL_UART_DMAStop(&huart3);

          /* 5. 算出到底收到了多少个字节？
             计算公式：大水缸总容量 - DMA 还没搬运完的数量 = 实际收到的长度 */
          uint16_t rx_len = 256 - __HAL_DMA_GET_COUNTER(huart3.hdmarx);


          /* 如果您板子上有 LED，比如 PC13，让它翻转一下！ */
                  // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

          #ifdef Q_SPY
                  QS_BEGIN_ID(QS_USER, 0)
                      QS_STR("MB RX LEN:"); QS_U32(3, rx_len);
                  QS_END()
          #endif
          /* 6. 只要收到了数据，就打包发给业务大脑！ */
          if (rx_len > 0 && rx_len < 256) {

              /* 【QP/C 魔法】：向内存池申请一个极其珍贵的事件快递盒！ */
              MbRxEvt *pe = Q_NEW(MbRxEvt, MB_RX_FRAME_SIG);

              if (pe != (MbRxEvt *)0) {
                  /* 把 DMA 水缸里的数据，倒进快递盒里 */
                  memcpy(pe->frame, dma_rx_buf, rx_len);
                  pe->len = rx_len;

                  /* 极其丝滑地投递给 AO_Modbus 活动对象，优先级为 0 (最高即可) */
                  QACTIVE_POST(AO_Modbus, &pe->super, 0U);
              }
          }

          /* 7. 重新挂上 DMA 倒挡，准备迎接下一帧报文！ */
          HAL_UART_Receive_DMA(&huart3, dma_rx_buf, 256);
      }
  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */

  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */
  if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_IDLE) != RESET) {

            /* 3. 必须的常规操作：清除 IDLE 标志位 (先读 SR 再读 DR 即可清除) */
            __HAL_UART_CLEAR_IDLEFLAG(&huart4);

            /* 4. 强行停止 DMA，防止新数据冲刷我们的缓冲缸 */
            HAL_UART_DMAStop(&huart4);

            /* 5. 算出到底收到了多少个字节？
               计算公式：大水缸总容量 - DMA 还没搬运完的数量 = 实际收到的长度 */
            uint16_t rx_len = 256 - __HAL_DMA_GET_COUNTER(huart4.hdmarx);

            /* 6. 只要收到了数据，就打包发给业务大脑！ */
            if (rx_len > 0 && rx_len < 256) {

                /* 【QP/C 魔法】：向内存池申请一个极其珍贵的事件快递盒！ */
                MbRxEvt *pe = Q_NEW(MbRxEvt, MB_RX_FRAME_SIG);

                if (pe != (MbRxEvt *)0) {
                    /* 把 DMA 水缸里的数据，倒进快递盒里 */
                    memcpy(pe->frame, dma_rx_buf, rx_len);
                    pe->len = rx_len;

                    /* 极其丝滑地投递给 AO_Modbus 活动对象，优先级为 0 (最高即可) */
                    QACTIVE_POST(AO_Modbus, &pe->super, 0U);
                }
            }

            /* 7. 重新挂上 DMA 倒挡，准备迎接下一帧报文！ */
            HAL_UART_Receive_DMA(&huart4, dma_rx_buf, 256);
        }
  /* USER CODE END UART4_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
