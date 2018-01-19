/*
 * obc.h
 *
 *  Created on: Oct 6, 2017
 *      Author: admin
 */


// choose whether system goes into debug mode or not
#define _DEBUG

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
	INITIAL,
	SUN_POINTING,
	NADIR_POINTING,
	SAFETY,
	NUM_MODES,
};

// All potential obc commands
enum obc_command {
	SUN_POINT,
	NADIR_POINT,
	BEGIN_IMAGE,
	DOWNLINK,
	EDIT_PARAM,
	MAX_CMD,
};

struct queue_message {
	int id;
	int data;
};

void obc_main(void);

