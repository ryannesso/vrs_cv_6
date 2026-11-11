/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "lps25hb.h"
#include "hts221.h"
#include <stdio.h>
#include <stdint.h>

void SystemClock_Config(void);

static int32_t absolute_i32(int32_t value)
{
  return value < 0 ? -value : value;
}

static void SendMeasurements(int16_t temperature_c_x10, uint16_t humidity_rh,
                             int32_t pressure_hpa_x100, int32_t altitude_cm)
{
  char line[96];
  int length = snprintf(line, sizeof(line), "%ld.%ld,%u,%ld.%02ld,%ld.%02ld\r\n",
      (long)(temperature_c_x10 / 10), (long)absolute_i32(temperature_c_x10 % 10),
      (unsigned)humidity_rh, (long)(pressure_hpa_x100 / 100),
      (long)absolute_i32(pressure_hpa_x100 % 100), (long)(altitude_cm / 100),
      (long)absolute_i32(altitude_cm % 100));
  if (length > 0) USART2_Write((const uint8_t *)line, (uint16_t)length);
}

int main(void)
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  SystemClock_Config();

  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();

  LPS25HB_Handle pressure_sensor = {0};
  HTS221_Handle environment_sensor = {0};
  int16_t temperature_c_x10;
  uint16_t humidity_rh;
  int32_t pressure_hpa_x100;

  LPS25HB_RegisterBus(&pressure_sensor, I2C_MasterRead, I2C_MasterWrite);
  HTS221_RegisterBus(&environment_sensor, I2C_MasterRead, I2C_MasterWrite);

  if (!LPS25HB_Init(&pressure_sensor) || !HTS221_Init(&environment_sensor))
  {
    USART2_WriteString("sensor initialization failed\r\n");
    while (1) { LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_3); LL_mDelay(250U); }
  }

  while (1)
  {
	  if (LPS25HB_ReadPressure(&pressure_sensor, &pressure_hpa_x100) &&
	      HTS221_ReadValues(&environment_sensor, &temperature_c_x10, &humidity_rh))
	  {
		  SendMeasurements(temperature_c_x10, humidity_rh, pressure_hpa_x100,
			  LPS25HB_GetRelativeAltitudeCm(&pressure_sensor, pressure_hpa_x100));
		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_3);
	  }
	  else
	  {
		  USART2_WriteString("sensor read error\r\n");
	  }
	  LL_mDelay(1000U);
  }
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();  
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
    
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  
  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_HSI);
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
