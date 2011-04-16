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

#include "check/pycheck/fifo_checker_visio.h"

PyHFIFOCheckerPtr PyHFIFOChecker::m_instance;

std::list<HMscPtr> PyHFIFOChecker::check(HMscPtr hmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv("pycheck.fifo_checker");
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<HMscPtr> ret = exp->checkHMsc(hmsc, chm);
  delete exp;
  return ret;
}

void PyHFIFOChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyHFIFOChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}


PyBFIFOCheckerPtr PyBFIFOChecker::m_instance;

std::list<BMscPtr> PyBFIFOChecker::check(BMscPtr bmsc, ChannelMapperPtr chm){
  PyConv *exp;
  try{
    exp = new PyConv("pycheck.fifo_checker");
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<BMscPtr> ret = exp->checkBMsc(bmsc, chm);
  delete exp;
  return ret;
}

void PyBFIFOChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyBFIFOChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}
