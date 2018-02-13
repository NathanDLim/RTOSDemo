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
	// The timestamp
	long time;
	// The tick at time of timestamp
	int tick;
	// TODO: Add in position data
	float pos[3];
	float vel[3];
};

void task_gps(void *arg);

/*
 * Function: gps_init
 *
 * Creates the mutex for the gps data. Must be called before gps_task is created, or gps_get_timestamp.
 * TODO: Should poll GPS to check if it is the expected module and there are responses.
 */
void gps_init();

/*
 * Function: gps_get_timestamp
 *
 * This function returns the current timestamp based on the last data received by the GPS task.
 * TODO: Find another work around for this function so we don't use mutexes?
 */
long gps_get_timestamp();

