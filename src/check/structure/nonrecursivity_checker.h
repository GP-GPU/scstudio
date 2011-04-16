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
 * $Id: nonrecursivity_checker.h 1052 2011-02-27 17:46:37Z vacek $
 */

#include "data/checker.h"
#include "data/msc.h"
#include "check/structure/export.h"
#include "data/dfs_hmsc_traverser.h"
#include "check/pseudocode/msc_duplicators.h"


class NonrecursivityChecker;


typedef boost::shared_ptr<NonrecursivityChecker> NonrecursivityCheckerPtr;


class RecursiveException: public std::exception
{
public:

  virtual const char* what() const throw()
  {
    return "HMSC is recursive.";
  }
};
class RecursivityListener:public GrayNodeFoundListener
{
public:
  void on_gray_node_found(HMscNode *n);
};
class SCSTRUCTURE_EXPORT NonrecursivityChecker: public Checker, public HMscChecker
{
protected:
  
  /**
   * Common instance.
   */
  static NonrecursivityCheckerPtr m_instance;

  HMscPtr create_counter_example(const MscElementPListList& path);

   
public:
    
  NonrecursivityChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Nonrecursivity"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"recursivity/recursivity.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether a given hmsc contains consistent set of instances.
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
  
  
  static NonrecursivityCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = NonrecursivityCheckerPtr(new NonrecursivityChecker());
    return m_instance;
  }

};
 
// $Id: nonrecursivity_checker.h 1052 2011-02-27 17:46:37Z vacek $
