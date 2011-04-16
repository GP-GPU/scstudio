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
 * $Id: export.h 146 2009-01-03 16:08:45Z gotthardp $
 */

#ifndef _SCZ120_EXPORT_H
#define _SCZ120_EXPORT_H

#if defined(_MSC_VER)
#pragma warning(disable: 4251)

#if defined(scZ120_EXPORTS)
#define SCZ120_EXPORT __declspec(dllexport)
#else
#define SCZ120_EXPORT __declspec(dllimport)
#endif

#else
#define SCZ120_EXPORT
#endif

#endif /* _SCZ120_EXPORT_H */

// $Id: export.h 146 2009-01-03 16:08:45Z gotthardp $
