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
 * $Id: export.h 226 2009-04-22 07:54:18Z vacek $
 */

#ifndef _SCSTRUCTURE_EXPORT_H
#define _SCSTRUCTURE_EXPORT_H

#if defined(_MSC_VER)
#pragma warning(disable: 4251)

#if defined(scstructure_EXPORTS)
#define SCSTRUCTURE_EXPORT __declspec(dllexport)
#else
#define SCSTRUCTURE_EXPORT __declspec(dllimport)
#endif

#else
#define SCSTRUCTURE_EXPORT
#endif

#endif /* _SCSTRUCTURE_EXPORT_H */

// $Id: export.h 226 2009-04-22 07:54:18Z vacek $
