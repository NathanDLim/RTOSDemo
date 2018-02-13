/*
 * comm.h
 *
 *  Created on: Feb 12, 2018
 *      Author: Nathan
 */

#ifndef COMM_H_
#define COMM_H_

// A list of the commands that can be sent to the adc task
enum comm_commands {
	// Add specific data to the downlink packet
	COMM_CMD_ADD_HOUSEKEEP,
	COMM_CMD_ADD_PAYLOAD,
	// Send the data
	COMM_CMD_TRANSMIT,
	COMM_CMD_MAX,
};

void task_comm(void *arg);

#endif /* COMM_H_ */
