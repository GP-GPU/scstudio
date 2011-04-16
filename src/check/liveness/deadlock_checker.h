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
 * Copyright (c) 2008 Jindra Babica <babica@mail.muni.cz>
 *
 * $Id: deadlock_checker.h 789 2010-05-16 20:44:30Z vacek $
 */

#include <string>
#include <stack>
#include <exception>

#include "data/checker.h"
#include "data/msc.h"
#include "data/dfs_bmsc_graph_traverser.h"
#include "data/dfsb_hmsc_traverser.h"
#include "check/liveness/export.h"

class DeadlockChecker;

typedef boost::shared_ptr<DeadlockChecker> DeadlockCheckerPtr;

class FindDeadlockListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n);
};

class DeadlockException:public std::exception
{
  
public:
  virtual const char* what() const throw()
  {
    return "There was found deadlock in HMsc";
  }
};

class ResultFinderListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    if(n->is_attribute_set("is_deadlock"))
    {
      n->remove_attribute<bool>("is_deadlock");
      throw DeadlockException();
    }
  }
};



class SCLIVENESS_EXPORT DeadlockChecker: public Checker, public HMscChecker
{
  
protected:
  
  /**
   * Common instance.
   */
  static DeadlockCheckerPtr m_instance;
  
  HMscPtr create_counter_example(const MscElementPListList& path);

public:
    
  DeadlockChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Deadlock Free"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"deadlock/deadlock.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfy deadlock free property.
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
  
  ~DeadlockChecker(){}
  
  static DeadlockCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = DeadlockCheckerPtr(new DeadlockChecker());
    return m_instance;
  }

};
 
// $Id: deadlock_checker.h 789 2010-05-16 20:44:30Z vacek $
