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
#include "sTimer.h"
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

// Button Macros
#define BTN_0_INDEX 		13

// Remote update time modes
#define UPDATE_HOURS		0	
#define WAIT_FOR_HOURS		1
#define UPDATE_MINUTES		2
#define WAIT_FOR_MINUTES	3
#define UPDATE_SECONDS		4
#define UPDATE_DAYS			5
#define UPDATE_DATES		6
#define UPDATE_MONTHS		7
#define UPDATE_YEARS		8
#define WAIT_FOR_SECONDS	9
#define WAIT_FOR_DAYS		10
#define WAIT_FOR_DATES		11
#define WAIT_FOR_MONTHS		12
#define WAIT_FOR_YEARS		13

#define UART_TIMEOUT_CNT		200 // 10s, because software timer 2 cycle is 50ms
#define UART_TIMEOUT_MAX_TRIES	2

#define PI 3.14159265358979323846
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t remote_update_time_status;
uint8_t timeout_cnt = 0;
uint8_t uart_timer_cnt = 0; // Used for detecting timeout in UART communication
uint8_t uart_hour = 0;
uint8_t uart_minute = 0;
uint8_t uart_second = 0;
uint8_t uart_date = 0;
uint8_t uart_day = 0;
uint8_t uart_month = 0;
uint16_t uart_year = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void displayClock(int x_coor, int y_coor, int radius);
void displayLocation(int y_coor, char *str, char *GMT_str, uint8_t char_size);

float displaySecClockwise(int x_coor, int y_coor, int length,
		const uint8_t *second, uint16_t color);
float displayMinClockwise(int x_coor, int y_coor, int length,
		const uint8_t *second, const uint8_t *minute, uint16_t color);
float displayHourClockwise(int x_coor, int y_coor, int length,
		const uint8_t *minute, const uint8_t *hour, uint16_t color);

void displayTime(int x_coor, int y_coor, const uint8_t *second,
		const uint8_t *minute, const uint8_t *hour, uint8_t char_size,
		uint16_t color_sec, uint16_t color_min, uint16_t color_hour);
void displayDate(int x_coor, int y_coor, const uint8_t *date,
		const uint8_t *month, const uint16_t *year, uint8_t char_size,
		uint16_t color_date, uint16_t color_month, uint16_t color_year);
void displayTemp(int x_coor, int y_coor, float temperature, uint8_t char_size,
		uint16_t color_temp);
void displayDay(int x_coor, int y_coor, const uint8_t *day, uint8_t char_size,
		uint16_t color_day);

void displayTimeLed7Seg(const uint8_t *second, const uint8_t *minute,
		const uint8_t *hour);
void setTime(uint8_t *second, uint8_t *minute, uint8_t *hour, uint8_t *day,
		uint8_t *date, uint8_t *month, uint16_t *year);
void setAlarm1(uint8_t second, uint8_t minute, uint8_t hour, uint8_t day,
		uint8_t date);
void setAlarm2(uint8_t minute, uint8_t hour, uint8_t day, uint8_t date);

void increaseSec(void);
void decreaseSec(void);
void increaseMin(void);
void decreaseMin(void);
void increaseHour(void);
void decreaseHour(void);
void increaseDay(void);
void decreaseDay(void);
void increaseDate(void);
void decreaseDate(void);
void increaseMonth(void);
void decreaseMonth(void);
void increaseYear(void);
void decreaseYear(void);
int getDaysInMonth(int month, int year);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
enum State {
	Mode_init,
	Mode_word_clock,
	Mode_login,
	Mode_config_time,
	Mode_config_alarm,
	Mode_stopwatch,
	Mode_timers,
	Mode_monitor_register,
	Mode_remote_config_time
};
enum State current_mode = Mode_init;
enum State previous_mode = Mode_init;

