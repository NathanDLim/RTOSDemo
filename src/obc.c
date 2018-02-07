/*
 * obc.c
 *
 *  Created on: Oct 6, 2017
 *      Author: Nathan Lim
 */

//#include <hal/Utility/util.h>
//
//#include <FreeRTOS/FreeRTOS.h>
//#include <FreeRTOS/task.h>
//#include <FreeRTOS/queue.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "housekeep.h"
#include "adc.h"
#include "obc.h"
#include "gps.h"
#include "fram.h"

#include <stdio.h>


#define HIGH_PRIORITY		configMAX_PRIORITIES-1
#define MEDIUM_PRIORITY		configMAX_PRIORITIES-2
#define LOW_PRIORITY		configMAX_PRIORITIES-4

/* List of Priorities for all tasks on the system */
#define HOUSEKEEP_PRIORITY 		MEDIUM_PRIORITY
#define MODESWITCH_PRIORITY 	LOW_PRIORITY
#define ERROR_PRIORITY			MEDIUM_PRIORITY		// check and handle errors
#define ATTITUDE_PRIORITY 		MEDIUM_PRIORITY
#define DETUMBLE_PRIORITY 		MEDIUM_PRIORITY
#define REBASE_ADC_PRIORITY 	LOW_PRIORITY
#define GPS_PRIORITY 			MEDIUM_PRIORITY
#define PAYLOAD_RX_PRIORITY 	HIGH_PRIORITY 		// retrieve payload data
#define PAYLOAD_PRO_PRIORITY 	MEDIUM_PRIORITY		// process payload data
#define COMM_RX_PRIORITY 		MEDIUM_PRIORITY 	// receive and process commands
#define COMM_HANDLE_PRIORITY	MEDIUM_PRIORITY		// command timing setup
#define DATA_TX_PRIORITY 		HIGH_PRIORITY  		// frame and send data to ground
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
									0b100000000,	// Initial Mode
									0b0010011,	// Sun-pointing
									0b1110100,	// Nadir-pointing
									0b0001000		// Safety
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

static struct obc obc;

xQueueHandle adc_queue, gps_queue;

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
 * Return the timestamp. Calculated with the last update from GPS data.
 *
 * TODO: Fix wrap around. What happens when the time overflows?
 */
long obc_get_timestamp()
{
	int ticks_passed = xTaskGetTickCount() - obc.gps_tick;

	printf("Ticks passed = %i, gps_tick=%i\n", ticks_passed, obc.gps_tick);
	return obc.gps_time + ticks_passed * portTICK_PERIOD_MS;
}

/*
 * Performs operations necessary for each obc command
 */
