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
 * $Id: dfs_bmsc_graph_traverser.cpp 431 2009-10-16 20:20:17Z gotthardp $
 */

#include <stack>

#include "data/dfs_bmsc_graph_traverser.h"

void DFSListenersContainer::white_node_found(HMscNode* n)
{
  WhiteNodeFoundListenerPList::iterator l;
  for(l=m_white_node_found_listeners.begin();l!=m_white_node_found_listeners.end();l++)
    (*l)->on_white_node_found(n);
}

void DFSListenersContainer::gray_node_found(HMscNode* n)
{
  GrayNodeFoundListenerPList::iterator l;
  for(l=m_gray_node_found_listeners.begin();l!=m_gray_node_found_listeners.end();l++)
    (*l)->on_gray_node_found(n);
}

void DFSListenersContainer::black_node_found(HMscNode* n)
{
  BlackNodeFoundListenerPList::iterator l;
  for(l=m_black_node_found_listeners.begin();l!=m_black_node_found_listeners.end();l++)
    (*l)->on_black_node_found(n);
}

void DFSListenersContainer::node_finished(HMscNode* n)
{
  NodeFinishedListenerPList::iterator l;
  for(l=m_node_finished_listeners.begin();l!=m_node_finished_listeners.end();l++)
    (*l)->on_node_finished(n);
}

DFSListenersContainer::~DFSListenersContainer()
{
  
}

DFSBMscGraphTraverser::~DFSBMscGraphTraverser()
{
  cleanup_traversing_attributes();
}

void DFSBMscGraphTraverser::traverse(HMscPtr hmsc)
{
  push_top_attributes();
  traverse_node(hmsc->get_start().get());
  cleanup_traversing_attributes();
}

void DFSBMscGraphTraverser::traverse(HMscNode* node)
{
  push_top_attributes();
  traverse_node(node);
  cleanup_traversing_attributes();
}

bool DFSBMscGraphTraverser::traverse_node(HMscNode* node)
{
  m_reached_elements.back().push_back(node);
  if(is_processed(node))
  {
    return false;
  }
  white_node_found(node);
  ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>(node);
  bool ending_successors;
  if(ref_node!=NULL)
  {
    ending_successors = traverse_reference_node(ref_node);
  }
  else
  {
    PredecessorNode* pred_node = dynamic_cast<PredecessorNode*>(node);
    if(pred_node)
    {
      ending_successors = traverse_successors(pred_node);
    }
    else
    {
      //node is supposed to be EndNode
      ending_successors = true;
    }
  }
  node_finished(node);
  return ending_successors;
}

bool DFSBMscGraphTraverser::traverse_reference_node(ReferenceNode* ref_node)
{
  HMscPtr hmsc = ref_node->get_hmsc();
  bool inner = true;
  if(hmsc.get()!=NULL)
  {
    //this line ensures (yet another one) that HMsc will be traversed as many times
    //as it is referenced
    push_top_attributes();
    //node's successors aren't traversed if end node wasn't found in hmsc
    inner = traverse_node(hmsc->get_start().get());
    //this line ensures (yet another one) that HMsc will traversed as many times
    //as it is referenced
    cleanup_top_attributes();
  }
  return inner && traverse_successors(ref_node);
}

bool DFSBMscGraphTraverser::traverse_successors(PredecessorNode* predecessor)
{
  bool end_found = false;
  NodeRelationPtrVector::const_iterator relation;
  for(relation=predecessor->get_successors().begin();
    relation!=predecessor->get_successors().end();relation++)
  {
    const NodeRelationPtr& rel = *relation;
    m_reached_elements.back().push_back(rel.get());
    end_found = traverse_node(dynamic_cast<HMscNode*>(rel->get_successor())) || end_found;
    m_reached_elements.back().pop_back();
  }
  return end_found;
}

bool DFSBMscGraphTraverser::is_processed(HMscNode* node)
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

void DFSBMscGraphTraverser::cleanup_traversing_attributes()
{
  while(!m_colored_nodes.empty())
  {
    cleanup_top_attributes();
  }
}

void DFSBMscGraphTraverser::white_node_found(HMscNode* n)
{
  DFSListenersContainer::white_node_found(n);
  set_color(n,GRAY);
}

void DFSBMscGraphTraverser::gray_node_found(HMscNode* n)
{
  DFSListenersContainer::gray_node_found(n);
  m_reached_elements.back().pop_back();
}

void DFSBMscGraphTraverser::black_node_found(HMscNode* n)
{
  DFSListenersContainer::black_node_found(n);
  m_reached_elements.back().pop_back();
}

void DFSBMscGraphTraverser::node_finished(HMscNode* n)
{
  DFSListenersContainer::node_finished(n);
  m_reached_elements.back().pop_back();
  set_color(n,BLACK);
}

void DFSBMscGraphTraverser::cleanup_top_attributes()
{
  HMscNodePList& top = m_colored_nodes.back();
  HMscNodePList::iterator event;
  for(event=top.begin();event!=top.end();event++)
    (*event)->remove_attribute<NodeColor>(m_color_attribute);
  m_colored_nodes.pop_back();
  m_reached_elements.pop_back();
}
  
void DFSBMscGraphTraverser::push_top_attributes()
{
  m_colored_nodes.push_back(HMscNodePList());
  m_reached_elements.push_back(MscElementPList());
}

NodeColor& DFSBMscGraphTraverser::get_color(HMscNode* n)
{
  bool just_set;
  NodeColor& c = n->get_attribute<NodeColor>(m_color_attribute,WHITE,just_set);
  if(just_set)
  {
    m_colored_nodes.back().push_back(n);
  }
  return c;
}

void DFSBMscGraphTraverser::set_color(HMscNode* n, NodeColor c)
{
  NodeColor& col = get_color(n);
  col = c;
}

// $Id: dfs_bmsc_graph_traverser.cpp 431 2009-10-16 20:20:17Z gotthardp $
