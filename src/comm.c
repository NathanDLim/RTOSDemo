/*
 * comm.c
 *
 *  Created on: Nov 10, 2017
 *      Author: admin
 */


/*
 * Task here which will receive the incoming commands from ground and parse them into instructions.
 * The instructions will then be saved into the FRAM (non-volatile memory).
 */

/*
 * Task to read commands from the FRAM and schedule when each command should be activated
 * Once a command is executed, it is deleted from FRAM. If task cannot be accomplished (ie. time already passed), we discard it.
 *
 * This should be run when powered on, in case the OBC lost power and needs to reload all of its scheduled tasks.
 */
