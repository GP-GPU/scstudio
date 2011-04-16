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

#ifndef DEADLOCK_CHECKER_VISIO_H
#define DEADLOCK_CHECKER_VISIO_H

#include <Python.h>
#include <vector>
#include <list>
#include <map>
#include "data/msc.h"
#include "data/checker.h"
#include "data/pysc/py_conv.h"
#include "check/pycheck/export.h"

class PyHDeadlockChecker;

typedef boost::shared_ptr<PyHDeadlockChecker> PyHDeadlockCheckerPtr;

class SCPYCHECK_EXPORT PyHDeadlockChecker: public Checker, public HMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static PyHDeadlockCheckerPtr m_instance;

public:

  PyHDeadlockChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"PyHDeadlockFree"; }

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
  
  ~PyHDeadlockChecker(){}
  
  static PyHDeadlockCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = PyHDeadlockCheckerPtr(new PyHDeadlockChecker());
    return m_instance;
  }

};

class PyBDeadlockChecker;

typedef boost::shared_ptr<PyBDeadlockChecker> PyBDeadlockCheckerPtr;

class SCPYCHECK_EXPORT PyBDeadlockChecker: public Checker, public BMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static PyBDeadlockCheckerPtr m_instance;

public:

  PyBDeadlockChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"PyBDeadlockFree"; }

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
  
  ~PyBDeadlockChecker(){}
  
  static PyBDeadlockCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = PyBDeadlockCheckerPtr(new PyBDeadlockChecker());
    return m_instance;
  }

};

#endif
