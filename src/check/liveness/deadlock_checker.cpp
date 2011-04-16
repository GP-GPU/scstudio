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
 * $Id: deadlock_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "data/dfsb_hmsc_traverser.h"
#include "data/dfs_hmsc_traverser.h"
#include "check/liveness/deadlock_checker.h"
#include "check/pseudocode/msc_duplicators.h"
#include "data/node_finder.h"

DeadlockCheckerPtr DeadlockChecker::m_instance;

void FindDeadlockListener::on_white_node_found(HMscNode *n)
{
  if(!dynamic_cast<ReferenceNode *>(n))
    return;
  if(NodeFinder::successors(n, "dedlock_color")->empty())
    n->set_attribute("is_deadlock", true);
}


Checker::PreconditionList DeadlockChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}

HMscPtr DeadlockChecker::create_counter_example(const MscElementPListList& path)
{
  HMscPathDuplicator duplicator;
  HMscPtr example = duplicator.duplicate_path(path);
  MscElement* last = path.back().back();
  duplicator.get_copy(last)->set_marked(MARKED);
  return example;
}

std::list<HMscPtr> DeadlockChecker::check(HMscPtr hmsc, ChannelMapperPtr chm)
{
  FindDeadlockListener fi_de_l;
  ResultFinderListener re_fi_l;
  DFSHMscTraverser hmsc_traverser;
  hmsc_traverser.add_white_node_found_listener(&fi_de_l);
  hmsc_traverser.traverse(hmsc);
  hmsc_traverser.remove_all_listeners();
  hmsc_traverser.add_white_node_found_listener(&re_fi_l);
  std::list<HMscPtr> result;
  bool result_found = true;
  while(result_found)
  {
    result_found = false;
    
    try
    {
      hmsc_traverser.traverse(hmsc);
    }
    catch(DeadlockException)
    {
      result.push_back(create_counter_example(hmsc_traverser.get_reached_elements()));
      result_found = true;
      hmsc_traverser.cleanup_traversing_attributes();
    }
  }

  return result;
  /*
  //this will be used do show eventual counterexample
  HMscPtr p(NULL);
  DeadlockListener listener;
  DFSBMscGraphTraverser traverser;
  traverser.add_white_node_found_listener(&listener);
  traverser.add_gray_node_found_listener(&listener);
  traverser.add_node_finished_listener(&listener);
  try
  {
    traverser.traverse(hmsc);
  }
  catch (DeadlockException)
  {
    p = create_counter_example(traverser.get_reached_elements());
    traverser.cleanup_traversing_attributes();
  }

  if(p != NULL)
    result.push_back(p);
  return result;
  */
}

void DeadlockChecker::cleanup_attributes()
{
}

// $Id: deadlock_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
