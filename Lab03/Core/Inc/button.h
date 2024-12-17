/*
 * button.h
 *
 *  Created on: Sep 25, 2023
 *      Author: HaHuyen
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "spi.h"
#include "gpio.h"

extern uint16_t button_count[16];

void button_init();
void button_Scan();

#define BTN_0 		13
#define BTN_1 		0
#define BTN_2 		1
#define BTN_3 		2
#define BTN_4 		4
#define BTN_5 		5
#define BTN_6 		6
#define BTN_8 		9
#define BTN_7 		8
#define BTN_9 		10
#define BTN_UP 		3
#define BTN_DOWN	7
#define BTN_RIGHT	11
#define BTN_LEFT	15
#define BTN_E		12
#define BTN_B		14


#endif /* INC_BUTTON_H_ */

