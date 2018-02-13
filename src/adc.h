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
	ADC_CMD_SUN_POINT,
	ADC_CMD_NADIR_POINT,
	// Set the reaction wheel's speed
	ADC_CMD_SET_REACT_SPEED,
	ADC_CMD_MAX,
};

void task_adc(void *arg);

#endif /* ADC_H_ */
