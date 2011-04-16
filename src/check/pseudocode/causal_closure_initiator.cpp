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
 * $Id: causal_closure_initiator.cpp 126 2008-12-01 20:46:33Z babicaj $
 */

#include <stack>

#include "check/pseudocode/causal_closure_initiator.h"

void CausalClosureInitiator::cleanup_attributes()
{
  while(!m_modified_events.empty())
  {
    Event* e = m_modified_events.top();
    e->remove_attribute<BoolVector>(m_causal_closure_attribute);
    m_modified_events.pop();
  }
}

void CausalClosureInitiator::initialize(const EventPVector& events, 
  VisualClosureInitiator& visual_closure_init, ChannelMapperPtr mapper)
{
  //initialize matrix
  size_t events_count = events.size();
  EventPList::const_iterator i;
  //closure[x][y]==true <=> x << y
  std::vector<BoolVector*> closure(events_count);
  for(size_t i=0;i<events_count;i++)
  {
    Event* i_event = events[i];
    BoolVector& causal_closure = get_causal_closure(i_event);
    causal_closure.resize(events_count,false);
    closure[i] = &causal_closure;
    m_modified_events.push(i_event);
    //for any event e: e << e
    causal_closure[i] = true;
    //send event << receive matching event
    if(i_event->is_send() && i_event->is_matched())
      causal_closure[visual_closure_init.get_topology_index(
        i_event->get_matching_event())] = true;
  }
  for(size_t i=0;i<events_count;i++)
  {
    Event* i_event = events[i];
    BoolVector& i_causal_closure = *closure[i];
    BoolVector& i_visual_closure = visual_closure_init.get_visual_closure(i_event);
    for(size_t j = 0; j < events_count; j++)
    {
      Event* j_event = events[j];
      //i_event and j_event are from the same instance, j_event is send event and
      //i_event < j_event
      if(i_event->get_instance()==j_event->get_instance() && 
        j_event->is_send() && i_visual_closure[j])
      {
        i_causal_closure[j] = true;
      }
      //i_event and j_event are send events of the same channel and i_event<j_event
      if(i_event->is_send() && i_event->is_matched() && 
        j_event->is_send() && j_event->is_matched() && 
        mapper->same_channel(i_event,j_event) && i_visual_closure[j])
      {
        get_causal_closure(i_event->get_matching_event())[
          visual_closure_init.get_topology_index(j_event->get_matching_event())
        ] = true;
      }
    }
  }
  square_closure(closure);
}

void CausalClosureInitiator::square_closure(std::vector<BoolVector*>& closure)
{
  size_t events_count = closure.size();
  bool changed;
  do
  {
    changed = false;
    for(size_t i=0; i<events_count; i++)
      for(size_t j=0; j<events_count; j++)
        if(!(*closure[i])[j] && multiply(closure,i,j))
          (*closure[i])[j] = changed = true;
  }
  while(changed);
}

bool CausalClosureInitiator::multiply(std::vector<BoolVector*>& closure, size_t row, size_t column)
{
  for(size_t index=0;index<closure.size();index++)
  {
    if((*closure[row])[index] && (*closure[index])[column])
      return true;
  }
  return false;
}

// $Id: causal_closure_initiator.cpp 126 2008-12-01 20:46:33Z babicaj $
