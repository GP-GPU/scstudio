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
 * $Id: module.cpp 438 2009-10-25 15:32:55Z gotthardp $
 */

#include "beautify.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCBEAUTIFY_EXPORT
Transformer** init_transformers()
{
  Transformer **result = new Transformer* [2];
  result[0] = new Beautify();
  result[1] = NULL;

  return result;
}

// $Id: module.cpp 438 2009-10-25 15:32:55Z gotthardp $
