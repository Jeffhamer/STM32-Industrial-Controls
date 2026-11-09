/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

#include "stm32g4xx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

typedef enum
{
	MODE_AUTO,
	MODE_FOUR_WAY_STOP,
	MODE_TWO_WAY_STOP,
	MODE_FAULT
} TrafficMode;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define RCC_OSC_IN_Pin GPIO_PIN_0
#define RCC_OSC_IN_GPIO_Port GPIOF
#define RCC_OSC_OUT_Pin GPIO_PIN_1
#define RCC_OSC_OUT_GPIO_Port GPIOF
#define TRAFFIC_A_YELLOW_Pin GPIO_PIN_7
#define TRAFFIC_A_YELLOW_GPIO_Port GPIOC
#define TRAFFIC_A_GREEN_Pin GPIO_PIN_8
#define TRAFFIC_A_GREEN_GPIO_Port GPIOC
#define TRAFFIC_B_RED_Pin GPIO_PIN_9
#define TRAFFIC_B_RED_GPIO_Port GPIOC
#define TRAFFIC_B_YELLOW_Pin GPIO_PIN_8
#define TRAFFIC_B_YELLOW_GPIO_Port GPIOA
#define TRAFFIC_B_GREEN_Pin GPIO_PIN_9
#define TRAFFIC_B_GREEN_GPIO_Port GPIOA
#define TRAFFIC_A_RED_Pin GPIO_PIN_10
#define TRAFFIC_A_RED_GPIO_Port GPIOA
#define T_SWDIO_Pin GPIO_PIN_13
#define T_SWDIO_GPIO_Port GPIOA
#define T_SWCLK_Pin GPIO_PIN_14
#define T_SWCLK_GPIO_Port GPIOA
#define MODE_BUTTON_Pin GPIO_PIN_10
#define MODE_BUTTON_GPIO_Port GPIOC
#define MODE_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define T_SWO_Pin GPIO_PIN_3
#define T_SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define DEBOUNCE_TIME_MS	150U
#define FAULT_RESET_TIME_MS 2000U
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
