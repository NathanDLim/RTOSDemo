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

xSemaphoreHandle adc_mutex;

static struct adc_data adc_data;

void init_adc()
{
	if (adc_mutex == NULL)
		return;

	adc_mutex = xSemaphoreCreateMutex();
	if (adc_mutex == NULL)
		printf("Error creating ADC mutex!");
}

void task_attitude(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;
	struct queue_message message;

	//init_adc();
	for (;;) {
		if (xQueueReceive(queue, &message, 0) == pdTRUE) {
			// message received
			if (message.id == ADC_SUN_POINT) {
				printf("ADC switching to sun pointing\n");
				fflush(stdout);
			} else if (message.id == ADC_NADIR_POINT) {
				printf("ADC switching to nadir pointing\n");
				fflush(stdout);
			}

		}
		vTaskDelay(100);
	}
}


