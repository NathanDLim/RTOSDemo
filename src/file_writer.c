/*
 * file_writer.c
 *
 *  Created on: Feb 7, 2018
 *      Author: Nathan
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>

#include "file_writer.h"
#include "obc.h"
#include "error_check.h"

/*
 * file_writer task.
 *
 * This task is meant to write to the SD card
 */
void task_file_writer(void *arg)
{
	struct multi_queue mq = *(struct multi_queue *)arg;
	if (mq.num != 2) {
		error("File_w input arg incorrect\n");
		return;
	}
	xQueueHandle file_queue = *mq.q[0];
	xQueueHandle error_queue = *mq.q[1];

	// Error Queue message
	struct queue_message em;
	em.id = ERROR_CHK;

	char name[30];
	struct file_queue_message message;

	// main task loop
	for (;;) {
		// Do a blocking read to wait for data to write
		// TODO: make blocking indefinite with portMAX_DELAY
		if (xQueueReceive(file_queue, &message, 500) == pdTRUE) {
			FILE *fp;
			switch(message.id) {
				case FOLDER_PAYLOAD:
					snprintf(name, ARRAY_SIZE(name), "payload/payload%s", message.file_name);
					break;
				case FOLDER_HOUSEKEEP:
					snprintf(name, ARRAY_SIZE(name), "housekeep/housekeep_%s", message.file_name);
					break;
				default:
					error("ERROR: Invalid folder\n");
					continue;
			}
			debug("printing to file w/ name %s\n", name);
			fp = fopen(name, "a");
			if (fp == NULL) {
				error("ERROR: cannot open file %s\n", name);
				continue;
			}
			fprintf(fp, "%s", message.data);
			fclose(fp);
		}
	}

	// Should never reach this point;
	for (;;) {
		error_set_fram(ERROR_TASK_FAIL);
		error_send_message(&error_queue, &em);
		vTaskDelay(1000);
	}
}
