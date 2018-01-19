/*
 * adc.h
 *
 *  Created on: Oct 6, 2017
 *      Author: admin
 */

#ifndef ADC_H_
#define ADC_H_

#define ADC_SUN_POINT 101
#define ADC_NADIR_POINT 202

/* Stores all of the data necessary for ADC  */
struct adc_data {

};

struct attitude {

};

void task_attitude(void *arg);
void task_detumble(void *arg);
void task_rebase_adc(void *arg);

#endif /* ADC_H_ */
