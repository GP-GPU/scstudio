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
 * $Id: reportmessage.cpp 583 2010-02-10 23:05:56Z gotthardp $
 */

#include "stdafx.h"
#include "document.h"
#include "reportmessage.h"

void ShapesReference::OnClick(CDocumentMonitor *monitor) const
{
  monitor->SelectShapes(m_shapes);
}

void DrawingReference::OnClick(CDocumentMonitor *monitor) const
{
  monitor->DisplayDocument(m_drawing);
}

void HelpReference::OnClick(CDocumentMonitor *monitor) const
{
  monitor->InvokeHelp(m_help);
}

// $Id: reportmessage.cpp 583 2010-02-10 23:05:56Z gotthardp $
