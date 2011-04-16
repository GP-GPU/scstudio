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
 * $Id: race_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include <list>


#include "check/race/race_checker.h"

BMscRaceCheckingListener::BMscRaceCheckingListener(RaceChecker* checker, ChannelMapperPtr mapper)
{
  m_checker = checker;
  m_mapper = mapper;
}

void BMscRaceCheckingListener::on_white_node_found(ReferenceNode* node)
{
  BMscPtr b = node->get_bmsc();
  if(b.get())
  {
    std::list<BMscPtr> example = m_checker->check_bmsc(b,m_mapper);
    if(!example.empty())
      node->set_attribute("rc_bmsc", example);
    //precompute MinP for b
  }
}

//////////////////////////////////////////////////////////////////////

ExtremeEvents& MinimalEventsInitiator::get_events(BMsc* b)
{
  ExtremeEvents extreme(m_instance_marker->get_count());

  bool just_set;
  ExtremeEvents& e = b->get_attribute<ExtremeEvents>(m_events_attribute,extreme,just_set);
  if(just_set)
  {
    m_modified_bmscs.push_back(b);
  }
  return e;
}

void MinimalEventsInitiator::on_white_node_found(ReferenceNode* node)
{
  BMscPtr b = node->get_bmsc();
  const InstancePtrList& instances = b->get_instances();
  //compute minimal Events of Instances
  EventPList minimal_events;
  InstancePtrList::const_iterator i;
  for(i=instances.begin();i!=instances.end();i++)
  {
    EventPListPtr events = DFSInstanceEventsTraverser::topology_order((*i).get());
    EventPList::const_iterator e1;
    for(e1 = events->begin();e1!=events->end();e1++)
    {
      EventPList::const_iterator e2;
      for(e2 = events->begin();e2!=events->end();e2++)
      {
        if(e1!=e2)
        {
          BoolVector& e2_causal = m_caus_initiator->get_causal_closure(*e2);
          size_t e1_index = m_vis_initiator->get_topology_index(*e1);
          //e2<<e1 => e1 isn't minimal
          if(e2_causal[e1_index]) break;
        }
      }
      //'while loop' hasn't found any Event e2 to be less than e1
      if(e2==events->end())
        minimal_events.push_back(*e1);
    }
  }
  /*
  For each Event e1 from minimal_events find Instances containing less Events
  than Event e1 in minimal_events.
  Note that if there exists any Event e2<e1 at Instance i, e2 must be grater
  or equal to some Event e3 in minimal_events, therefore we will use only
  minimal_events to seek.
  */
  EventPList::const_iterator e;
  ExtremeEvents& extreme_events = get_events(b.get());
  m_modified_bmscs.push_back(b.get());
  for(e=minimal_events.begin();e!=minimal_events.end();e++)
  {
    EDInstancesPtr e_instances(
      new EventDependentInstances(*e,m_instance_marker->get_count()));
    size_t e_index = m_vis_initiator->get_topology_index(*e);
    EventPList::const_iterator f;
    for(f=minimal_events.begin();f!=minimal_events.end();f++)
    {
      //if f is less than e (not equal) then Instance of f must be inserted
      if(e!=f)
      {
        BoolVector& closure = m_caus_initiator->get_causal_closure(*f);
        if(closure[e_index])
        {
          size_t f_id = m_instance_marker->get_instance_id((*f)->get_instance());
          e_instances->set_dependent(f_id);
        }
      }
    }
    size_t e_id = m_instance_marker->get_instance_id((*e)->get_instance());
    extreme_events.add_extreme_event(e_id,e_instances);
  }
}

void MinimalEventsInitiator::cleanup_attributes(){
  while(!m_modified_bmscs.empty())
  {
    BMsc* b = m_modified_bmscs.back();
    b->remove_attribute<ExtremeEvents>(m_events_attribute);
    m_modified_bmscs.pop_back();
  }
}

ExtremeEvents& MaximalEventsInitiator::get_events_less(BMsc* b)
{
  ExtremeEvents extreme(m_instance_marker->get_count());
  bool just_set;
  ExtremeEvents& e = b->get_attribute<ExtremeEvents>(m_events_less_attribute,extreme,just_set);
  if(just_set)
  {
    m_modified_bmscs.push_back(b);
  }
  return e;
}

