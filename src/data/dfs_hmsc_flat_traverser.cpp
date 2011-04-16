
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
 * Copyright (c) 2009 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: dfs_hmsc_flat_traverser.cpp 522 2009-12-14 18:51:56Z kocianon $
 */

#include "data/dfs_hmsc_flat_traverser.h"

void InnerHMscFoundListener::inner_hmsc_found(HMscPtr hmsc, ReferenceNode* refNode)
{
  on_inner_hmsc_found(hmsc,refNode);
  if(m_hmscs.find(hmsc.get())==m_hmscs.end())
  {
    m_hmscs.insert(hmsc.get());
    on_new_inner_hmsc_found(hmsc,refNode);
  }
}

  void InnerHMscFoundListener::on_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* ref)
{}
  void InnerHMscFoundListener::on_new_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* ref)
  {}

/////////////////////////////////////////////////////

  void InnerBMscFoundListener::inner_bmsc_found(BMscPtr bmsc, ReferenceNode* refNode)
{
  on_inner_bmsc_found(bmsc,refNode);
  if(m_bmscs.find(bmsc.get())==m_bmscs.end())
  {
    m_bmscs.insert(bmsc.get());
    on_new_inner_bmsc_found(bmsc,refNode);
  }
}

void InnerBMscFoundListener::on_inner_bmsc_found(BMscPtr bmsc, ReferenceNode* ref)
{}

void InnerBMscFoundListener::on_new_inner_bmsc_found(BMscPtr bmsc, ReferenceNode* ref)
{}

/////////////////////////////////////////////////////
bool DFSHMscFlatTraverser::traverse_reference_node(ReferenceNode* ref)
{
  HMscPtr hmsc = ref->get_hmsc();
  if(hmsc.get()){
    inner_hmsc_found(hmsc,ref);
  }
  else
  {
    BMscPtr bmsc = ref->get_bmsc();
    if(bmsc.get()){
      inner_bmsc_found(bmsc,ref);
    }
  }

  return traverse_successors(ref);
}


void DFSHMscFlatTraverser::inner_hmsc_found(HMscPtr hmsc, ReferenceNode* ref)
{
  InnerHMscFoundListenerPList::iterator l;
  for(l=m_inner_hmsc_listeners.begin();l!=m_inner_hmsc_listeners.end();l++)
    (*l)->inner_hmsc_found(hmsc,ref);
}

void DFSHMscFlatTraverser::inner_bmsc_found(BMscPtr bmsc, ReferenceNode* ref)
{
  InnerBMscFoundListenerPList::iterator l;
  for(l=m_inner_bmsc_listeners.begin();l!=m_inner_bmsc_listeners.end();l++)
  {
    (*l)->inner_bmsc_found(bmsc,ref);
  }
}

void DFSHMscFlatTraverser::add_inner_hmsc_listener(InnerHMscFoundListener* l)
{
  m_inner_hmsc_listeners.push_back(l);
}

   void DFSHMscFlatTraverser::remove_inner_hmsc_listeners()
{
  m_inner_hmsc_listeners.erase(
                               m_inner_hmsc_listeners.begin(),m_inner_hmsc_listeners.end());
}

void
DFSHMscFlatTraverser::add_inner_bmsc_listener(InnerBMscFoundListener* l)
{
  m_inner_bmsc_listeners.push_back(l);
}

   void DFSHMscFlatTraverser::remove_inner_bmsc_listeners()
{
  m_inner_bmsc_listeners.erase(
                               m_inner_bmsc_listeners.begin(),m_inner_bmsc_listeners.end());
}
////////////////////////////////////////////////////////
 void DFSHMscsTraverse::traverse(HMscPtr hmsc)
{
  m_to_traverse.push(hmsc.get()); // push to the stack
  add_hmsc_as_traversed(hmsc); // add this hmsc as it was traversed
  m_traverser->add_inner_hmsc_listener(this);
  m_traverser->add_inner_bmsc_listener(this);
  m_traverser->add_white_node_found_listener(this);
  add_parent(NULL,hmsc.get());
  while(!m_to_traverse.empty())
  {
    m_active_hmsc = m_to_traverse.front();
    m_to_traverse.pop();
    hmsc_start(m_active_hmsc);

    m_traverser->traverse(m_active_hmsc);
    m_traverser->cleanup_traversing_attributes();

    hmsc_finished(m_active_hmsc);
  }
}

