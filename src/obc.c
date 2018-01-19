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



/* List of tasks on the system that can be turned on and off. */
enum task {
	// gather and store sensor data
	HOUSEKEEP,			// 0
	// calculate attitude controls
	ATTITUDE,			// 1
	// stabilize satellite
	DETUMBLE,			// 2
	// rebase attitude controls from current GPS data
	REBASE_ADC,			// 3
	// gather GPS data
	GPS,				// 4
	// receive data from payload
	PAYLOAD_RX,			// 5
	// process payload data
	PAYLOAD_PRO,		// 6
	// receive communication data
	COMM_RX,			// 7
	// handle communication commands
	COMM_HANDLE,		// 8
	// send data to ground station
	DATA_TX,			//9
	NUM_TASKS,
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

int obc_mode;
xQueueHandle adc_queue;

void mode_switching(void *arg) {
	size_t i;

	// loop forever
	for (;;) {
		printf("now switching to mode %i\n", obc_mode);
		for (i = 0; i < ARRAY_SIZE(task); ++i) {
			if (task[i] == 0)
				continue;
			else if (mode_mask[obc_mode] & BIT(i)) {
				vTaskResume(task[i]);
				//printf("resume task %i\n", i + 1);
			} else {
				vTaskSuspend(task[i]);
				//printf("suspend task %i\n", i + 1);
			}
		}
		//fflush(stdout);

		// once the mode has been changed, we can suspend this task until the next time
		// we need to change modes
		vTaskSuspend(NULL);
	}
}

/*
 * Performs operations necessary for each obc command
 */
int execute_obc_command(enum obc_command cmd, int option)
{
	if (cmd >= MAX_CMD)
		return -1;

	switch (cmd) {
		case SUN_POINT:
			// send message to adc to new target
		case NADIR_POINT:
			// send message to adc to new target
		case BEGIN_IMAGE:
			// send message to payload to begin imaging
		case DOWNLINK:
			// send message to communications to compile downlink packet and send it
		case EDIT_PARAM:
			// change a parameter in the FRAM
		default:
			return -1;
	}
	return 0;

}

void task_command_handler(void *arg)
{
	unsigned char deviceID[9] = {0};
	unsigned char FRAMread[10] = {0};
	int retVal, i;

	// Initialization and error checking
	/*
	retVal = FRAM_start();
	if(retVal != 0) {
		TRACE_WARNING(" Error during FRAM_start: %d \n\r", retVal);
		while(1);
	}

	retVal = FRAM_getDeviceID(deviceID);
	if(retVal != 0) {
		TRACE_WARNING(" Error during FRAM_protectBlocks: %d \n\r", retVal);
		while(1);
	}

	TRACE_DEBUG_WP("Device ID: ");
	for(i=0; i<sizeof(deviceID); i++) {
		TRACE_DEBUG_WP("0x%02X ", deviceID[i]);
	}
	TRACE_DEBUG_WP("\n\r");
	*/

	// Main loop
	for (;;) {
		// read the command stack pointer and grab the next command
		/*
		retVal = FRAM_read(FRAMread, FRAM_COMMAND_STACK_POINTER, ARRAY_SIZE(FRAMread));
		if(retVal != 0) {
			TRACE_WARNING(" Error during FRAM_read: %d \n\r", retVal);
			while(1);
		}

		// check the time stamp for the next command and either execute it or delay

		// debug print out the command
		for(i=0; i<ARRAY_SIZE(FRAMread); i++) {
			TRACE_DEBUG_WP("0x%X, ", FRAMread[i]);
		}
		*/

		vTaskSuspend(NULL);
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
		//obc_mode = num - 1;

		// test message
		message.id = 101;
		message.data = num;

		if (xQueueSend(adc_queue, (void *) &message, 0) == pdFALSE)
			printf("Error sending adc queue message\n");

		printf("sending num = %i\n", num);
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
	obc_mode = 1;
}

/*
 * Creates all of the tasks
 */
void obc_main(void)
{
	obc_init();
	xTaskHandle task_mode;
	adc_queue = xQueueCreate(10, sizeof(struct queue_message));

	xTaskCreate(task_housekeep, (const char*)"housekeep", configMINIMAL_STACK_SIZE, NULL, HOUSEKEEP_PRIORITY, &task[HOUSEKEEP]);
	xTaskCreate(task_attitude, (const char*)"ADC", configMINIMAL_STACK_SIZE, (void *) &adc_queue, ATTITUDE_PRIORITY, &task[ATTITUDE]);
	xTaskCreate(task_gps, (const  char*)"GPS", configMINIMAL_STACK_SIZE, NULL, GPS_PRIORITY, &task[GPS]);
	xTaskCreate(mode_switching, (const char*)"Mode Switching", configMINIMAL_STACK_SIZE, NULL, LOW_PRIORITY, &task_mode);
	xTaskCreate(task_command_handler, (const char*)"Command Handler", configMINIMAL_STACK_SIZE, NULL, MEDIUM_PRIORITY, &task[COMM_HANDLE]);


#ifdef _DEBUG
	xTaskCreate(task_debug, (const char*)"Debug", configMINIMAL_STACK_SIZE, &task_mode, configMAX_PRIORITIES-2, NULL);
#endif



	vTaskStartScheduler();
}
