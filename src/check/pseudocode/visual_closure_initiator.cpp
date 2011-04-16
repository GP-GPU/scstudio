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
 * $Id: visual_closure_initiator.cpp 433 2009-10-17 14:59:43Z gotthardp $
 */

#include "check/pseudocode/visual_closure_initiator.h"

void VisualClosureInitiator::make_closure(std::vector<BoolVector*>& closure_matrix, size_t succ_index, size_t pred_index)
{
  if(!(*closure_matrix[pred_index])[succ_index])
  {
    (*closure_matrix[pred_index])[succ_index] = true;
    for(size_t g=0; g<pred_index; g++)
    {
      if((*closure_matrix[g])[pred_index])
        (*closure_matrix[g])[succ_index] = true;
    }
  }
}

void VisualClosureInitiator::make_closure(std::vector<BoolVector*>& closure_matrix, size_t succ_index, EventArea* pred_area)
{
  if(pred_area->is_empty())
  {
    if(!pred_area->is_first())
      make_closure(closure_matrix,succ_index,pred_area->get_previous());
  }
  else
  {
    StrictOrderArea* strict_area = dynamic_cast<StrictOrderArea*>(pred_area);
    if(strict_area)
    {
      make_closure(closure_matrix,succ_index,get_topology_index(strict_area->get_last().get()));
    }
    else
    {
      CoregionArea* coregion_area = dynamic_cast<CoregionArea*>(pred_area);
      CoregionEventPVector::const_iterator max;
      const CoregionEventPVector& max_events = coregion_area->get_maximal_events();
      for(max=max_events.begin();max!=max_events.end();max++)
      {
        make_closure(closure_matrix,succ_index,get_topology_index(*max));
      }
    }
  }
}

VisualClosureInitiator::~VisualClosureInitiator()
{
  cleanup_attributes();
}

void VisualClosureInitiator::cleanup_attributes()
{
  EventPList::iterator e;
  for(e=m_modified_events.begin();e!=m_modified_events.end();e++)
  {
    (*e)->remove_attribute<size_t>(m_topology_index_attribute);
    (*e)->remove_attribute<BoolVector>(m_visual_closure_attribute);
  }
  m_modified_events.erase(m_modified_events.begin(),m_modified_events.end());
}

void VisualClosureInitiator::initialize(const EventPVector& event_topology)
{
  //initialize vectors of visual closure of events
  //closure[x][y]==true <=> x<*y
  std::vector<BoolVector*> closure(event_topology.size());
  for(size_t i=0;i<event_topology.size();i++)
  {
    Event* event = event_topology[i];
    BoolVector& event_closure = get_visual_closure(event);
    event_closure.resize(event_topology.size(),false);
    set_topology_index(event,i);
    closure[i] = &event_closure;
    m_modified_events.push_back(event);
  }
  //compute visual closures of events' ordering
  make_closure(closure,event_topology);
}

void VisualClosureInitiator::make_closure(std::vector<BoolVector*>& closure, 
  const EventPVector& event_topology)
{
  for(size_t e=0; e<event_topology.size(); e++)
  {
    (*closure[e])[e] = true;
    Event* event_e = event_topology[e];
    //send event is predecessor
    if(event_e->is_receive() && event_e->is_matched())
    {
      make_closure(closure,e,get_topology_index(event_e->get_matching_event()));
    }
    make_closure(closure,e,event_e);    
  }
}

void VisualClosureInitiator::make_closure(std::vector<BoolVector*>& closure, 
  size_t e, Event* event_e)
{
  StrictEvent* strict_event = dynamic_cast<StrictEvent*>(event_e);
  if(strict_event)
  {
    if(!strict_event->is_first())
    {
      //complete closure with direct predecessor
      make_closure(closure,e,get_topology_index(strict_event->get_predecessor()));
    }
    else if(strict_event->get_area()->get_previous())
    {
      make_closure(closure,e,strict_event->get_area()->get_previous());
    }          
  }
  else
  {
    CoregionEvent* coregion_event = dynamic_cast<CoregionEvent*>(event_e);
    if(!coregion_event->is_minimal())
    {
      //complete closure with direct predecessors
      CoregEventRelPtrVector::const_iterator pred_rel;
      const CoregEventRelPtrVector& predecessors = coregion_event->get_predecessors();
      for(pred_rel=predecessors.begin();pred_rel!=predecessors.end();pred_rel++)
        make_closure(closure,e,get_topology_index((*pred_rel)->get_predecessor()));
    }
    else if(coregion_event->get_area()->get_previous())
    {
      make_closure(closure,e,coregion_event->get_area()->get_previous());
    }
  }
}

size_t VisualClosureInitiator::get_topology_index(Event* e)
{
  return e->get_attribute<size_t>(m_topology_index_attribute,0);
}

void VisualClosureInitiator::set_topology_index(Event* e,size_t i)
{
  e->set_attribute<size_t>(m_topology_index_attribute,i);
}

// $Id: visual_closure_initiator.cpp 433 2009-10-17 14:59:43Z gotthardp $
