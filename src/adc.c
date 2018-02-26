/*
 * adc.c
 *
 *  Created on: Oct 6, 2017
 *      Author: admin
 */

//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//#include <freertos/semphr.h>
//#include <freertos/queue.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <stdio.h>

#include "adc.h"
#include "obc.h"
#include "gps.h"
#include "error_check.h"

/*
 * The Attitude and Determination Control (ADC) task.
 *
 * Data owned:
 *
 * In: A handle to double queue holding the adc_queue and the error_queue. All messaged to the adc task come through the adc_queue
 */
void task_adc(void *arg)
{
	struct multi_queue mq = *(struct multi_queue *)arg;
	if (mq.num != 2) {
		error("ADC input arg incorrect\n");
		return;
	}
	xQueueHandle adc_queue = *mq.q[0];
	xQueueHandle error_queue = *mq.q[1];
	struct queue_message message;
	struct error_message em;
	em.id = ADC;

	for (;;) {

		// We do a non-blocking check to see if there is a message waiting in the queue
		if (xQueueReceive(adc_queue, &message, 0) == pdTRUE) {
			switch (message.id) {
				case ADC_CMD_SUN_POINT:
					debug("ADC switching to sun pointing\n");
					break;
				case ADC_CMD_NADIR_POINT:
					debug("ADC switching to nadir pointing\n");
					break;
				case ADC_CMD_SET_REACT_SPEED:
					debug("Setting the reaction wheel speed to %li\n", message.data);
					break;
				default:
					error("Error in ADC message");
					break;
			}
		}

//		printf("time = %li\n", gps_get_timestamp());
//		fflush(stdout);
		error_send_message(&error_queue, &em);
		vTaskDelay(1000);
	}

	// Should never reach this point;
	for (;;) {
		error_send_message(&error_queue, &em);
		error_set_fram(ERROR_TASK_FAIL);
		vTaskDelay(1000);
	}
}


