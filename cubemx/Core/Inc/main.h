/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define Enable50V_190V_Pin GPIO_PIN_13
#define Enable50V_190V_GPIO_Port GPIOC
#define Dots_Pin GPIO_PIN_14
#define Dots_GPIO_Port GPIOC
#define TimeSync_Pin GPIO_PIN_15
#define TimeSync_GPIO_Port GPIOC
#define Tube3_Pin GPIO_PIN_0
#define Tube3_GPIO_Port GPIOD
#define Tube5_Pin GPIO_PIN_1
#define Tube5_GPIO_Port GPIOD
#define Digit0_EnableBusARGB_Pin GPIO_PIN_0
#define Digit0_EnableBusARGB_GPIO_Port GPIOA
#define Digit1_EnableBusShiftRegister_Pin GPIO_PIN_1
#define Digit1_EnableBusShiftRegister_GPIO_Port GPIOA
#define Digit2_Blanking_Pin GPIO_PIN_2
#define Digit2_Blanking_GPIO_Port GPIOA
#define Digit3_Strobe_Pin GPIO_PIN_3
#define Digit3_Strobe_GPIO_Port GPIOA
#define Digit4_Heatwire_Pin GPIO_PIN_4
#define Digit4_Heatwire_GPIO_Port GPIOA
#define Digit5_Pin GPIO_PIN_5
#define Digit5_GPIO_Port GPIOA
#define Digit6_Pin GPIO_PIN_6
#define Digit6_GPIO_Port GPIOA
#define Digit7_Pin GPIO_PIN_7
#define Digit7_GPIO_Port GPIOA
#define Tube0_Pin GPIO_PIN_10
#define Tube0_GPIO_Port GPIOB
#define Tube1_Pin GPIO_PIN_11
#define Tube1_GPIO_Port GPIOB
#define Tube2_Pin GPIO_PIN_12
#define Tube2_GPIO_Port GPIOB
#define Tube4_Pin GPIO_PIN_14
#define Tube4_GPIO_Port GPIOB
#define Digit8_SR_CLK_Pin GPIO_PIN_8
#define Digit8_SR_CLK_GPIO_Port GPIOA
#define Digit9_SR_Data_Pin GPIO_PIN_9
#define Digit9_SR_Data_GPIO_Port GPIOA
#define LeftComma_Pin GPIO_PIN_4
#define LeftComma_GPIO_Port GPIOB
#define RightComma_Pin GPIO_PIN_5
#define RightComma_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
