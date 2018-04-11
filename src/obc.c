/*
 * obc.c
 *
 *  Created on: Oct 6, 2017
 *      Author: Nathan Lim
 *
 * TODO: Split this file up to move task creation into the main.cc
 */

//#include <hal/Utility/util.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "obc.h"
#include "comm.h"
#include "housekeep.h"
#include "adc.h"
#include "gps.h"
#include "fram.h"
#include "file_writer.h"
#include "error_check.h"

#include <stdio.h>
#include <sys\timeb.h>

#define HIGH_PRIORITY		configMAX_PRIORITIES-1
#define MEDIUM_PRIORITY		configMAX_PRIORITIES-2
#define LOW_PRIORITY		configMAX_PRIORITIES-4

/* List of Priorities for all tasks on the system */
#define HOUSEKEEP_PRIORITY 		MEDIUM_PRIORITY
#define MODESWITCH_PRIORITY 	LOW_PRIORITY
#define ERROR_PRIORITY			HIGH_PRIORITY		// check and handle errors
#define ATTITUDE_PRIORITY 		LOW_PRIORITY
#define GPS_PRIORITY 			MEDIUM_PRIORITY
#define PAYLOAD_PRO_PRIORITY 	MEDIUM_PRIORITY		// process payload data
#define COMM_RX_PRIORITY 		MEDIUM_PRIORITY 	// receive and process commands
#define COMM_HANDLE_PRIORITY	MEDIUM_PRIORITY		// command timing setup
#define COMM_PRIORITY 			HIGH_PRIORITY  		// frame and send data to ground
#define FILE_W_PRIORITY 		MEDIUM_PRIORITY		// write data to file
#define SD_PRIORITY				MEDIUM_PRIORITY		// send and retrieve SD card data

/* List of all possible Satellite modes */
enum mode {
	MODE_INITIAL,
	MODE_SUN_POINTING,
	MODE_NADIR_POINTING,
	MODE_SAFETY,
	MODE_MAX_NUM,
};

/* List of task handles. The entries match up with enum task. */
xTaskHandle task[9];

/*
 * Matrix for determining which tasks should be active during each mode.
 * The index of the masks follows enum mode defined in obc.h.
 *
 * ex. If mode_mask[1] refers to SUN_POINTING mode.
 * In order to stop task 0 from being run during sun pointing mode, and run all other tasks, we set
 * 		mode_mask[1] = 0b00000001
 *
 * The numbering of the tasks are shown in enum task, where each entry represents the bit number.
 */
const unsigned int mode_mask[] = {
									0b100000000,
									0b0010011,
									0b1110100,
									0b0001000
								 };

struct obc {
	/* Debugging list of obc_commands */
	struct obc_command command_list[10];
	// The number of commands in the command list
	int command_num;

	// Current mode of the satellite
	enum mode mode;
	// The last GPS update
	long gps_time;
	// The tick count at the GPS update time
	int gps_tick;
	// TODO: add GPS position data at last update
};

// Our OBC structure
static struct obc obc;

// Queues
xQueueHandle adc_queue, gps_queue, file_w_queue, comm_tx_queue, error_queue;

/*
 * Task for switching which mode the satellite is in.
 *
 * It suspends all of the tasks that are not needed in that mode.
 */
void mode_switching(void _UNUSED *arg) {
	size_t i;

	// loop forever
	for (;;) {
		debug("now switching to mode %i\n", obc.mode);
		for (i = 0; i < ARRAY_SIZE(task); ++i) {
			if (task[i] == 0)
				continue;
			else if (mode_mask[obc.mode] & BIT(i)) {
				vTaskResume(task[i]);
			} else {
				vTaskSuspend(task[i]);
			}
		}

		// once the mode has been changed, we can suspend this task until the next time
		// we need to change modes
		vTaskSuspend(NULL);
	}
}

/*
 * Function to sort the obc commands in order of execution time
 * TODO: Remove duplicate ID numbers
 */