enum State_config {
	Mode_config_second,
	Mode_config_minute,
	Mode_config_hour,
	Mode_config_day,
	Mode_config_date,
	Mode_config_month,
	Mode_config_year
};
enum State_config current_mode_config = Mode_config_second;
enum State_config previous_mode_config = Mode_config_minute;
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
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */
	system_init();
	/* USER CODE END 2 */

	sTimer4Set(1000, 50); // interrupt every 50ms
	sTimer2Set(0, 500); // interrupt every 500ms

	int clock_radius = 100;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		if (sTimer4GetFlag()) {
			button_Scan();
			uart_timer_cnt++;
		}

		switch (current_mode) {
		case Mode_init: {
			led7_SetColon(1);
			setTime(&set_time.second, &set_time.minute, &set_time.hour,
					&set_time.day, &set_time.date, &set_time.month,
					&set_time.year);
			ds3231ReadTime();
			current_time.alarm_on = false;
			current_mode = Mode_config_time;
			break;
		}
		case Mode_word_clock: {
			if (previous_mode != current_mode) {
				lcdClear(WHITE);
				clock_radius = 100;
				displayClock(LCD_WIDTH / 2, 110, clock_radius);

				(void) displaySecClockwise(LCD_WIDTH / 2, 110,
						clock_radius - 30, &current_time.second, BLUE);
				(void) displayMinClockwise(LCD_WIDTH / 2, 110,
						clock_radius - 40, &current_time.second,
						&current_time.minute, BLACK);
				(void) displayHourClockwise(LCD_WIDTH / 2, 110,
						clock_radius - 50, &current_time.minute,
						&current_time.hour, RED);

				displayTime(LCD_WIDTH / 2, 240, &current_time.second,
						&current_time.minute, &current_time.hour, 32, BLACK,
						BLACK, BLACK);
				displayDate(LCD_WIDTH / 2, 240 + 32, &current_time.date,
						&current_time.month, &current_time.year, 24, DARKBLUE,
						DARKBLUE, DARKBLUE);
				displayDay(20, 320 - 10 - 24, &current_time.day, 24, RED);

				previous_mode = current_mode;
			}

			if (sTimer2GetFlag()) {
//				debugSystem();
				test_LedDebug();

				ds3231ReadTime();

				(void) displaySecClockwise(LCD_WIDTH / 2, 110,
						clock_radius - 30, &current_time.second, BLUE);
				(void) displayMinClockwise(LCD_WIDTH / 2, 110,
						clock_radius - 40, &current_time.second,
						&current_time.minute, BLACK);
				(void) displayHourClockwise(LCD_WIDTH / 2, 110,
						clock_radius - 50, &current_time.minute,
						&current_time.hour, RED);

				displayTime(LCD_WIDTH / 2, 240, &current_time.second,
						&current_time.minute, &current_time.hour, 32, BLACK,
						BLACK, BLACK);
				if (current_time.second == 0 && current_time.minute == 0
						&& current_time.hour == 0) {
					displayDate(LCD_WIDTH / 2, 240 + 32, &current_time.date,
							&current_time.month, &current_time.year, 24,
							DARKBLUE, DARKBLUE, DARKBLUE);
					displayDay(20, 320 - 34, &current_time.day, 24, RED);
				}

				displayTimeLed7Seg(&current_time.second, &current_time.minute,
						&current_time.hour);
			}

			if (button_count[12] == 1) {
				current_mode = Mode_config_time;
				button_count[12] += 1;
			}

			break;
		}
		case Mode_config_time: {
			if (previous_mode != current_mode) {
				ds3231ReadTime();
				set_time.second = current_time.second;
				set_time.minute = current_time.minute;
				set_time.hour = current_time.hour;
				set_time.day = current_time.day;
				set_time.date = current_time.date;
				set_time.month = current_time.month;
				set_time.year = current_time.year;

				lcdClear(WHITE);
				clock_radius = 100;
				displayClock(LCD_WIDTH / 2, 110, clock_radius);

				current_mode_config = Mode_config_second;
				previous_mode_config = Mode_config_minute;

				previous_mode = current_mode;
			}

			switch (current_mode_config) {
			case Mode_config_second: {
				if (previous_mode_config != current_mode_config) {
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLACK);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							BLUE);
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, RED, BLACK,
							BLACK);
					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE,
							DARKBLUE, DARKBLUE);
					displayDay(20, 320 - 10 - 24, &set_time.day, 24, DARKBLUE);

					displayTimeLed7Seg(&set_time.second, &set_time.minute,
							&set_time.hour);

					previous_mode_config = current_mode_config;
				}

				if (sTimer2GetFlag()) {
					static int counter = 0;
					counter += 1;

					if (counter % 4 == 0) {
//						led7SegDebugTurnOff(6);
//						led7SegDebugTurnOff(7);
//						led7SegDebugTurnOff(8);
					} else if (counter % 2 == 0) {
						displayTimeLed7Seg(&set_time.second, &set_time.minute,
								&set_time.hour);
					}
				}

				if (button_count[11] % 30 == 1) // check button is held 1.5 second
						{
					current_mode_config = Mode_config_minute;
					button_count[11] += 1;
				} else if (button_count[3] % 20 == 1) {
					increaseSec();

					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLACK);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							BLUE);
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, RED, BLACK,
							BLACK);

					button_count[3] += 1;
				} else if (button_count[7] % 20 == 1) {
					decreaseSec();

					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLACK);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							BLUE);
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, RED, BLACK,
							BLACK);

					button_count[7] += 1;
				}

				break;
			}
			case Mode_config_minute: {
				if (previous_mode_config != current_mode_config) {
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							BLUE);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, RED,
							BLACK);
					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE,
							DARKBLUE, DARKBLUE);
					displayDay(20, 320 - 10 - 24, &set_time.day, 24, DARKBLUE);

					displayTimeLed7Seg(&set_time.second, &set_time.minute,
							&set_time.hour);

					previous_mode_config = current_mode_config;
				}

				if (sTimer2GetFlag()) {
					static int counter = 0;
					counter += 1;

					if (counter % 4 == 0) {
//						led7SegTurnOff(2);
//						led7SegTurnOff(3);
					} else if (counter % 2 == 0) {
						displayTimeLed7Seg(&set_time.second, &set_time.minute,
								&set_time.hour);
					}
				}

				if (button_count[11] % 30 == 1) // check button is held 1.5 second
						{
					current_mode_config = Mode_config_hour;
					button_count[11] += 1;
				} else if (button_count[15] % 30 == 1) {
					current_mode_config = Mode_config_second;
					button_count[15] += 1;
				} else if (button_count[3] % 20 == 1) {
					increaseMin();

					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							BLUE);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, RED,
							BLACK);

					button_count[3] += 1;
				} else if (button_count[7] % 20 == 1) {
					decreaseMin();

					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							BLUE);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, RED,
							BLACK);

					button_count[7] += 1;
				}

				break;
			}
			case Mode_config_hour: {
				if (previous_mode_config != current_mode_config) {
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLUE);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, BLACK,
							RED);
					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE,
							DARKBLUE, DARKBLUE);
					displayDay(20, 320 - 10 - 24, &set_time.day, 24, DARKBLUE);

					displayTimeLed7Seg(&set_time.second, &set_time.minute,
							&set_time.hour);

					previous_mode_config = current_mode_config;
				}

				if (sTimer2GetFlag()) {
					static int counter = 0;
					counter += 1;

					if (counter % 4 == 0) {
//						led7SegTurnOff(0);
//						led7SegTurnOff(1);
					} else if (counter % 2 == 0) {
						displayTimeLed7Seg(&set_time.second, &set_time.minute,
								&set_time.hour);
					}
				}

				if (button_count[11] % 30 == 1) // check button is held 1.5 second
						{
					current_mode_config = Mode_config_day;
					button_count[11] += 1;
				} else if (button_count[15] % 30 == 1) {
					current_mode_config = Mode_config_minute;
					button_count[15] += 1;
				} else if (button_count[3] % 20 == 1) // check button is held 1 second
						{
					increaseHour();

					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLUE);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, BLACK,
							RED);

					button_count[3] += 1;
				} else if (button_count[7] % 20 == 1) {
					decreaseHour();

					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLUE);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, BLACK,
							RED);

					button_count[7] += 1;
				}

				break;
			}
			case Mode_config_day: {
				if (previous_mode_config != current_mode_config) {
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLUE);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, BLACK,
							BLACK);
					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE,
							DARKBLUE, DARKBLUE);
					displayDay(20, 320 - 10 - 24, &set_time.day, 24, RED);

					displayTimeLed7Seg(&set_time.second, &set_time.minute,
							&set_time.hour);

					previous_mode_config = current_mode_config;
				}

				if (button_count[11] % 30 == 1) // check button is held 1.5 second
						{
					current_mode_config = Mode_config_date;
					button_count[11] += 1;
				} else if (button_count[15] % 30 == 1) {
					current_mode_config = Mode_config_hour;
					button_count[15] += 1;
				} else if (button_count[3] % 20 == 1) // check button is held 1 second
						{
					increaseDay();

					displayDay(20, 320 - 10 - 24, &set_time.day, 24, RED);

					button_count[3] += 1;
				} else if (button_count[7] % 20 == 1) {
					decreaseDay();

					displayDay(20, 320 - 10 - 24, &set_time.day, 24, RED);

					button_count[7] += 1;
				}

				break;
			}
			case Mode_config_date: {
				if (previous_mode_config != current_mode_config) {
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLUE);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, BLACK,
							BLACK);
					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, RED, DARKBLUE,
							DARKBLUE);
					displayDay(20, 320 - 10 - 24, &set_time.day, 24, DARKBLUE);

					displayTimeLed7Seg(&set_time.second, &set_time.minute,
							&set_time.hour);

					previous_mode_config = current_mode_config;
				}

				if (button_count[11] % 30 == 1) // check button is held 1.5 second
						{
					current_mode_config = Mode_config_month;
					button_count[11] += 1;
				} else if (button_count[15] % 30 == 1) {
					current_mode_config = Mode_config_day;
					button_count[15] += 1;
				} else if (button_count[3] % 20 == 1) // check button is held 1 second
						{
					increaseDate();

					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, RED, DARKBLUE,
							DARKBLUE);

					button_count[3] += 1;
				} else if (button_count[7] % 20 == 1) {
					decreaseDate();

					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, RED, DARKBLUE,
							DARKBLUE);

					button_count[7] += 1;
				}

				break;
			}
			case Mode_config_month: {
				if (previous_mode_config != current_mode_config) {
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLUE);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, BLACK,
							BLACK);
					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE, RED,
							DARKBLUE);
					displayDay(20, 320 - 10 - 24, &set_time.day, 24, DARKBLUE);

					displayTimeLed7Seg(&set_time.second, &set_time.minute,
							&set_time.hour);

					previous_mode_config = current_mode_config;
				}

				if (button_count[11] % 30 == 1) // check button is held 1.5 second
						{
					current_mode_config = Mode_config_year;
					button_count[11] += 1;
				} else if (button_count[15] % 30 == 1) {
					current_mode_config = Mode_config_date;
					button_count[15] += 1;
				} else if (button_count[3] % 20 == 1) // check button is held 1 second
						{
					increaseMonth();

					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE, RED,
							DARKBLUE);

					button_count[3] += 1;
				} else if (button_count[7] % 20 == 1) {
					decreaseMonth();

					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE, RED,
							DARKBLUE);

					button_count[7] += 1;
				}

				break;
			}
			case Mode_config_year: {
				if (previous_mode_config != current_mode_config) {
					(void) displaySecClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 30, &set_time.second, BLACK);
					(void) displayMinClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 40, &set_time.second,
							&set_time.minute, BLUE);
					(void) displayHourClockwise(LCD_WIDTH / 2, 110,
							clock_radius - 50, &set_time.minute, &set_time.hour,
							RED);

					displayTime(LCD_WIDTH / 2, 240, &set_time.second,
							&set_time.minute, &set_time.hour, 32, BLACK, BLACK,
							BLACK);
					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE,
							DARKBLUE, RED);
					displayDay(20, 320 - 10 - 24, &set_time.day, 24, DARKBLUE);

					displayTimeLed7Seg(&set_time.second, &set_time.minute,
							&set_time.hour);

					previous_mode_config = current_mode_config;
				}

				if (button_count[15] % 30 == 1) {
					current_mode_config = Mode_config_month;
					button_count[15] += 1;
				} else if (button_count[3] % 20 == 1) // check button is held 1 second
						{
					increaseYear();

					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE,
							DARKBLUE, RED);

					button_count[3] += 1;
				} else if (button_count[7] % 20 == 1) {
					decreaseYear();

					displayDate(LCD_WIDTH / 2, 240 + 32, &set_time.date,
							&set_time.month, &set_time.year, 24, DARKBLUE,
							DARKBLUE, RED);

					button_count[7] += 1;
				}

				break;
			}
			}

			if (button_count[12] == 1) {
				setTime(&set_time.second, &set_time.minute, &set_time.hour,
						&set_time.day, &set_time.date, &set_time.month,
						&set_time.year);
				ds3231ReadTime();
				current_mode = Mode_remote_config_time;
				lcdClear(BLACK);
				remote_update_time_status = UPDATE_HOURS;
				button_count[12] += 1;
			} else if (button_count[14] == 1) {
				current_mode = Mode_word_clock;
				button_count[14] += 1;
			}
			break;
		}
		case Mode_config_alarm: {
			break;
		}
		case Mode_stopwatch: {
			break;
		}
		case Mode_timers: {
			break;
		}
		case Mode_monitor_register: {
			break;
		}
		case Mode_remote_config_time: {
			if (button_count[14] == 1) {
				current_mode = Mode_word_clock;
				button_count[14] += 1;
				break;
			}
			switch (remote_update_time_status) {
			case UPDATE_HOURS:
				uart_Rs232SendString("hours");
				lcdShowString(10, 20, "Updating hours...", GREEN, BLACK, 16, 0);
				remote_update_time_status = WAIT_FOR_HOURS;
				uart_timer_cnt = 0;
				break;
			case WAIT_FOR_HOURS:
				// Check for timeout
				if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
					if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
						lcdClear(BLACK);
						lcdShowString(10, 100, "Wating hours timeout", RED,
						BLACK, 16, 0);
						current_mode = Mode_word_clock;
					} else {
						remote_update_time_status = UPDATE_HOURS;
						uart_timer_cnt = 0;
						timeout_cnt++;
					}
				} else {
					if (uart_receive_flag) {
						uart_receive_flag = 0;
						uint8_t hour_received = 0;
						uint8_t valid_format = 1;
						// Get the latest data
						while (uart_ring_buffer.length > 0) {
							uint8_t received_char = 0;
							ringBufferPop(&uart_ring_buffer, &received_char);
							if (received_char > '9' || received_char < '0') {
								valid_format = 0;
							} else {
								hour_received = hour_received * 10
										+ (received_char - '0');
							}
						}

						// Check hour format
						if (hour_received < 24 && valid_format) {
							lcdClear(BLACK);
							remote_update_time_status = UPDATE_MINUTES;
							uart_hour = hour_received;
							timeout_cnt = 0;
						} else {
							remote_update_time_status = UPDATE_HOURS;
							timeout_cnt = 0;
							lcdShowString(10, 100, "Wrong format of hours",
							RED, BLACK, 16, 0);
							lcdShowString(10, 120, "Try again", RED, BLACK, 16,
									0);
						}
					}
				}
				break;
			case UPDATE_MINUTES:
				uart_Rs232SendString("minutes");
				lcdShowString(10, 20, "Updating minutes...", GREEN, BLACK, 16,
						0);
				remote_update_time_status = WAIT_FOR_MINUTES;
				uart_timer_cnt = 0;
				break;
			case WAIT_FOR_MINUTES:
				// Check for timeout
				if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
					if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
						lcdClear(BLACK);
						lcdShowString(10, 100, "Wating minutes timeout", RED,
						BLACK, 16, 0);
						current_mode = Mode_word_clock;
					} else {
						remote_update_time_status = UPDATE_MINUTES;
						uart_timer_cnt = 0;
						timeout_cnt++;
					}
				} else {
					if (uart_receive_flag) {
						uart_receive_flag = 0;
						uint8_t minute_received = 0;
						uint8_t valid_format = 1;
						// Get the latest data
						while (uart_ring_buffer.length > 0) {
							uint8_t received_char = 0;
							ringBufferPop(&uart_ring_buffer, &received_char);
							if (received_char > '9' || received_char < '0') {
								valid_format = 0;
							} else {
								minute_received = minute_received * 10
										+ (received_char - '0');
							}
						}

						// Check minute format
						if (minute_received < 60 && valid_format) {
							lcdClear(BLACK);
							remote_update_time_status = UPDATE_SECONDS;
							uart_minute = minute_received;
							timeout_cnt = 0;
						} else {
							remote_update_time_status = UPDATE_MINUTES;
							timeout_cnt = 0;
							lcdShowString(10, 100, "Wrong format of minutes",
							RED, BLACK, 16, 0);
							lcdShowString(10, 120, "Try again", RED, BLACK, 16,
									0);
						}
					}
				}
				break;
			case UPDATE_SECONDS:
				uart_Rs232SendString("seconds");
				lcdShowString(10, 20, "Updating seconds...", GREEN, BLACK, 16,
						0);
				remote_update_time_status = WAIT_FOR_SECONDS;
				uart_timer_cnt = 0;
				break;
			case WAIT_FOR_SECONDS:
				// Check for timeout
				if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
					if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
						lcdClear(BLACK);
						lcdShowString(10, 100, "Wating seconds timeout", RED,
						BLACK, 16, 0);
						current_mode = Mode_word_clock;
					} else {
						remote_update_time_status = UPDATE_SECONDS;
						uart_timer_cnt = 0;
						timeout_cnt++;
					}
				} else {
					if (uart_receive_flag) {
						uart_receive_flag = 0;
						uint8_t second_received = 0;
						uint8_t valid_format = 1;
						// Get the latest data
						while (uart_ring_buffer.length > 0) {
							uint8_t received_char = 0;
							ringBufferPop(&uart_ring_buffer, &received_char);
							if (received_char > '9' || received_char < '0') {
								valid_format = 0;
							} else {
								second_received = second_received * 10
										+ (received_char - '0');
							}
						}

						// Check second format
						if (second_received < 60 && valid_format) {
							remote_update_time_status = UPDATE_DAYS;
							lcdClear(BLACK);
							uart_second = second_received;
							timeout_cnt = 0;
						} else {
							remote_update_time_status = UPDATE_SECONDS;
							timeout_cnt = 0;
							lcdShowString(10, 100, "Wrong format of seconds",
							RED, BLACK, 16, 0);
							lcdShowString(10, 120, "Try again", RED, BLACK, 16,
									0);
						}
					}
				}
				break;
			case UPDATE_DAYS:
				uart_Rs232SendString("days");
				lcdShowString(10, 20, "Updating days...", GREEN, BLACK, 16,
						0);
				remote_update_time_status = WAIT_FOR_DAYS;
				uart_timer_cnt = 0;
				break;
			case WAIT_FOR_DAYS:
				// Check for timeout
				if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
					if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
						lcdClear(BLACK);
						lcdShowString(10, 100, "Wating days timeout", RED,
						BLACK, 16, 0);
						current_mode = Mode_word_clock;
					} else {
						remote_update_time_status = UPDATE_DAYS;
						uart_timer_cnt = 0;
						timeout_cnt++;
					}
				} else {
					if (uart_receive_flag) {
						uart_receive_flag = 0;
						uint16_t day_received = 0;
						uint8_t valid_format = 1;
						// Get the latest data
						while (uart_ring_buffer.length > 0) {
							uint8_t received_char = 0;
							ringBufferPop(&uart_ring_buffer, &received_char);
							if (received_char > '9' || received_char < '0') {
								valid_format = 0;
							} else {
								day_received = day_received * 10
										+ (received_char - '0');
							}
						}

						// Check second format
						if (day_received < 8 && day_received > 0 && valid_format) {
							lcdClear(BLACK);
							remote_update_time_status = UPDATE_YEARS;
							uart_day = day_received;
							timeout_cnt = 0;
						} else {
							remote_update_time_status = UPDATE_DAYS;
							timeout_cnt = 0;
							lcdShowString(10, 100, "Wrong format of days",
							RED, BLACK, 16, 0);
							lcdShowString(10, 120, "Try again", RED, BLACK, 16,
									0);
						}
					}
				}
				break;
			case UPDATE_YEARS:
				uart_Rs232SendString("years");
				lcdShowString(10, 20, "Updating years...", GREEN, BLACK, 16,
						0);
				remote_update_time_status = WAIT_FOR_YEARS;
				uart_timer_cnt = 0;
				break;
			case WAIT_FOR_YEARS:
				// Check for timeout
				if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
					if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
						lcdClear(BLACK);
						lcdShowString(10, 100, "Wating years timeout", RED,
						BLACK, 16, 0);
						current_mode = Mode_word_clock;
					} else {
						remote_update_time_status = UPDATE_YEARS;
						uart_timer_cnt = 0;
						timeout_cnt++;
					}
				} else {
					if (uart_receive_flag) {
						uart_receive_flag = 0;
						uint16_t year_received = 0;
						uint8_t valid_format = 1;
						// Get the latest data
						while (uart_ring_buffer.length > 0) {
							uint8_t received_char = 0;
							ringBufferPop(&uart_ring_buffer, &received_char);
							if (received_char > '9' || received_char < '0') {
								valid_format = 0;
							} else {
								year_received = year_received * 10
										+ (received_char - '0');
							}
						}

						// Check second format
						if (year_received < 10000 && valid_format) {
							lcdClear(BLACK);
							remote_update_time_status = UPDATE_MONTHS;
							uart_year = year_received;
							timeout_cnt = 0;
						} else {
							remote_update_time_status = UPDATE_YEARS;
							timeout_cnt = 0;
							lcdShowString(10, 100, "Wrong format of years",
							RED, BLACK, 16, 0);
							lcdShowString(10, 120, "Try again", RED, BLACK, 16,
									0);
						}
					}
				}
				break;
			case UPDATE_MONTHS:
				uart_Rs232SendString("months");
				lcdShowString(10, 20, "Updating months...", GREEN, BLACK, 16,
						0);
				remote_update_time_status = WAIT_FOR_MONTHS;
				uart_timer_cnt = 0;
				break;
			case WAIT_FOR_MONTHS:
				// Check for timeout
				if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
					if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
						lcdClear(BLACK);
						lcdShowString(10, 100, "Wating months timeout", RED,
						BLACK, 16, 0);
						current_mode = Mode_word_clock;
					} else {
						remote_update_time_status = UPDATE_MONTHS;
						uart_timer_cnt = 0;
						timeout_cnt++;
					}
				} else {
					if (uart_receive_flag) {
						uart_receive_flag = 0;
						uint8_t month_received = 0;
						uint8_t valid_format = 1;
						// Get the latest data
						while (uart_ring_buffer.length > 0) {
							uint8_t received_char = 0;
							ringBufferPop(&uart_ring_buffer, &received_char);
							if (received_char > '9' || received_char < '0') {
								valid_format = 0;
							} else {
								month_received = month_received * 10
										+ (received_char - '0');
							}
						}

						// Check second format
						if (month_received < 13 && valid_format) {
							lcdClear(BLACK);
							remote_update_time_status = UPDATE_DATES;
							uart_month = month_received;
							timeout_cnt = 0;
						} else {
							remote_update_time_status = UPDATE_MONTHS;
							timeout_cnt = 0;
							lcdShowString(10, 100, "Wrong format of months",
							RED, BLACK, 16, 0);
							lcdShowString(10, 120, "Try again", RED, BLACK, 16,
									0);
						}
					}
				}
				break;
			case UPDATE_DATES:
				uart_Rs232SendString("dates");
				lcdShowString(10, 20, "Updating dates...", GREEN, BLACK, 16,
						0);
				remote_update_time_status = WAIT_FOR_DATES;
				uart_timer_cnt = 0;
				break;
			case WAIT_FOR_DATES:
				// Check for timeout
				if (uart_timer_cnt >= UART_TIMEOUT_CNT) {
					if (timeout_cnt >= UART_TIMEOUT_MAX_TRIES) {
						lcdClear(BLACK);
						lcdShowString(10, 100, "Wating dates timeout", RED,
						BLACK, 16, 0);
						current_mode = Mode_word_clock;
					} else {
						remote_update_time_status = UPDATE_DATES;
						uart_timer_cnt = 0;
						timeout_cnt++;
					}
				} else {
					if (uart_receive_flag) {
						uart_receive_flag = 0;
						uint8_t date_received = 0;
						uint8_t valid_format = 1;
						// Get the latest data
						while (uart_ring_buffer.length > 0) {
							uint8_t received_char = 0;
							ringBufferPop(&uart_ring_buffer, &received_char);
							if (received_char > '9' || received_char < '0') {
								valid_format = 0;
							} else {
								date_received = date_received * 10
										+ (received_char - '0');
							}
						}

						// Check date format
						if (date_received <= getDaysInMonth(uart_month, uart_year) && valid_format) {
							current_mode = Mode_word_clock;
							uart_date = date_received;
							lcdClear(BLACK);
							lcdShowString(10, 100, "Update time successfully",
							GREEN, BLACK, 16, 0);
							set_time.second = uart_second;
							set_time.minute = uart_minute;
							set_time.hour = uart_hour;
							set_time.day = uart_day;
							set_time.year = uart_year;
							set_time.month = uart_month;
							set_time.date = uart_date;
							setTime(&set_time.second, &set_time.minute,
									&set_time.hour, &set_time.day,
									&set_time.date, &set_time.month,
									&set_time.year);
							ds3231ReadTime();
						} else {
							remote_update_time_status = UPDATE_DATES;
							timeout_cnt = 0;
							lcdShowString(10, 100, "Date not valid",
							RED, BLACK, 16, 0);
							lcdShowString(10, 120, "Try again", RED, BLACK, 16,
									0);
						}
					}
				}
				break;
			default:
				break;
			}
			break;
		}
		default: {
			current_mode = Mode_init;
		}
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
	initTimer2();
	initTimer4();
	led7_init();
	button_init();
	initLCD();
	initds3231();
	uart_init_rs232();
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
void setTime(uint8_t *second, uint8_t *minute, uint8_t *hour, uint8_t *day,
		uint8_t *date, uint8_t *month, uint16_t *year) {
	ds3231SetSec(*second);
	ds3231SetMin(*minute);
	ds3231SetHour(*hour);
	ds3231SetDay(*day);
	ds3231SetDate(*date);
	ds3231SetMonth(*month);
	ds3231SetYear(*year);
}

