/*
 * ring_buffer.c
 *
 *  Created on: Nov 20, 2024
 *      Author: ADMIN
 */

#include "ring_buffer.h"

void ringBufferPush(ring_buffer_t *buf, uint8_t value)
{
	buf->data[buf->tail] = value;
	buf->tail++;
	if (buf->tail == RING_BUFFER_MAX_SIZE)
	{
		buf->tail = 0;
	}
	if (buf->length < RING_BUFFER_MAX_SIZE)
	{
		buf->length++;
	}
}

uint8_t ringBufferPop(ring_buffer_t *buf, uint8_t *data)
{
	if (buf->length > 0)
	{
		*data = buf->data[buf->head];
		buf->head++;
		buf->length--;
		return RING_BUFFER_SUCCESS;
	}
	return RING_BUFFER_ERR_EMPTY;
}

uint8_t ringBufferPeek(ring_buffer_t *buf, uint8_t *data)
{
	if (buf->length > 0)
	{
		*data = buf->data[buf->head];
		return RING_BUFFER_SUCCESS;
	}
	return RING_BUFFER_ERR_EMPTY;
}

uint8_t isRingBufferEmpty(ring_buffer_t *buf)
{
	return buf->length;
}
