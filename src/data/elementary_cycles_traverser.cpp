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
 * Copyright (c) 2010 Vaclav Vacek <vacek@ics.muni.cz>
 *
 * $Id: elementary_cycles_traverser.cpp 746 2010-05-08 12:14:32Z gotthardp $
 */

#include "data/elementary_cycles_traverser.h"

void ElementaryCyclesTraverser::cycle_found(const MscElementPList &cycle)
{
  ElementaryCycleListenerPList::iterator l;
  for(l=m_cycle_listeners.begin();l!=m_cycle_listeners.end();l++)
    (*l)->on_elementary_cycle_found(cycle);
}

MscElementPListList TarjanCycles::circuit_enumeration(HMscNode *n)
{
  m_point_stack.clear();
  m_marked_stack.clear();
  m_mark.clear();
  m_result.clear();
  m_current_path.clear();
  m_s = n->get_attribute("Cycles_number", 0); 
  m_mark.resize(m_vertex_count, false);

  backtrack(n);
  return m_result;
}

bool TarjanCycles::backtrack(HMscNode *n)
{
  bool f = false, g = false;
  //The default value 0 is never to be used - the attribute is set for sure.
  unsigned v = n->get_attribute("Cycles_number", 0); 
  unsigned j, w;

  m_point_stack.push_back(v);
  m_current_path.push_back(n); //!!
  m_mark[v] = true;
  m_marked_stack.push_back(v);
  PredecessorNode* predecessor = dynamic_cast<PredecessorNode*>(n);
  NodeRelationPtrVector::const_iterator relation;

  //for all successors
  if(predecessor)
  {
    for(relation=predecessor->get_successors().begin();
    relation!=predecessor->get_successors().end();relation++)
    {
      const NodeRelationPtr& rel = *relation;

      f = false;
      w = dynamic_cast<HMscNode*>(rel->get_successor())->get_attribute("Cycles_number", 0);

      if(w < m_s)
        continue;
      if(m_restriction != "")
        if(!(dynamic_cast<HMscNode*>(rel->get_successor())->is_attribute_set(m_restriction)))
          continue;

      if(w == m_s)
      {
        m_current_path.push_back(rel.get());
        m_current_path.push_back(dynamic_cast<HMscNode*>(rel.get()->get_successor()));
        m_result.push_back(m_current_path);
        m_current_path.pop_back();
        m_current_path.pop_back();
        f = true;
      }
      else if(!m_mark[w])
      {
        m_current_path.push_back(rel.get());
        g = backtrack(dynamic_cast<HMscNode*>(rel->get_successor()));
        m_current_path.pop_back();
        f = f || g;
      }
    }
  }
  if(f == true)
  {
    j = m_marked_stack.size() - 1;
    while(m_marked_stack[j] != v && j >=0)
    {
      m_mark[m_marked_stack[j]] = false;
      m_marked_stack.pop_back();
      j--;
    }
    if(j >= 0)
    {
      m_marked_stack.pop_back();
      m_mark[v] = false;
    }
  }
  if(!m_marked_stack.empty())
  {
    if(m_marked_stack.back()==v)
    {
      m_mark[v] = false;
      m_marked_stack.pop_back();
    }
  }

  m_point_stack.pop_back();
  m_current_path.pop_back();
  return f;
}

void ElementaryCyclesTraverser::traverse(HMscPtr hmsc)
{
  AssignListener assign_li;
  CleanupListener clean_li;
  m_traverser.add_white_node_found_listener(&assign_li);
  m_traverser.traverse(hmsc);
  m_traverser.remove_all_listeners();

  CycleListener cycle_li(assign_li.get_vertex_count(), m_cycle_listeners, m_restriction_name);
  m_traverser.add_white_node_found_listener(&cycle_li);
  m_traverser.traverse(hmsc);
  m_traverser.remove_all_listeners();

  m_traverser.add_white_node_found_listener(&clean_li);
  m_traverser.traverse(hmsc);
  m_traverser.remove_all_listeners();
}

void CycleListener::on_white_node_found(HMscNode *n)
{
  if(m_restriction != "")
    if(!n->is_attribute_set(m_restriction))
      return;
  TarjanCycles cycle_finder(m_vertex_count, m_restriction);
  MscElementPListList cycle_list;
  cycle_list = cycle_finder.circuit_enumeration(n);
  MscElementPListList::const_iterator cycles;
  ElementaryCycleListenerPList::const_iterator listeners;

  for(cycles = cycle_list.begin(); cycles != cycle_list.end(); cycles++)
    for(listeners = m_listeners.begin(); listeners != m_listeners.end(); listeners++)
      (*listeners)->on_elementary_cycle_found(*cycles);

}

// $Id: elementary_cycles_traverser.cpp 746 2010-05-08 12:14:32Z gotthardp $
