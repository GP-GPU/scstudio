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
 * Copyright (c) 2010 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: module.cpp 605 2010-02-20 17:01:05Z gotthardp $
 */

#include "data/simulator.h"
#include "montecarlo/montecarlo.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCMONTECARLO_EXPORT
Simulator* init_simulator()
{
  return new MonteCarlo();
}

// $Id: module.cpp 605 2010-02-20 17:01:05Z gotthardp $
