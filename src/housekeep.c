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
#include "gps.h"
#include "file_writer.h"
#include "error_check.h"

#define HOUSEKEEP_PACKET_SIZE 200

void task_housekeep(void *arg)
{
	struct multi_queue mq = *(struct multi_queue *)arg;
	if (mq.num != 2) {
		error("Housekeep input arg incorrect\n");
		return;
	}
	xQueueHandle file_queue = *mq.q[0];
	xQueueHandle error_queue = *mq.q[1];

	// Error Queue message
	struct queue_message em;
	em.id = HOUSEKEEP;

	const int delay = (300000 / portTICK_RATE_MS); // every 5 minutes
	char packet[HOUSEKEEP_PACKET_SIZE];
	struct file_queue_message message;
	int day_of_year;


	// TODO: Wait until time is an even multiple of housekeeping delay before starting
	// NOTE: The debugging uses a lower housekeeping period
	int sec = gps_get_timestamp() % 5;
	vTaskDelay((5 - sec) / portTICK_RATE_MS);

	for (;;) {
		debug("Now performing Housekeeping task\n");
		// TODO: gather all of the data and create a housekeeping packet

		// get the timestamp for this housekeeping data
		long time = gps_get_timestamp();

		// place the housekeep data in the buffer
		snprintf(packet, ARRAY_SIZE(packet), "%i:%i:%i Housekeeping data!!\n", (time % 86400)/3600, (time % 3600) / 60, time % 60);

		message.id = FOLDER_HOUSEKEEP;
		message.data = packet;
		message.size = HOUSEKEEP_PACKET_SIZE;
		snprintf(message.file_name, ARRAY_SIZE(message.file_name), "%03d", day_of_year);
		if (xQueueSend(file_queue, (void *) &message, 0) == pdFALSE)
					error("Error sending housekeep queue message\n");
//		fflush(stdout);

		vTaskDelay(20000);
	}

	// Should never reach this point;
	for (;;) {
		error_set_fram(ERROR_TASK_FAIL);
		error_send_message(&error_queue, &em);
		vTaskDelay(1000);
	}
}
