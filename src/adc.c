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

/*
 * The Attitude and Determination Control (ADC) task.
 *
 * Data owned:
 *
 * In: A handle to a queue. All messaged to the adc task come through this queue.
 */
void task_attitude(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;
	struct queue_message message;

	for (;;) {

		// We do a non-blocking check to see if there is a message waiting in the queue
		if (xQueueReceive(queue, &message, 0) == pdTRUE) {
			switch (message.id) {
				case ADC_SUN_POINT:
					printf("ADC switching to sun pointing\n");
					break;
				case ADC_NADIR_POINT:
					printf("ADC switching to nadir pointing\n");
					break;
				case ADC_SET_REACT_SPEED:
					printf("Setting the reaction wheel speed to %i\n", message.data);
					break;
				default:
					printf("Error in ADC message");
					break;
			}

			fflush(stdout);
		}


		vTaskDelay(100);
	}
}


