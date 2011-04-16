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
 * $Id: dfs_inner_hmsc_traverser.cpp 455 2009-11-13 17:17:17Z kocianon $
 */

#include "data/dfs_inner_hmsc_traverser.h"

bool DFSInnerHMscTraverser::traverse_node(InnerNode* node)
{
  NodeColor c = get_color(node);
  if(c==BLACK)
  {
    black_node_found(node);
    return false;
  }
  if(c==GRAY)
  {
    gray_node_found(node);
    return false;
  }
  white_node_found(node);
  ReferenceNode* refNode = dynamic_cast<ReferenceNode*>(node);
  if(refNode!=NULL)
  {
    HMscPtr hmsc = refNode->get_hmsc();
    if(hmsc.get()!=NULL)
    {
      m_reached_nodes.push_back(InnerNodePList());
      inner_hmsc_found(refNode);
      //node's successors are traversed in all cases
      traverse_successors(hmsc->get_start()->get_successors());
      inner_hmsc_finished(refNode);
      m_reached_nodes.pop_back();
    }
  }
  traverse_successors(node->get_successors());
  node_finished(node);
  //return value is no more needed
  return true;
}

void InnerHMscTraverser::inner_hmsc_found(ReferenceNode* refNode)
{
  InnerHMscListenerPList::iterator l;
  for(l=m_inner_hmsc_listeners.begin();l!=m_inner_hmsc_listeners.end();l++)
    (*l)->on_inner_hmsc_found(e);

}

void InnerHMscTraverser::inner_hmsc_finished(ReferenceNode* refNode)
{
  InnerHMscListenerPList::iterator l;
  for(l=m_inner_hmsc_listeners.begin();l!=m_inner_hmsc_listeners.end();l++)
    (*l)->on_inner_hmsc_finished(e);

}

InnerNodePSet ref_successors(InnerNode* node)
{
  InnerNodePStack successorsStack;
  InnerNodePStack cleanupStack;
  InnerNodePSet ref_successorsPSet;

  InnerNodePSet successors = node->get_successors();
  InnerNodePSet::const_iterator successor;


  for(successor=successors.begin();successor!=successors.end();successor++)
    successorsStack.push(successors);

  while(!successorsStack.empty())
  {
    InnerNodeP node = successorsStack.top();
    successosStack.pop();
    if(!get_visited(node))
    {
      set_visited(node);
      cleanupStack.push(node);
      ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>(node);
        if(ref_node==NULL)
        {
          successors = node->get_successors();
          for(successor=successors.begin();successor!=successors.end();successor++)
          {
            successorsStack.push(*successor);
          }
        }
        else ref_successorsPSet.insert(node);
    }
  }

  while(!cleanupStack.empty())
  {
    InnerNode node = cleanupStack.top();
    node->remove_attribute<bool>(m_visited);
    cleanupStack.pop();
  }
  return ref_successorsPSet;

}

// $Id: dfs_inner_hmsc_traverser.cpp 455 2009-11-13 17:17:17Z kocianon $
