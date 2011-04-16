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
 * $Id: fifo_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "check/order/fifo_checker.h"
#include "check/pseudocode/visual_closure_initiator.h"
#include "check/pseudocode/msc_duplicators.h"

const std::string FifoChecker::channel_id_attribute = "fifo_channel_id";
FifoCheckerPtr FifoChecker::m_instance;

Checker::PreconditionList FifoChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  result.push_back(PrerequisiteCheck(L"Acyclic", PrerequisiteCheck::PP_REQUIRED));

  return result;
}

BMscPtr FifoChecker::create_counter_example(BMscPtr& bmsc, Event* e_1, Event* e_2)
{
  BMscDuplicator duplicator;
  BMscPtr new_bmsc = duplicator.duplicate_bmsc(bmsc);
  Event* events[] = {e_1,e_2};
  for(size_t i=0;i<2;i++)
  {
    //event is surely complete
    Event* copy = duplicator.get_event_copy(events[i]);
    copy->get_complete_message()->set_marked(MARKED);
  }
  return new_bmsc;
}

std::list<BMscPtr> FifoChecker::check(BMscPtr bmsc, ChannelMapperPtr mapper)
{
  ChannelMapperPtr chm = mapper->copy();
  VisualClosureInitiator closure_initiator;
  TopologicalOrderListener topology_listener;
  DFSEventsTraverser traverser;
  traverser.add_event_finished_listener(&topology_listener);
  traverser.traverse(bmsc);
  EventPVector topology(topology_listener.get_topology().size());
  topology.assign(
    topology_listener.get_topology().begin(),
    topology_listener.get_topology().end()
  );
  closure_initiator.initialize(topology);
  std::list<BMscPtr> result;
  for(size_t e=0; e<topology.size(); e++)
  {
    BoolVector& closure_e = closure_initiator.get_visual_closure(topology[e]);
    for(size_t f=0; f<topology.size(); f++)
    {
      if(topology[e]->is_receive() && topology[f]->is_receive() &&
        topology[e]->is_matched() && topology[f]->is_matched() && 
        mapper->same_channel(topology[e],topology[f]) && closure_e[f] &&
        !closure_initiator.get_visual_closure(topology[e]->get_matching_event())[
          closure_initiator.get_topology_index(topology[f]->get_matching_event())])
      {
        result.push_back(create_counter_example(bmsc,topology[e],topology[f]));
      }
    }
  }
  cleanup_attributes();
  return result;
}

void FifoChecker::cleanup_attributes()
{
  while(!m_modified_events.empty())
  {
    Event* e = m_modified_events.top();
    m_modified_events.pop();
    e->remove_attribute<size_t>(channel_id_attribute);
  }
}

// $Id: fifo_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
