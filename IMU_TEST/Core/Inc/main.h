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
#include "stm32h7xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IMU_SDA_Pin GPIO_PIN_0
#define IMU_SDA_GPIO_Port GPIOF
#define IMU_SCL_Pin GPIO_PIN_1
#define IMU_SCL_GPIO_Port GPIOF
#define A1_Pin GPIO_PIN_7
#define A1_GPIO_Port GPIOF
#define A2_Pin GPIO_PIN_8
#define A2_GPIO_Port GPIOF
#define A3_Pin GPIO_PIN_9
#define A3_GPIO_Port GPIOF
#define A4_Pin GPIO_PIN_10
#define A4_GPIO_Port GPIOF
#define A5_Pin GPIO_PIN_0
#define A5_GPIO_Port GPIOC
#define A6_Pin GPIO_PIN_1
#define A6_GPIO_Port GPIOC
#define A7_Pin GPIO_PIN_2
#define A7_GPIO_Port GPIOC
#define NRF24_IRQ_Pin GPIO_PIN_4
#define NRF24_IRQ_GPIO_Port GPIOC
#define NRF24_CE_Pin GPIO_PIN_5
#define NRF24_CE_GPIO_Port GPIOC
#define NRF24_CS_Pin GPIO_PIN_0
#define NRF24_CS_GPIO_Port GPIOB
#define OK_STATUS_Pin GPIO_PIN_14
#define OK_STATUS_GPIO_Port GPIOD
#define ERR_STATUS_Pin GPIO_PIN_15
#define ERR_STATUS_GPIO_Port GPIOD
#define DS18B20_REFRI_Pin GPIO_PIN_5
#define DS18B20_REFRI_GPIO_Port GPIOD
#define D1_Pin GPIO_PIN_4
#define D1_GPIO_Port GPIOB
#define D2_Pin GPIO_PIN_5
#define D2_GPIO_Port GPIOB
#define D3_Pin GPIO_PIN_6
#define D3_GPIO_Port GPIOB
#define D4_Pin GPIO_PIN_7
#define D4_GPIO_Port GPIOB
#define D5_Pin GPIO_PIN_8
#define D5_GPIO_Port GPIOB
#define D6_Pin GPIO_PIN_9
#define D6_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
