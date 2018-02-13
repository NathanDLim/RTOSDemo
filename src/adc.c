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

/*
 * The Attitude and Determination Control (ADC) task.
 *
 * Data owned:
 *
 * In: A handle to a queue. All messaged to the adc task come through this queue.
 */
void task_adc(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;
	struct queue_message message;

	for (;;) {

		// We do a non-blocking check to see if there is a message waiting in the queue
		if (xQueueReceive(queue, &message, 0) == pdTRUE) {
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

		printf("time = %li\n", gps_get_timestamp());
		fflush(stdout);
		vTaskDelay(100);
	}
}


