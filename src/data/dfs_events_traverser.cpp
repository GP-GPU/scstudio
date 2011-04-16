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
 * $Id: dfs_events_traverser.cpp 719 2010-04-14 21:26:46Z vacek $
 */

#include "data/dfs_events_traverser.h"
#include "check/pseudocode/utils.h"

DFSEventsTraverser::DFSEventsTraverser(const std::string& color)
{
  m_color = color;
}

DFSEventsTraverser::~DFSEventsTraverser()
{
  cleanup_traversing_attributes();
}

void DFSEventsTraverser::traverse(BMscPtr bmsc)
{
  InstancePtrList::const_iterator instance;
  const InstancePtrList& instances = bmsc->get_instances();
  for(instance=instances.begin(); instance!=instances.end(); instance++)
  {
    traverse_area((*instance)->get_first().get(), NULL);
  }
  cleanup_traversing_attributes();
}

void DFSEventsTraverser::traverse_area(EventArea* area, Event* predecessor)
{
  if(!area) return;
  StrictOrderArea* strict = dynamic_cast<StrictOrderArea*>(area);
  if(strict)
  {
    if(strict->is_empty())
      traverse_area(strict->get_next().get(), predecessor);
    else
      if(predecessor != NULL)
        event_successor(predecessor, strict->get_first().get());
      traverse_strict_event(strict->get_first().get());
  }
  else
  {
    CoregionArea* coregion = dynamic_cast<CoregionArea*>(area);
    //Does CoregionArea have any event?
    if(coregion->is_empty())
      traverse_area(coregion->get_next().get(), predecessor);
    else
    {
      const CoregionEventPVector& minimals = coregion->get_minimal_events();
      CoregionEventPVector::const_iterator min;
      for(min=minimals.begin(); min!=minimals.end(); min++)
      {
        if(predecessor != NULL)
          event_successor(predecessor, (*min));
        traverse_coregion_event(*min);
      }
    }
  }
}

void DFSEventsTraverser::traverse_strict_event(StrictEvent* event)
{
  m_reached_elements.push_back(event);
  if(!is_processed(event))
  {
    traverse_matching_event(event);
    if(event->get_successor().get())
    {
      event_successor(event, event->get_successor().get());
      traverse_strict_event(event->get_successor().get());
    }
    else
      traverse_area(event->get_area()->get_next().get(), event);

    event_finished(event);
  }
}

void DFSEventsTraverser::traverse_coregion_event(CoregionEvent* event)
{
  m_reached_elements.push_back(event);
  if(!is_processed(event))
  {
    traverse_matching_event(event);
    if(event->get_successors().size()!=0)
    {
      const CoregEventRelPtrVector& successors = event->get_successors();
      CoregEventRelPtrVector::const_iterator successor;
      for(successor=successors.begin(); successor!=successors.end(); successor++)
      {
        m_reached_elements.push_back((*successor).get());
        event_successor(event, (*successor)->get_successor());
        traverse_coregion_event((*successor)->get_successor());
        m_reached_elements.pop_back();
      }
    }
    else
      traverse_area(event->get_area()->get_next().get(), event);

    event_finished(event);
  }
}

void DFSEventsTraverser::traverse_matching_event(Event* event)
{
  if(event->is_send() && event->get_matching_event())
  {
    m_reached_elements.push_back(event->get_message().get());
    send_receive_pair(event, event->get_matching_event());

    StrictEvent* strict = dynamic_cast<StrictEvent*>(event->get_matching_event());
    if(strict)
    {
      traverse_strict_event(strict);
    }
    else
    {
      CoregionEvent* e = dynamic_cast<CoregionEvent*>(event->get_matching_event());
      traverse_coregion_event(e);
    }

    m_reached_elements.pop_back();
  }
}

bool DFSEventsTraverser::is_processed(Event* event)
{
  NodeColor c = get_color(event);
  if(c==BLACK) 
  {
    black_event_found(event);
    return true;
  }
  if(c==GRAY)
  {
    gray_event_found(event);
    return true;
  }
  white_event_found(event);
  return false;
}

void DFSEventsTraverser::cleanup_traversing_attributes()
{
  EventPList::iterator event;
  for(event=m_colored_events.begin();event!=m_colored_events.end();event++)
    (*event)->remove_attribute<NodeColor>(m_color);
  m_colored_events.clear();
}

void DFSEventsTraverser::white_event_found(Event* e)
{
  set_color(e,GRAY);
  m_colored_events.push_back(e);
  WhiteEventFoundListenerPList::iterator l;
  for(l=white_event_found_listeners.begin();l!=white_event_found_listeners.end();l++)
    (*l)->on_white_event_found(e);
}

void DFSEventsTraverser::gray_event_found(Event* e)
{
  GrayEventFoundListenerPList::iterator l;
  for(l=gray_event_found_listeners.begin();l!=gray_event_found_listeners.end();l++)
    (*l)->on_gray_event_found(e);
  m_reached_elements.pop_back();
}

void DFSEventsTraverser::black_event_found(Event* e)
{
  BlackEventFoundListenerPList::iterator l;
  for(l=black_event_found_listeners.begin();l!=black_event_found_listeners.end();l++)
    (*l)->on_black_event_found(e);
  m_reached_elements.pop_back();
}

void DFSEventsTraverser::event_finished(Event* e)
{
  set_color(e,BLACK);
  EventFinishedListenerPList::iterator l;
  for(l=event_finished_listeners.begin();l!=event_finished_listeners.end();l++)
    (*l)->on_event_finished(e);
  m_reached_elements.pop_back();
}

void DFSEventsTraverser::event_successor(Event* event, Event* successor)
{
  EventSuccessorListenerPList::iterator l;
  for(l=event_successor_listeners.begin();l!=event_successor_listeners.end();l++)
    (*l)->on_event_successor(event, successor);
}

void DFSEventsTraverser::send_receive_pair(Event* send, Event* receive)
{
  SendReceivePairListenerPList::iterator l;
  for(l=send_receive_pair_listeners.begin();l!=send_receive_pair_listeners.end();l++)
    (*l)->on_send_receive_pair(send, receive);
}

EventPListPtr DFSEventsTraverser::topology_order(BMscPtr b)
{
  EventPListPtr topology(new EventPList);
  DFSEventsTraverser traverser;
  TopologyOrderListener listener(topology.get());
  traverser.add_event_finished_listener(&listener);
  traverser.traverse(b);
  return topology;
}

NodeColor DFSEventsTraverser::get_color(Event* e)
{
  return e->get_attribute<NodeColor>(m_color,WHITE);
}

void DFSEventsTraverser::remove_white_event_found_listeners()
{
  white_event_found_listeners.clear();
}

void DFSEventsTraverser::remove_gray_event_found_listeners()
{
  gray_event_found_listeners.clear();
}

void DFSEventsTraverser::remove_black_event_found_listeners()
{
  black_event_found_listeners.clear();
}

void DFSEventsTraverser::remove_event_finished_listeners()
{
  event_finished_listeners.clear();
}

void DFSEventsTraverser::remove_event_successor_listeners()
{
  event_successor_listeners.clear();
}

void DFSEventsTraverser::remove_send_receive_pair_listeners()
{
  send_receive_pair_listeners.clear();
}

void DFSEventsTraverser::remove_all_listeners()
{
  remove_white_event_found_listeners();
  remove_gray_event_found_listeners();
  remove_black_event_found_listeners();
  remove_event_finished_listeners();
  remove_event_successor_listeners();
  remove_send_receive_pair_listeners();
}

// $Id: dfs_events_traverser.cpp 719 2010-04-14 21:26:46Z vacek $
