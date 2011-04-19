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

#include "check/pycheck/livelock_checker_visio.h"

PyHLivelockCheckerPtr PyHLivelockChecker::m_instance;

std::list<HMscPtr> PyHLivelockChecker::check(HMscPtr hmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv("pycheck.livelock_checker");
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<HMscPtr> ret = exp->checkHMsc(hmsc, chm);
  delete exp;
  return ret;
}

void PyHLivelockChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyHLivelockChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}


PyBLivelockCheckerPtr PyBLivelockChecker::m_instance;

std::list<BMscPtr> PyBLivelockChecker::check(BMscPtr bmsc, ChannelMapperPtr chm){
  std::list<BMscPtr> ret;
  return ret;
}

void PyBLivelockChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyBLivelockChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}