void displayClock(int x_coor, int y_coor, int radius) {
	const uint8_t char_size = 24;

	lcdDrawCircle(x_coor, y_coor, DARKBLUE, radius + 2, 1);
	lcdDrawCircle(x_coor, y_coor, WHITE, radius, 1);

	for (int i = 0; i < 12; i++) {
		float angle = (i * 30) * (PI / 180); // Convert angle to radians
		int x = x_coor + (radius - 15) * sin(angle);
		int y = y_coor + (radius - 15) * -cos(angle);

		lcdShowIntNumCenter(x, y, ((i == 0) ? 12 : i), 2, BLACK, WHITE,
				char_size, 1);
	}
}

/**
 * @brief this function draw new clockwise and erased old clockwise
 * @param x_coor, y_coor center of clock
 * @param length length of clockwise
 * @param *second, *minute a pointer to variable store time value
 * @param color color of clockwise
 * @return float angle of clockwise
 */
float displaySecClockwise(int x_coor, int y_coor, int length,
		const uint8_t *second, uint16_t color) {
	static float angle_sec;
	int x, y;

	x = x_coor + (length) * sin(angle_sec);
	y = y_coor + (length) * -cos(angle_sec);
	lcdDrawLine(x_coor, y_coor, x, y, WHITE);

	angle_sec = (*second * 6) * (PI / 180);
	x = x_coor + (length) * sin(angle_sec);
	y = y_coor + (length) * -cos(angle_sec);
	lcdDrawLine(x_coor, y_coor, x, y, color);

	return angle_sec;
}
float displayMinClockwise(int x_coor, int y_coor, int length,
		const uint8_t *second, const uint8_t *minute, uint16_t color) {
	static float angle_min;
	int x, y;

	x = x_coor + (length) * sin(angle_min);
	y = y_coor + (length) * -cos(angle_min);
	lcdDrawLine(x_coor, y_coor, x, y, WHITE);

	angle_min = ((*minute + *second / 60.0) * 6) * (PI / 180);
	x = x_coor + (length) * sin(angle_min);
	y = y_coor + (length) * -cos(angle_min);
	lcdDrawLine(x_coor, y_coor, x, y, color);

	return angle_min;
}
float displayHourClockwise(int x_coor, int y_coor, int length,
		const uint8_t *minute, const uint8_t *hour, uint16_t color) {
	static float angle_hour;
	int x, y;

	x = x_coor + (length) * sin(angle_hour);
	y = y_coor + (length) * -cos(angle_hour);
	lcdDrawLine(x_coor, y_coor, x, y, WHITE);

	angle_hour = ((*hour % 12 + *minute / 60.0) * 30) * (PI / 180); // 360 degrees / 12 hours = 30 degrees per hour
	x = x_coor + (length) * sin(angle_hour);
	y = y_coor + (length) * -cos(angle_hour);
	lcdDrawLine(x_coor, y_coor, x, y, color);

	return angle_hour;
}