void MaximalEventsInitiator::on_white_node_found(ReferenceNode* node)
{
  BMscPtr b = node->get_bmsc();
  const InstancePtrList& instances = b->get_instances();
  //compute maximal Events of Instances
  EventPList maximal_events;
  InstancePtrList::const_iterator i;
  for(i=instances.begin();i!=instances.end();i++)
  {
    EventPListPtr events = DFSInstanceEventsTraverser::topology_order((*i).get());
    EventPList::const_iterator e1 = events->begin();
    while(e1!=events->end())
    {
      BoolVector& e1_causal = m_caus_initiator->get_causal_closure(*e1);
      EventPList::const_iterator e2 = events->begin();
      while(e2!=events->end())
      {
        //we are looking only for different events to be greater
        if(*e1!=*e2)
        {
          size_t e2_index = m_vis_initiator->get_topology_index(*e2);
          //e1<<e2 => e1 isn't maximal
          if(e1_causal[e2_index]) break;
        }
        e2++;
      }
      //'while loop' hasn't found any Event e2 to be greater than e1
      if(e2==events->end())
        maximal_events.push_back(*e1);
      e1++;
    }
  }
  /*
   * For any Event e1 from maximal_events find Instances containing any Event e2
   * such that e2<<e1 and instances containing any Event e3 such that e1<<e3.
   */
  EventPList::const_iterator e;
  ExtremeEvents& extreme_events_greater = get_events_greater(b.get());
  ExtremeEvents& extreme_events_less = get_events_less(b.get());
  m_modified_bmscs.push_back(b.get());
  EventPListPtr events = DFSEventsTraverser::topology_order(b);
  for(e=maximal_events.begin();e!=maximal_events.end();e++)
  {
    EDInstancesPtr e_instances_greater(
      new EventDependentInstances(*e,m_instance_marker->get_count()));
    EDInstancesPtr e_instances_less(
      new EventDependentInstances(*e,m_instance_marker->get_count()));
    size_t e_index = m_vis_initiator->get_topology_index(*e);
    const BoolVector& e_closure = m_caus_initiator->get_causal_closure(*e);
    EventPList::const_iterator f;
    /*
     Note that in the article only Events from MinP (used D \in MinP notation in
     the article) are supposed to be checked. Nothing changes if we check all
     Events moreover we don't need to find MinP.
     */
    for(f=events->begin();f!=events->end();f++)
    {
      //we are looking only for events of different instances
      if((*e)->get_instance()!=(*f)->get_instance())
      {
        const BoolVector& f_closure = m_caus_initiator->get_causal_closure(*f);
        size_t f_index = m_vis_initiator->get_topology_index(*f);
        size_t f_instance_id = m_instance_marker->get_instance_id((*f)->get_instance());
        //f is less than e
        if(f_closure[e_index])
        {
          //e_instances_greater->set_dependent(f_instance_id);
          e_instances_less->set_dependent(f_instance_id);
        }
        //e is less than f
        if(e_closure[f_index])
        {
          //e_instances_less->set_dependent(f_instance_id);
          e_instances_greater->set_dependent(f_instance_id);
        }
      }
    }
    size_t e_instance_id = m_instance_marker->get_instance_id((*e)->get_instance());
    extreme_events_greater.add_extreme_event(e_instance_id,e_instances_greater);
    extreme_events_less.add_extreme_event(e_instance_id,e_instances_less);
  }
}

void MaximalEventsInitiator::cleanup_attributes(){
  while(!m_modified_bmscs.empty())
  {
    BMsc* b = m_modified_bmscs.back();
    b->remove_attribute<ExtremeEvents>(m_events_attribute);
    m_modified_bmscs.pop_back();
  }
}

RaceChecker::RaceChecker()
{
  m_min_events_initiator = MinimalEventsInitiator(
    &m_visual_initiator,&m_causal_initiator,&m_instance_marker);
  m_max_events_initiator = MaximalEventsInitiator(
    &m_visual_initiator,&m_causal_initiator,&m_instance_marker);
}

Checker::PreconditionList RaceChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;

  // checker for HMSC assumes msc to be deadlock free, livelock free, acyclic and FIFO
  HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);
  if(hmsc != NULL)
  {
    result.push_back(PrerequisiteCheck(L"Deadlock Free", PrerequisiteCheck::PP_REQUIRED));
    result.push_back(PrerequisiteCheck(L"Livelock Free", PrerequisiteCheck::PP_REQUIRED));
    result.push_back(PrerequisiteCheck(L"Nonrecursivity", PrerequisiteCheck::PP_REQUIRED)); // Due to BMscGraphDuplicator
  }
  
  result.push_back(PrerequisiteCheck(L"Acyclic", PrerequisiteCheck::PP_REQUIRED));
  result.push_back(PrerequisiteCheck(L"FIFO", PrerequisiteCheck::PP_REQUIRED));

  return result;
}

