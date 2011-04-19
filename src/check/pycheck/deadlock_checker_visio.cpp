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

#include "check/pycheck/deadlock_checker_visio.h"

PyHDeadlockCheckerPtr PyHDeadlockChecker::m_instance;

std::list<HMscPtr> PyHDeadlockChecker::check(HMscPtr hmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv("pycheck.deadlock_checker");
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<HMscPtr> ret = exp->checkHMsc(hmsc, chm);
  delete exp;
  return ret;
}

void PyHDeadlockChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyHDeadlockChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}


PyBDeadlockCheckerPtr PyBDeadlockChecker::m_instance;

std::list<BMscPtr> PyBDeadlockChecker::check(BMscPtr bmsc, ChannelMapperPtr chm){
  std::list<BMscPtr> ret;
  return ret;
}

void PyBDeadlockChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyBDeadlockChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}