void displayTime(int x_coor, int y_coor, const uint8_t *second,
		const uint8_t *minute, const uint8_t *hour, uint8_t char_size,
		uint16_t color_sec, uint16_t color_min, uint16_t color_hour) {
	lcdShowIntNumCenter(x_coor - char_size * 2, y_coor, *hour, 2, color_hour,
	WHITE, char_size, 0);
	lcdShowIntNumCenter(x_coor, y_coor, *minute, 2, color_min, WHITE, char_size,
			0);
	lcdShowIntNumCenter(x_coor + char_size * 2, y_coor, *second, 2, color_sec,
	WHITE, char_size, 0);

	lcdShowStringCenter(x_coor + char_size, y_coor, ":", BLACK, WHITE,
			char_size, 1);
	lcdShowStringCenter(x_coor - (char_size * 2) + char_size, y_coor, ":",
	BLACK, WHITE, char_size, 1);
}

void displayDate(int x_coor, int y_coor, const uint8_t *date,
		const uint8_t *month, const uint16_t *year, uint8_t char_size,
		uint16_t color_date, uint16_t color_month, uint16_t color_year) {
	lcdShowIntNumCenter(x_coor - char_size * 2, y_coor, *date, 2, color_date,
	WHITE, char_size, 0);
	lcdShowIntNumCenter(x_coor, y_coor, *month, 2, color_month, WHITE,
			char_size, 0);
	lcdShowIntNumCenter(x_coor + char_size * 2 + char_size / 2.0, y_coor, *year,
			4, color_year, WHITE, char_size, 0);

	lcdShowStringCenter(x_coor + char_size, y_coor, "/", color_month, WHITE,
			char_size, 1);
	lcdShowStringCenter(x_coor - (char_size * 2) + char_size, y_coor, "/",
			color_month, WHITE, char_size, 1);
}

