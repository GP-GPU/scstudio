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
 *
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: module.cpp 151 2009-01-04 19:25:27Z gotthardp $
 */

#include "check/order/acyclic_checker.h"
#include "check/order/fifo_checker.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCORDER_EXPORT
Checker** init_checkers()
{
  Checker **result = new Checker* [3];
  result[0] = new AcyclicChecker();
  result[1] = new FifoChecker();
  result[2] = NULL;

  return result;
}

// $Id: module.cpp 151 2009-01-04 19:25:27Z gotthardp $
