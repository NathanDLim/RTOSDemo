/*
 * gps.h
 *
 *  Created on: Oct 17, 2017
 *      Author: admin
 */

struct gps_queue_message {
	int id;
	// The timestamp
	long time;
	// The tick at time of timestamp
	int tick;
	// Add in position data
};

struct gps_data {

};

void task_gps(void *arg);
