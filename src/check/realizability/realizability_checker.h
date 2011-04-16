/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2008 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: realizability_checker.h 789 2010-05-16 20:44:30Z vacek $
 */

#include <iostream>
#include "data/checker.h"
#include "data/msc.h"
#include "check/pseudocode/msc_duplicators.h"
#include "check/realizability/export.h"
#include "data/dfs_refnode_hmsc_traverser.h"

class RealizabilityChecker;


typedef boost::shared_ptr<RealizabilityChecker> RealizabilityCheckerPtr;

class SCREALIZABILITY_EXPORT RealizabilityChecker: public Checker, public HMscChecker
{
protected:
  
  /**
   * Common instance.
   */
  static RealizabilityCheckerPtr m_instance;

 
        
public:
  RealizabilityChecker(){};
  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Strong Realizability"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"realizability/realizability.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfy universal boundedness property.
   */
  std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr chm);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes()
  {}
  
  /**
   * Supports all mappers
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }
  
  
  static RealizabilityCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = RealizabilityCheckerPtr(new RealizabilityChecker());
    return m_instance;
  }

};
 
// $Id: realizability_checker.h 789 2010-05-16 20:44:30Z vacek $
