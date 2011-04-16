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

#ifndef FIFO_CHECKER_VISIO_H
#define FIFO_CHECKER_VISIO_H

#include <Python.h>
#include <vector>
#include <list>
#include <map>
#include "data/msc.h"
#include "data/checker.h"
#include "data/pysc/py_conv.h"
#include "check/pycheck/export.h"

class PyHFIFOChecker;

typedef boost::shared_ptr<PyHFIFOChecker> PyHFIFOCheckerPtr;

class SCPYCHECK_EXPORT PyHFIFOChecker: public Checker, public HMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static PyHFIFOCheckerPtr m_instance;

public:

  PyHFIFOChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"PyHFIFOFree"; }

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
  
  ~PyHFIFOChecker(){}
  
  static PyHFIFOCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = PyHFIFOCheckerPtr(new PyHFIFOChecker());
    return m_instance;
  }

};

class PyBFIFOChecker;

typedef boost::shared_ptr<PyBFIFOChecker> PyBFIFOCheckerPtr;

class SCPYCHECK_EXPORT PyBFIFOChecker: public Checker, public BMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static PyBFIFOCheckerPtr m_instance;

public:

  PyBFIFOChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"PyBFIFOFree"; }

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
  
  ~PyBFIFOChecker(){}
  
  static PyBFIFOCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = PyBFIFOCheckerPtr(new PyBFIFOChecker());
    return m_instance;
  }

};

#endif