void displayTemp(int x_coor, int y_coor, float temperature, uint8_t char_size,
		uint16_t color_temp) {
	lcdShowFloatNum(x_coor, y_coor, temperature, 4, color_temp, WHITE,
			char_size, 0);
	return;
}
void displayDay(int x_coor, int y_coor, const uint8_t *day, uint8_t char_size,
		uint16_t color_day) {
	switch (*day) {
	case 1: {
		lcdShowString(x_coor, y_coor, "Sunday   ", color_day, WHITE, char_size,
				0);
		break;
	}
	case 2: {
		lcdShowString(x_coor, y_coor, "Monday   ", color_day, WHITE, char_size,
				0);
		break;
	}
	case 3: {
		lcdShowString(x_coor, y_coor, "Tuesday  ", color_day, WHITE, char_size,
				0);
		break;
	}
	case 4: {
		lcdShowString(x_coor, y_coor, "Wednesday", color_day, WHITE, char_size,
				0);
		break;
	}
	case 5: {
		lcdShowString(x_coor, y_coor, "Thursday ", color_day, WHITE, char_size,
				0);
		break;
	}
	case 6: {
		lcdShowString(x_coor, y_coor, "Friday", color_day, WHITE, char_size, 0);
		break;
	}
	case 7: {
		lcdShowString(x_coor, y_coor, "Saturday", color_day, WHITE, char_size,
				0);
		break;
	}
	}
	return;
}

