/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 */

#include "check/pycheck/deadlock_checker_visio.h"
#include "check/pycheck/livelock_checker_visio.h"
#include "check/pycheck/acyclic_checker_visio.h"
#include "check/pycheck/fifo_checker_visio.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCPYCHECK_EXPORT
Checker** init_checkers()
{
  Checker **result = new Checker* [9];

  result[0] = new PyBDeadlockChecker();
  result[1] = new PyHDeadlockChecker();
  result[2] = new PyBLivelockChecker();
  result[3] = new PyHLivelockChecker();
  result[4] = new PyBAcyclicChecker();
  result[5] = new PyHAcyclicChecker();
  result[6] = new PyBFIFOChecker();
  result[7] = new PyHFIFOChecker();
  result[8] = NULL;
  return result;
}
