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
 * $Id: export.h 304 2009-09-10 18:35:08Z gotthardp $
 */

#ifndef _SCPCAP_EXPORT_H
#define _SCPCAP_EXPORT_H

#if defined(_MSC_VER)
#pragma warning(disable: 4251)

#if defined(scpcap_EXPORTS)
#define SCPCAP_EXPORT __declspec(dllexport)
#else
#define SCPCAP_EXPORT __declspec(dllimport)
#endif

#else
#define SCPCAP_EXPORT
#endif

#endif /* _SCPCAP_EXPORT_H */

// $Id: export.h 304 2009-09-10 18:35:08Z gotthardp $
