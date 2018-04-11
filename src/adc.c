/*
 * adc.c
 *
 *  Created on: Oct 6, 2017
 *      Author: admin
 */

//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//#include <freertos/semphr.h>
//#include <freertos/queue.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#include "adc.h"
#include "obc.h"
#include "gps.h"
#include "error_check.h"

#include "support/rt_nonfinite.h"
#include "support/rtwtypes.h"
#include "support/inertial_vectors_initialize.h"
#include "support/inertial_vectors.h"


void TRIAD(const double sb[3], const double mb[3], const double si[3], const
           double mi[3], double C_BI[9]);

// TEST INPUTS (from GPS and from SENSORS)
	  double dv1[30] = {
		  -6892.262655,	-1091.380544,	0,
		  -6892.418879,	-1090.367788,	7.488073159,
		  -6892.567019,	-1089.353752,	14.97613753,
		  -6892.707073,	-1088.338439,	22.46418434,
		  -6892.839041,	-1087.321849,	29.9522048,
		  -6892.962924,	-1086.303984,	37.44019012,
		  -6893.078721,	-1085.284845,	44.92813152,
		  -6893.186433,	-1084.264432,	52.41602021,
		  -6893.286057,	-1083.242747,	59.90384742,
		  -6893.377596,	-1082.219792,	67.39160436}; //position

	  int K; //clock

	  double dv2[30] = {
		      -0.904788993,	-0.266557484,	-0.332120438,
			  -0.158898044,	0.905840784,	0.392688026,
			  -0.158898231, 0.905840739,	0.392688053,
			  0.139606603,	0.779877265,	0.610165099,
			  0.080258652,	0.748039950,	0.658782803,
			  -0.005178045,	0.752570215,	0.658491655,
			  -0.011778743,	0.765570454,	0.643244231,
			  0.004726984,	0.767669656,	0.640828335,
			  0.011649219,	0.764444995,	0.644583699,
			  0.00801855,	0.763472328,	0.645790761}; //sb

	  double dv3[30] = {
			 -0.460280158,	-0.346641751,	0.817301458,
			 -0.050195684,	-0.173068567,	0.983629841,
			 -0.047171104,	-0.172589631,	0.983863662,
			 0.174281723,	-0.422494718,	0.889451569,
			 0.136299439,	-0.466627476,	0.873888587,
			 0.188600636,	-0.447741661,	0.874046455,
			 0.205439026,	-0.426516086,	0.880839847,
			 0.211799965,	-0.425966092,	0.879598581,
			 0.212919071,	-0.431490109,	0.876630912,
			 0.215205615,	-0.431810294,	0.875914615}; // mb

/*
 * The Attitude and Determination Control (ADC) task.
 *
 * Data owned:
 *
 * In: A handle to double queue holding the adc_queue and the error_queue. All messaged to the adc task come through the adc_queue
 */
