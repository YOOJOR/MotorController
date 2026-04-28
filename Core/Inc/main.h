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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MOTOR_EN2_Pin GPIO_PIN_0
#define MOTOR_EN2_GPIO_Port GPIOA
#define MOTOR_DIR2_Pin GPIO_PIN_1
#define MOTOR_DIR2_GPIO_Port GPIOA
#define MOTOR_EN1_Pin GPIO_PIN_2
#define MOTOR_EN1_GPIO_Port GPIOA
#define MOTOR_DIR1_Pin GPIO_PIN_3
#define MOTOR_DIR1_GPIO_Port GPIOA
#define LASER_Pin GPIO_PIN_5
#define LASER_GPIO_Port GPIOA
#define MOTOR_STEP1_Pin GPIO_PIN_6
#define MOTOR_STEP1_GPIO_Port GPIOA
#define D3LED_Pin GPIO_PIN_7
#define D3LED_GPIO_Port GPIOA
#define MOTOR_STEP2_Pin GPIO_PIN_13
#define MOTOR_STEP2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
