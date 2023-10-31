/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "st7735.h"
#include "DefineFont.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char LCD_buff[16];
int32_t prevCounter = 0;
int32_t currCounter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void encoder_init(void);
void loop(void);
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
  MX_SPI1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	
	encoder_init();
	
	lcdInit();
	ClearLcdMemory();
	LCD_SetFont(Arial_15x17,black);
	LCD_ShowString(5,0,"start...");
	LCD_Refresh();
	
	LED_RED(OFF);
	
	DIR_DRIVE(BACKWARD);
	DRIVE_ENABLE(OFF);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {		
	//	for (uint8_t count = 0; count < 100; count++)
		{
			loop();
		//	HAL_Delay(5);
		}
		
	/*	DRIVE_ENABLE(ON);
		HAL_Delay(3);
		DIR_DRIVE(FORWARD);
		HAL_Delay(5);
		for (uint8_t i = 0; i < 9; i++)
		{
			STEP(ON);
			HAL_Delay(3);
			STEP(OFF);
			HAL_Delay(3);
		}
		STEP(ON);
		DIR_DRIVE(BACKWARD);
		HAL_Delay(3);
		STEP(OFF);
		//HAL_Delay(6);
		DRIVE_ENABLE(OFF);*/
		
//		TOOGLE_LED_RED();
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
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(72000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void encoder_init(void) 
{
    
  LL_TIM_SetCounter(TIM1, 32760); // начальное значение счетчика:
	
	/* Enable the encoder interface channels */
	LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH1);
	LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH2);

  LL_TIM_EnableCounter(TIM1);     // не забываем включить таймер!
}

void loop(void) 
{
	currCounter = LL_TIM_GetCounter(TIM1); //текущее показание энкодера
	currCounter = 32767 - (((currCounter-1) & 0x0FFFF) / 2);
	int32_t need_step = 0;
/*if(currCounter > 32768/2) 
	{
		// Преобразуем значения счетчика из: 32766, 32767, .., в значения: ... -2, -1, 0, 1, 2 ...
		currCounter = currCounter - 32768;
  }*/
	
	
	if(currCounter != prevCounter) 
	{
		int32_t delta = currCounter-prevCounter;
    prevCounter = currCounter;
    // защита от дребезга контактов и переполнения счетчика (переполнение будет случаться очень редко)
    if((delta > -10) && (delta < 10)) 
		{    
			snprintf(LCD_buff, sizeof(LCD_buff), "%06d %03d", currCounter, delta);
			ClearLcdMemory();
			LCD_ShowString(3, 10, LCD_buff);
			LCD_Refresh();
			prevCounter = currCounter;
			if (delta < 0)
			{
				DRIVE_ENABLE(ON);
				DIR_DRIVE(BACKWARD);
				HAL_Delay(5);
				need_step = 5*delta;
				while (need_step < 0)
				{
					STEP(ON);
					HAL_Delay(3);
					STEP(OFF);
					HAL_Delay(3);
					need_step++;
				}
				STEP(ON);
				DIR_DRIVE(FORWARD);
				HAL_Delay(3);
				STEP(OFF);
				DRIVE_ENABLE(OFF);
			}
			else
			{
				if (delta > 0)
				{
					DRIVE_ENABLE(ON);
					DIR_DRIVE(FORWARD);
					HAL_Delay(5);
					need_step = 5*delta;
					while (need_step > 0)
					{
						STEP(ON);
						HAL_Delay(3);
						STEP(OFF);
						HAL_Delay(3);
						need_step--;
					}
					STEP(ON);
					DIR_DRIVE(BACKWARD);
					HAL_Delay(3);
					STEP(OFF);
					DRIVE_ENABLE(OFF);
				}
			}
		}
	}
}
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

#ifdef  USE_FULL_ASSERT
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
