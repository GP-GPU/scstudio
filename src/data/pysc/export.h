/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 3.0, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 */

#ifndef _SCPYCONV_EXPORT_H
#define _SCPYCONV_EXPORT_H

#if defined(_MSC_VER)
#pragma warning(disable: 4251)

#if defined(scpyconv_EXPORTS)
#define SCPYCONV_EXPORT __declspec(dllexport)
#else
#define SCPYCONV_EXPORT __declspec(dllimport)
#endif

#else
#define SCPYCONV_EXPORT
#endif

#endif /* _SCPYCONV_EXPORT_H */
