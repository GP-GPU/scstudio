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
 * Copyright (c) 2008 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: livelock_checker.h 992 2010-10-20 15:21:23Z vacek $
 */

#include "data/checker.h"
#include "data/msc.h"
#include "data/dfs_hmsc_traverser.h"
#include "data/dfsb_hmsc_traverser.h"
#include "check/liveness/export.h"
#include "data/elementary_cycles_traverser.h"

#include <string>
#include <stack>
#include <exception>

class ForwardMarker: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    n->set_attribute("lc_reach", true);
  }
};

class BackwardMarker: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    n->remove_attribute<bool>("lc_reach");
  }
};

class HMscFinder: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    if(!dynamic_cast<StartNode*>(n))
      return;

    DFSBHMscTraverser back_traverser;
    BackwardMarker b_m;
    back_traverser.add_white_node_found_listener(&b_m);
    back_traverser.traverse(n->get_owner());
  }
};

class FoundCycle: public ElementaryCycleListener
{
public:
  void on_elementary_cycle_found(const MscElementPList &cycle)
  {
    MscElementPListList result, dummy;
    MscElementPList::const_iterator it;
    bool was_reference = false;
    for(it = cycle.begin(); it != cycle.end(); it++)
      if(dynamic_cast<ReferenceNode*>(*it))
        was_reference = true;
    if(!was_reference)
      return;
    result = cycle.front()->get_attribute("lc_cycle", dummy);
    result.push_back(cycle);
    cycle.front()->remove_attribute<MscElementPListList>("lc_cycle");
    cycle.front()->set_attribute("lc_cycle", result);
  }
};

class CycleFinder: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    if(!dynamic_cast<StartNode*>(n))
      return;

    ElementaryCyclesTraverser cycle_traverser;
    FoundCycle f_c;
    cycle_traverser.enable_restriction("lc_reach");
    cycle_traverser.add_cycle_listener(&f_c);
    cycle_traverser.traverse(n->get_owner());
  }
};

class Unmarker: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    n->remove_attribute<bool>("lc_reach");
  }
};



class CatchResults: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    if(n->is_attribute_set("lc_cycle"))
    {
      MscElementPListList result, dummy;
      result = n->get_attribute("lc_cycle", dummy);
      n->remove_attribute<MscElementPListList>("lc_cycle");
      throw result;
    }
  }
};

// main class
class LivelockChecker;

typedef boost::shared_ptr<LivelockChecker> LivelockCheckerPtr;



/**
 * LivelockChecker goes throw hmsc using traverser looking for end nodes 
 * to maek all nodes on the way backwards reachable ( mark_reachable()).
 * Then goes again looking for circle (gray listener) that is not reachable.
 */
class SCLIVENESS_EXPORT LivelockChecker: public Checker, public HMscChecker
{
protected:
  /**
   * Common instance
   */
  static LivelockCheckerPtr m_instance;
  
  HMscPtr create_counter_example(const MscElementPListList& to_cycle, MscElementPList cycle);
  
public:

  LivelockChecker();

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Livelock Free"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"livelock/livelock.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  static LivelockCheckerPtr instance();
  
  bool is_supported(ChannelMapperPtr chm);
  
  ~LivelockChecker();
  
  std::list<HMscPtr> check(HMscPtr h, ChannelMapperPtr chm);

  void cleanup_attributes();

};

// $Id: livelock_checker.h 992 2010-10-20 15:21:23Z vacek $
