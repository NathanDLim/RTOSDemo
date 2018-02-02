/*
 * obc.h
 *
 *  Created on: Oct 6, 2017
 *      Author: admin
 */


// choose whether system goes into debug mode or not
//#define _DEBUG

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

/* List of all possible Satellite modes */
enum mode {
	MODE_INITIAL,
	MODE_SUN_POINTING,
	MODE_NADIR_POINTING,
	MODE_SAFETY,
	MODE_MAX_NUM,
};

// All potential obc commands
enum obc_command_type {
	CMD_SUN_POINT,
	CMD_NADIR_POINT,
	CMD_BEGIN_IMAGE,
	CMD_DOWNLINK,
	CMD_EDIT_PARAM,
	CMD_MAX_NUM,
};

/* List of tasks on the system that can be turned on and off. */
enum task {
	// command control task
	COMMAND,
	// calculate attitude controls
	ATTITUDE,
	// process payload data
	PAYLOAD,
	// receive communication data
	COMM_RX,
	// send data to ground station
	DATA_TX,
	// gather GPS data
	GPS,
	// gather and store sensor data
	HOUSEKEEP,
	NUM_TASKS,
};

/*
 * The struct which holds a command from ground station.
 *
 * 'id' is an integer with two parts. The top part is to identify which task the command relates to, and
 * the bottom part identifies a specific action related to that task. The top part should only have one bit set,
 * and the set bit indicates which task it goes to. The bit number is based off the 'task' enum.
 *
 * The 'OBC_ID_TASK_BIT' macro indicates where the split in the id occurs.
 */
struct obc_command {
	int id;
	long execution_time;
	int data;
};

struct queue_message {
	int id;
	int data;
};

void obc_main(void);
