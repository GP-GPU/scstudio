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
 * $Id: nonrecursivity_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "check/structure/nonrecursivity_checker.h"

NonrecursivityCheckerPtr NonrecursivityChecker::m_instance;

void RecursivityListener::on_gray_node_found(HMscNode *n)
{
  StartNode* startnode = dynamic_cast<StartNode*>(n);
  if(startnode != NULL)
  {
    throw RecursiveException();
  }
}

Checker::PreconditionList NonrecursivityChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}

HMscPtr NonrecursivityChecker::create_counter_example(const MscElementPListList& path)
{
  HMscPathDuplicator duplicator;
  HMscPtr example = duplicator.duplicate_path(path);

  MscElementPListList::const_iterator h;
  MscElementPList::const_iterator l;

  for(h=path.begin();h!=path.end();h++)
  {
    for(l = h->begin(); l != h->end(); l++)
      duplicator.get_copy(*l)->set_marked(MARKED);
  }
  example->get_start()->set_marked(MARKED);
  return example;
}

std::list<HMscPtr> NonrecursivityChecker::check(HMscPtr hmsc, ChannelMapperPtr chm)
{
  DFSHMscTraverser recursive_traverser;
  RecursivityListener rec_listener;
  HMscPtr p(NULL);
  recursive_traverser.add_gray_node_found_listener(&rec_listener);
  std::list<HMscPtr> result;

  try
  {
    recursive_traverser.traverse(hmsc);
  }
  catch(RecursiveException)
  {
    p = create_counter_example(recursive_traverser.get_reached_elements());
    recursive_traverser.cleanup_traversing_attributes();
	  result.push_back(p);
    return result;
  }
  return result;
}

void NonrecursivityChecker::cleanup_attributes()
{
}

// $Id: nonrecursivity_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
