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
#define PY_MODULE "pyscuser"

PyHUniversalCheckerPtr PyHUniversalChecker::m_instance;

std::map<char *, wchar_t *> list_checkers(const char *var){
  std::map<char *, wchar_t *> ret;
  if(!Py_IsInitialized())
    Py_Initialize();
  std::cout << "Importing module pyscuser";
  PyObject *name = PyUnicode_FromString("pyscuser");
  if(!name){
    std::cout << "Cannot create PyUnicode pyscuser";
    return ret;
  }
  PyObject *module = PyImport_Import(name);
  if(!module){
    std::cout << "Module pyscuser cannot be imported.";
    std::cout << "You should install it.";
    if(PyErr_Occurred()){
      PyErr_Print();
      return ret;
    }
  }
  PyObject *dict = PyModule_GetDict(module);
  if(!dict){
    std::cout << "Cannot extract dictionary from pyscuser.";
    return ret;
  }
  PyObject *checkers = PyDict_GetItemString(dict, var);
  if(!checkers){
    std::cout << "Cannot find variable " << var << "in dictionary of pyscuser.";
    return ret;
  }
  for(int i = 0;i < PyList_Size(checkers);i++){
    PyObject *mem = PyList_GetItem(checkers, i);
    Py_ssize_t psize = PyUnicode_GetSize(mem);
    wchar_t *wret = new wchar_t [psize + 1];
    char *cret = new char [psize + 1];
    PyUnicode_AsWideChar((PyUnicodeObject *)mem, wret, psize);
    wret[psize] = '\0';
    sprintf(cret, "%ls", wret);
    ret[cret] = wret;
  }
  Py_XDECREF(name);
  Py_XDECREF(module);
  return ret;
}

std::list<HMscPtr> PyHUniversalChecker::check(HMscPtr hmsc, ChannelMapperPtr chm){
  std::map<char *, wchar_t *> checkers = list_checkers("hcheckers");
  std::map<char *, wchar_t *>::iterator it = checkers.begin();
  std::list<HMscPtr> ret;
  if(!checkers.size())
    return ret;
  std::cout << "\nfirst: " << (*it).first << "\nsecond: ";
  std::wcout << (*it).second << std::endl;
  PyConv * exp;
  try{
    exp = new PyConv((*it).first);
  }
  catch(int){
    std::cout << "Cannot initialize checker:" << (*it).second << std::endl;
    return ret;
  }
  std::list<HMscPtr> hret = exp->checkHMsc(hmsc, chm);
  if(hret.size()){
    HMscPtr checker_name(new HMsc((*it).second));
    ret.push_back(checker_name);
    for(std::list<HMscPtr>::iterator hit = hret.begin();hit != hret.end();hit++)
      ret.push_back(*hit);
  }
  delete[] (*it).first;
  delete[] (*it).second;
  it++;
  for(;it != checkers.end();it++){
    if(!exp->reinit((*it).first)){
      std::cout << "Cannot initialize checker:" << (*it).second << std::endl;
      continue;
    }
    std::list<HMscPtr> hret = exp->checkHMsc(hmsc, chm);
    if(hret.size()){
      HMscPtr checker_name(new HMsc((*it).second));
      ret.push_back(checker_name);
      for(std::list<HMscPtr>::iterator hit = hret.begin();hit != hret.end();hit++)
        ret.push_back(*hit);
    }
    delete[] (*it).first;
    delete[] (*it).second;
  }
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
  std::map<char *, wchar_t *> checkers = list_checkers("bcheckers");
  std::map<char *, wchar_t *>::iterator it = checkers.begin();
  std::list<BMscPtr> ret;
  if(!checkers.size())
    return ret;
  std::cout << "\nfirst: " << (*it).first << "\nsecond: ";
  std::wcout << (*it).second << std::endl;
  PyConv * exp;
  try{
    exp = new PyConv((*it).first);
  }
  catch(int){
    std::cout << "Cannot initialize checker:" << (*it).second << std::endl;
    return ret;
  }
  std::list<BMscPtr> bret = exp->checkBMsc(bmsc, chm);
  if(bret.size()){
    BMscPtr checker_name(new BMsc((*it).second));
    ret.push_back(checker_name);
    for(std::list<BMscPtr>::iterator hit = bret.begin();hit != bret.end();hit++)
      ret.push_back(*hit);
  }
  delete (*it).first;
  delete (*it).second;
  it++;
  for(;it != checkers.end();it++){
    if(!exp->reinit((*it).first)){
      std::cout << "Cannot initialize checker:" << (*it).second << std::endl;
      continue;
    }
    std::list<BMscPtr> bret = exp->checkBMsc(bmsc, chm);
    if(bret.size()){
      BMscPtr checker_name(new BMsc((*it).second));
      ret.push_back(checker_name);
      for(std::list<BMscPtr>::iterator hit = bret.begin();hit != bret.end();hit++)
        ret.push_back(*hit);
    }
    delete (*it).first;
    delete (*it).second;
  }
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