void sort_command_list()
{
	int i, flag = 1;
	struct obc_command tmp;

	/* Simple bubble sort to sort commands in terms of execution time */
	while(flag != 0) {
		flag = 0;
		for (i = 0; i < obc.command_num - 1; i++) {
			if (obc.command_list[i].execution_time > obc.command_list[i + 1].execution_time) {
				tmp = obc.command_list[i];
				obc.command_list[i] = obc.command_list[i + 1];
				obc.command_list[i + 1] = tmp;
				flag = 1;
			}

		}
	}

	// debug print the command list
//	for (i = 0; i < obc.command_num; i++) {
//		printf("cmd type: %i, time:%li, data:%i\n", obc.command_list[i].id, obc.command_list[i].execution_time, obc.command_list[i].data);
//	}
//	fflush(stdout);
}

/*
 * Performs operations necessary for each obc command
 */
int execute_obc_command(uint16_t cmd, long option)
{
	uint16_t task_id = cmd >> OBC_CODE_TASK_BIT;
//	debug("cmd = %x, cmd-1 = %x\n", cmd, task_id);
	// Check if there is more than one task bit sent
	if ((task_id & (task_id - 1)) != 0) {
		error("Error: More than one task bit set\n");
		return -1;
	}

	struct queue_message message;
	// The message ID being sent to the task is the bottom part of the command ID
	message.id = cmd & (OBC_CODE_TASK_BIT - 1);
	message.data = option;

	switch (task_id) {
		// Command handling command
		case BIT(CONTROL):
			debug("Doing Control work\n");
			break;
		// ADC command
		case BIT(ADC):
			// check that the command number is acceptable by adc
			if (message.id >= ADC_CMD_MAX)
				return -1;

			if (xQueueSend(adc_queue, (void *) &message, 0) == pdFALSE) {
				error("Error sending adc queue message\n");
				return -1;
			}
			break;
		case BIT(COMM_TX):
			if (xQueueSend(comm_tx_queue, (void *) &message, 0) == pdFALSE) {
				error("Error sending comm tx queue message\n");
				return -1;
			}
			break;
		default:
			return -1;
	}
	return 0;

}

/*
 * Task to execute obc commands at the appropriate time.
 *
 * TODO: There should be some obc_commands added in to do clean up and data management from time to time.
 */
void task_command_handler(void _UNUSED *arg)
{
	struct obc_command cmd;
	struct queue_message em;
	em.id = CONTROL;
	char buffer[2048];

	// Main loop
	for (;;) {
//		vTaskGetRuntimeStats();
		vTaskList(buffer);
		printf(buffer);
//		printf("time: %li\n", gps_get_timestamp());
		// read the command stack pointer and grab the next command
		if (obc.command_num == 0) {
			debug("no tasks to run\n");
			fflush(stdout);
			vTaskDelay(5000);
			continue;
		}

		cmd = obc.command_list[0];

		// if the command should be run now or delayed
		long timestamp = gps_get_timestamp();
		if (cmd.execution_time > timestamp) {
//			debug("delay for %i s\n", (cmd.execution_time - timestamp) / portTICK_PERIOD_MS * 1000);
			vTaskDelay((cmd.execution_time - timestamp) / portTICK_PERIOD_MS * 1000);
			continue;
		}

		// run the actual command
		if (execute_obc_command(cmd.id, cmd.data) != 0){
			error("error executing command\n");
			error_send_message(&error_queue, &em);
		}

		// TODO: await a confirmation that the task was complete before removing it?
		// remove the command just executed
		obc.command_list[0] = obc.command_list[obc.command_num - 1];
		obc.command_num--;

		// sort the command list
		sort_command_list();

		//vTaskDelay(100);
	}

	// Should never reach this point;
	for (;;) {
		error_set_fram(ERROR_TASK_FAIL);
		error_send_message(&error_queue, &em);
		vTaskDelay(1000);
	}
}

#ifdef _DEBUG

#endif

/*
 * Initializes the obc structure
 */
