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
 * Copyright (c) 2009 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: utils.cpp 433 2009-10-17 14:59:43Z gotthardp $
 */

#include "check/pseudocode/utils.h"

void EventFirstSuccessors::get_first_area_events(EventArea* area,Event* e)
{
  if(!area)
    return;

  StrictOrderArea* strict = dynamic_cast<StrictOrderArea*>(area);
  if(strict)
  {
    if(strict->is_empty())
      get_first_area_events(strict->get_next().get(),e);
    else 
      m_succs.insert(strict->get_first().get());
  }
  else
  {
    CoregionArea* coregion = dynamic_cast<CoregionArea*>(area);
    if(coregion->is_empty())
      get_first_area_events(coregion->get_next().get(),e);
    else
    {
      const CoregionEventPVector& minimals = coregion->get_minimal_events();
      CoregionEventPVector::const_iterator min;
      for(min=minimals.begin(); min!=minimals.end(); min++)
        m_succs.insert(*min);
    }
  }
}

void EventFirstSuccessors::get_strict_successors(StrictEvent *e)
{
  if(e->get_successor().get())
    m_succs.insert(e->get_successor().get());
  else 
    get_first_area_events(e->get_area()->get_next().get(),e);
}


void EventFirstSuccessors::get_coregion_successors(CoregionEvent *e)
{
  if(e->get_successors().size()!=0)
  {
    const CoregEventRelPtrVector& successors = e->get_successors();
    CoregEventRelPtrVector::const_iterator succ;
    for(succ=successors.begin(); succ!=successors.end(); succ++)
    {
      m_succs.insert((*succ)->get_successor());
    }
  }
  else
    get_first_area_events(e->get_area()->get_next().get(),e);
}

void EventFirstSuccessors::get_successors(Event *e)
{
  StrictEvent* strict = dynamic_cast<StrictEvent*>(e);
  if(strict)
  {
    get_strict_successors(strict);
  }
  else
  {
    CoregionEvent* coregion = dynamic_cast<CoregionEvent*>(e);
    get_coregion_successors(coregion);
  }
  // if its send event
  if(e->is_send() && e->get_matching_event())
  {
     m_succs.insert(e->get_matching_event());
  }
}

//////////////////////////////////////////////////////////

WhiteNodeMarker::WhiteNodeMarker(const std::string& mark):
m_mark(mark)
{
}

WhiteNodeMarker::~WhiteNodeMarker()
{
  cleanup_attributes();
}

void WhiteNodeMarker::cleanup_attributes()
{
  remove_attributes<HMscNodePList,bool>(m_modified,m_mark);
}

void WhiteNodeMarker::on_white_node_found(HMscNode* n)
{
  bool& mark = get_mark(n);
  mark = true;
}

bool& WhiteNodeMarker::get_mark(HMscNode* n)
{ 
  bool just_set;
  bool& mark = n->get_attribute(m_mark,false,just_set);
  if(just_set)
  {
    m_modified.push_back(n);
  }
  return mark;
}

//////////////////////////////////////////////////////////


NoendingNodesEliminator::NoendingNodesEliminator()
{
  m_back_traverser.add_white_node_found_listener(&m_marker);
}

void NoendingNodesEliminator::eliminate(HMscPtr& hmsc)
{
  m_back_traverser.traverse(hmsc);
  HMscNodePList noending;
  HMscNodePtrSet::const_iterator i;
  for(i=hmsc->get_nodes().begin();i!=hmsc->get_nodes().end();i++)
  {
    if(!m_marker.get_mark((*i).get()))
    {
      noending.push_back((*i).get());
    }
  }
  m_marker.cleanup_attributes();
  HMscNodePList::const_iterator n;
  for(n=noending.begin();n!=noending.end();n++)
  {
    (*n)->get_owner()->remove_node(*n);
  }
}

///////////////////////////////////////////////////////////////

void InstanceIdMarker::set_instance_id(Instance* i)
{
  StringSizeTMap::iterator it = m_identifiers.find(i->get_label());
  size_t id;
  if(it!=m_identifiers.end())
  {
    id = (*it).second;
  }
  else
  {
    id = m_identifiers.size();
    m_identifiers[i->get_label()] = id;
  }
  i->set_attribute<size_t>(m_instance_id_attribute,id);
  m_modified_instances.push_back(i);
}
  
InstanceIdMarker::InstanceIdMarker(const std::string& instance_id_attribute)
{
  m_instance_id_attribute = instance_id_attribute;
}

void InstanceIdMarker::on_white_node_found(ReferenceNode* node)
{
  BMscPtr bmsc = node->get_bmsc();
  if(bmsc.get())
  {
    InstancePtrList::const_iterator i;
    const InstancePtrList& instances = bmsc->get_instances();
    for(i=instances.begin();i!=instances.end();i++)
    {
      InstancePtr instance = *i;
      set_instance_id(instance.get());
    }
  }
}

size_t InstanceIdMarker::get_instance_id(Instance* i)
{
  size_t id = i->get_attribute<size_t>(m_instance_id_attribute,0);
  return id;
}

void InstanceIdMarker::cleanup_attributes()
{
  while(!m_modified_instances.empty())
  {
    m_modified_instances.front()->remove_attribute<size_t>(m_instance_id_attribute);
    m_modified_instances.pop_front();
  }
  m_identifiers.clear();
}

size_t InstanceIdMarker::get_count()
{
  return m_identifiers.size();
}

// $Id: utils.cpp 433 2009-10-17 14:59:43Z gotthardp $
