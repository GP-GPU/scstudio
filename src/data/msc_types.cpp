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
 * $Id: msc_types.cpp 246 2009-05-13 07:46:16Z gotthardp $
 */

#include <stdlib.h>

#include "data/msc_types.h"

std::wstring TOWSTRING(const std::string& s)
{
  std::wstring result;

  size_t size = s.length();
  const char* pos = s.data();

  while(size > 0)
  {
    wchar_t wc;
    int inc = mbtowc(&wc, pos, size);

    if(inc > 0)
    {
      result.push_back(wc);

      size = size - inc;
      pos = pos + inc;
    }
  }

  return result;
}

// $Id: msc_types.cpp 246 2009-05-13 07:46:16Z gotthardp $
