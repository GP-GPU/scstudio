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
 * $Id: dfsb_hmsc_traverser.cpp 450 2009-11-04 15:11:10Z kocianon $
 */

#include <stack>

#include "data/dfsb_hmsc_traverser.h"

DFSBHMscTraverser::~DFSBHMscTraverser()
{
  cleanup_traversing_attributes();
}

void DFSBHMscTraverser::traverse(HMscPtr hmsc)
{
  HMscNodePtrSet::const_iterator n;
  const HMscNodePtrSet& nodes = hmsc->get_nodes();
  for(n=nodes.begin();n!=nodes.end();n++)
  {
    if(dynamic_cast<EndNode*>((*n).get()))
    {
      traverse_node((*n).get());
    }
  }
  cleanup_traversing_attributes();
}

void DFSBHMscTraverser::traverse(HMscNode* node, bool cleanup)
{
  traverse_node(node);
  if(cleanup)
  {
    cleanup_traversing_attributes();
  }
}

bool DFSBHMscTraverser::traverse_node(HMscNode* node)
{
  m_reached_elements.push_back(node);
  if(is_processed(node))
  {
    return false;
  }
  white_node_found(node);
  SuccessorNode* succ = dynamic_cast<SuccessorNode*>(node);
  if(succ!=NULL)
  {
    traverse_predecessors(succ);
  }
  node_finished(node);
  return false;
}

void DFSBHMscTraverser::traverse_predecessors(SuccessorNode* succ)
{
  NodeRelationPtrVector::const_iterator relation;
  for(relation=succ->get_predecessors().begin();
    relation!=succ->get_predecessors().end();relation++)
  {
    const NodeRelationPtr& rel = *relation;
    m_reached_elements.push_back(rel.get());
    traverse_node(dynamic_cast<HMscNode*>(rel->get_predecessor()));
    m_reached_elements.pop_back();
  }
}

bool DFSBHMscTraverser::is_processed(HMscNode* node)
{
  NodeColor c = get_color(node);
  if(c==BLACK)
  {
    black_node_found(node);
    return true;
  }
  if(c==GRAY)
  {
    gray_node_found(node);
    return true;
  }
  return false;
}

void DFSBHMscTraverser::cleanup_traversing_attributes()
{
  HMscNodePList::const_iterator n;
  for(n=m_colored_nodes.begin();n!=m_colored_nodes.end();n++)
    (*n)->remove_attribute<NodeColor>(m_color_attribute);
  m_colored_nodes.erase(m_colored_nodes.begin(),m_colored_nodes.end());
  m_reached_elements.erase(m_reached_elements.begin(),m_reached_elements.end());
}

void DFSBHMscTraverser::white_node_found(HMscNode* n)
{
  DFSListenersContainer::white_node_found(n);
  set_color(n,GRAY);
}

void DFSBHMscTraverser::gray_node_found(HMscNode* n)
{
  DFSListenersContainer::gray_node_found(n);
  m_reached_elements.pop_back();
}

void DFSBHMscTraverser::black_node_found(HMscNode* n)
{
  DFSListenersContainer::black_node_found(n);
  m_reached_elements.pop_back();
}

void DFSBHMscTraverser::node_finished(HMscNode* n)
{
  DFSListenersContainer::node_finished(n);
  m_reached_elements.pop_back();
  set_color(n,BLACK);
}

NodeColor& DFSBHMscTraverser::get_color(HMscNode* n)
{
  bool just_set;
  NodeColor& c = n->get_attribute<NodeColor>(m_color_attribute,WHITE,just_set);
  if(just_set)
  {
    m_colored_nodes.push_back(n);
  }
  return c;
} 

void DFSBHMscTraverser::set_color(HMscNode* n, NodeColor c)
{
  NodeColor& col = get_color(n);
  col = c;
}

// $Id: dfsb_hmsc_traverser.cpp 450 2009-11-04 15:11:10Z kocianon $
