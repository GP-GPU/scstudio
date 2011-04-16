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
 * $Id: module.cpp 616 2010-02-23 14:09:29Z lkorenciak $
 */

#include "check/time/constraint_syntax.h"
#include "check/time/time_consistency.h"
#include "check/time/time_trace_race.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCTIME_EXPORT
Checker** init_checkers()
{
  Checker **result = new Checker* [4];
  result[0] = new ConstraintsChecker(); 
  result[1] = new ConsistencyChecker(); 
  result[2] = new TimeRaceChecker(); 
  result[3] = NULL;

  return result;
}

#include "check/time/tightening.h"

// module initialization function
extern "C" SCTIME_EXPORT
Transformer** init_transformers()
{
  Transformer **result = new Transformer* [2];
  result[0] = new Tighter();
  result[1] = NULL;

  return result;
}

// $Id: module.cpp 616 2010-02-23 14:09:29Z lkorenciak $
