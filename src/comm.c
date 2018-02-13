/*
 * comm.c
 *
 *  Created on: Nov 10, 2017
 *      Author: admin
 */


/*
 * Task here which will receive the incoming commands from ground and parse them into instructions.
 * The instructions will then be saved into the FRAM (non-volatile memory).
 */

/*
 * Task to read commands from the FRAM and schedule when each command should be activated
 * Once a command is executed, it is deleted from FRAM. If task cannot be accomplished (ie. time already passed), we discard it.
 *
 * This should be run when powered on, in case the OBC lost power and needs to reload all of its scheduled tasks.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>

#include "obc.h"
#include "comm.h"

void task_comm(void *arg)
{
	xQueueHandle queue = *(xQueueHandle *)arg;
	struct queue_message message;
	char name[32];
	char buf[200];

	for (;;) {
		if (xQueueReceive(queue, &message, 500) == pdTRUE) {
			uint8_t day = message.data >> 24;

			switch (message.id) {
				case COMM_CMD_ADD_HOUSEKEEP:
					snprintf(name, ARRAY_SIZE(name), "housekeep/housekeep_%04d", day);
					debug("Comm adding housekeeping data from file %s\n", name);
					break;
				case COMM_CMD_ADD_PAYLOAD:
					debug("Comm adding payload data from day %i\n", day);
					break;
				case COMM_CMD_TRANSMIT:
					debug("Comm transmitting data");
					break;
				default:
					error("Error in Comm message");
					break;
			}

//			FILE *fp;
//			fp = fopen(name, "r");
//			if (fp == NULL) {
//				error("ERROR: file not found\n");
//				continue;
//			}
//
//			 while (fgets(buf, ARRAY_SIZE(buf), fp) != NULL) {
//				 printf(buf);
//			 }
//			 fflush(stdout);

		}

		vTaskDelay(1000);
	}
}