void task_adc(void *arg)
{
	/* Queue set up */
	struct multi_queue mq = *(struct multi_queue *)arg;
	if (mq.num != 2) {
		error("ADC input arg incorrect\n");
		return;
	}
	xQueueHandle adc_queue = *mq.q[0];
	xQueueHandle error_queue = *mq.q[1];
	// TODO: make one queue message?
	struct queue_message message;
	// Error queue message
	struct queue_message em;
	em.id = ADC;

	long timestamp;
	int arr_offset = 0;
	int i;


	/* Triad algorithm set up */
	rt_InitInfAndNaN(8U);
	  inertial_vectors_initialize();
	double si[3];
	double mi[3];
	double sb[3];
	double mb[3];
	double r[3];
	double C_BI[9];

	/* ADC task loop */
	for (;;) {

		// We do a non-blocking check to see if there is a message waiting in the queue
		if (xQueueReceive(adc_queue, &message, 0) == pdTRUE) {
			switch (message.id) {
				case ADC_CMD_SUN_POINT:
					debug("ADC switching to sun pointing\n");
					break;
				case ADC_CMD_NADIR_POINT:
					debug("ADC switching to nadir pointing\n");
					break;
				case ADC_CMD_SET_REACT_SPEED:
					debug("Setting the reaction wheel speed to %li\n", message.data);
					break;
				default:
					error("Error in ADC message");
					break;
			}
		}


//		printf("time = %li\n", gps_get_timestamp());
//		fflush(stdout);
//		error_send_message(&error_queue, &em);
//		vTaskDelay(1000);


		/* The following is test TRIAD algorithm code */
		int i0;

			  for (K = 0; K < 10; K=K+3) {
			    for (i0 = 0; i0 < 3; i0++) {
			    	r[i0] = dv1[K+i0];
			    	sb[i0] = dv2[K+i0];
			    	mb[i0] = dv3[K+i0];
				 }

				// Check if satellite is in eclipse
				if (sb[0]==0 && sb[1]==0 && sb[2]==0) {
					printf("satellite is in eclipse");
				}
				else{
					/* Get inertial vectors*/
					inertial_vectors(K, r, si, mi);
					/*  Call TRIAD here */
					TRIAD(si, mi, sb, mb, C_BI);

					/*  get timestamp from GPS */

					/*  print time and Rotation Matrix */
					// print the time from GPS
					printf("\n\nRotation Matrix: \n");
					int i = 0;
					for (i = 0; i < 9; i++) {
						printf("%.4f ", C_BI[i]);
						if (i == 2 || i==5 || i==8) {
							printf("\n");
						}
					}
				}
			  } // Go to next time

	    /* End of test TRIAD algorithm

	 	/*  get timestamp from GPS */
	 	timestamp = gps_get_timestamp();

	 	vTaskDelay(500);
	}

	// Should never reach this point;
	for (;;) {
		error_set_fram(ERROR_TASK_FAIL);
		error_send_message(&error_queue, &em);
		vTaskDelay(1000);
	}
}


/* Function Definitions */

/*
 * TRIAD_test function performs the TRIAD algorithm for attitude
 * determination given sensor and propagator data
 * Arguments    : const double sb[3]
 *                const double mb[3]
 *                const double si[3]
 *                const double mi[3]
 *                double C_BI[9]
 * Return Type  : void
 */