void DFSHMscsTraverse::on_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* refNode)
{
  add_parent(refNode,hmsc.get());
}

void DFSHMscsTraverse::on_inner_bmsc_found(BMscPtr bmsc, ReferenceNode* refNode)
{
//  std::cout << "bmsc found" << std::endl;
  add_parent(refNode,bmsc.get());
}

void DFSHMscsTraverse::on_new_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* refNode)
{
  m_to_traverse.push(hmsc.get());
}

void on_inner_bmsc_found(BMscPtr hmsc, ReferenceNode* refNode);


////////////////////////////////////////////////////////
bool DFSRefNodeFlatHMscTraverser::traverse_successors(PredecessorNode* predecessor)
{
  bool end_found = false;
  HMscNodePListPtr successors = m_finder.find_successors(dynamic_cast<HMscNode*>(predecessor));
  m_finder.cleanup_traversing_attributes();

  //TODO: m_reached_elements should contain path to succ
  HMscNodePList::const_iterator succ;
  for(succ=successors->begin();succ!=successors->end();succ++)
  {
    end_found = traverse_node(*succ) || end_found;
  }
  return end_found;
}

void DFSRefNodeFlatHMscTraverser::cleanup_traversing_attributes()
{
  DFSBMscGraphTraverser::cleanup_traversing_attributes();
  m_finder.cleanup_traversing_attributes();
}
/////////////////////////////////////////////////

std::list<ReferenceNode*> DFSHMscsTraverse::get_parents(Msc* child)
{
  return m_attr_parent_msc.get(child);
}

std::list<Msc*> DFSHMscsTraverse::get_parents(ReferenceNode* child)
{
  return m_attr_parent_ref.get(child);
}

void DFSHMscsTraverse::add_parent(ReferenceNode* parent,Msc* child)
{
  
  std::list<ReferenceNode*>& list = m_attr_parent_msc.get(child);
  if(parent)
    list.push_back(parent);
}

void DFSHMscsTraverse::add_parent(Msc* parent,ReferenceNode* child)
{
  std::list<Msc*>& list = m_attr_parent_ref.get(child);
  list.push_back(parent);
}


  void DFSHMscsTraverse::hmsc_start(HMscPtr h)
  {
    TraverseHMscStartListenerPList::iterator it;
    for(it=m_hmsc_start_listeners.begin();it!=m_hmsc_start_listeners.end();it++)
      (*it)->on_hmsc_traverse_start(h);
  }


  void DFSHMscsTraverse::hmsc_finished(HMscPtr h)
  {
    TraverseHMscFinishListenerPList::iterator it;
    for(it=m_hmsc_finish_listeners.begin();it!=m_hmsc_finish_listeners.end();it++)
      (*it)->on_hmsc_traverse_finish(h);
  }


    void DFSHMscsTraverse::add_hmsc_finish_listener(TraverseHMscFinishListenerP l)
   {
     m_hmsc_finish_listeners.push_back(l);
   }

   void DFSHMscsTraverse::remove_hmsc_finish_listeners()
   {
     m_hmsc_finish_listeners.erase(
       m_hmsc_finish_listeners.begin(),m_hmsc_finish_listeners.end());
   }

   void DFSHMscsTraverse::add_hmsc_start_listener(TraverseHMscStartListenerP l)
   {
     m_hmsc_start_listeners.push_back(l);
   }

   void DFSHMscsTraverse::remove_hmsc_start_listeners()
   {
     m_hmsc_start_listeners.erase(
       m_hmsc_start_listeners.begin(),m_hmsc_start_listeners.end());
   }

   void DFSHMscsTraverse::on_white_node_found(ReferenceNode* node)
   {
     add_parent(m_active_hmsc.get(),node);
   }


// $Id: dfs_hmsc_flat_traverser.cpp 522 2009-12-14 18:51:56Z kocianon $
