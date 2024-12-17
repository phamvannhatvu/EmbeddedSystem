/*
 * traffic_light.c
 *
 *  Created on: Nov 29, 2024
 *      Author: Amyra Nguyen
 */


#include <traffic_light.h>
#include <lcd.h>
#include <button.h>
#include <software_timer.h>
uint16_t traffic_light_max_time[3] = {999, 3, 2};   // User settings for traffic light time
uint16_t traffic_light_counter[2]; 	// Current counter for normal mode
uint8_t traffic_light_state[2];      // Current light state for normal mode
uint8_t traffic_light_led_coordinate[2][2];
uint8_t traffic_light_number_coordinate[2][2];
uint8_t current_mode = MODE_NORMAL;

// Modify mode
uint8_t temp_max_counter = 0;
uint8_t temp_button_press_counter = 0;
uint8_t temp_button_press_index = 16;

void traffic_light_init(){
	// Setup left side
	traffic_light_reset(LEFT_SIDE, RED_LIGHT);
	traffic_light_led_coordinate[LEFT_SIDE][X_COR] = 60;
	traffic_light_led_coordinate[LEFT_SIDE][Y_COR] = 120;
	traffic_light_number_coordinate[LEFT_SIDE][X_COR] = 35;
	traffic_light_number_coordinate[LEFT_SIDE][Y_COR] = 110;

	// Setup right side
	traffic_light_reset(RIGHT_SIDE, GREEN_LIGHT);
	traffic_light_led_coordinate[RIGHT_SIDE][X_COR] = 160;
	traffic_light_led_coordinate[RIGHT_SIDE][Y_COR] = 120;
	traffic_light_number_coordinate[RIGHT_SIDE][X_COR] = 135;
	traffic_light_number_coordinate[RIGHT_SIDE][Y_COR] = 110;

	current_mode = MODE_NORMAL;
	// Modify mode
	edit_index = 0;
	temp_max_counter = 0;
}
void traffic_light_reset(uint8_t side, uint8_t color){
	traffic_light_counter[side] = traffic_light_max_time[color];
	traffic_light_counter[side] = (traffic_light_counter[side] + 1);
	traffic_light_state[side] = color;
}
void blink_edit_mode(){
	if(edit_index == 4){
		toggle_led7seg(1);
		toggle_led7seg(2);
		toggle_led7seg(3);
	}
	else
		toggle_led7seg(temp_edit_index);
}

