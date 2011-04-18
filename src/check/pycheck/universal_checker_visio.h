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
 *
 */

#ifndef UNIVERSAL_CHECKER_VISIO_H
#define UNIVERSAL_CHECKER_VISIO_H

#include <Python.h>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <cstring>
#include <fstream>
#include "data/msc.h"
#include "data/checker.h"
#include "data/pysc/py_conv.h"
#include "check/pycheck/export.h"

class PyHUniversalChecker;

typedef boost::shared_ptr<PyHUniversalChecker> PyHUniversalCheckerPtr;

class SCPYCHECK_EXPORT PyHUniversalChecker: public Checker, public HMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static PyHUniversalCheckerPtr m_instance;

public:

  PyHUniversalChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"PyHUniversalFree"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L""; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfies the property.
   */
  std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr chm);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes();
  
  /**
   * Supports all mappers
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }
  
  ~PyHUniversalChecker(){}
  
  static PyHUniversalCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = PyHUniversalCheckerPtr(new PyHUniversalChecker());
    return m_instance;
  }

};

class PyBUniversalChecker;

typedef boost::shared_ptr<PyBUniversalChecker> PyBUniversalCheckerPtr;

class SCPYCHECK_EXPORT PyBUniversalChecker: public Checker, public BMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static PyBUniversalCheckerPtr m_instance;

public:

  PyBUniversalChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"PyBUniversalFree"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L""; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfies the property.
   */
  std::list<BMscPtr> check(BMscPtr hmsc, ChannelMapperPtr chm);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes();
  
  /**
   * Supports all mappers
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }
  
  ~PyBUniversalChecker(){}
  
  static PyBUniversalCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = PyBUniversalCheckerPtr(new PyBUniversalChecker());
    return m_instance;
  }

};

#endif
