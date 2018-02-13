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
#include "obc.h"

#include <stdio.h>

static struct gps_data gps_data;
SemaphoreHandle_t gps_mutex;

void gps_init()
{
	if (gps_mutex == NULL) {
		gps_mutex = xSemaphoreCreateMutex();
	}
}

void task_gps(void _UNUSED *arg)
{


	for (;;) {
		printf("GPS task running\n");

		// Read the GPS

		// Update the gps_data
		if (gps_mutex != NULL && xSemaphoreTake(gps_mutex, 50) == pdTRUE) {
			gps_data.tick = xTaskGetTickCount();
			gps_data.time = xTaskGetTickCount() * portTICK_PERIOD_MS;
			xSemaphoreGive(gps_mutex);
		}

		vTaskDelay(5000);
	}

	// Should never reach this point;
	for (;;) ;
}

/*
 * Return the timestamp. Calculated with the last update from GPS data.
 *
 * TODO: Fix wrap around. What happens when the time overflows?
 */
long gps_get_timestamp()
{
	if (gps_mutex != NULL && xSemaphoreTake(gps_mutex, 10) == pdTRUE) {
		int ticks_passed = xTaskGetTickCount() - gps_data.tick;
		int time = gps_data.time + ticks_passed * portTICK_PERIOD_MS;

		xSemaphoreGive(gps_mutex);
		return time;
	}

	error("ERROR: GPS semaphore take failure\n");

	return 0;
}