// Finite state machine init
void fsm_normal_init(){
	lcd_clearScr();
	traffic_light_reset(LEFT_SIDE, RED_LIGHT);
	traffic_light_reset(RIGHT_SIDE, GREEN_LIGHT);
}
void fsm_modify_init(uint8_t color){
	lcd_clearScr();
	temp_edit_index = 0;
	temp_max_counter = traffic_light_max_time[color];
	led_Off(0);
	led_On(1);
	led_On(2);
	led_On(3);
}
// Finite state machine operate
void fsm_normal(uint16_t system_timer){
	if(system_timer > 0) return;

	// Running traffic light timer
	traffic_light_counter[LEFT_SIDE]--;
	traffic_light_counter[RIGHT_SIDE]--;

	// Display mode
	lcd_ShowStr(50, 30, "NORMAL MODE", WHITE, RED, 24, 0);

	// Display & switch LED mode
	for(int side = LEFT_SIDE; side <= RIGHT_SIDE; side++){
		switch(traffic_light_state[side]) {
		case RED_LIGHT:
			// Display traffic light led & number
			lcd_DrawCircle(traffic_light_led_coordinate[side][X_COR], traffic_light_led_coordinate[side][Y_COR], RED, 40, 1);
			lcd_ShowIntNum(traffic_light_number_coordinate[side][X_COR], traffic_light_number_coordinate[side][Y_COR], traffic_light_counter[side], 3, WHITE, RED, 24);
			if(traffic_light_counter[side] <= 1) {// Change from RED to GREEN
				traffic_light_reset(side, GREEN_LIGHT);
			}
			break;
		case GREEN_LIGHT:
			// Display traffic light led & number
			lcd_DrawCircle(traffic_light_led_coordinate[side][X_COR], traffic_light_led_coordinate[side][Y_COR], GREEN, 40, 1);
			lcd_ShowIntNum(traffic_light_number_coordinate[side][X_COR], traffic_light_number_coordinate[side][Y_COR], traffic_light_counter[side], 3, BLACK, GREEN, 24);
			if(traffic_light_counter[side] <= 1) {// Change from GREEN to YELLOW
				traffic_light_reset(side, YELLOW_LIGHT);
			}
			break;
		case YELLOW_LIGHT:
			// Display traffic light led & number
			lcd_DrawCircle(traffic_light_led_coordinate[side][X_COR], traffic_light_led_coordinate[side][Y_COR], YELLOW, 40, 1);
			lcd_ShowIntNum(traffic_light_number_coordinate[side][X_COR], traffic_light_number_coordinate[side][Y_COR], traffic_light_counter[side], 3, BLACK, YELLOW, 24);
			if(traffic_light_counter[side] <= 1) {// Change from YELLOW to RED
				traffic_light_reset(side, RED_LIGHT);
			}
			break;
		}
	}
}
void fsm_modify_light(uint16_t system_timer, uint8_t color){
	if(system_timer > 0) return;
	else blink_edit_mode();
	// Display LED 7 segments
	led7_SetDigit(0, 0, 0);
	led7_SetDigit(0, 1, 0);
	led7_SetDigit((temp_max_counter / 10) % 10, 2, 0);
	led7_SetDigit(temp_max_counter % 10, 3, 0);

	// Display mode
	switch(color){
	case RED_LIGHT:
		lcd_ShowStr(50, 30, "MODIFY RED", WHITE, RED, 24, 0);
		lcd_DrawCircle(traffic_light_led_coordinate[LEFT_SIDE][X_COR], traffic_light_led_coordinate[LEFT_SIDE][Y_COR], RED, 40, 1);
		lcd_ShowIntNum(traffic_light_number_coordinate[RIGHT_SIDE][X_COR], traffic_light_number_coordinate[RIGHT_SIDE][Y_COR], temp_max_counter, 4, WHITE, RED, 24);
		break;
	case GREEN_LIGHT:
		lcd_ShowStr(50, 30, "NORMAL GREEN", BLACK, GREEN, 24, 0);
		lcd_DrawCircle(traffic_light_led_coordinate[LEFT_SIDE][X_COR], traffic_light_led_coordinate[LEFT_SIDE][Y_COR], GREEN, 40, 1);
		lcd_ShowIntNum(traffic_light_number_coordinate[RIGHT_SIDE][X_COR], traffic_light_number_coordinate[RIGHT_SIDE][Y_COR], temp_max_counter, 4, BLACK, GREEN, 24);
		break;
	case YELLOW_LIGHT:
		lcd_ShowStr(50, 30, "NORMAL YELLOW", BLACK, YELLOW, 24, 0);
		lcd_DrawCircle(traffic_light_led_coordinate[LEFT_SIDE][X_COR], traffic_light_led_coordinate[LEFT_SIDE][Y_COR], YELLOW, 40, 1);
		lcd_ShowIntNum(traffic_light_number_coordinate[RIGHT_SIDE][X_COR], traffic_light_number_coordinate[RIGHT_SIDE][Y_COR], temp_max_counter, 4, BLACK, YELLOW, 24);
		break;
	}

	// Increase & decrease value
	if(button_count[BTN_UP] >= 1){// Increase value
		// Increase once
		if(button_count[BTN_UP] == 1){
			temp_max_counter = (temp_max_counter + 1) % 100;
		}

		// Increase multiple
		if(button_count[BTN_UP] == 20){
			// Only increase every 500ms
			if(temp_button_press_counter != BTN_UP){
				temp_button_press_counter = 0;
			} else temp_max_counter = temp_button_press_counter + 1;
			if(temp_button_press_counter == 5){
				// Wait until 250ms to increase once: Keep between the range 1-99 while increase
				temp_max_counter = (temp_max_counter < 99) ? temp_max_counter + 1: 1;
				temp_button_press_counter = 0;
			}
		}
		temp_button_press_index = BTN_UP;

	} else if(button_count[BTN_DOWN] >= 1){// Decrease value
		// Decrease once
		if(button_count[BTN_DOWN] == 1){
			if(temp_max_counter == 0) temp_max_counter = 99;
			else temp_max_counter = temp_max_counter - 1;
		}

		// Decrease multiple
		if(button_count[BTN_DOWN] == 20){
			// Only increase every 500ms
			if(temp_button_press_counter != BTN_DOWN){
				temp_button_press_counter = 0;
			} else temp_max_counter = temp_button_press_counter + 1;
			if(temp_button_press_counter == 5){
				// Wait until 250ms to increase once: Keep between the range 1-99 while increase
				temp_max_counter = (temp_max_counter < 99) ? temp_max_counter + 1: 1;
				temp_button_press_counter = 0;
			}
		}
		temp_button_press_index = BTN_DOWN;

	}

	// Save data
	if(button_count[14] == 1){
		  traffic_light_max_time[color] = temp_max_counter;
	}

}
