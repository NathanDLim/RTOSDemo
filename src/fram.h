/*
 * fram.h
 *
 *  Created on: Nov 10, 2017
 *      Author: Nathan Lim
 */

/*
 * This file contains the address offsets for the FRAM.
 */

#ifndef FRAM_H_
#define FRAM_H_

// The base address for addressing the FRAM
#define FRAM_BASE 0x00000
#define FRAM_ADD(x) (FRAM_BASE + x)

// Where the list of commands ends
#define FRAM_COMMAND_STACK_POINTER 	0x00
// Keep track of what errors have occurred
#define FRAM_ERROR_CODE 			0x01
// Last recorded GPS time
#define FRAM_GPS_TIME 				0x02
// Last recorded GPS location
#define FRAM_GPS_LOC 				0x03
// The command stack begins here
#define FRAM_COMMAND_STACK 			0x04

#endif /* FRAM_H_ */