void TRIAD(const double sb[3], const double mb[3], const double si[3], const
           double mi[3], double C_BI[9])
{
  double b_sb[9];
  double b_si[9];
  int i0;
  double s_cross_m_b[3];
  double s_cross_m_i[3];
  int i1;
  double x;
  double dv0[9];
  double t1_cross_t2_b[3];
  double b_x;
  double dv1[9];
  double t1_cross_t2_i[3];
  int i2;

  /* Inputs: */
  /* sb - Sun vector in body fixed reference frame */
  /* mb - Magnetic field vector in body fixed reference frame */
  /* si - Sun vector in ECI */
  /* mi - Magnetic field vector in ECI */
  /* Outputs: */
  /* C_BI - The attitude matrix/rotation matrix between ECI and body fixed */
  /* Comments: */
  /*  Pat Rousso, Carleton University */
  /*  June, 2017 */
  /* Skew Matrix */
  /* skew = @(A) [0 -A(3) A(2) ; A(3) 0 -A(1) ; -A(2) A(1) 0]; */
  /* Defining variable for cross product between s and m: */
  /* Function returns the skew matrix of a 3x1 matrix */
  /* Inputs: */
  /* A - 3x1 matrix */
  /* Outputs: */
  /* skew_mtrx - the skew matrix of A */
  b_sb[0] = 0.0;
  b_sb[3] = -sb[2];
  b_sb[6] = sb[1];
  b_sb[1] = sb[2];
  b_sb[4] = 0.0;
  b_sb[7] = -sb[0];
  b_sb[2] = -sb[1];
  b_sb[5] = sb[0];
  b_sb[8] = 0.0;

  /* Function returns the skew matrix of a 3x1 matrix */
  /* Inputs: */
  /* A - 3x1 matrix */
  /* Outputs: */
  /* skew_mtrx - the skew matrix of A */
  b_si[0] = 0.0;
  b_si[3] = -si[2];
  b_si[6] = si[1];
  b_si[1] = si[2];
  b_si[4] = 0.0;
  b_si[7] = -si[0];
  b_si[2] = -si[1];
  b_si[5] = si[0];
  b_si[8] = 0.0;
  for (i0 = 0; i0 < 3; i0++) {
    s_cross_m_b[i0] = 0.0;
    s_cross_m_i[i0] = 0.0;
    for (i1 = 0; i1 < 3; i1++) {
      s_cross_m_b[i0] += b_sb[i0 + 3 * i1] * mb[i1];
      s_cross_m_i[i0] += b_si[i0 + 3 * i1] * mi[i1];
    }
  }

  /* TRIAD Algorithm */
  /* Triad vector in Body Fixed reference frame */
  x = sqrt((s_cross_m_b[0] * s_cross_m_b[0] + s_cross_m_b[1] * s_cross_m_b[1]) +
           s_cross_m_b[2] * s_cross_m_b[2]);
  for (i0 = 0; i0 < 3; i0++) {
    s_cross_m_b[i0] /= x;
  }

  /* Defining variable for cross product between t1b and t2b: */
  /* Function returns the skew matrix of a 3x1 matrix */
  /* Inputs: */
  /* A - 3x1 matrix */
  /* Outputs: */
  /* skew_mtrx - the skew matrix of A */
  dv0[0] = 0.0;
  dv0[3] = -sb[2];
  dv0[6] = sb[1];
  dv0[1] = sb[2];
  dv0[4] = 0.0;
  dv0[7] = -sb[0];
  dv0[2] = -sb[1];
  dv0[5] = sb[0];
  dv0[8] = 0.0;
  for (i0 = 0; i0 < 3; i0++) {
    t1_cross_t2_b[i0] = 0.0;
    for (i1 = 0; i1 < 3; i1++) {
      t1_cross_t2_b[i0] += dv0[i0 + 3 * i1] * s_cross_m_b[i1];
    }
  }

  x = sqrt((t1_cross_t2_b[0] * t1_cross_t2_b[0] + t1_cross_t2_b[1] *
            t1_cross_t2_b[1]) + t1_cross_t2_b[2] * t1_cross_t2_b[2]);

  /* Triad vector in ECI reference frame */
  b_x = sqrt((s_cross_m_i[0] * s_cross_m_i[0] + s_cross_m_i[1] * s_cross_m_i[1])
             + s_cross_m_i[2] * s_cross_m_i[2]);
  for (i0 = 0; i0 < 3; i0++) {
    s_cross_m_i[i0] /= b_x;
  }

  /* Defining variable for cross product between t1i and t2i: */
  /* Function returns the skew matrix of a 3x1 matrix */
  /* Inputs: */
  /* A - 3x1 matrix */
  /* Outputs: */
  /* skew_mtrx - the skew matrix of A */
  dv1[0] = 0.0;
  dv1[3] = -si[2];
  dv1[6] = si[1];
  dv1[1] = si[2];
  dv1[4] = 0.0;
  dv1[7] = -si[0];
  dv1[2] = -si[1];
  dv1[5] = si[0];
  dv1[8] = 0.0;
  for (i0 = 0; i0 < 3; i0++) {
    t1_cross_t2_i[i0] = 0.0;
    for (i1 = 0; i1 < 3; i1++) {
      t1_cross_t2_i[i0] += dv1[i0 + 3 * i1] * s_cross_m_i[i1];
    }
  }

  b_x = sqrt((t1_cross_t2_i[0] * t1_cross_t2_i[0] + t1_cross_t2_i[1] *
              t1_cross_t2_i[1]) + t1_cross_t2_i[2] * t1_cross_t2_i[2]);

  /* Defining TRIAD vectors in Body Fixed and ECI reference frames */
  /* Attitude Matrix C_BI = C_bt * C_ti */
  for (i0 = 0; i0 < 3; i0++) {
    b_sb[i0] = sb[i0];
    b_sb[3 + i0] = s_cross_m_b[i0];
    b_sb[6 + i0] = t1_cross_t2_b[i0] / x;
    b_si[3 * i0] = si[i0];
    b_si[1 + 3 * i0] = s_cross_m_i[i0];
    b_si[2 + 3 * i0] = t1_cross_t2_i[i0] / b_x;
  }

  for (i0 = 0; i0 < 3; i0++) {
    for (i1 = 0; i1 < 3; i1++) {
      C_BI[i0 + 3 * i1] = 0.0;
      for (i2 = 0; i2 < 3; i2++) {
        C_BI[i0 + 3 * i1] += b_sb[i0 + 3 * i2] * b_si[i2 + 3 * i1];
      }
    }
  }
}