BMscPtr RaceChecker::create_counter_example(Event* e1, Event* e2)
{
  BMscDuplicator duplicator;
  BMscPtr original = e1->get_instance()->get_bmsc();

  e1->set_marked();
  e2->set_marked();
  e1->get_message().get()->set_marked();
  e2->get_message().get()->set_marked();

  BMscPtr copy = duplicator.duplicate_bmsc(original);

  e1->set_marked(NONE);
  e2->set_marked(NONE);
  e1->get_message().get()->set_marked(NONE);
  e2->get_message().get()->set_marked(NONE);
  return copy;
}

bool RaceChecker::check_events(Event* e1, Event* e2)
{
  //e1<e2 or they are unordered because of topology order with respect to <
  BoolVector& e1_visual_order = m_visual_initiator.get_visual_closure(e1);
  BoolVector& e1_causal_order = m_causal_initiator.get_causal_closure(e1);
  size_t e2_index = m_visual_initiator.get_topology_index(e2);
  if(e1_visual_order[e2_index] && e1->get_instance()->get_label()==e2->get_instance()->get_label())
  {
    //if e1<e2 check causality
    return e1_causal_order[e2_index];
  }
  return true;
}

std::list<BMscPtr> RaceChecker::check_bmsc(BMscPtr bmsc, ChannelMapperPtr mapper)
{
  EventPListPtr t = DFSEventsTraverser::topology_order(bmsc);
  EventPVector topology(
    t->begin(),
    t->end()
  );
  m_visual_initiator.initialize(topology);
  m_causal_initiator.initialize(topology,m_visual_initiator,mapper);
  std::list<BMscPtr> result;
  for(size_t i = 0; i < topology.size(); i++)
  {
    for(size_t j=i+1; j< topology.size(); j++)
    {
      if(!check_events(topology[i],topology[j]))
      {
        result.push_back(create_counter_example(topology[i],topology[j]));
      }
    }
  }
  return result;
}

std::list<BMscPtr> RaceChecker::check(BMscPtr bmsc, ChannelMapperPtr mapper)
{
  std::list<BMscPtr> result = check_bmsc(bmsc,mapper);
  // note: no cleanup in check_bmsc() as it's called in check_hmsc
  cleanup_attributes();
  return result;
}

void RaceChecker::cleanup_attributes()
{
  m_visual_initiator.cleanup_attributes();
  m_causal_initiator.cleanup_attributes();
  m_instance_marker.cleanup_attributes();
  m_min_events_initiator.cleanup_attributes();
  m_max_events_initiator.cleanup_attributes();
  m_graph_duplicator.cleanup_attributes();
}

void RaceChecker::prepare_hmsc(HMscPtr hmsc,ChannelMapperPtr mapper)
{
  //mark instances by id and count number of different instances
  DFSBMscGraphTraverser traverser;
  traverser.add_white_node_found_listener(&m_instance_marker);
  traverser.traverse(hmsc);

  traverser.remove_all_listeners();
  //note that it is not neccessary to cleanup traversing attributes
  //compute MinP and MaxP

  traverser.add_white_node_found_listener(&m_min_events_initiator);
  traverser.add_white_node_found_listener(&m_max_events_initiator);
  traverser.traverse(hmsc);

}

std::list<HMscPtr> RaceChecker::check(HMscPtr hmsc, ChannelMapperPtr mapper)
{
  //transform hmsc into BMsc graph
  HMscPtr transformed = m_graph_duplicator.duplicate_hmsc(hmsc);
  m_graph_duplicator.cleanup_attributes();
  

  DFSBMscGraphTraverser traverser;
  AddInstance a_i;
  traverser.add_white_node_found_listener(&a_i);
  traverser.traverse(transformed);
  traverser.remove_all_listeners();

  //compute causal closure of all BMsc and check BMscs to be race free
  //WARNING: causal closure for the HMsc checker is computed by the BMsc checker!
  std::list<HMscPtr> res = check_bmscs(transformed,mapper);
  

  //precompute possible things for race checking - e.g. MinP
  prepare_hmsc(transformed,mapper);
  //check hmsc to be race free

  std::list<HMscPtr> res2 = check_hmsc(transformed,mapper);
  cleanup_attributes();

  res.splice(res.begin(), res2);

  //The dummy instance is removed and names of reference nodes are made unique.
  PostProcess r_i;
  traverser.add_white_node_found_listener(&r_i);

  for(std::list<HMscPtr>::iterator it = res.begin(); it!=res.end(); it++)
  {
    traverser.traverse(*it);
    r_i.reset();
  }
  return res;
}

