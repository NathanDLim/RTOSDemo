/* Calculating Vectors in Inertial Frame*/

/* This function combines the sim_time, sun_vector and mag_vector blocks
from the ADCS Simulink model to determine the inertial frame vectors
for the TRIAD algorithm. It requires Clock and Position data from the GPS.*/

/* Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: inertial_vectors.c
 *
 * MATLAB Coder version            : 4.0
 * C/C++ source code generated on  : 23-Mar-2018 10:54:41
 */

/* Include Files */
#include <math.h>
#include "rt_nonfinite.h"
#include "inertial_vectors.h"
#include "mod.h"

/* Function Declarations */
static double rt_powd_snf(double u0, double u1);

/* Function Definitions */

/*
 * Arguments    : double u0
 *                double u1
 * Return Type  : double
 */
static double rt_powd_snf(double u0, double u1)
{
  double y;
  double d0;
  double d1;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = rtNaN;
  } else {
    d0 = fabs(u0);
    d1 = fabs(u1);
    if (rtIsInf(u1)) {
      if (d0 == 1.0) {
        y = 1.0;
      } else if (d0 > 1.0) {
        if (u1 > 0.0) {
          y = rtInf;
        } else {
          y = 0.0;
        }
      } else if (u1 > 0.0) {
        y = 0.0;
      } else {
        y = rtInf;
      }
    } else if (d1 == 0.0) {
      y = 1.0;
    } else if (d1 == 1.0) {
      if (u1 > 0.0) {
        y = u0;
      } else {
        y = 1.0 / u0;
      }
    } else if (u1 == 2.0) {
      y = u0 * u0;
    } else if ((u1 == 0.5) && (u0 >= 0.0)) {
      y = sqrt(u0);
    } else if ((u0 < 0.0) && (u1 > floor(u1))) {
      y = rtNaN;
    } else {
      y = pow(u0, u1);
    }
  }

  return y;
}

/*
 * User Inputs
 *  Calculating Initial Time Variables
 * Arguments    : double clock
 *                const double r[3]
 *                double s_I[3]
 *                double m_I[3]
 * Return Type  : void
 */
