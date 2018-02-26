/*
 * error_check.h
 *
 *  Created on: Feb 26, 2018
 *      Author: Nathan
 */

#ifndef ERROR_CHECK_H_
#define ERROR_CHECK_H_

#include "FreeRTOS.h"
#include "queue.h"


/*
 * Which each bit in the FRAM error word represents when set.
 */
enum error_bit {
	ERROR_QUEUE,
	ERROR_TASK_FAIL,
	ERROR_UKNOWN,
};

struct error_message {
	int id;
	uint16_t data;
};

void task_error_check(void *arg);

/*
 * Function: error_write_fram
 *
 * Purpose: Sets this bit on the FRAM error word.
 * in: which bit should be set
 */
void error_set_fram(int bit);

/*
 * Function: send_error
 *
 * Purpose: This function is placed here to avoid having it declared in each class. It tries to send the message on the error queue,
 * 		but if it is not successful, it sets a bit on the FRAM.
 * in: pointer to the error queue, and pointer to the error message to be sent.
 */
void error_send_message(xQueueHandle*, struct error_message *);

#endif /* ERROR_CHECK_H_ */
