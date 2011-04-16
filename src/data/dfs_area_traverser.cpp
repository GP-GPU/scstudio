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
 * $Id: dfs_area_traverser.cpp 433 2009-10-17 14:59:43Z gotthardp $
 */

#include "data/dfs_area_traverser.h"

DFSAreaTraverser::DFSAreaTraverser(const std::string& color):
DFSEventsTraverser(color)
{

}

void DFSAreaTraverser::traverse_area(EventArea* area)
{
  if(!area) return;
  StrictOrderArea* strict = dynamic_cast<StrictOrderArea*>(area);
  if(strict)
  {
    if(!strict->is_empty())
      traverse_strict_event(strict->get_first().get());
  }
  else
  {
    CoregionArea* coregion = dynamic_cast<CoregionArea*>(area);
    const CoregionEventPVector& minimals = coregion->get_minimal_events();
    CoregionEventPVector::const_iterator min;
    for(min=minimals.begin(); min!=minimals.end(); min++)
      traverse_coregion_event(*min);
  }
}

void DFSAreaTraverser::traverse(BMscPtr bmsc)
{
  InstancePtrList::const_iterator instance;
  const InstancePtrList& instances = bmsc->get_instances();
  for(instance=instances.begin(); instance!=instances.end(); instance++)
  {
    EventArea* area = (*instance)->get_first().get();
    while(area)
    {
      traverse_area(area);
      area = area->get_next().get();
    }
  }
  cleanup_traversing_attributes();
}

void DFSAreaTraverser::traverse_coregion_event(CoregionEvent* event)
{
  m_reached_elements.push_back(event);
  if(!is_processed(event))
  {
    const CoregEventRelPtrVector& successors = event->get_successors();
    CoregEventRelPtrVector::const_iterator successor;
    for(successor=successors.begin(); successor!=successors.end(); successor++)
    {
      m_reached_elements.push_back((*successor).get());
      traverse_coregion_event((*successor)->get_successor());
      m_reached_elements.pop_back();
    }
    event_finished(event);
  }
}

void DFSAreaTraverser::traverse_strict_event(StrictEvent* event)
{
  m_reached_elements.push_back(event);
  if(!is_processed(event))
  {
    if(event->get_successor().get())
      traverse_strict_event(event->get_successor().get());
    event_finished(event);
  }
}

// $Id: dfs_area_traverser.cpp 433 2009-10-17 14:59:43Z gotthardp $