std::list<HMscPtr> RaceChecker::check_hmsc(HMscPtr hmsc,ChannelMapperPtr mapper)
{
  HMscPtr error;
  FootprintTraverser t(hmsc.get(),mapper.get(),m_instance_marker.get_count(),
    &m_min_events_initiator,&m_max_events_initiator);
  t.traverse();

  std::list<HMscPtr> counterexamples;
  for(std::list<RaceInHMscException>::const_iterator i = t.get_counterexamples().begin();
    i != t.get_counterexamples().end();
    i++)
    counterexamples.push_back(create_counter_example(*i));

  t.cleanup();
  return counterexamples;
}

/**
 * Checks HMsc to have only race free BMscs
 */
std::list<HMscPtr> RaceChecker::check_bmscs(HMscPtr hmsc, ChannelMapperPtr mapper)
{
  std::list<HMscPtr> counter_example;

  BMscRaceCheckingListener bmsc_race_listener(this,mapper);
  BMscResultsCatcher result_catcher;
  DFSBMscGraphTraverser traverser;

  traverser.add_white_node_found_listener(&bmsc_race_listener);
  traverser.traverse(hmsc);
  traverser.remove_all_listeners();
  traverser.cleanup_traversing_attributes();

  traverser.add_white_node_found_listener(&result_catcher);

  bool result_found;
  do
  {
    result_found = false;
    try
    {
      traverser.traverse(hmsc);
    }
    catch(std::list<BMscPtr>& e)
    {
      std::list<HMscPtr> list = create_counter_example(traverser.get_reached_elements(),e);
      counter_example.splice(counter_example.begin(), list);
      result_found = true;
    }
    traverser.cleanup_traversing_attributes();
  }while(result_found);
  
  traverser.cleanup_traversing_attributes();

  return counter_example;
}

std::list<HMscPtr> RaceChecker::create_counter_example(const MscElementPListList& path, std::list<BMscPtr> examples)
{
  HMscFlatPathDuplicator duplicator;
  std::list<BMscPtr>::const_iterator example;
  std::list<HMscPtr> result;
  for(example = examples.begin(); example != examples.end(); example++)
  {
    //result contains path in already duplicated HMsc
    HMscPtr hmsc = duplicator.duplicate_path(path.back());
    //set proper msc to last element
    ReferenceNode* last_copy = dynamic_cast<ReferenceNode*>(
      duplicator.get_copy(path.back().back()));
    last_copy->set_msc(*example);
    last_copy->set_marked();
    result.push_back(hmsc);
    duplicator.cleanup_attributes();
  }
  return result;
}

