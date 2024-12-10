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
uint8_t count_led_debug = 0;
uint8_t count_modify = 0;
uint8_t count_timer = 0;
uint8_t temp = 0;
uint8_t temp_timer = 0;
uint8_t flag = 0;
enum Mode {
	modeInit,
	modeNormal,
	modeModify,
	modeTimer,
};
enum Mode currentMode = modeInit;
uint8_t temp_time[7];
uint8_t timer[7];
uint8_t timerPresent[7];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void displayTime();
void updateTime();
void fsm();
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
  MX_TIM2_Init();
  MX_SPI1_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 lcd_Clear(BLACK);
 updateTime();
  while (1)
  {
	  while(!flag_timer2);
	  flag_timer2 = 0;
	  button_Scan();
	  ds3231_ReadTime();
//	  displayTime();
	  fsm();
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void system_init(){
	  HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	  HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	  HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);
	  timer_init();
	  led7_init();
	  button_init();
	  lcd_init();
	  ds3231_init();
	  setTimer2(50);
}

void test_LedDebug(){
	count_led_debug = (count_led_debug + 1)%20;
	if(count_led_debug == 0){
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

void test_7seg(){
	led7_SetDigit(0, 0, 0);
	led7_SetDigit(5, 1, 0);
	led7_SetDigit(4, 2, 0);
	led7_SetDigit(7, 3, 0);
}
void test_button(){
	for(int i = 0; i < 16; i++){
		if(button_count[i] == 1){
			led7_SetDigit(i/10, 2, 0);
			led7_SetDigit(i%10, 3, 0);
		}
	}
}

void updateTime(){
	ds3231_Write(ADDRESS_YEAR, 24);
	ds3231_Write(ADDRESS_MONTH, 5);
	ds3231_Write(ADDRESS_DATE, 30);
	ds3231_Write(ADDRESS_DAY, 5);
	ds3231_Write(ADDRESS_HOUR, 9);
	ds3231_Write(ADDRESS_MIN, 47);
	ds3231_Write(ADDRESS_SEC, 50);
}

uint8_t isButtonUp()
{
    if (button_count[3] == 1)
        return 1;
    else
        return 0;
}
uint8_t isButtonDown()
{
    if (button_count[7] == 1)
        return 1;
    else
        return 0;
}
char* returnMode() {
	switch (currentMode) {
		case modeInit:
			return "Initial";
			break;
		case modeNormal:
			return "Time Now";
			break;
		case modeModify:
			return "Modify Time";
			break;
		case modeTimer:
			return "Alarm";
			break;
	}
	return "Unknown!";
}

void checkTime() {
	switch (temp) {
		case 0:
			if (temp_time[temp] > 24) {
				temp_time[temp] = 0;
			}
			break;
		case 1:
			if (temp_time[temp] > 59) {
				temp_time[temp] = 0;
			}
			break;
		case 2:
			if (temp_time[temp] > 59) {
				temp_time[temp] = 0;
			}
			break;
		case 3:
			if (temp_time[temp] > 8) {
				temp_time[temp] = 2;
			}
			break;
		case 4:
			if (temp_time[temp] > 31) {
				temp_time[temp] = 0;
			}
			break;
		case 5:
			if (temp_time[temp] > 12) {
				temp_time[temp] = 0;
			}
			break;
		case 6:
			if (temp_time[temp] > 99) {
				temp_time[temp] = 0;
			}
			break;
	}
}

void checkTimer() {
	switch (temp_timer) {
		case 0:
			if (timer[temp_timer] > 24) {
				timer[temp_timer] = 0;
			}
			break;
		case 1:
			if (timer[temp_timer] > 59) {
				timer[temp_timer] = 0;
			}
			break;
		case 2:
			if (timer[temp_timer] > 59) {
				timer[temp_timer] = 0;
			}
			break;
		case 3:
			if (timer[temp_timer] > 8) {
				timer[temp_timer] = 2;
			}
			break;
		case 4:
			if (timer[temp_timer] > 31) {
				timer[temp_timer] = 0;
			}
			break;
		case 5:
			if (timer[temp_timer] > 12) {
				timer[temp_timer] = 0;
			}
			break;
		case 6:
			if (timer[temp_timer] > 99) {
				timer[temp_timer] = 0;
			}
			break;
	}
}

void displayTime(){
	lcd_StrCenter(0, 50, returnMode(), WHITE, BLACK, 24, 0);
	lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
}

void modifyTime() {
	count_modify = (count_modify + 1) % 10;
	if (count_modify > 5) {
		switch (temp){
			case 0:
				lcd_ShowIntNum(70, 100, "  ", 2, BLACK, BLACK, 24);
				break;
			case 1:
				lcd_ShowIntNum(110, 100, "  ", 2, BLACK, BLACK, 24);
				break;
			case 2:
				lcd_ShowIntNum(150, 100, "  ", 2, BLACK, BLACK, 24);
				break;
			case 3:
				lcd_ShowIntNum(20, 130, "  ", 2, BLACK, BLACK, 24);
				break;
			case 4:
				lcd_ShowIntNum(70, 130, "  ", 2, BLACK, BLACK, 24);
				break;
			case 5:
				lcd_ShowIntNum(110, 130, "  ", 2, BLACK, BLACK, 24);
				break;
			case 6:
				lcd_ShowIntNum(150, 130, "  ", 2, BLACK, BLACK, 24);
				break;
		}
	} else {
		lcd_StrCenter(0, 50, returnMode(), WHITE, BLACK, 24, 0);
		lcd_ShowIntNum(70, 100, temp_time[0], 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(110, 100, temp_time[1], 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(150, 100, temp_time[2], 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(20, 130, temp_time[3], 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(70, 130, temp_time[4], 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(110, 130, temp_time[5], 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(150, 130, temp_time[6], 2, YELLOW, BLACK, 24);
	}

	if (button_count[3] == 1) {
		temp_time[temp]++;
		checkTime();
	}

	if (button_count[3] > 40 && button_count[3] % 4 == 1) {
		temp_time[temp]++;
		checkTime();
	}

	if (button_count[12] == 1) {
		switch (temp){
			case 0:
				ds3231_Write(ADDRESS_HOUR, temp_time[temp]);
				break;
			case 1:
				ds3231_Write(ADDRESS_MIN, temp_time[temp]);
				break;
			case 2:
				ds3231_Write(ADDRESS_SEC, temp_time[temp]);
				break;
			case 3:
				ds3231_Write(ADDRESS_DAY, temp_time[temp]);
				break;
			case 4:
				ds3231_Write(ADDRESS_DATE, temp_time[temp]);
				break;
			case 5:
				ds3231_Write(ADDRESS_MONTH, temp_time[temp]);
				break;
			case 6:
				ds3231_Write(ADDRESS_YEAR, temp_time[temp]);
				break;
		}
		temp++;
		if (temp > 6) {
			temp = 0;
		}
	}
}

void modifyTimer() {
	count_timer = (count_timer + 1) % 10;
	if (count_timer > 5) {
		switch (temp_timer){
			case 0:
				lcd_ShowIntNum(70, 100, "  ", 2, BLACK, BLACK, 24);
				break;
			case 1:
				lcd_ShowIntNum(110, 100, "  ", 2, BLACK, BLACK, 24);
				break;
			case 2:
				lcd_ShowIntNum(150, 100, "  ", 2, BLACK, BLACK, 24);
				break;
			case 3:
				lcd_ShowIntNum(20, 130, "  ", 2, BLACK, BLACK, 24);
				break;
			case 4:
				lcd_ShowIntNum(70, 130, "  ", 2, BLACK, BLACK, 24);
				break;
			case 5:
				lcd_ShowIntNum(110, 130, "  ", 2, BLACK, BLACK, 24);
				break;
			case 6:
				lcd_ShowIntNum(150, 130, "  ", 2, BLACK, BLACK, 24);
				break;
		}
	} else {
		lcd_StrCenter(0, 50, returnMode(), WHITE, BLACK, 24, 0);
		lcd_ShowIntNum(70, 100, timer[0], 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(110, 100, timer[1], 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(150, 100, timer[2], 2, GREEN, BLACK, 24);
		lcd_ShowIntNum(20, 130, timer[3], 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(70, 130, timer[4], 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(110, 130,timer[5], 2, YELLOW, BLACK, 24);
		lcd_ShowIntNum(150, 130, timer[6], 2, YELLOW, BLACK, 24);
	}

	if (button_count[3] == 1) {
		timer[temp_timer]++;
		checkTimer();
	}

	if (button_count[3] > 40 && button_count[3] % 4 == 1) {
		timer[temp_timer]++;
		checkTimer();
	}

	if (button_count[12] == 1) {
		switch (temp_timer){
			case 0:
				timerPresent[temp_timer] = timer[temp_timer];
				break;
			case 1:
				timerPresent[temp_timer] = timer[temp_timer];
				break;
			case 2:
				timerPresent[temp_timer] = timer[temp_timer];
				break;
			case 3:
				timerPresent[temp_timer] = timer[temp_timer];
				break;
			case 4:
				timerPresent[temp_timer] = timer[temp_timer];
				break;
			case 5:
				timerPresent[temp_timer] = timer[temp_timer];
				break;
			case 6:
				timerPresent[temp_timer] = timer[temp_timer];
				break;
		}
		temp_timer++;
		if (temp_timer> 6) {
			temp_timer = 0;
		}
	}
}

void fsm() {
	switch (currentMode) {
		case modeInit:
			currentMode = modeNormal;
			break;
		case modeNormal:
			temp = 0;
			temp_timer = 0;
			for (int i = 0; i < 7; i++) {
				temp_time[i] = 0;
				timer[i] = 0;
			}
			displayTime();

			if (ds3231_hours == timerPresent[0] && ds3231_min == timerPresent[1]) {
				lcd_Fill(0, 200, 240, 300, RED);
				lcd_StrCenter(0, 240, "Wake up!", YELLOW, RED, 16, 0);
			}
			else {
				lcd_Fill(0, 200, 240, 300, BLACK);
				lcd_StrCenter(0, 240, "Wake up!", BLACK, BLACK, 16, 0);
			}

			if (button_count[0] == 1) {
				lcd_Fill(0, 50, 240, 75, BLACK);
				currentMode = modeModify;
			}
			break;
		case modeModify:
			for (int i = 0; i < 7; i++) {
				if (temp_time[i] == 0 && flag == 0) {
					switch (i) {
						case 0:
							temp_time[i] = ds3231_hours;
							break;
						case 1:
							temp_time[i] = ds3231_min;
							break;
						case 2:
							temp_time[i] = ds3231_sec;
							break;
						case 3:
							temp_time[i] = ds3231_day;
							break;
						case 4:
							temp_time[i] = ds3231_date;
							break;
						case 5:
							temp_time[i] = ds3231_month;
							break;
						case 6:
							temp_time[i] = ds3231_year;
							break;
					}
				}
			}
			flag = 1;
			modifyTime();
			if (button_count[0] == 1) {
				flag = 0;
				lcd_Fill(0, 50, 240, 75, BLACK);
				currentMode = modeTimer;
			}
			break;
		case modeTimer:
			for (int i = 0; i < 7; i++) {
				if (timer[i] == 0 && flag == 0) {
					switch (i) {
						case 0:
							timer[i] = timerPresent[i];
							break;
						case 1:
							timer[i] = timerPresent[i];
							break;
						case 2:
							timer[i] = timerPresent[i];
							break;
						case 3:
							timer[i] = timerPresent[i];
							break;
						case 4:
							timer[i] = timerPresent[i];
							break;
						case 5:
							timer[i] = timerPresent[i];
							break;
						case 6:
							timer[i] = timerPresent[i];
							break;
					}
				}
			}
			flag = 1;
			modifyTimer();
			if (button_count[0] == 1) {
				flag = 0;
				lcd_Fill(0, 50, 240, 75, BLACK);
				currentMode = modeNormal;
			}
			break;
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
