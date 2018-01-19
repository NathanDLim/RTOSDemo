/*
 * housekeeping.c
 *
 *  Created on: Oct 17, 2017
 *      Author: admin
 */

//#include <FreeRTOS/FreeRTOS.h>
//#include <FreeRTOS/task.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>

void task_housekeep(void *arg)
{
	const int delay = (300000 / portTICK_RATE_MS); // every 5 minutes

	for (;;) {
		printf("Now performing Housekeeping task\n");
		fflush(stdout);
		vTaskDelay(delay);
	}

	for (;;) {
		vTaskDelay(10);
	}
}