HMscPtr RaceChecker::create_counter_example(RaceInHMscException e)
{
  //build run in BMsc graph
  FootprintPtr f = e.get_footprint();
  MscElementPList path;
  do
  {
	  path.insert(path.begin(),f->get_path().begin(),f->get_path().end());
    f = f->get_previous();
  }while(f);

  //mark elements
  e.get_first()->get_complete_message()->set_marked();
  e.get_second()->get_complete_message()->set_marked();
  e.get_first()->set_marked();
  e.get_second()->set_marked();

  //The duplicator duplicates the marked BMscs.
  HMscFlatPathDuplicator duplicator;
  HMscPtr result = duplicator.duplicate_path(path);
  
  //Unmark elements in the original BMsc
  e.get_first()->get_complete_message()->set_marked(NONE);
  e.get_second()->get_complete_message()->set_marked(NONE);
  e.get_first()->set_marked(NONE);
  e.get_second()->set_marked(NONE);

  MscElementPList::const_iterator elem;
  for(elem=path.begin();elem!=path.end();elem++)
  {
    ReferenceNode* r = dynamic_cast<ReferenceNode*>(*elem);
    if(r && (r->get_bmsc().get()==e.get_first()->get_instance()->get_bmsc() ||
      r->get_bmsc().get()==e.get_second()->get_instance()->get_bmsc()))
    {
      duplicator.get_copy(r)->set_marked();
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////////

FootprintTraverser::FootprintTraverser(
  HMsc* hmsc,
  ChannelMapper* mapper,
  size_t instances_count,
  MinimalEventsInitiator* min,
  MaximalEventsInitiator* max)
{
  m_min = min;
  m_max = max;
  m_mapper = mapper;
  FootprintPtr initial(new Footprint(hmsc->get_start().get(),instances_count));
  todo.insert(initial);
  add_white_node_found_listener(this);
  add_gray_node_found_listener(this);
}

FootprintPtr FootprintTraverser::extract_todo()
{
  FootprintPtr first = *todo.begin();
  todo.erase(todo.begin());
  return first;
}

void FootprintTraverser::traverse()
{
  while(!todo.empty())
  {
    m_footprint = extract_todo();
    done.insert(m_footprint);
    m_first_node = m_footprint->get_node();
    m_first_node->set_attribute("rc_fn", true); //FIRST node may be traversed
    traverse(m_footprint->get_node());
    m_first_node->remove_attribute<bool>("rc_fn");
  }
}

void FootprintTraverser::traverse(HMscNode* node)
{
  push_top_attributes();
  set_color(node,GRAY); //set node to GRAY to avoid him to be traversed
  m_reached_elements.back().push_back(node); //push him on top to simulate it was already traversed
  PredecessorNode* pred = dynamic_cast<PredecessorNode*>(node);
  if(pred)
  {
    traverse_successors(pred);
  }
  cleanup_traversing_attributes();
}

void FootprintTraverser::on_white_node_found(ReferenceNode* node)
{
  //When the BMsc is idle, no action is performed.
  BMscPtr bmsc = node->get_bmsc();
  const InstancePtrList& inst = bmsc->get_instances();
  InstancePtrList::const_iterator iti;

  for(iti = inst.begin(); iti != inst.end(); iti++)
    if((*iti)->has_events())
      break;
  iti = inst.begin();
  if(iti == inst.end())
  {
    FootprintPtr f(new Footprint(
		m_footprint,this->get_reached_elements().back(),
		m_max->get_events_less(node->get_bmsc().get()),
		m_max->get_events_greater(node->get_bmsc().get())));
    todo.insert(f);
    return;
  }

  FootprintPtr f(new Footprint(
		m_footprint,this->get_reached_elements().back(),
		m_max->get_events_less(node->get_bmsc().get()),
		m_max->get_events_greater(node->get_bmsc().get())));
  check_race(node->get_bmsc().get(),f);

  if(todo.find(f)==todo.end() && done.find(f)==done.end())
  {
    todo.insert(f);
  }
}

void FootprintTraverser::check_race(BMsc* bmsc, FootprintPtr& f)
{
  ExtremeEvents& min_p = m_min->get_events(bmsc);
  const EDInstancesPtrSetVector& bmsc_edi = min_p.get_events_instances();
  for(size_t i=0; i<bmsc_edi.size(); i++)
  {
    EDInstancesPtrSet::const_iterator b;
    for(b=bmsc_edi[i].begin();b!=bmsc_edi[i].end();b++)
    {
      Event* b_event = (*b)->get_event();
      if(b_event->is_receive())
      {
        const BoolVector& b_instances = (*b)->get_instances();
        const EDInstancesPtrSetVector& fooprint_edi = m_footprint->get_events_instances();
        EDInstancesPtrSet::const_iterator a;
        for(a=fooprint_edi[i].begin();a!=fooprint_edi[i].end();a++)
        {
          Event* a_event = (*a)->get_event();
          if(a_event->is_receive() && !m_mapper->same_channel(a_event,b_event))
          {
            m_counterexamples.push_back(RaceInHMscException(f,b_event,a_event,get_reached_elements().back()));
          }
          else if(a_event->is_send())
          {
            const BoolVector& a_instances = (*a)->get_instances();
            size_t j = 0;
            for(; j<a_instances.size(); j++)
            {
              if(a_instances[j] && b_instances[j])
              {
                break;
              }
            }
            if(j==a_instances.size())
            {
              m_counterexamples.push_back(RaceInHMscException(f,b_event,a_event,get_reached_elements().back()));
            }
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

RaceInHMscException::RaceInHMscException(FootprintPtr& footprint,Event* first, Event* second,
                                         const MscElementPList& path_to_second):
  m_footprint(footprint),m_first(first),
   m_second(second),m_path_to_second(path_to_second)
{
}

FootprintPtr& RaceInHMscException::get_footprint()
{
  return m_footprint;
}

Event* RaceInHMscException::get_first()
{
  return m_first;
}

Event* RaceInHMscException::get_second()
{
  return m_second;
}

MscElementPList& RaceInHMscException::get_path_to_second()
{
  return m_path_to_second;
}

// $Id: race_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
