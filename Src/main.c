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

#define LPS25HB_FIRST_SAMPLE_DELAY_MS 1100U

void SystemClock_Config(void);

typedef struct
{
  int16_t temperature_tenths;
  uint16_t humidity_percent;
  int32_t pressure_hundredths;
  int32_t altitude_centimetres;
} Measurement;

static LPS25HB_Handle pressure_sensor;
static HTS221_Handle climate_sensor;

static int32_t Magnitude(int32_t number)
{
  return (number < 0) ? -number : number;
}

static uint8_t StartSensors(void)
{
  LPS25HB_RegisterBus(&pressure_sensor, I2C_MasterRead, I2C_MasterWrite);
  HTS221_RegisterBus(&climate_sensor, I2C_MasterRead, I2C_MasterWrite);

  return (uint8_t)(LPS25HB_Init(&pressure_sensor) && HTS221_Init(&climate_sensor));
}

static uint8_t ReadMeasurement(Measurement *measurement)
{
  if (!LPS25HB_ReadPressure(&pressure_sensor, &measurement->pressure_hundredths))
  {
    return 0U;
  }

  if (!HTS221_ReadValues(&climate_sensor, &measurement->temperature_tenths,
                         &measurement->humidity_percent))
  {
    return 0U;
  }

  measurement->altitude_centimetres = LPS25HB_GetRelativeAltitudeCm(
      &pressure_sensor, measurement->pressure_hundredths);
  return 1U;
}

static void WriteMeasurement(const Measurement *measurement)
{
  char csv_row[96];
  int length = snprintf(csv_row, sizeof(csv_row), "%ld.%ld,%u,%ld.%02ld,%ld.%02ld\r\n",
      (long)(measurement->temperature_tenths / 10),
      (long)Magnitude(measurement->temperature_tenths % 10),
      (unsigned)measurement->humidity_percent,
      (long)(measurement->pressure_hundredths / 100),
      (long)Magnitude(measurement->pressure_hundredths % 100),
      (long)(measurement->altitude_centimetres / 100),
      (long)Magnitude(measurement->altitude_centimetres % 100));

  if ((length > 0) && ((size_t)length < sizeof(csv_row)))
  {
    USART2_Write((const uint8_t *)csv_row, (uint16_t)length);
  }
}

static void ReportSensorError(const char *message)
{
  USART2_WriteString(message);
}

static void SignalInitializationFailure(void)
{
  ReportSensorError("sensor initialization failed\r\n");

  for (;;)
  {
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_3);
    LL_mDelay(250U);
  }
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

  Measurement measurement;

  if (StartSensors() == 0U)
  {
    SignalInitializationFailure();
  }

  /* LPS25HB runs at 1 Hz: wait for a measurement made after power-on. */
  LL_mDelay(LPS25HB_FIRST_SAMPLE_DELAY_MS);

  for (;;)
  {
    if (ReadMeasurement(&measurement) != 0U)
    {
      WriteMeasurement(&measurement);
      LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_3);
    }
    else
    {
      ReportSensorError("sensor read error\r\n");
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
