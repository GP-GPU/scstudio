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
 * Copyright (c) 2008 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: export.h 270 2009-07-28 22:06:49Z vacek $
 */

#ifndef _MODELCHECKING_EXPORT_H
#define _MODELCHECKING_EXPORT_H

#if defined(_MSC_VER)
#pragma warning(disable: 4251)

#if defined(scmodelchecking_EXPORTS)
#define SCMODELCHECKING_EXPORT __declspec(dllexport)
#else
#define SCMODELCHECKING_EXPORT __declspec(dllimport)
#endif

#else
#define SCMODELCHECKING_EXPORT
#endif

#endif /* _MODELCHECKING_EXPORT_H */

// $Id: export.h 270 2009-07-28 22:06:49Z vacek $
