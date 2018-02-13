/*
 * obc.h
 *
 *  Created on: Oct 6, 2017
 *      Author: admin
 */


// choose whether system goes into debug mode or not
#define _DEBUG

#define _UNUSED __attribute__((__unused__))

/*
 * Bit selection.
 * BIT(0) = 0b0001
 * BIT(1) = 0b0010
 * BIT(2) = 0b0100
 * ... etc
 */
#define BIT(x) (1 << (x))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// Defines which bit in the obc_command.id begins referencing the task id
#define OBC_ID_TASK_BIT 4

#ifdef _DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

#define error(...) printf(__VA_ARGS__)

// All potential obc commands
enum obc_command_type {
	OBC_CMD_SUN_POINT,
	OBC_CMD_NADIR_POINT,
	OBC_CMD_BEGIN_IMAGE,
	OBC_CMD_DOWNLINK,
	OBC_CMD_EDIT_PARAM,
	OBC_CMD_MAX,
};

/* List of tasks on the system that can be turned on and off. */
enum task {
	// command control task
	CONTROL,
	// perform all adc functions
	ADC,
	// process payload data
	PAYLOAD,
	// receive communication data
	COMM_RX,
	// send data to ground station
	COMM_TX,
	// gather GPS data
	GPS,
	// gather and store sensor data
	HOUSEKEEP,
	// write data to file
	FILE_W,
	NUM_TASKS,
};

/*
 * The struct which holds a command from ground station.
 *
 * 'id' is an unsigned integer with two parts. The top part is to identify which task the command relates to, and
 * the bottom part identifies a specific action related to that task. The top part should only have one bit set,
 * and the set bit indicates which task it goes to. The bit number is based off the 'task' enum.
 *
 * The 'OBC_ID_TASK_BIT' macro indicates where the split in the id occurs.
 */
struct obc_command {
	uint16_t id;
	long execution_time;
	long data;
};

/*
 * A generic queue message
 */
struct queue_message {
	int id;
	long data;
};

/*
 * Function: obc_main
 *
 * This is the function that creates all obc tasks and their associated queues.
 */
void obc_main(void);


