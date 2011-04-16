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
 * $Id: dfs_inner_hmsc_traverser.h 357 2009-09-23 17:22:40Z kocianon $
 */

#ifndef _DFS_HMSC_TRAVERSER_H
#define _DFS_HMSC_TRAVERSER_H

#include "data/dfs_bmsc_graph_traverser.h"


/**
 * Traverses all ReferenceNodes in HMsc and referenced HMsc in depth 
 * first search manner. I.e. unlike DFSBMscGraphTraverser this traverser doesn't
 * care about presence of EndNodes in referenced HMsc in ReferenceNodes. 
 * Successors of the ReferenceNode are traversed no matter there is EndNode in
 * it's HMsc.
 *
 * @warning Non-recursive HMsc is expected.
 */

class InnerHMscListener
{
public:
  virtual ~InnerHMscListener()
  {
  
  }

  virtual void on_inner_hmsc_found(ReferenceNode* refNode)=0;
  virtual void on_inner_hmsc_finished(ReferenceNode* refNode)=0;

};

typedef InnerHMscListener* InnerHMscListenerP;
typedef std::list<InnerHMscListenerP> InnerHMscListenerPList;

class SCMSC_EXPORT DFSInnerHMscTraverser:public DFSBMscGraphTraverser
{
  
protected:
  InnerHMscListenerPList m_inner_hmsc_listeners;

  bool traverse_node(InnerNode* node);
  void inner_hmsc_found(ReferenceNode* refNode);
  void inner_hmsc_finished(ReferenceNode* refNode);
  InnerNodePSet ref_successors(InnerNode* node);
  std::string m_visited;

public:
   void add_inner_hmsc_listener(InnerHMscListenerP l)
   {
     m_inner_hmsc_listeners.push_back(l);
   }

   void remove_inner_hmsc_listeners()
   {
      m_inner_hmsc_listeners.erase(
      m_inner_hmsc_listeners.begin(),m_inner_hmsc_listeners.end());
   }

  DFSInnerHMscTraverser(
    const std::string& color_attribute = "dfs_inner_hmsc_traverse_color", const std::string& visited_attribute = "dfs_inner_hmsc_traverse_visited"):
    DFSBMscGraphTraverser(color_attribute)
  {
    m_visited = visited_attribute;
  }

  bool get_visited(InnerNode* n)
  {
    return n->get_attribute<bool>(m_visited,false);
  }
  void set_visited(InnerNode* n)
  {
    n->set_attribute<bool>(m_visited,true);
  }
  
};

#endif /* _DFS_HMSC_TRAVERSER_H */

// $Id: dfs_inner_hmsc_traverser.h 357 2009-09-23 17:22:40Z kocianon $
