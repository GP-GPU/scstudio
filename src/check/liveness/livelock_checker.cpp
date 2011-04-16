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
 * $Id: livelock_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "check/liveness/livelock_checker.h"
#include "data/dfs_refnode_hmsc_traverser.h"
#include "check/pseudocode/msc_duplicators.h"


LivelockCheckerPtr LivelockChecker::m_instance;


Checker::PreconditionList LivelockChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList precon_list;
  precon_list.push_back(PrerequisiteCheck(L"Nonrecursivity", PrerequisiteCheck::PP_REQUIRED)); // Due to BMscGraphDuplicator
  return precon_list;
}

std::list<HMscPtr> LivelockChecker::check(HMscPtr hmsc, ChannelMapperPtr chm)
{
  DFSHMscTraverser forward_traverser;
  ElementaryCyclesTraverser cycle_traverser;

  ForwardMarker f_m;
  HMscFinder h_f;
  FoundCycle f_c;
  Unmarker un_m;
  
  forward_traverser.add_white_node_found_listener(&f_m);
  forward_traverser.traverse(hmsc);
  forward_traverser.remove_all_listeners();

  forward_traverser.add_white_node_found_listener(&h_f);
  forward_traverser.traverse(hmsc);
  forward_traverser.remove_all_listeners();

  cycle_traverser.enable_restriction("lc_reach");
  cycle_traverser.add_cycle_listener(&f_c);
  cycle_traverser.traverse(hmsc);

  forward_traverser.add_white_node_found_listener(&un_m);
  forward_traverser.traverse(hmsc);
  forward_traverser.remove_all_listeners();

  CatchResults ca_re;
  forward_traverser.add_white_node_found_listener(&ca_re);
  bool was_result = true;
  std::list<HMscPtr> result;
  while(was_result)
  {
    was_result = false;
    try{
      forward_traverser.traverse(hmsc);
    }
    catch(MscElementPListList &cycles)
    {
      MscElementPListList::iterator it;
      MscElementPListList unb_cycles;
      for(it = cycles.begin(); it != cycles.end(); it++)
      {
        result.push_back(create_counter_example(forward_traverser.get_reached_elements(), *it));
      }
      forward_traverser.cleanup_traversing_attributes();
      was_result = true;
    }
  }
  return result;
}

HMscPtr LivelockChecker::create_counter_example(const MscElementPListList& to_cycle, MscElementPList cycle)
{
  HMscPtr p;
  HMscPathDuplicator duplicator;
  MscElementPList::const_iterator iter;
  MscElementPListList path = to_cycle;
  cycle.pop_back(); //The second occurence of the first node of the cycle is removed
  cycle.pop_back(); //together with the relation leading to the node.
  for(iter = cycle.begin(), iter++; iter!=cycle.end(); iter++) //first node is already stored in the previous step
    path.back().push_back(*iter);

  


  p = duplicator.duplicate_path(path);

  PredecessorNode* last = dynamic_cast<PredecessorNode*>(duplicator.get_copy(path.back().back()));
  SuccessorNode* first = dynamic_cast<SuccessorNode*>(duplicator.get_copy(*(cycle.begin())));
  last->add_successor(first)->set_marked();


  for(iter=cycle.begin();iter!=cycle.end();iter++)
  {
    duplicator.get_copy(*iter)->set_marked(MARKED);
  }
  duplicator.cleanup_attributes();

  return p;
}

LivelockChecker::LivelockChecker()
{
}



LivelockCheckerPtr LivelockChecker::instance()
{
  if(!m_instance.get())
    m_instance = LivelockCheckerPtr(new LivelockChecker());
  return m_instance;
}

bool LivelockChecker::is_supported(ChannelMapperPtr chm)
{
  return true;
}

LivelockChecker::~LivelockChecker()
{
  cleanup_attributes();
}

void LivelockChecker::cleanup_attributes()
{
}

// $Id: livelock_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
