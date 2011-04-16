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
 * Copyright (c) 2008 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: communication_graph.cpp 513 2009-12-09 15:25:08Z vacek $
 */

#include "check/pseudocode/communication_graph.h"

const std::string CommunicationGraph::lexical_order_attribute = "lexical_order";

void CommunicationGraphListener::on_white_event_found(Event *e)
{
  unsigned from, to;
  bool status;

  if(e->is_send())
  {
    if(!e->is_matched())
      return;
    from = e->get_instance()->get_attribute(m_index_attribute, 0, status);
    to = e->get_matching_event()->get_instance()->get_attribute(m_index_attribute, 0, status);
    m_graph.at(from).at(to) += 1;
  }
}


void CommunicationGraph::create_from_bmsc_with_param(BMscPtr bmsc, const std::string & param_name, unsigned graph_size)
{
  m_graph.clear();
  if(graph_size == 0)
    graph_size = bmsc->get_instances().size();
  m_graph.resize(graph_size);
  for(unsigned i = 0; i < graph_size; i++)
  {
    m_graph.at(i).resize(graph_size);
    for(unsigned j = 0; j < graph_size; j++)
      m_graph.at(i).at(j) = 0;
  }

  DFSEventsTraverser graph_creator;
  CommunicationGraphListener graph_creator_listener(m_graph, param_name);
  graph_creator.add_white_event_found_listener(&graph_creator_listener);
  graph_creator.traverse(bmsc);
}

void CommunicationGraph::create_from_bmsc_with_map(BMscPtr bmsc, const std::map<std::wstring, unsigned> & instance_map)
{
  InstancePtrList::const_iterator it;
  unsigned current_num;
  for(it = bmsc->get_instances().begin(); it != bmsc->get_instances().end(); it++)
  {
    current_num = instance_map.find((*it)->get_label())->second;
    (*it)->set_attribute(lexical_order_attribute, current_num);
  }

  create_from_bmsc_with_param(bmsc, lexical_order_attribute, instance_map.size());

  for(it = bmsc->get_instances().begin(); it != bmsc->get_instances().end(); it++)
    (*it)->remove_attribute<unsigned>(lexical_order_attribute);

}
void CommunicationGraph::create_from_bmsc(BMscPtr bmsc)
{
  std::map<std::wstring, InstancePtr> instance_map;
  std::map<std::wstring, InstancePtr>::iterator instance_map_iterator;
  const InstancePtrList& instance_list = bmsc->get_instances();
  InstancePtrList::const_iterator instance_list_iterator;
  
  for(instance_list_iterator =  instance_list.begin();
      instance_list_iterator != instance_list.end();
      instance_list_iterator++)
  {
    instance_map[instance_list_iterator->get()->get_label()] = *instance_list_iterator;
  }
  unsigned num = 0;
  for(instance_map_iterator =  instance_map.begin();
      instance_map_iterator != instance_map.end();
      instance_map_iterator++)
  {
    instance_map_iterator->second.get()->set_attribute(lexical_order_attribute, num);
    num++;
  }
 
  create_from_bmsc_with_param(bmsc, lexical_order_attribute, num);

  for(instance_list_iterator =  instance_list.begin();
      instance_list_iterator != instance_list.end();
      instance_list_iterator++)
  {
    instance_list_iterator->get()->remove_attribute<unsigned>(lexical_order_attribute);
  }
}

void CommunicationGraph::merge(const CommunicationGraph &graph)
{
  if(m_graph.size() != graph.get_graph().size())
    throw std::runtime_error("Incompatible graphs to be merged");

  for(unsigned i = 0; i < m_graph.size(); i++)
    for(unsigned j = 0; j < m_graph.size(); j++)
      m_graph[i][j] += graph.get_graph()[i][j];
}

void CommunicationGraph::tarjan(unsigned v)
{
  unsigned i;
  m_indexes.at(v) = m_index;
  m_lowlink.at(v) = m_index;
  m_index++;
  m_vertex_stack.push_back(v);
  m_in_stack.at(v) = true;

  for(i = 0; i < m_graph.size(); i++)
  {
    if(m_graph.at(v).at(i) > 0)
    {
      if(m_indexes.at(i) == -1 || m_in_stack.at(i))
      {
        if(m_indexes.at(i) == -1)
          tarjan(i);
        m_lowlink.at(v) = std::min(m_lowlink.at(i), m_lowlink.at(v));
      }
    }
  }
  if(m_lowlink.at(v) == m_indexes.at(v))
  {
    do
    {
      i = m_vertex_stack.back();
      m_vertex_stack.pop_back();
      m_in_stack.at(i) = false;
      m_scc.at(i) = m_current_scc;
    }while(v != i);
    m_current_scc++;
  }

}
bool CommunicationGraph::is_strongly_connected()
{
  m_vertex_stack.clear();
  m_indexes.clear();
  m_lowlink.clear();
  m_scc.clear();
  m_in_stack.clear();
  unsigned v, w;
  m_indexes.resize(m_graph.size());
  m_lowlink.resize(m_graph.size());
  m_in_stack.resize(m_graph.size());
  m_index = 0;
  m_current_scc = 1;
  m_scc.resize(m_graph.size());
  for(v = 0; v < m_graph.size(); v++)
  {
    m_indexes.at(v) = -1;
    m_lowlink.at(v) = -1;
    m_in_stack.at(v) = false;
    m_scc.at(v) = 0;
  }

  for(v = 0; v < m_graph.size(); v++)
  {
    if(m_indexes.at(v) == -1)
      tarjan(v);
  }


  for(v = 0; v < m_graph.size(); v++)
    for(w = 0; w < m_graph.size(); w++)
      if(m_graph.at(v).at(w) > 0 && m_scc.at(v) != m_scc.at(w))
        return false;
  return true;
}

// $Id: communication_graph.cpp 513 2009-12-09 15:25:08Z vacek $