void inertial_vectors(double clock, const double r[3], double s_I[3], double
                      m_I[3])
{
  double M_anom;
  double obliq;
  double rmod;
  double y;
  double b_y;
  double m_unit_idx_0;
  double r_dot_m;
  double mag_vect_idx_0;

  /*  Simulation Time */
  /*  starting date of the simulation */
  /*  Find the initial JD  */
  /*  Convert to Julian centuries since J2000 */
  /*  Initial Greenwich Mean Sidereal Time  */
  /*  1 sec = 1/240 deg */
  /*  For each time step: */
  /*  sun_vector calculates the geocentric sun position vector  */
  /*  Inputs:  */
  /*    J_date - Julian date from January 1, 4713 BC */
  /*  Outputs: */
  /*    sun_vect(1) - The normalized Sun-Earth position vector [x] */
  /*    sun_vect(2) - The normalized Sun-Earth position vector [y] */
  /*    sun_vect(3) - The normalized Sun-Earth position vector [z] */
  /*    sun_vect(4) - The true Sun-Earth position vector [x] */
  /*    sun_vect(5) - The true Sun-Earth position vector [y] */
  /*    sun_vect(6) - The true Sun-Earth position vector [z] */
  /*  Coments: */
  /*    Updated by Kevin Stadnyk 2016 - Carleton University */
  /*    Created by Justin Kernot 2016 - Carleton University */
  /*  Key Variables */
  /*  Converts degrees to radians */
  /*  Parameter Definitions */
  /*  This is unneeded now that the TUT1 date is calculated in the clock */
  /*  Convert the inputted Julian date to the Julian centuries since J2000 */
  /*  TUT1 = (J_date-2451545)/36525; */
  /*  Mean anomaly of Sun [deg] */
  M_anom = b_mod(7015.7510017038321);

  /*  Mean longitude of the Sun [deg] */
  /*  Ecliptic longitude of the Sun [deg] */
  M_anom = b_mod((b_mod(6939.0014013476357) + 1.914666471 * sin(M_anom *
    0.017453292519943295)) + 0.019994643 * sin(2.0 * M_anom *
    0.017453292519943295));

  /*  Obliquity of the ecliptic [deg] */
  obliq = b_mod(23.436885801557839);

  /*  Sun-Earth distance [km] (without last term is in AU) */
  /* S_E_dis = (1.000140612-0.016708617*cos(M_anom*d2r)-0.000139589*cos(2*M_anom*d2r))*149597870.66; */
  /*  Sun-Earth Vector Calculation */
  /*  Calculate the Sun-Earth unit vector */
  /*  Calculate the Sun-Earth vector [km] */
  /* sun_vect_ECI(4) = S_E_dis.*sun_vect_ECI(1); */
  /* sun_vect_ECI(5) = S_E_dis.*sun_vect_ECI(2); */
  /* sun_vect_ECI(6) = S_E_dis.*sun_vect_ECI(3); */
  s_I[0] = cos(M_anom * 0.017453292519943295);
  s_I[1] = cos(obliq * 0.017453292519943295) * sin(M_anom * 0.017453292519943295);
  s_I[2] = sin(obliq * 0.017453292519943295) * sin(M_anom * 0.017453292519943295);

  /*  mag_vect calculates the geocentric magnetic field vector  */
  /*  Inputs:  */
  /*    r - 3x1 matrix of position comonents of the spacecraft in ECI */
  /*    GMST_ini - Grenetch time */
  /*    dt - change in time */
  /*  Outputs: */
  /*    mag_vect(1) - the normalized Earth magnetic field vector [x] */
  /*    mag_vect(2) - the normalized Earth magnetic field vector [y] */
  /*    mag_vect(3) - the normalized Earth magnetic field vector [z] */
  /*    mag_vect(4) - the true Earth magentic field vector [x] */
  /*    mag_vect(5) - the true Earth magentic field vector [y] */
  /*    mag_vect(6) - the true Earth magentic field vector [z] */
  /*  Comments: */
  /*    Kevin Stadnyk 2016 - Carleton Univeristy */
  /*    Gabrielle Sevigny 2016 - Carleton University */
  /*  Key Variables / Global Variables */
  /*  Earth Constants */
  /*  mean radius of the Earth (used in magnetic) (km) */
  /*  Sidereal rotation period (s) (Earth rotates every 23 hours, 56 minutes, 4 seconds) */
  /*  average rotation rate of the Earth (rad/s) */
  /*  Gaussian Coefficients (per IGRF-12 and valid for 2015 - 2020) */
  /*  nT */
  /*  nT */
  /*  nT */
  /*  Initializing vectors  */
  /*  Parameter Defintitions */
  /*  Calculate H0, constant characterizing the Earth's magnetic field  */
  /*  nT */
  /*  Calculate the coelevation of the magnetic dipole */
  /*  rad */
  /*  Calculate the east longitude of the magnetic dipole */
  /*  rad -> double check this equation */
  /*  Calculate the magnitude of the position vector */
  rmod = sqrt((r[0] * r[0] + r[1] * r[1]) + r[2] * r[2]);

  /*  Calculate the position unit vector */
  obliq = r[0] / rmod;
  y = r[1] / rmod;
  b_y = r[2] / rmod;
  M_anom = (2.4389190014145928E+6 + 7.2921065908806518E-5 * clock) +
    1.8740431760082377;

  /*  Calculate the dipole unit vector */
  m_unit_idx_0 = 0.16828878751893375 * cos(M_anom);
  M_anom = 0.16828878751893375 * sin(M_anom);

  /*  Calculate the dot product of the position and dipole */
  r_dot_m = (obliq * m_unit_idx_0 + y * M_anom) + b_y * -0.98573773590920577;

  /*  Earth Magnetic Field Vector Calculation */
  /*  Calculate the magnetic field vector */
  mag_vect_idx_0 = 7.724486898712202E+15 / rt_powd_snf(rmod, 3.0) * (3.0 *
    r_dot_m * obliq - m_unit_idx_0);
  m_unit_idx_0 = 7.724486898712202E+15 / rt_powd_snf(rmod, 3.0) * (3.0 * r_dot_m
    * y - M_anom);
  M_anom = 7.724486898712202E+15 / rt_powd_snf(rmod, 3.0) * (3.0 * r_dot_m * b_y
    - -0.98573773590920577);

  /*  Don't need right now.... */
  /*  Calculate the magnetic field unit vector */
  obliq = sqrt((mag_vect_idx_0 * mag_vect_idx_0 + m_unit_idx_0 * m_unit_idx_0) +
               M_anom * M_anom);
  m_I[0] = mag_vect_idx_0 / obliq;
  m_I[1] = m_unit_idx_0 / obliq;
  m_I[2] = M_anom / obliq;
}

/*
 * File trailer for inertial_vectors.c
 *
 * [EOF]
 */
