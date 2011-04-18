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

#include "check/pycheck/universal_checker_visio.h"
#define get_file_name() "run_test_file"

PyHUniversalCheckerPtr PyHUniversalChecker::m_instance;

std::list<HMscPtr> PyHUniversalChecker::check(HMscPtr hmsc, ChannelMapperPtr chm){
  ifstream f;
  f.open(get_file_name());
  string inp;
  if(!f.is_open()){
    throw 13;
  }
  getline(f, inp);
  f.close();
  cinp = new char [inp.size()+1];
  strcpy(cinp, inp.c_str());
  PyConv *exp;
  try{
    exp = new PyConv(cinp);
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<HMscPtr> ret = exp->checkHMsc(hmsc, chm);
  delete exp;
  return ret;
}

void PyHUniversalChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyHUniversalChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}


PyBUniversalCheckerPtr PyBUniversalChecker::m_instance;

std::list<BMscPtr> PyBUniversalChecker::check(BMscPtr bmsc, ChannelMapperPtr chm){
  ifstream f;
  f.open(get_file_name());
  string inp;
  if(!f.is_open()){
    throw 13;
  }
  getline(f, inp);
  f.close();
  cinp = new char [inp.size()+1];
  PyConv *exp;
  try{
    exp = new PyConv(cinp);
  }
  catch(int e){
    std::cout << "Cannot initialize checker" << std::endl;
    throw 15;
  }
  std::list<BMscPtr> ret = exp->checkBMsc(bmsc, chm);
  delete exp;
  return ret;
}

void PyBUniversalChecker::cleanup_attributes()
{
}

Checker::PreconditionList PyBUniversalChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}
