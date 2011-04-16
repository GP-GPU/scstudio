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
 * $Id: module.cpp 546 2010-01-10 14:47:31Z gotthardp $
 */

#include "data/searcher.h"
#include "membership/membership_alg.h"

// module initialization function
// note: the Visio add-on searches for a function of this name
extern "C" SCMEMBERSHIP_EXPORT
Searcher* init_membership()
{
  return new MembershipAlg();
}

// $Id: module.cpp 546 2010-01-10 14:47:31Z gotthardp $