int execute_obc_command(int cmd, long option)
{
	int task_id = cmd >> OBC_ID_TASK_BIT;
	debug("cmd = %x, cmd-1 = %x\n", cmd, task_id);
	// Check if there is more than one task bit sent
	if ((task_id & (task_id - 1)) != 0) {
		error("Error: More than one task bit set\n");
		return -1;
	}

	struct queue_message message;
	// The message ID being sent to the task is the bottom part of the command ID
	message.id = cmd & (OBC_ID_TASK_BIT - 1);
	message.data = option;

	switch (task_id) {
		// Command handling command
		case BIT(COMMAND):
			if (xQueueSend(adc_queue, (void *) &message, 0) == pdFALSE) {
				error("Error sending adc queue message\n");
				return -1;
			}
			// send message to adc to new target
			break;
		// ADC command
		case BIT(ATTITUDE):
			// check that the command number is acceptable by adc
			if (message.id >= ADC_CMD_MAX)
				return -1;

			if (xQueueSend(adc_queue, (void *) &message, 0) == pdFALSE) {
				error("Error sending adc queue message\n");
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
 */
void task_command_handler(void _UNUSED *arg)
{
	struct obc_command cmd;
	int ticks;
	struct gps_queue_message message;

	// Main loop
	for (;;) {
		// Check if there is a message from GPS updating the time.
		if (xQueueReceive(gps_queue, &message, 0) == pdTRUE) {
			obc.gps_time = message.time;
			obc.gps_tick = message.tick;
			printf("Setting gps_tick to %i\n", message.tick);
			printf("GPS tick = %i", obc.gps_tick);
		}
		//debug("Time running = %li\n", obc_get_timestamp());
		fflush(stdout);

		// read the command stack pointer and grab the next command
		if (obc.command_num == 0) {
			debug("no tasks to run\n");
			fflush(stdout);
			vTaskDelay(5000);
			continue;
		}
		cmd = obc.command_list[0];

		// if the command should be run now or delayed
		long timestamp = obc_get_timestamp();
		if (cmd.execution_time > timestamp) {
			vTaskDelay((cmd.execution_time - timestamp) / portTICK_PERIOD_MS);
			continue;
		}

		if (execute_obc_command(cmd.id, cmd.data) != 0){
			error("error executing command\n");
		}

		// remove the command just executed
		obc.command_list[0] = obc.command_list[obc.command_num - 1];
		obc.command_num--;

		// sort the command list
		sort_command_list();

		//vTaskDelay(100);
	}
}

#ifdef _DEBUG
void task_debug(void *arg)
{
	//xTaskHandle *hmode_switch = (xTaskHandle *)arg;
	unsigned int num = 0;
	struct queue_message message;

	for (;;) {
		/*
		printf("Debug Mode\n");
		printf("Enter a number to select the mode of operation\n");
		printf("1. Initial Mode\n");
		printf("2. Sun Pointing Mode\n");
		printf("3. Nadir Pointing Mode\n");
		printf("4. Safety Mode\n>> ");
		fflush(stdout);
		*/
		//while(UTIL_DbguGetIntegerMinMax(&num, 1, 4) == 0);

		//scanf("%d", &num);

		num = num > 4 ? 1 : num + 1;
		if (num < 1 || num > 4)
			continue;
		//obc.mode = num - 1;

		// test message
		message.id = 101;
		message.data = num;

		if (xQueueSend(adc_queue, (void *) &message, 0) == pdFALSE)
			error("Error sending adc queue message\n");

		debug("sending num = %i\n", num);
		fflush(stdout);

		//vTaskResume(*hmode_switch);
		vTaskDelay(2000);
	}
}
#endif

/*
 * Initializes the obc structure
 */
void obc_init(void)
{
	obc.mode = 1;

	/* List of test obc_commands */
	obc.command_list[0].id = (BIT(ATTITUDE) << OBC_ID_TASK_BIT) | 0;
	obc.command_list[0].data = 10;
	obc.command_list[0].execution_time = 100;

	obc.command_list[1].id = (BIT(ATTITUDE) << OBC_ID_TASK_BIT) | 2;
	obc.command_list[1].data = 80;
	obc.command_list[1].execution_time = 500;

	obc.command_list[2].id = (BIT(ATTITUDE) | BIT(COMMAND)) << OBC_ID_TASK_BIT;
	obc.command_list[2].data = 100;
	obc.command_list[2].execution_time = 3400;

	obc.command_list[3].id = (BIT(ATTITUDE) << OBC_ID_TASK_BIT) | 1;
	obc.command_list[3].data = 10;
	obc.command_list[3].execution_time = 8524;
	obc.command_num = 4;
}

/*
 * Creates all of the tasks
 */
void obc_main(void)
{
	obc_init();
//	xTaskHandle task_mode;
	adc_queue = xQueueCreate(5, sizeof(struct queue_message));
	gps_queue = xQueueCreate(1, sizeof(struct gps_queue_message));

	if (gps_queue == 0)
	{
		error("Could not create gps queue\n");
	}

	/* Create tasks */
	xTaskCreate(task_housekeep, (const char*)"housekeep", configMINIMAL_STACK_SIZE, NULL, HOUSEKEEP_PRIORITY, &task[HOUSEKEEP]);
	xTaskCreate(task_attitude, (const char*)"ADC", configMINIMAL_STACK_SIZE, (void *) &adc_queue, ATTITUDE_PRIORITY, &task[ATTITUDE]);
	xTaskCreate(task_gps, (const  char*)"GPS", configMINIMAL_STACK_SIZE, (void *) &gps_queue, GPS_PRIORITY, &task[GPS]);
	//xTaskCreate(mode_switching, (const char*)"Mode Switching", configMINIMAL_STACK_SIZE, NULL, LOW_PRIORITY, &task_mode);
	xTaskCreate(task_command_handler, (const char*)"Command Handler", configMINIMAL_STACK_SIZE, NULL, MEDIUM_PRIORITY, &task[COMMAND]);


#ifdef _DEBUG
	//xTaskCreate(task_debug, (const char*)"Debug", configMINIMAL_STACK_SIZE, &task_mode, configMAX_PRIORITIES-2, NULL);
#endif



	vTaskStartScheduler();
}
