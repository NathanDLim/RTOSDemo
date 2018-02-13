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
	// TODO: Add in position data
	float pos[3];
	float vel[3];
};

struct gps_data {

};

void task_gps(void *arg);
