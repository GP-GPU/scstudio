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
 * $Id: module.cpp 270 2009-07-28 22:06:49Z vacek $
 */

#include "data/modelchecking/divine.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCMODELCHECKING_EXPORT
Formatter** init_formatters()
{
  Formatter **result = new Formatter* [2];
  result[0] = new Divine();
  result[1] = NULL;

  return result;
}

// $Id: module.cpp 270 2009-07-28 22:06:49Z vacek $
