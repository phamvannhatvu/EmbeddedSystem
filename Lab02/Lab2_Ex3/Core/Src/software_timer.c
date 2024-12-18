/*
 * software_timer.c
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#include "software_timer.h"

#define TIMER_CYCLE_2 1
#define TIMER_CYCLE_1Hz (1000 / (1 * 4))
#define TIMER_CYCLE_25Hz (1000 / (25 * 4))
#define TIMER_CYCLE_100Hz (1000 / (100 * 4))


//software timer variable
uint16_t flag_timer2 = 0;
uint16_t timer2_counter = 0;
uint16_t timer2_MUL = 0;
uint16_t timer_modify_count = 0;


/**
  * @brief  Init timer interrupt
  * @param  None
  * @retval None
  */
void timer_init(){
	HAL_TIM_Base_Start_IT(&htim2);
}


/**
  * @brief  Set duration of software timer interrupt
  * @param  duration Duration of software timer interrupt
  * @retval None
  */
void setTimer2(uint16_t duration){
	timer2_MUL = duration/TIMER_CYCLE_2;
	timer2_counter = timer2_MUL;
	flag_timer2 = 0;
}

/**
  * @brief  Timer interrupt routine
  * @param  htim TIM Base handle
  * @note	This callback function is called by system
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){
		if(timer2_counter > 0){
			timer2_counter--;
			if(timer2_counter == 0) {
				flag_timer2 = 1;
				timer2_counter = timer2_MUL;
			}
		}
		// Change TIMER_CYCLE accordingly
//		timer_modify_count = (timer_modify_count + 1) % TIMER_CYCLE_1Hz;
//		timer_modify_count = (timer_modify_count + 1) % TIMER_CYCLE_25Hz;
		timer_modify_count = (timer_modify_count + 1) % TIMER_CYCLE_100Hz;
		colon_blink = (colon_blink + 1) % COLON_CYCLE_2Hz;
		if(timer_modify_count == 0)
			led7_Scan();
		if(colon_blink == 0){
			colon_status = !colon_status;
			led7_SetColon(colon_status);
		}
	}
}

