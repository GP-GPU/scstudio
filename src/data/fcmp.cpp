/*
 fcmp
 Copyright (c) 1998-2000 Theodore C. Belding
 University of Michigan Center for the Study of Complex Systems
 <mailto:Ted.Belding@umich.edu>
 <http://www-personal.umich.edu/~streak/>		

 This file is part of the fcmp distribution. fcmp is free software;
 you can redistribute and modify it under the terms of the GNU Library
 General Public License (LGPL), version 2 or later.  This software
 comes with absolutely no warranty. See the file COPYING for details
 and terms of copying.

 Description:
 
 Knuth's floating point comparison operators, from:
 Knuth, D. E. (1998). The Art of Computer Programming.
 Volume 2: Seminumerical Algorithms. 3rd ed. Addison-Wesley.
 Section 4.2.2, p. 233. ISBN 0-201-89684-2.

 Input parameters:
 x1, x2: numbers to be compared
 epsilon: determines tolerance

 epsilon should be carefully chosen based on the machine's precision,
 the observed magnitude of error, the desired precision, and the
 magnitude of the numbers to be compared. See the fcmp README file for
 more information.

 This routine may be used for both single-precision (float) and
 double-precision (double) floating-point numbers.
 
 Returns:
 -1 if x1 < x2
  0 if x1 == x2
  1 if x1 > x2
*/

#include "fcmp.h"
#include <math.h>

int fcmp(double x1, double x2, double epsilon) {
  int exponent;
  double delta;
  double difference;
  
  /* Get exponent(max(fabs(x1), fabs(x2))) and store it in exponent. */

  /* If neither x1 nor x2 is 0, */
  /* this is equivalent to max(exponent(x1), exponent(x2)). */

  /* If either x1 or x2 is 0, its exponent returned by frexp would be 0, */
  /* which is much larger than the exponents of numbers close to 0 in */
  /* magnitude. But the exponent of 0 should be less than any number */
  /* whose magnitude is greater than 0. */
  
  /* So we only want to set exponent to 0 if both x1 and */
  /* x2 are 0. Hence, the following works for all x1 and x2. */

  frexp(fabs(x1) > fabs(x2) ? x1 : x2, &exponent);

  /* Do the comparison. */

  /* delta = epsilon * pow(2, exponent) */

  /* Form a neighborhood around x2 of size delta in either direction. */
  /* If x1 is within this delta neighborhood of x2, x1 == x2. */
  /* Otherwise x1 > x2 or x1 < x2, depending on which side of */
  /* the neighborhood x1 is on. */
  
  delta = ldexp(epsilon, exponent); 
  
  difference = x1 - x2;

  if (difference > delta)
    return 1; /* x1 > x2 */
  else if (difference < -delta) 
    return -1;  /* x1 < x2 */
  else /* -delta <= difference <= delta */
    return 0;  /* x1 == x2 */
}

// $Id: fcmp.cpp 341 2009-09-19 10:29:05Z gotthardp $
