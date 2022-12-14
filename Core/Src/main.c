/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hd44780.h"
#include "stm32_device.h"
#include "stdio.h"
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
volatile uint32_t freq = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void set_lcd_brightness();
void print_freq_format(int freq_value);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */
	HAL_Delay(100);
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);

	set_lcd_brightness();

	lcdInit(&lcdConfig);
	lcdClrScr();

	uint8_t freq_scaler = 1;
	uint8_t LCD_BUFFER[12] = { };
	char str_Hz[] = "Hz";
	char str_MHz[] = "MHz";
	char str_M[] = "M";
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {
		// ?????????? ??????????????

		print_freq_format(freq * freq_scaler);

		if (HAL_GPIO_ReadPin(BAND_20M_GPIO_Port, BAND_20M_Pin)
				&& HAL_GPIO_ReadPin(BAND_40M_GPIO_Port, BAND_40M_Pin)) {
			freq_scaler = 1;
			lcdGoto(2, 0);
			sprintf(LCD_BUFFER, "%2d%s %-2d" "%s", 7, str_MHz, 40, str_M);
			lcdPuts(LCD_BUFFER);
		}

		else if (!HAL_GPIO_ReadPin(BAND_20M_GPIO_Port, BAND_20M_Pin)) {
			freq_scaler = 2;
			lcdGoto(2, 0);
			sprintf(LCD_BUFFER, "%2d%s %-2d" "%s", 14, str_MHz, 20, str_M);
			lcdPuts(LCD_BUFFER);
		} else if (!HAL_GPIO_ReadPin(BAND_40M_GPIO_Port, BAND_40M_Pin)) {
			freq_scaler = 3;
			lcdGoto(2, 0);
			sprintf(LCD_BUFFER, "%2d%s %-2d" "%s", 21, str_MHz, 10, str_M);
			lcdPuts(LCD_BUFFER);
		} else {
			freq_scaler = 31;
			lcdGoto(2, 3);
			lcdPuts("ERR");
		}

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim1) {
		freq = TIM2->CNT + (TIM3->CNT << 16);
		HAL_TIM_Base_Stop_IT(&htim1);
		__HAL_TIM_SET_COUNTER(&htim2, 0x0000);
		__HAL_TIM_SET_COUNTER(&htim3, 0x0000);
		HAL_TIM_Base_Start_IT(&htim1);
	}
}
void set_lcd_brightness() {
	HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_SET);
}

void print_freq_format(int freq_value) {
	int DIG1, DIG2, DIG3, DIG4, DIG5, DIG6;
	char LCD_BUFFER[12];
	DIG1 = freq_value % 100000000 / 10000000;
	DIG2 = freq_value % 10000000 / 1000000;
	DIG3 = freq_value % 1000000 / 100000;
	DIG4 = freq_value % 100000 / 10000;
	DIG5 = freq_value % 10000 / 1000;
	DIG6 = freq_value % 1000 / 100;
	sprintf(LCD_BUFFER, "%d%d.%d%d%d.%d00Hz", DIG1, DIG2, DIG3, DIG4, DIG5, DIG6);
	lcdGoto(1, 0);
	lcdPuts(LCD_BUFFER);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
