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
 * $Id: communication_graph.h 513 2009-12-09 15:25:08Z vacek $
 */

#ifndef _COMMUNICATION_GRAPH_H
#define _COMMUNICATION_GRAPH_H

#include <vector>
#include <map>
#include "data/msc.h"
#include "data/dfs_events_traverser.h"
#include "check/pseudocode/export.h"

typedef std::vector<std::vector<unsigned> > Graph;

class SCPSEUDOCODE_EXPORT CommunicationGraph
{
public:
  bool is_strongly_connected(void);
  CommunicationGraph(void)
    :m_size(0){}
  CommunicationGraph(BMscPtr bmsc)
  {
    m_size = 0;
    this->create_from_bmsc(bmsc);
  }
  //! Fills the internal Graph structure according to the communication specified by the given bmsc.
  /**
   *  Indexes of instances are given by the lexical order of instances' labels.
   */
  void create_from_bmsc(BMscPtr bmsc);

  //! Fills the internal Graph structure according to the communication specified by the given bmsc.
  /**
   *  Indexes of instances are taken from the map which maps instances' labels to indexes.
   */
  void create_from_bmsc_with_map(BMscPtr bmsc, const std::map<std::wstring, unsigned> & instance_map);

  //! Fills the internal Graph structure according to the communication specified by the given bmsc.
  /**
   *  Indexes of instances are taken from the dynamic attribute whose name is input.
   *  If no size of the graph is specified, the number of instances in the bmsc is used.
   */
  void create_from_bmsc_with_param(BMscPtr bmsc, const std::string & param_name, unsigned graph_size = 0);
  void clear(void)
  {
    m_graph.clear();
    m_size = 0;
  }
  void merge(const CommunicationGraph &graph);
  static const std::string lexical_order_attribute;

  const Graph& get_graph() const
  {
    return m_graph;
  }

private:
  unsigned m_size;
  unsigned m_index;
  unsigned m_current_scc;
  std::vector<unsigned> m_vertex_stack;
  Graph m_graph;
  std::vector<int> m_indexes;
  std::vector<int> m_lowlink;
  std::vector<bool> m_in_stack;
  std::vector<unsigned> m_scc;
  void tarjan(unsigned v);
};

class CommunicationGraphListener:public WhiteEventFoundListener
{
private:
  Graph& m_graph;
  std::string m_index_attribute;
public:
  void on_white_event_found(Event *e);
  CommunicationGraphListener(Graph& destination, std::string index_attribute)
    :m_graph(destination), m_index_attribute(index_attribute)
  {}
};
#endif /* _COMMUNICATION_GRAPH_H */

// $Id: communication_graph.h 513 2009-12-09 15:25:08Z vacek $

