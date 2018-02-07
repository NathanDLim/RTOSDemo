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
#include "queue.h"

#include <stdio.h>

#include "obc.h"
#include "file_writer.h"

#define HOUSEKEEP_PACKET_SIZE 200

void task_housekeep(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;

	const int delay = (300000 / portTICK_RATE_MS); // every 5 minutes
	char packet[HOUSEKEEP_PACKET_SIZE];
	struct file_queue_message message;

	for (;;) {
		debug("Now performing Housekeeping task\n");
		// TODO: gather all of the data and create a housekeeping packet


		// get the timestamp for this housekeeping data
		long time = obc_get_timestamp();
		snprintf(packet, ARRAY_SIZE(packet), "%li", time);

		message.id = FOLDER_HOUSEKEEP;
		message.data = packet;
		message.size = HOUSEKEEP_PACKET_SIZE;
		snprintf(message.file_name, 10, "%li", time);
		if (xQueueSend(queue, (void *) &message, 0) == pdFALSE)
					error("Error sending housekeep queue message\n");
//		fflush(stdout);

		vTaskDelay(2000);
	}

	for (;;) {
		vTaskDelay(10);
	}
}
