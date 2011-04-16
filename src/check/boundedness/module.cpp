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
 * $Id: module.cpp 206 2009-03-25 17:03:43Z vacek $
 */

#include "check/boundedness/universal_boundedness_checker.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCBOUNDEDNESS_EXPORT
Checker** init_checkers()
{
  Checker **result = new Checker* [2];
  result[0] = new UniversalBoundednessChecker();
  result[1] = NULL;

  return result;
}

// $Id: module.cpp 206 2009-03-25 17:03:43Z vacek $
