/*
 * gps.h
 *
 *  Created on: Oct 17, 2017
 *      Author: admin
 *
 * TODO: Find another work around for propagating gps data so we don't use mutexes?
 */


struct gps_data {
	// The timestamp
	long time;
	// The tick at time of timestamp
	int tick;
	float pos[3];
	float vel[3];
};

void task_gps(void *arg);

/*
 * Function: gps_init
 *
 * Creates the mutex for the gps data. Must be called before gps_task is created, or any other gps functions are called
 * TODO: Should poll GPS to check if it is the expected module and there are responses.
 */
void gps_init();

/*
 * Function: gps_get_data
 *
 * This function copies the latest gps_data into the supplied pointer.
 *
 * in/out: pointer to gps_data struct where the current gps_data will be placed
 * out: returns 0 if successful. -1 if not successful.
 */
int gps_get_data(struct gps_data *data);

/*
 * Function: gps_get_timestamp
 *
 * This function returns the current timestamp based on the last data received by the GPS task and the number
 * 		of ticks that have occurred since then.
 * TODO: Fix wrap around. What happens when the time overflows?
 *
 * out: the current timestamp
 */
long gps_get_timestamp();

/*
 * Function: gps_get_julian_day
 *
 * out: Returns the Julian day number since the start of the year. From 0 to 365
 */
int gps_get_julian_day();