void obc_init(void)
{
	gps_init();
	obc.mode = 1;

	/* This part is all for debugging */
	struct timeb t;
	ftime(&t);

	long start = t.time;

	/* List of test obc_commands */
	obc.command_list[0].id = (BIT(ADC) << OBC_CODE_TASK_BIT) | 0;
	obc.command_list[0].data = 10;
	obc.command_list[0].execution_time = start + 2;

	obc.command_list[1].id = (BIT(COMM_TX) << OBC_CODE_TASK_BIT) | 0;
	obc.command_list[1].data = 43 << 24;
	obc.command_list[1].execution_time = start + 5;

	obc.command_list[2].id = (BIT(ADC) | BIT(CONTROL)) << OBC_CODE_TASK_BIT;
	obc.command_list[2].data = 100;
	obc.command_list[2].execution_time = start + 12;

	obc.command_list[3].id = (BIT(ADC) << OBC_CODE_TASK_BIT) | 1;
	obc.command_list[3].data = 10;
	obc.command_list[3].execution_time = start + 4;
	obc.command_num = 4;

//	int i;
//	for (i = 0; i < 4; i++)
//		debug("%i:%li\n", i, obc.command_list[i].execution_time);
}

/*
 * Creates all of the tasks
 */
void obc_main(void)
{
	obc_init();
//	xTaskHandle task_mode;

	/* Create queues */
	adc_queue = xQueueCreate(5, sizeof(struct queue_message));
//	gps_queue = xQueueCreate(1, sizeof(struct gps_queue_message));
	file_w_queue = xQueueCreate(10, sizeof(struct file_queue_message));
	comm_tx_queue = xQueueCreate(10, sizeof(struct queue_message));
	error_queue = xQueueCreate(10, sizeof(struct queue_message));

	/* Declare the input queues to each task
	 * TODO: use one multiqueue and change the queues after creating the task?
	 */
	struct multi_queue housekeep_queues;
	housekeep_queues.q[0] = &file_w_queue;
	housekeep_queues.q[1] = &error_queue;
	housekeep_queues.num = 2;

	struct multi_queue adc_queues;
	adc_queues.q[0] = &adc_queue;
	adc_queues.q[1] = &error_queue;
	adc_queues.num = 2;

	struct multi_queue file_w_queues;
	file_w_queues.q[0] = &file_w_queue;
	file_w_queues.q[1] = &error_queue;
	file_w_queues.num = 2;

	struct multi_queue comm_tx_queues;
	comm_tx_queues.q[0] = &comm_tx_queue;
	comm_tx_queues.q[1] = &error_queue;
	comm_tx_queues.num = 2;

	/* Create tasks */
	xTaskCreate(task_housekeep, (const char*)"Housekp", configMINIMAL_STACK_SIZE, (void *) &housekeep_queues, HOUSEKEEP_PRIORITY, &task[HOUSEKEEP]);
	xTaskCreate(task_adc, (const char*)"ADC", configMINIMAL_STACK_SIZE*3, (void *) &adc_queues, ATTITUDE_PRIORITY, &task[ADC]);
	xTaskCreate(task_gps, (const  char*)"GPS", configMINIMAL_STACK_SIZE * 2, (void *) &error_queue, GPS_PRIORITY, &task[GPS]);
	//xTaskCreate(mode_switching, (const char*)"Mode Switching", configMINIMAL_STACK_SIZE, NULL, LOW_PRIORITY, &task_mode);
	xTaskCreate(task_command_handler, (const char*)"Control", configMINIMAL_STACK_SIZE*10, NULL, MEDIUM_PRIORITY, &task[CONTROL]);
	xTaskCreate(task_file_writer, (const char*)"File Writer", configMINIMAL_STACK_SIZE, (void *) &file_w_queues, FILE_W_PRIORITY, &task[FILE_W]);
	xTaskCreate(task_comm, (const char*)"Comm Down", configMINIMAL_STACK_SIZE, (void *) &comm_tx_queues, COMM_PRIORITY, &task[COMM_TX]);
	xTaskCreate(task_error_check, (const char*)"Error", configMINIMAL_STACK_SIZE, (void *) &error_queue, ERROR_PRIORITY, &task[ERROR_CHK]);


#ifdef _DEBUG
	//xTaskCreate(task_debug, (const char*)"Debug", configMINIMAL_STACK_SIZE, &task_mode, configMAX_PRIORITIES-2, NULL);
#endif

	vTaskStartScheduler();
}
