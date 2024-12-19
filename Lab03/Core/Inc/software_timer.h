/*
 * software_timer.h
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "software_timer.h"

#include "tim.h"
#include "led_7seg.h"

#define MODE_NORMAL_TICKS 20
#define MODE_MODIFY_TICKS 5

extern uint16_t flag_timer2;
extern uint16_t mode_counter;

void timer_init();
void setTimer2(uint16_t duration);
void set_led7(uint8_t status);

#endif /* INC_SOFTWARE_TIMER_H_ */
