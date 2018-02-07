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

/*
 * file_writer task.
 *
 * This task is meant to write to the SD card
 */
void task_file_writer(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;
	char name[30];
	struct file_queue_message message;

	// main task loop
	for (;;) {
		// Do a blocking read to wait for data to write
		// TODO: make blocking indefinite with portMAX_DELAY
		if (xQueueReceive(queue, &message, 500) == pdTRUE) {
			FILE *fp;
			switch(message.id) {
				case FOLDER_PAYLOAD:
					snprintf(name, ARRAY_SIZE(name), "payload/%s", message.file_name);
					break;
				case FOLDER_HOUSEKEEP:
					snprintf(name, ARRAY_SIZE(name), "housekeep/%s", message.file_name);
					break;
				default:
					error("ERROR: Invalid folder\n");
					continue;
			}
			debug("printing to file w/ name %s\n", name);
			fp = fopen(name, "w");
			if (fp == NULL) {
				error("ERROR: cannot open file %s\n", message.file_name);
				continue;
			}
			fprintf(fp, "%s", message.data);
			fclose(fp);
		}
	}
}
