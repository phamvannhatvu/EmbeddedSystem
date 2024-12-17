/*
 * traffic_light.h
 *
 *  Created on: Nov 29, 2024
 *      Author: Amyra Nguyen
 */

#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_
#include "gpio.h"
#include "stdlib.h"
#include <string.h>

// Light color
#define RED_LIGHT 0
#define GREEN_LIGHT 1
#define YELLOW_LIGHT 2

// Light side
#define LEFT_SIDE 0
#define RIGHT_SIDE 1

// System mode
#define MODE_NORMAL 0
#define MODE_MODIFY_RED 1
#define MODE_MODIFY_GREEN 2
#define MODE_MODIFY_YELLOW 3

extern uint8_t current_mode;

void traffic_light_init();
void traffic_light_reset(uint8_t side, uint8_t color);
void fsm_normal(uint16_t system_timer);
void fsm_modify_light(uint16_t system_timer, uint8_t color);
void blink_edit_mode();

#endif /* INC_TRAFFIC_LIGHT_H_ */
