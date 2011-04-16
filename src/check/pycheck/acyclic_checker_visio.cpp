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

#include "check/pycheck/acyclic_checker_visio.h"

PyHAcyclicCheckerPtr PyHAcyclicChecker::m_instance;

std::list<HMscPtr> PyHAcyclicChecker::check(HMscPtr hmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv("pycheck.acyclic_checker");
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<HMscPtr> ret = exp->checkHMsc(hmsc, chm);
  delete exp;
  return ret;
}

void PyHAcyclicChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyHAcyclicChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}


PyBAcyclicCheckerPtr PyBAcyclicChecker::m_instance;

std::list<BMscPtr> PyBAcyclicChecker::check(BMscPtr bmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv("pycheck.acyclic_checker");
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<BMscPtr> ret = exp->checkBMsc(bmsc, chm);
  delete exp;
  return ret;
}

void PyBAcyclicChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyBAcyclicChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}
