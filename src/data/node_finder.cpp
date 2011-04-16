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
 * $Id: node_finder.cpp 455 2009-11-13 17:17:17Z kocianon $
 */

#include "data/node_finder.h"

NodeFinder::NodeFinder(const std::string& color_attribute)
  :DFSBMscGraphTraverser(color_attribute), m_running_forward(true)
{

}

bool NodeFinder::is_terminal(HMscNode *node)
{
  if(dynamic_cast<ConnectionNode*>(node))
    return false;
  else return true;
}

bool NodeFinder::traverse_node(HMscNode* node)
{
  m_reached_elements.back().push_back(node);
  if(is_processed(node))
  {
    //return value is no more needed
    return true;
  }
  white_node_found(node);
  //traverse successors only if node is not terminal
  if(!is_terminal(node))
  {
    if(m_running_forward)
    {
      PredecessorNode* pred_node = dynamic_cast<PredecessorNode*>(node);
      if(pred_node)
      {
        traverse_successors(pred_node);
      }
    }
    else
    {
      SuccessorNode* succ_node = dynamic_cast<SuccessorNode*>(node);
      if(succ_node)
      {
        traverse_predecessors(succ_node);
      }
    }
    //else node is supposed to be EndNode
  }
  node_finished(node);
  //return value is no more needed
  return true;
}

HMscNodePListPtr NodeFinder::find_successors(HMscNode* node)
{
  m_running_forward = true;
  HMscNodePListPtr successorlist(new HMscNodePList);
  HMscNodePListPtr skipped(new HMscNodePList);
  HMscNodePListPtr result(new HMscNodePList);

  ReferenceNodeFinderListener l(successorlist.get());
  add_white_node_found_listener(&l);
  add_gray_node_found_listener(&l);
  push_top_attributes();
  node->set_attribute("first_node", true);
  set_color(node,GRAY); //set node to GRAY to avoid him to be traversed
  m_reached_elements.back().push_back(node); //push him on top to simulate it was already traversed
  PredecessorNode* pred = dynamic_cast<PredecessorNode*>(node);
  if(pred)
  {
    traverse_successors(pred);
  }
  cleanup_traversing_attributes();
  remove_all_listeners();
  HMscNodePList::iterator it;
  for(it = successorlist->begin(); it != successorlist->end(); it++)
    if(is_terminal(*it))
      result->push_back(*it);
    else
      skipped->push_back(*it);
  m_result = result;
  m_skipped_nodes = skipped;

  node->remove_attribute<bool>("first_node");

  return result;
}

HMscNodePListPtr NodeFinder::find_predecessors(HMscNode* node)
{
  m_running_forward = false;
  HMscNodePListPtr predecessors(new HMscNodePList);
  HMscNodePListPtr skipped(new HMscNodePList);
  HMscNodePListPtr result(new HMscNodePList);

  ReferenceNodeFinderListener l(predecessors.get());
  add_white_node_found_listener(&l);
  add_gray_node_found_listener(&l);
  push_top_attributes();
  node->set_attribute("first_node", true);
  set_color(node,GRAY); //set node to GRAY to avoid him to be traversed
  m_reached_elements.back().push_back(node); //push him on top to simulate it was already traversed
  SuccessorNode* succ = dynamic_cast<SuccessorNode*>(node);
  if(succ)
  {
    traverse_predecessors(succ);
  }
  cleanup_traversing_attributes();
  remove_all_listeners();
  HMscNodePList::iterator it;
  for(it = predecessors->begin(); it != predecessors->end(); it++)
    if(is_terminal(*it))
      result->push_back(*it);
    else
      skipped->push_back(*it);

  m_result = result;
  m_skipped_nodes = skipped;

  node->remove_attribute<bool>("first_node");
  //m_running_forward = true;

  return result;
}

HMscNodePListPtr NodeFinder::successors(HMscNode* node, const std::string& color_attribute)
{
  NodeFinder finder(color_attribute);
  return finder.find_successors(node);
}

template<class T>
HMscNodePListPtr NodeFinder::successors(HMscNode* node, const std::string& color_attribute)
{
  T finder(color_attribute);
  return finder.find_successors(node);
}

HMscNodePListPtr NodeFinder::predecessors(HMscNode* node, const std::string& color_attribute)
{
  NodeFinder finder(color_attribute);
  return finder.find_predecessors(node);
}

template<class T>
HMscNodePListPtr NodeFinder::predecessors(HMscNode* node, const std::string& color_attribute)
{
  T finder(color_attribute);
  return finder.find_predecessors(node);
}

void NodeFinder::find_node(HMscNode* start, HMscNode* desired)
{
  FindNodeListener l(desired);
  add_white_node_found_listener(&l);
  push_top_attributes();
  set_color(start,GRAY); //set node to GRAY to avoid him to be traversed
  m_reached_elements.back().push_back(start); //push him on top to simulate it was already traversed
  PredecessorNode* pred = dynamic_cast<PredecessorNode*>(start);
  if(pred)
  {
    try
    {
      traverse_successors(pred);
    }
    catch(...)
    {

    }
  }
  remove_white_node_found_listeners();
}

FindNodeListener::FindNodeListener(HMscNode* desired):
m_desired(desired)
{

}

void FindNodeListener::on_white_node_found(HMscNode* n)
{
  if(m_desired==n)
  {
    throw 1;
  }
}

void NodeFinder::traverse_predecessors(SuccessorNode *succ)
{
  NodeRelationPtrVector::const_iterator relation;
  for(relation=succ->get_predecessors().begin();
    relation!=succ->get_predecessors().end();relation++)
  {
    const NodeRelationPtr& rel = *relation;
    m_reached_elements.back().push_back(rel.get());
    traverse_node(dynamic_cast<HMscNode*>(rel->get_predecessor()));
    m_reached_elements.back().pop_back();
  }
}

// $Id: node_finder.cpp 455 2009-11-13 17:17:17Z kocianon $
