/*
 * ring_buffer.h
 *
 *  Created on: Nov 20, 2024
 *      Author: ADMIN
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#include "stdlib.h"
#include "stdint.h"

#define RING_BUFFER_SUCCESS			0
#define RING_BUFFER_ERR_EMPTY		1
#define RING_BUFFER_MAX_SIZE		100

typedef struct
{
	uint8_t length;
	uint8_t head;
	uint8_t tail;
	uint8_t data[RING_BUFFER_MAX_SIZE];
} ring_buffer_t;

void ringBufferPush(ring_buffer_t *buf, uint8_t value);
uint8_t ringBufferPop(ring_buffer_t *buf,  uint8_t *data);
uint8_t ringBufferPeek(ring_buffer_t *buf, uint8_t *data);

#endif /* INC_RING_BUFFER_H_ */
