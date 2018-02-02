/*
 * adc.h
 *
 *  Created on: Oct 6, 2017
 *      Author: admin
 */

#ifndef ADC_H_
#define ADC_H_


// A list of the commands that can be sent to the adc task
enum adc_commands {
	// Configure the pointing target
	ADC_SUN_POINT,
	ADC_NADIR_POINT,
	// Set the reaction wheel's speed
	ADC_SET_REACT_SPEED,
	ADC_MAX_COMMAND,
};

void task_attitude(void *arg);
void task_detumble(void *arg);
void task_rebase_adc(void *arg);

#endif /* ADC_H_ */
