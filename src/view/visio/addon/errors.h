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
 * Copyright (c) 2007-2010 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: errors.h 546 2010-01-10 14:47:31Z gotthardp $
 */

#pragma once

#include <sstream>

#ifdef DEBUG
#define TRACE(message) \
{ \
  std::basic_stringstream<TCHAR> buffer; \
  buffer << __FILE__ << "(" << __LINE__ << "): " << message << std::endl; \
  OutputDebugString(buffer.str().c_str()); \
}
#else
#define TRACE(message) /* no operation */
#endif

void DisplayException(Visio::IVApplicationPtr vsoApp,
  const std::wstring& bstrMessage, UINT nFlags);

// $Id: errors.h 546 2010-01-10 14:47:31Z gotthardp $
