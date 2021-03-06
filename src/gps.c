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
#include "error_check.h"

#include <stdio.h>
#include <sys\timeb.h>

static struct gps_data gps_data;
SemaphoreHandle_t gps_mutex;
// queue for sending errors
xQueueHandle error_queue;

void gps_init()
{
	if (gps_mutex == NULL) {
		gps_mutex = xSemaphoreCreateMutex();
	}
}

void task_gps(void *arg)
{
	error_queue = *(xQueueHandle *)arg;
	// Error Queue message
	struct queue_message em;
	em.id = GPS;

	struct timeb t;
	for (;;) {
		printf("GPS task running\n");

		// Read the GPS
		ftime(&t);
		// Update the gps_data
		if (gps_mutex != NULL && xSemaphoreTake(gps_mutex, 50) == pdTRUE) {
			gps_data.tick = xTaskGetTickCount();
			gps_data.time = t.time;
			xSemaphoreGive(gps_mutex);
		}

		// debugging testing the error queue
		em.data = 0x10;
		error_send_message(&error_queue, &em);

		vTaskDelay(50000);
	}


	em.data = 0x20;
	// Should never reach this point;
	for (;;) {
		error_set_fram(ERROR_TASK_FAIL);
		error_send_message(&error_queue, &em);
		vTaskDelay(1000);
	}
}


int gps_get_data(struct gps_data *data)
{
	if (gps_mutex != NULL && xSemaphoreTake(gps_mutex, 10) == pdTRUE) {
		data->time = gps_data.time;
		data->tick = gps_data.tick;
		uint8_t i;
		for (i = 0; i < ARRAY_SIZE(gps_data.pos); i++) {
			data->pos[i] = gps_data.pos[i];
			data->vel[i] = gps_data.vel[i];
		}
		xSemaphoreGive(gps_mutex);
		return 0;
	}

	error("ERROR: GPS semaphore take failure\n");

	return -1;
}

/*
 * Function: gps_get_timestamp
 *
 * This function returns the current timestamp based on the last data received by the GPS task.
 * In seconds.
 * TODO: Fix wrap around. What happens when the time overflows?
 *
 * in/out: pointer to gps_data struct where the current gps_data will be placed
 * out: returns 0 if successful. -1 if not successful.
 */
long gps_get_timestamp()
{
	if (gps_mutex != NULL && xSemaphoreTake(gps_mutex, 10) == pdTRUE) {
		int ticks_passed = xTaskGetTickCount() - gps_data.tick;
		int time = gps_data.time + ticks_passed / 1000;//portTICK_PERIOD_MS;
		xSemaphoreGive(gps_mutex);
		return time;
	}

	error("ERROR: GPS semaphore take failure\n");
	return -1;
}

int gps_get_julian_day()
{
	long timestamp = gps_get_timestamp();
	if (timestamp == -1)
		return -1;

	return (timestamp % 31557600) / 86400;
}

