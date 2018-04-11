/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 * File: mod.c
 *
 * MATLAB Coder version            : 4.0
 * C/C++ source code generated on  : 23-Mar-2018 10:54:41
 */

/* Include Files */
#include <math.h>
#include "rt_nonfinite.h"
#include "inertial_vectors.h"
#include "mod.h"

/* Function Definitions */

/*
 * Arguments    : double x
 * Return Type  : double
 */
double b_mod(double x)
{
  double r;
  if ((!rtIsInf(x)) && (!rtIsNaN(x))) {
    if (x == 0.0) {
      r = 0.0;
    } else {
      r = fmod(x, 360.0);
      if (r == 0.0) {
        r = 0.0;
      } else {
        if (x < 0.0) {
          r += 360.0;
        }
      }
    }
  } else {
    r = rtNaN;
  }

  return r;
}

/*
 * File trailer for mod.c
 *
 * [EOF]
 */
