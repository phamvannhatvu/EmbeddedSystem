/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "uart.h"
#include "ring_buffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// System modes
#define NORMAL_MODE 		0
#define REMOTE_UPDATE_TIME	1

// Remote update time modes
#define UPDATE_HOURS		0	
#define WAIT_FOR_HOURS		1
#define UPDATE_MINUTES		2
#define WAIT_FOR_MINUTES	3
#define UPDATE_SECONDS		4
#define WAIT_FOR_SECONDS		5
 
#define UART_TIMEOUT_CNT		200 // 10s, because software timer 2 cycle is 50ms
#define UART_TIMEOUT_MAX_TRIES	3

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t system_status = NORMAL_MODE;
uint8_t remote_update_time_status;
uint8_t timeout_cnt = 0;
uint8_t uart_timer_cnt = 0; // Used for detecting timeout in UART communication
uint8_t hour = 0;
uint8_t minute = 0;
uint8_t second = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void test_Uart();
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
	MX_TIM2_Init();
	MX_SPI1_Init();
	MX_FSMC_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	system_init();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */
		if (flag_timer2) {
			// Sample code
			flag_timer2 = 0;
			button_Scan();
			test_LedDebug();
			ds3231_ReadTime();


			// test_Uart();
			switch (system_status) {
				case NORMAL_MODE:
					// TODO: placeholder for button event for switch to remote update time mode
					if (1) {
						timeout_cnt = 0;
						lcd_Clear(BLACK);
						system_status = REMOTE_UPDATE_TIME;
						remote_update_time_status = UPDATE_HOURS;
					}
					break;
				case REMOTE_UPDATE_TIME:
					switch (remote_update_time_status) {
						case UPDATE_HOURS:
							uart_Rs232SendString("hours");
							lcd_Clear(BLACK);
							lcd_ShowStr(10, 20, "Updating hours...", GREEN, BLACK, 16, 0);
							remote_update_time_status = WAIT_FOR_HOURS;
							uart_timer_cnt = 0;
							break;
						case WAIT_FOR_HOURS:
							// Check for timeout
							if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
								if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
									lcd_Clear(BLACK);
									lcd_ShowStr(10, 100, "Wating hours timeout", RED, BLACK, 16, 0);
									system_status = NORMAL_MODE;
								} else {
									remote_update_time_status = UPDATE_HOURS;
									uart_timer_cnt = 0;
									timeout_cnt++;
								}
							} else {
								if (uart_receive_flag) {
									uart_receive_flag = 0;
									uint8_t hour_received = 24;
									// Get the latest data
									while (uart_ring_buffer.length > 0) {
										ringBufferPop(&uart_ring_buffer, &hour_received);
									}

									// Check hour format
									if (hour_received < 24) {
										remote_update_time_status = UPDATE_MINUTES;
										hour = hour_received;
										timeout_cnt = 0;
									} else {
										remote_update_time_status = UPDATE_HOURS;
										timeout_cnt = 0;
										lcd_ShowStr(10, 100, "Wating format of hours, try again", RED, BLACK, 16, 0);
									}
								} else {
									uart_timer_cnt++;
								}
							}
							break;
						case UPDATE_MINUTES:
							uart_Rs232SendString("minutes");
							lcd_Clear(BLACK);
							lcd_ShowStr(10, 20, "Updating minutes...", GREEN, BLACK, 16, 0);
							remote_update_time_status = WAIT_FOR_MINUTES;
							uart_timer_cnt = 0;
							break;
						case WAIT_FOR_MINUTES:
							// Check for timeout
							if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
								if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
									lcd_Clear(BLACK);
									lcd_ShowStr(10, 100, "Wating minutes timeout", RED, BLACK, 16, 0);
									system_status = NORMAL_MODE;
								} else {
									remote_update_time_status = UPDATE_MINUTES;
									uart_timer_cnt = 0;
									timeout_cnt++;
								}
							} else {
								if (uart_receive_flag) {
									uart_receive_flag = 0;
									uint8_t minute_received = 60;
									// Get the latest data
									while (uart_ring_buffer.length > 0) {
										ringBufferPop(&uart_ring_buffer, &minute_received);
									}

									// Check hour format
									if (minute_received < 60) {
										remote_update_time_status = UPDATE_SECONDS;
										minute = minute_received;
										timeout_cnt = 0;
									} else {
										remote_update_time_status = UPDATE_MINUTES;
										timeout_cnt = 0;
										lcd_ShowStr(10, 100, "Wating format of minutes, try again", RED, BLACK, 16, 0);
									}
								} else {
									uart_timer_cnt++;
								}
							}
							break;
						case UPDATE_SECONDS:
							uart_Rs232SendString("seconds");
							lcd_Clear(BLACK);
							lcd_ShowStr(10, 20, "Updating seconds...", GREEN, BLACK, 16, 0);
							remote_update_time_status = WAIT_FOR_SECONDS;
							uart_timer_cnt = 0;
							break;
						case WAIT_FOR_SECONDS:
							// Check for timeout
							if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
								if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
									lcd_Clear(BLACK);
									lcd_ShowStr(10, 100, "Wating seconds timeout", RED, BLACK, 16, 0);
									system_status = NORMAL_MODE;
								} else {
									remote_update_time_status = UPDATE_SECONDS;
									uart_timer_cnt = 0;
									timeout_cnt++;
								}
							} else {
								if (uart_receive_flag) {
									uart_receive_flag = 0;
									uint8_t second_received = 60;
									// Get the latest data
									while (uart_ring_buffer.length > 0) {
										ringBufferPop(&uart_ring_buffer, &second_received);
									}

									// Check hour format
									if (second_received < 60) {
										system_status = NORMAL_MODE;
										// TODO: update time to RTC
										second = second_received;
										lcd_ShowStr(10, 100, "Update time successfully", GREEN, BLACK, 16, 0);
									} else {
										remote_update_time_status = UPDATE_SECONDS;
										timeout_cnt = 0;
										lcd_ShowStr(10, 100, "Wating format of seconds, try again", RED, BLACK, 16, 0);
									}
								} else {
									uart_timer_cnt++;
								}
							}
							break;
						default:
							break;
					}
			}
		}
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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void system_init() {
	HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);
	timer_init();
	led7_init();
	button_init();
	lcd_init();
	uart_init_rs232();
	setTimer2(50);
}

uint16_t count_led_debug = 0;

void test_LedDebug() {
	count_led_debug = (count_led_debug + 1) % 20;
	if (count_led_debug == 0) {
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

void test_button() {
	for (int i = 0; i < 16; i++) {
		if (button_count[i] == 1) {
			led7_SetDigit(i / 10, 2, 0);
			led7_SetDigit(i % 10, 3, 0);
		}
	}
}

void test_Uart() {
	if (button_count[12] == 1) {
		uart_Rs232SendNum(ds3231_hours);
		uart_Rs232SendString(":");
		uart_Rs232SendNum(ds3231_min);
		uart_Rs232SendString(":");
		uart_Rs232SendNum(ds3231_sec);
		uart_Rs232SendString("\n");
	}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
