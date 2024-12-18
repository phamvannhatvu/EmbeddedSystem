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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart.h"
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "sensor.h"
#include "buzzer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIZE62 10
#define HEIGHT62 150
#define MIN62 45
#define MAX62 50
#define OFFSET62 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void test_Buzzer();
void test_Adc();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t counter61 = 0;
char buffer61[20];
void ex61(){
	counter61 = (counter61 + 1) % 20;
	if (counter61 == 0) {
		sensor_Read();
		lcd_ShowStr(10, 20, "Power: ", RED, BLACK, 16, 0);
		lcd_ShowFloatNum(150, 20, sensor_GetVoltage() * sensor_GetCurrent() * 0.001, 4, RED, BLACK, 16);
		uart_Rs232SendString((uint8_t*)"\nPower: ");
		sprintf(buffer61, "%.2f", sensor_GetVoltage() * sensor_GetCurrent() * 0.001);
		uart_Rs232SendString((uint8_t*)buffer61);

		lcd_ShowStr(10, 40, "Light: ", RED, BLACK, 16, 0);
		lcd_ShowIntNum(150, 40, sensor_GetLight(), 4, RED, BLACK, 16);
		uart_Rs232SendString((uint8_t*)"\nLight: ");
		uart_Rs232SendNum(sensor_GetLight());

		lcd_ShowStr(10, 60, "Temperature: ", RED, BLACK, 16, 0);
		lcd_ShowFloatNum(150, 60, sensor_GetTemperature(), 4, RED, BLACK, 16);
		uart_Rs232SendString((uint8_t*)"\nTemperature: ");
		sprintf(buffer61, "%.2f", sensor_GetTemperature());
		uart_Rs232SendString((uint8_t*)buffer61);

		lcd_ShowStr(10, 80, "Potentiometer: ", RED, BLACK, 16, 0);
		lcd_ShowIntNum(150, 80, sensor_GetPotentiometer(), 4, RED, BLACK, 16);
		uart_Rs232SendString((uint8_t*)"\nPotentiometer: ");
		uart_Rs232SendNum(sensor_GetPotentiometer());
		uart_Rs232SendString((uint8_t*)"\n");

		if (sensor_GetPotentiometer() > 0.7 * 4095) {
			buzzer_SetVolume(50);
			uart_Rs232SendString((uint8_t*)"\nWARNING\n");
		} else {
			buzzer_SetVolume(0);
		}
	}
	ds3231_ReadTime();
	lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 16);
	lcd_ShowStr(95, 100, ":", GREEN, BLACK, 16, 0);
	lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 16);
	lcd_ShowStr(135, 100, ":", GREEN, BLACK, 16, 0);
	lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 16);
}
void ex61_rt(){
	ds3231_init();
	ds3231_Write(ADRESS_YEAR, 23);
	ds3231_Write(ADRESS_MONTH, 10);
	ds3231_Write(ADRESS_DATE, 20);
	ds3231_Write(ADRESS_DAY, 6);
	ds3231_Write(ADRESS_HOUR, 20);
	ds3231_Write(ADRESS_MIN, 11);
	ds3231_Write(ADRESS_SEC, 23);
}
void ex61_uart(){
	uart_init_rs232();
}
typedef struct {
	float arr[SIZE62];
    int front;
    int rear;
} queue62;
void queue62_init(queue62 *q) {
    q->front = -1;
    q->rear = -1;
}
void queue62_insert(queue62 *q, float value) {
    if ((q->rear + 1) % SIZE62 == q->front) {
        q->front = (q->front + 1) % SIZE62;
    }
    if (q->front == -1) {
        q->front = 0;
    }
    q->rear = (q->rear + 1) % SIZE62;
    q->arr[q->rear] = value;
}
queue62 q62;
uint16_t counter62 = 0;
uint8_t x62 = 0;
uint8_t y62 = 0;
uint8_t oldy62 = 0;
void ex62(queue62 *q) {
	counter62 = (counter62 + 1) % (20 * 15);
	lcd_DrawLine(40, 300, 40, 300 - HEIGHT62, WHITE);
	lcd_DrawLine(40, 300, 220, 300, WHITE);
	lcd_ShowIntNum(10, 300 - 10, MIN62, 2, WHITE, BLACK, 16);
	lcd_ShowIntNum(10, 300 - HEIGHT62 - 10, MAX62, 2, WHITE, BLACK, 16);
	if (counter62 == 0) {
		sensor_Read();
		queue62_insert(q, sensor_GetVoltage() * sensor_GetCurrent() * 0.001);
		lcd_Fill(40 + OFFSET62, 300 - HEIGHT62 + OFFSET62, 220 - OFFSET62, 300 - OFFSET62, BLACK);

		if (q->front != -1) {
			x62 = 50;
			for (int i = q->front; i != q->rear; i = (i + 1) % SIZE62) {
				y62 = (q->arr[i] - MIN62) * (HEIGHT62 / (MAX62 - MIN62));
				lcd_DrawPoint(x62, 300 - y62, RED);
				if (i != q->front) {
					lcd_DrawLine(x62 - 20, 300 - oldy62, x62, 300 - y62, YELLOW);
				}
				oldy62 = y62;
				x62 += 20;
			}
		}
	}
}
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
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_FSMC_Init();
  MX_I2C1_Init();
  MX_TIM13_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 lcd_Clear(BLACK);
 ex61_rt();
 ex61_uart();
 queue62_init(&q62);
  while (1)
  {
	  while(!flag_timer2);
	  flag_timer2 = 0;
	  button_Scan();
//	  test_LedDebug();
//	  test_Adc();
//	  test_Buzzer();
	  ex61();
	  ex62(&q62);
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
	ds3231_init();
	timer_init();
	button_init();
	lcd_init();
	sensor_init();
	buzzer_init();
	setTimer2(50);
}

uint8_t count_led_debug = 0;

void test_LedDebug(){
	count_led_debug = (count_led_debug + 1)%20;
	if(count_led_debug == 0){
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
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

uint8_t isButtonRight()
{
    if (button_count[11] == 1)
        return 1;
    else
        return 0;
}

uint8_t count_adc = 0;

void test_Adc(){
	count_adc = (count_adc + 1)%20;
	if(count_adc == 0){
		sensor_Read();
		lcd_ShowStr(10, 100, "Voltage:", RED, BLACK, 16, 0);
		lcd_ShowFloatNum(130, 100,sensor_GetVoltage(), 4, RED, BLACK, 16);
		lcd_ShowStr(10, 120, "Current:", RED, BLACK, 16, 0);
		lcd_ShowFloatNum(130, 120,sensor_GetCurrent(), 4, RED, BLACK, 16);
		lcd_ShowStr(10, 140, "Light:", RED, BLACK, 16, 0);
		lcd_ShowIntNum(130, 140, sensor_GetLight(), 4, RED, BLACK, 16);
		lcd_ShowStr(10, 160, "Potentiometer:", RED, BLACK, 16, 0);
		lcd_ShowIntNum(130, 160, sensor_GetPotentiometer(), 4, RED, BLACK, 16);
		lcd_ShowStr(10, 180, "Temperature:", RED, BLACK, 16, 0);
		lcd_ShowFloatNum(130, 180,sensor_GetTemperature(), 4, RED, BLACK, 16);
	}
}

void test_Buzzer(){
	if(isButtonUp()){
		buzzer_SetVolume(50);
	}

	if(isButtonDown()){
		buzzer_SetVolume(0);
	}

	if(isButtonRight()){
		buzzer_SetVolume(25);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
