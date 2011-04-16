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
 * $Id: module.cpp 361 2009-09-24 14:27:29Z vacek $
 */

#include "check/realizability/realizability_checker.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCREALIZABILITY_EXPORT
Checker** init_checkers()
{
  Checker **result = new Checker* [2];
  result[0] = new RealizabilityChecker();
  result[1] = NULL;

  return result;
}

// $Id: module.cpp 361 2009-09-24 14:27:29Z vacek $
