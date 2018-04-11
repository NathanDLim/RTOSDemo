/*
 * error_check.c
 *
 *  Created on: Feb 26, 2018
 *      Author: Nathan Lim
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "obc.h"
#include "error_check.h"

void error_nonrecover()
{
	error("NONRECOVERABLE ERROR!\n");
}

void task_error_check(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;
	struct queue_message message;

	for (;;) {
		if (xQueueReceive(queue, &message, 500) == pdTRUE) {
			debug("Received error message %i\n", message.id);

			switch(message.id){
				case CONTROL:
					debug("Control error\n");
					break;
				case ADC:
					debug("ADC error\n");
					break;
				case PAYLOAD:
					debug("Payload error\n");
					break;
				case COMM_RX:
					debug("Comm_rx error\n");
					break;
				case COMM_TX:
					debug("Comm_tx error\n");
					break;
				case GPS:
					debug("GPS error\n");
					break;
				case HOUSEKEEP:
					debug("Housekeep error\n");
					break;
				case FILE_W:
					debug("file_w error\n");
					break;
				default:
					error("ERROR: invalid error queue id\n");
			}
		}

		fflush(stdout);

		// check the FRAM for queue errors or other serious errors.

		vTaskDelay(500);
	}
}

void error_set_fram(int bit)
{
	// TODO: actually write to FRAM
	error("CRITICAL ERROR: setting bit %i\n", bit);
}

void error_send_message(xQueueHandle *err_q, struct queue_message *err)
{
	if (xQueueSend(*err_q, (void *) err, 0) == pdFALSE) {
		error("Error sending error queue message\n");
		error_set_fram(ERROR_QUEUE);
	}
}


