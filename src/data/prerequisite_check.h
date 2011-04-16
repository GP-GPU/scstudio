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
 * $Id: prerequisite_check.h 268 2009-07-28 08:21:28Z gotthardp $
 */

#ifndef _PREQUISITE_CHECK_H
#define _PREQUISITE_CHECK_H

#include <string>

struct PrerequisiteCheck
{
  enum Priority
  {
    PP_REQUIRED,    //! error if not satisfied
    PP_RECOMMENDED, //! warning if not satisfied
    PP_DISREGARDED  //! may not be satisfied
  };

  PrerequisiteCheck(const std::wstring& name, Priority prio)
    : property_name(name), priority(prio)
  { }

  std::wstring property_name;
  Priority priority;
};

#endif /* _PREQUISITE_CHECK_H */

// $Id: prerequisite_check.h 268 2009-07-28 08:21:28Z gotthardp $
