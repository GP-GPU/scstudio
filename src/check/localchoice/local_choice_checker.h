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
 * $Id: local_choice_checker.h 1029 2011-02-02 22:17:59Z madzin $
 */

#include <iostream>
#include "data/checker.h"
#include "data/msc.h"
#include "check/pseudocode/msc_duplicators.h"
#include "check/localchoice/export.h"
#include "data/dfs_refnode_hmsc_traverser.h"
#include "data/dfs_hmsc_traverser.h"

class LocalChoiceChecker;


typedef boost::shared_ptr<LocalChoiceChecker> LocalChoiceCheckerPtr;

class NonlocalNode: public std::exception
{
public: 
  virtual const char* what() const throw()
  {
    return "A nonlocal choice node has been found.";
  }
};

class ResultFinder: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    if(n->get_marked() == MARKED)
    {
      n->set_marked(NONE);
      throw NonlocalNode();
    }
  }
};


class FindNonLocalListener: public WhiteNodeFoundListener
{
private:
  bool m_nonlocal_found;
public:
  FindNonLocalListener(void)
    :m_nonlocal_found(false)
  {}
  void on_white_node_found(HMscNode *n);
  bool nonlocal_found(void)
  {
    return m_nonlocal_found;
  }
};

class InitListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n);
};

class IterationListener: public WhiteNodeFoundListener
{
private:
  bool m_change;
public:
  IterationListener(void)
    :m_change(true){}
  void reset(void)
  {
    m_change = false;
  }
  void on_white_node_found(HMscNode *n);
  bool changed(void)
  { 
    return m_change;
  }
};

class CleanupListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n);
};

class BMscInitListener: public WhiteEventFoundListener
{
private:
  bool m_idle;
  bool m_first;
  bool m_mep;
public:
  void on_white_event_found(Event *e);
  BMscInitListener(void)
    :m_idle(true),m_first(true),m_mep(false)
  {}
  void reset(void)
  {
    m_idle = true;
    m_first = true;
    m_mep = false;
  }
  bool is_idle(void)
  { 
    return m_idle;
  }
  bool is_mep(void)
  {
    return m_mep;
  }
};

class SCLOCALCHOICE_EXPORT LocalChoiceChecker: public Checker, public HMscChecker
{
protected:
  
  /**
   * Common instance.
   */
  static LocalChoiceCheckerPtr m_instance;

  BMscGraphDuplicator m_graph_duplicator;
 
        
public:
  static const std::string lc_mep_attribute;
  static const std::string lc_nip_attribute;
    
  LocalChoiceChecker(){};
  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Local Choice"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"localchoice/localchoice.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfy universal boundedness property.
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
  
  
  static LocalChoiceCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = LocalChoiceCheckerPtr(new LocalChoiceChecker());
    return m_instance;
  }

};
 
// $Id: local_choice_checker.h 1029 2011-02-02 22:17:59Z madzin $