void displayTimeLed7Seg(const uint8_t *second, const uint8_t *minute,
		const uint8_t *hour) {
//	if (((*second / 10) >> 0) & 0x01) {
//		led7SegDebugTurnOn(6);
//	} else {
//		led7SegDebugTurnOff(6);
//	}
//	if (((*second / 10) >> 1) & 0x01) {
//		led7SegDebugTurnOn(7);
//	} else {
//		led7SegDebugTurnOff(7);
//	}
//	if (((*second / 10) >> 2) & 0x01) {
//		led7SegDebugTurnOn(8);
//	} else {
//		led7SegDebugTurnOff(8);
//	}

	led7_SetDigit(*hour / 10, 0, 0);
	led7_SetDigit(*hour % 10, 1, 0);

	led7_SetDigit(*minute / 10, 2, 0);
	led7_SetDigit(*minute % 10, 3, 0);

	return;
}

void increaseSec() {
	if (set_time.second < 59) {
		set_time.second += 1;
	} else {
		set_time.second = 0;
		if (set_time.minute < 59) {
			set_time.minute += 1;
		} else {
			set_time.minute = 0;
			if (set_time.hour < 23) {
				set_time.hour += 1;
			} else {
				set_time.hour = 0;
			}
		}
	}
}
void decreaseSec() {
	if (set_time.second > 0) {
		set_time.second -= 1;
	} else {
		set_time.second = 59;
		if (set_time.minute > 0) {
			set_time.minute -= 1;
		} else {
			set_time.minute = 59;
			if (set_time.hour > 0) {
				set_time.hour -= 1;
			} else {
				set_time.hour = 23;
			}
		}
	}
}
void increaseMin() {
	if (set_time.minute < 59) {
		set_time.minute += 1;
	} else {
		set_time.minute = 0;
		if (set_time.hour < 23) {
			set_time.hour += 1;
		} else {
			set_time.hour = 0;
		}
	}
}

