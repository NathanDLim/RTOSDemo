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

//xSemaphoreHandle gps_mutex;

static struct gps_data gps_data;

void task_gps(void *arg)
{
	//gps_mutex = xSemaphoreCreateMutex();
	//if (gps_mutex == NULL)
	//	printf("Error creating GPS mutex!");

	for (;;) {
		printf("GPS task running\n");
		fflush(stdout);
		vTaskDelay(5000);
	}
}

void gps_get_data(struct gps_data data)
{
//	xSemaphoreTake(&gps_mutex, portMAX_DELAY);
//	memcpy(&data, &gps_data, sizeof(struct gps_data));
//	xSempaphoreGive(&gps_mutex);
}
