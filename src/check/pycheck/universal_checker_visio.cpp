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
  PyObject *name = PyString_FromString("pyscuser");
  if(!name){
    std::cout << "Cannot create PyString pyscuser";
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
     sprintf(cret, "%ls", wret);
     PyUnicode_AsWideChar((PyUnicodeObject *)mem, wret, psize);
     wret[psize] = '\0';
     sprintf(cret, "%ls", wret);
     ret[cret] = wret;
  }
  return ret;
}

/*  std::ifstream f;
  f.open(get_file_name());
  std::string inp;
  char *cinp;
  if(!f.is_open()){
    std::cout << "Cannot open conf file:" << get_file_name() << std::endl;
    throw 13;
  }
  while(f.good()){
    getline(f, inp);
    if(inp != ""){
      cinp = new char [inp.size()+1];
      strcpy(cinp, inp.c_str());
      ret.push_back(cinp);
    }
  }
  f.close();*/

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
    throw 15;
  }
  ret = exp->checkHMsc(hmsc, chm);
  if(ret.size()){
    HMscPtr checker_name(new HMsc((*it).second));
    ret.push_back(checker_name);
    for(std::list<HMscPtr>::iterator hit = ret.begin();hit != ret.end();hit++)
      ret.push_back(*hit);
  }
  it++;
  for(;it != checkers.end();it++){
    if(!exp->reinit((*it).first)){
      std::cout << "Cannot initialize checker:" << (*it).second << std::endl;
      continue;
      //throw 11;
    }
    ret = exp->checkHMsc(hmsc, chm);
    if(ret.size()){
      HMscPtr checker_name(new HMsc((*it).second));
      ret.push_back(checker_name);
      for(std::list<HMscPtr>::iterator hit = ret.begin();hit != ret.end();hit++)
        ret.push_back(*hit);
//    return ret;
    }
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
//  std::list<char *> checkers = list_checkers();
//  std::list<char *>::iterator it = checkers.begin();
  std::list<BMscPtr> ret;
/*  if(!checkers.size())
    return ret;
  PyConv * exp;
  try{
    exp = new PyConv(*it);
  }
  catch(int){
    std::cout << "Cannot initialize checker:" << *it << std::endl;
    throw 15;
  }
  it++;
  for(;it != checkers.end();it++){
    if(!exp->reinit(*it)){
      std::cout << "Cannot initialize checker:" << *it << std::endl;
      throw 11;
    }
    ret = exp->checkBMsc(bmsc, chm);
    if(ret.size()){
      std::cout << "Supplied BMsc failed in module " << *it << std::endl;
      return ret;
    }
  }
  delete exp;*/
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
