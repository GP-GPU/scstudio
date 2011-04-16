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
 * $Id: errors.cpp 546 2010-01-10 14:47:31Z gotthardp $
 */

#include "stdafx.h"
#include "errors.h"
#include "dllmodule.h"

#include <string>

void DisplayException(Visio::IVApplicationPtr vsoApp,
  const std::wstring& bstrMessage, UINT nFlags)
{
  if (vsoApp->GetAlertResponse() == 0)
  {
    MessageBox(GetActiveWindow(), bstrMessage.c_str(), LoadStringResource(IDS_ADDON_NAME).c_str(), nFlags);
  }
  else
    OutputDebugString(bstrMessage.c_str());
}

// $Id: errors.cpp 546 2010-01-10 14:47:31Z gotthardp $
