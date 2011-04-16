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
 * Copyright (c) 2007-2008 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: fcmp.h 576 2010-02-07 14:58:10Z gotthardp $
 */

#ifndef _FCMP_H
#define _FCMP_H

#include <limits>

#include "data/export.h"

int SCMSC_EXPORT fcmp(double x1, double x2, double epsilon =
  std::numeric_limits<float>::epsilon());

#endif /* _FCMP_H */

// $Id: fcmp.h 576 2010-02-07 14:58:10Z gotthardp $