void decreaseMin() {
	if (set_time.minute > 0) {
		set_time.minute -= 1;
	} else {
		set_time.minute = 59;
		if (set_time.hour > 0) {
			set_time.hour -= 1;
		} else {
			set_time.hour = 23;
		}
	}
}

void increaseHour() {
	if (set_time.hour < 23) {
		set_time.hour += 1;
	} else {
		set_time.hour = 0;
	}
}

void decreaseHour() {
	if (set_time.hour > 0) {
		set_time.hour -= 1;
	} else {
		set_time.hour = 23;
	}
}

void increaseDay() {
	if (set_time.day < 7) {
		set_time.day += 1;
	} else {
		set_time.day = 1;
	}
}

void decreaseDay() {
	if (set_time.day > 1) {
		set_time.day -= 1;
	} else {
		set_time.day = 7;
	}
}
void increaseDate() {
	int daysInMonth = getDaysInMonth(set_time.month, set_time.year);
	if (set_time.date < daysInMonth) {
		set_time.date += 1;
	} else {
		set_time.date = 1;
		if (set_time.month < 12) {
			set_time.month += 1;
		} else {
			set_time.month = 1;
			set_time.year += 1;
		}
	}
}

void decreaseDate() {
	if (set_time.date > 1) {
		set_time.date -= 1;
	} else {
		if (set_time.month > 1) {
			set_time.month -= 1;
		} else {
			set_time.month = 12;
			set_time.year -= 1;
		}
		set_time.date = getDaysInMonth(set_time.month, set_time.year);
	}
}

