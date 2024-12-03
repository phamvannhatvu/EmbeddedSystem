

#include "software_timer.h"

#define TIMER_CYCLE_2 1

int snake_move_flag = 0;
int button_read_flag = 0;

int snake_move_counter = 0;
int button_read_counter = 0;

uint16_t flag_timer2 = 0;
uint16_t timer2_counter = 0;
uint16_t timer2_MUL = 0;

void timer_init(){
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start(&htim1);
}

void timer_EnableDelayUs(){
	HAL_TIM_Base_Start(&htim1);
}

void setTimer2(uint16_t duration){
	timer2_MUL = duration/TIMER_CYCLE_2;
	timer2_counter = timer2_MUL;
	flag_timer2 = 0;
}
void setTimer_snake(uint16_t duration){
	int timer_snake_MUL = duration/TIMER_CYCLE_2;
	snake_move_counter = timer_snake_MUL;
	snake_move_flag = 0;
}
void setTimer_button(uint16_t duration){
	int timer_button_MUL = duration/TIMER_CYCLE_2;
	button_read_counter = timer_button_MUL;
	button_read_flag = 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){
		if(timer2_counter > 0){
			timer2_counter--;
			if(timer2_counter == 0) {
				flag_timer2 = 1;
				timer2_counter = timer2_MUL;
			}
		}
		if(button_read_counter > 0){
			button_read_counter--;
			if(button_read_counter == 0) {
				button_read_flag = 1;
			}
		}
		if(snake_move_counter > 0){
			snake_move_counter--;
			if(snake_move_counter == 0) {
				snake_move_flag = 1;
			}
		}
		led7_Scan();
	}
}

void delay_us (uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
}


