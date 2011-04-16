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
 * $Id: bmsc_race_checker.cpp 125 2008-11-29 21:18:12Z babicaj $
 */

#include "check/race/race_checker.h"

void MinimalEventsInitiator::on_white_node_found(ReferenceNode* node)
{
  BMscPtr b = node->get_bmsc();
  const InstancePtrList& instances = b->get_instances();
  //compute minimal Events of Instances
  EventPList minimal_events;
  InstancePtrList::const_iterator i;
  for(i=instances.begin();i!=instances.end();i++)
  {
    EventPList* events = DFSInstanceEventsTraverser::topology_order(*i);
    EventPList::const_iterator e1 = events->begin();
    while(e1!=events->end())
    {
      EventPList::const_iterator e2 = events->begin();
      while(e2!=events->end())
      {
        if(e1!=e2)
        {
          Event* e = *e2;
          BoolVector& e2_causal = m_caus_initiator->get_causal_closure(*e2);
          size_t e1_index = m_vis_initiator->get_topology_index(*e1);
          //e2<<e1 => e1 isn't minimal
          if(e2_causal[e1_index]) break;
        }
        e2++;
      }
      //'while loop' hasn't found any Event e2 to be less than e1
      if(e2==events.end())
        minimal_events.push_back(*e1);
      e1++;
    }
    delete events;
  }
  /*
  For the minimal_events find Instances containing less Events.
  Note that if there exists any less Event e1 at Instance i, e1 must be grater
  or equal to some Event e2 in minimal_events, therefore we will use only
  minimal_events to seek.
  */
  EventPList::const_iterator e;
  ExtremeEvents& extreme_events = get_minimal_events(b.get());
  for(e=minimal_events.begin();e!=minimal_events.end();e++)
  {
    EventDependentInstances e_instances(*e,m_instance_marker.get_count());
    size_t e_index = m_vis_initiator->get_topology_index(*e);
    EventPList::const_iterator f;
    for(f=minimal_events.begin();f!=minimal_events.end();f++)
    {
      //if f is less than e then Instance of f must be inserted
      BoolVector& closure = m_caus_initiator->get_causal_closure(*f);
      if(closure[e_index])
        e_instances.set_dependent(
          m_instance_marker->get_id((*f)->get_instance()));
    }
    extreme_events.add_extreme_event(e_instances);
  }
  m_modified_bmscs.push_back(b.get());
}

BMsc* BMscRaceChecker::create_counter_example(Event* e1, Event* e2)
{
  BMsc* bmsc = new BMsc(e1->get_instance()->get_bmsc());
  InstancePtr common_instance = new Instance(bmsc,e1->get_instance());
  bmsc->add_instance(common_instance);
  BoolVector& e2_causal = m_causal_initiator.get_causal_closure(e2);
  size_t e1_index = m_visual_initiator.get_topology_index(e1);
  MscMessagePtr e1_message;
  if(e1->is_send())
    e1_message = new MscMessage(
      common_instance.get(),NULL,e1->get_message().get());
  else
    e1_message = new MscMessage(
      NULL,common_instance.get(),e1->get_message().get());
  MscMessagePtr e2_message;
  if(e2->is_send())
    e2_message = new MscMessage(
      common_instance.get(),NULL,e2->get_message().get());
  else
    e2_message = new MscMessage(
      NULL,common_instance.get(),e2->get_message().get());
  //e2 << e1
  if(e2_causal[e1_index])
  {
    StrictOrderAreaPtr common_strict = new StrictOrderArea(
      common_instance.get());
    StrictEventPtr e1_strict = new StrictEvent(common_strict.get(),e1_message,e1);
    StrictEventPtr e2_strict = new StrictEvent(common_strict.get(),e2_message,e2);
    common_strict->set_first(e1_strict);
    e1_strict->set_successor(e2_strict);
    common_instance->set_first(common_strict);
  }
  //e2 || e1
  else
  {
    CoregionAreaPtr common_coregion = new CoregionArea(common_instance.get());
    CoregionEvent* e1_coregion = new CoregionEvent(
      common_coregion.get(),e1_message,e1);
    CoregionEvent* e2_coregion = new CoregionEvent(
      common_coregion.get(),e2_message,e2);
    common_coregion->add_minimal_event(e1_coregion);
    common_coregion->add_minimal_event(e2_coregion);
    common_instance->set_first(common_coregion);
  }
  return bmsc;
}

BMscPtr BMscRaceChecker::check(BMscPtr bmsc, ChannelMapperPtr mapper)
{
  DFSEventsTraverser traverser;
  TopologicalOrderListener topology_listener;
  traverser.add_event_finished_listener(this);
  traverser.traverse(bmsc);
  EventPVector topology(
    topology_listener.get_topology().begin(),
    topology_listener.get_topology().end()
  );
  m_visual_initiator.initialize(topology);
  m_causal_initiator.initialize(topology,m_visual_initiator,mapper);
  OrderCheckerListener order_checker_listener(&m_visual_initiator);
  DFSInstanceEventsTraverser instance_traverser;
  for(size_t i = 0; i < topology.size(); i++)
  {
    //in this block checkout whether there is any Event in race with topology[i]
    order_checker_listener.reset(
      topology[i],
      &m_visual_initiator.get_visual_closure(topology[i]),
      &m_causal_initiator.get_causal_closure(topology[i]));
    try
    {
      instance_traverser.traverse(topology[i]->get_instance());
      instance_traverser.cleanup_traversing_attributes();
    }
    catch(EventRaceException& e)
    {
      instance_traverser.cleanup_traversing_attributes();
      return create_counter_example(topology[i],e->get_event());
    }
  }
  return BMscPtr();
}

void BMscRaceChecker::cleanup_attributes()
{
  m_visual_initiator.cleanup_attributes();
  m_causal_initiator.cleanup_attributes();
  while(!m_modified_instaces.empty())
  {
    Instance* i = m_modified_instaces.front();
    m_modified_instaces.pop_front();
    i->remove_attribute<EventPList>(m_instance_events_attribute);
  }
}

// $Id: bmsc_race_checker.cpp 125 2008-11-29 21:18:12Z babicaj $