void increaseMonth() {
	if (set_time.month < 12) {
		set_time.month += 1;
	} else {
		set_time.month = 1;
		set_time.year += 1;
	}

	if (set_time.day
			> (uint8_t) (getDaysInMonth(set_time.month, set_time.year))) {
		set_time.day =
				(uint8_t) (getDaysInMonth(set_time.month, set_time.year));
	}
}

void decreaseMonth() {
	if (set_time.month > 1) {
		set_time.month -= 1;
	} else {
		set_time.month = 12;
		set_time.year -= 1;
	}

	if (set_time.day
			> (uint8_t) (getDaysInMonth(set_time.month, set_time.year))) {
		set_time.day =
				(uint8_t) (getDaysInMonth(set_time.month, set_time.year));
	}
}

void increaseYear() {
	set_time.year += 1;

	if (set_time.day
			> (uint8_t) (getDaysInMonth(set_time.month, set_time.year))) {
		set_time.day =
				(uint8_t) (getDaysInMonth(set_time.month, set_time.year));
	}
}

void decreaseYear() {
	if (set_time.year > 0) // Assuming year 0 is a valid boundary
			{
		set_time.year -= 1;
	}

	if (set_time.day
			> (uint8_t) (getDaysInMonth(set_time.month, set_time.year))) {
		set_time.day =
				(uint8_t) (getDaysInMonth(set_time.month, set_time.year));
	}
}

// Helper function to get the number of days in a month
int getDaysInMonth(int month, int year) {
	switch (month) {
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12: {
		return 31;
	}
	case 4:
	case 6:
	case 9:
	case 11: {
		return 30;
	}
	case 2: {
		if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
			return 29; // Leap year
		else
			return 28;
	}
	default:
		return 30; // Default case, should not happen
	}
}
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
