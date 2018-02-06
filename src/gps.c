/*
 * gps.c
 *
 *  Created on: Oct 17, 2017
 *      Author: admin
 */

//#include <FreeRTOS/FreeRTOS.h>
//#include <FreeRTOS/task.h>
//#include <FreeRTOS/semphr.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "gps.h"

#include <stdio.h>

//static struct gps_data gps_data;

void task_gps(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;
	struct gps_queue_message message;

	for (;;) {
		printf("GPS task running\n");

		message.id = 101;
		// message id is the tick count when the gps was read
		message.tick = xTaskGetTickCount();
		// message data is the time that was read
		// It currently uses the tick count. It will be updated once GPS is attached
		message.time = xTaskGetTickCount() * portTICK_PERIOD_MS;
		printf("GPS tick = %i, time = %li\n", message.tick, message.time);
		fflush(stdout);
		if (xQueueOverwrite(queue, (void *) &message) == pdFALSE) {
			printf("Error sending gps queue message\n");
			break;
		}

		vTaskDelay(5000);
	}

	// Should never reach this point;
	for (;;) ;
}

