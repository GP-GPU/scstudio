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
 * Copyright (c) 2008 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: realizability_checker.cpp 584 2010-02-11 09:32:03Z vacek $
 */

#include "check/realizability/realizability_checker.h"

RealizabilityCheckerPtr RealizabilityChecker::m_instance;



std::list<HMscPtr> RealizabilityChecker::check(HMscPtr hmsc, ChannelMapperPtr chm)
{
  std::list<HMscPtr> result;
  return result;
}


Checker::PreconditionList RealizabilityChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  result.push_back(PrerequisiteCheck(L"Universal Boundedness", PrerequisiteCheck::PP_REQUIRED));
  result.push_back(PrerequisiteCheck(L"Local Choice", PrerequisiteCheck::PP_REQUIRED));

  return result;
}

//  $Id: realizability_checker.cpp 584 2010-02-11 09:32:03Z vacek $
