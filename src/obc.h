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

struct obc_command {
	enum obc_command_type type;
	long execution_time;
	int data;
};

struct queue_message {
	int id;
	int data;
};

void obc_main(void);

