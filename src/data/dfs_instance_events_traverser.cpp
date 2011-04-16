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
 * $Id: dfs_instance_events_traverser.cpp 431 2009-10-16 20:20:17Z gotthardp $
 */

#include "data/dfs_instance_events_traverser.h"
#include "check/pseudocode/utils.h"

#include <string>

DFSInstanceEventsTraverser::DFSInstanceEventsTraverser(const std::string& color):DFSEventsTraverser(color)
{
  
}

void DFSInstanceEventsTraverser::traverse_strict_event(StrictEvent* event)
{
  m_reached_elements.push_back(event);
  if(!is_processed(event))
  {
    if(event->get_successor().get())
      traverse_strict_event(event->get_successor().get());
    else
      traverse_area(event->get_area()->get_next().get());
    event_finished(event);
  }
}

void DFSInstanceEventsTraverser::traverse(Instance* instance)
{
  traverse_area(instance->get_first().get());
  cleanup_traversing_attributes();
}

void DFSInstanceEventsTraverser::traverse_coregion_event(CoregionEvent* event)
{
  m_reached_elements.push_back(event);
  if(!is_processed(event))
  {
    if(event->has_successors())
    {
      const CoregEventRelPtrVector& successors = event->get_successors();
      CoregEventRelPtrVector::const_iterator successor;
      for(successor=successors.begin(); successor!=successors.end(); successor++)
      {
        m_reached_elements.push_back((*successor).get());
        traverse_coregion_event((*successor)->get_successor());
        m_reached_elements.pop_back();
      }
    }
    else
      traverse_area(event->get_area()->get_next().get());
    event_finished(event);
  }
}

EventPListPtr DFSInstanceEventsTraverser::topology_order(Instance* i)
{
  EventPListPtr topology(new EventPList);
  DFSInstanceEventsTraverser traverser;
  TopologyOrderListener listener(topology.get());
  traverser.add_event_finished_listener(&listener);
  traverser.traverse(i);
  return topology;
}

// $Id: dfs_instance_events_traverser.cpp 431 2009-10-16 20:20:17Z gotthardp $
