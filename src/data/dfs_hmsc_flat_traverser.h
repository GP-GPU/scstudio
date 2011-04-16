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
 * $Id: dfs_hmsc_flat_traverser.h 524 2009-12-16 13:12:08Z kocianon $
 */

#ifndef _DFS_HMSC_FLAT_TRAVERSER_H
#define _DFS_HMSC_FLAT_TRAVERSER_H

#include "data/dfs_bmsc_graph_traverser.h"
#include "data/node_finder.h"
#include "data/session_attribute.h"

class SCMSC_EXPORT InnerHMscFoundListener
{
protected:
  std::set<HMsc*> m_hmscs; // set of HMsc already found
public:
  InnerHMscFoundListener()
  {}
  InnerHMscFoundListener(std::set<HMsc*> hmscs):m_hmscs(hmscs)
  {}
  virtual ~InnerHMscFoundListener()
  {}

  void add_hmsc_as_traversed(HMscPtr hmsc)
  {
    m_hmscs.insert(hmsc.get());
  }

  void inner_hmsc_found(HMscPtr, ReferenceNode*);
  virtual void on_inner_hmsc_found(HMscPtr, ReferenceNode*);
  virtual void on_new_inner_hmsc_found(HMscPtr, ReferenceNode*);
};

class SCMSC_EXPORT InnerBMscFoundListener
{
private:
  std::set<BMsc*> m_bmscs;  // set of BMsc all ready found

public:
  InnerBMscFoundListener()
  {}
  InnerBMscFoundListener(std::set<BMsc*> bmscs):m_bmscs(bmscs)
  {}
  virtual ~InnerBMscFoundListener()
  {}

  void inner_bmsc_found(BMscPtr bmsc, ReferenceNode* refNode);
  virtual void on_inner_bmsc_found(BMscPtr, ReferenceNode*);
  virtual void on_new_inner_bmsc_found(BMscPtr, ReferenceNode*);
};

class SCMSC_EXPORT TraverseHMscStartListener
{
public:
  virtual ~TraverseHMscStartListener() {}

  virtual void on_hmsc_traverse_start(HMscPtr)=0;
};

class SCMSC_EXPORT TraverseHMscFinishListener
{
public:
  virtual ~TraverseHMscFinishListener() {}

  virtual void on_hmsc_traverse_finish(HMscPtr)=0;
};

typedef InnerHMscFoundListener* InnerHMscFoundListenerP;
typedef std::list<InnerHMscFoundListenerP> InnerHMscFoundListenerPList;

typedef InnerBMscFoundListener* InnerBMscFoundListenerP;
typedef std::list<InnerBMscFoundListenerP> InnerBMscFoundListenerPList;

typedef TraverseHMscStartListener* TraverseHMscStartListenerP;
typedef std::list<TraverseHMscStartListenerP> TraverseHMscStartListenerPList;


typedef TraverseHMscFinishListener* TraverseHMscFinishListenerP;
typedef std::list<TraverseHMscFinishListenerP> TraverseHMscFinishListenerPList;

/// Traverse just top layer of HMSc, events: inner hmsc, bmsc found
class SCMSC_EXPORT DFSHMscFlatTraverser:public DFSBMscGraphTraverser
{
protected:
  //! list of listeners
  InnerHMscFoundListenerPList m_inner_hmsc_listeners;
  InnerBMscFoundListenerPList m_inner_bmsc_listeners;

  void inner_hmsc_found(HMscPtr hmsc, ReferenceNode* ref);
  void inner_bmsc_found(BMscPtr bmsc, ReferenceNode* ref);

  bool traverse_reference_node(ReferenceNode* ref);

public:
   void add_inner_hmsc_listener(InnerHMscFoundListener* l);
   void remove_inner_hmsc_listeners();
   void add_inner_bmsc_listener(InnerBMscFoundListener* l);
   void remove_inner_bmsc_listeners();

  DFSHMscFlatTraverser(const std::string& color_attribute="dfs_hmsc_flat_traverse_color"):
    DFSBMscGraphTraverser(color_attribute)
  {
  }
};

/// DFSHMscFlatTraverser, traversing refnodes using RefNodeFinder
class SCMSC_EXPORT DFSRefNodeFlatHMscTraverser:public DFSHMscFlatTraverser
{
protected:

  bool traverse_successors(PredecessorNode* predecessor);
  RefNodeFinder m_finder;

public:

  DFSRefNodeFlatHMscTraverser(
    const std::string& color_attribute = "DFSRNFHMT_color"):
    DFSHMscFlatTraverser(color_attribute)
  {}

  void cleanup_traversing_attributes();

  virtual ~DFSRefNodeFlatHMscTraverser()
  {
    cleanup_traversing_attributes();
  }

};

/// Traversing HMsc using DFSHMscFlatTraverser and hmsc found event
class SCMSC_EXPORT DFSHMscsTraverse:
    public InnerHMscFoundListener
    ,public InnerBMscFoundListener
    ,public WhiteRefNodeFoundListener
{
private:
  std::queue<HMsc*> m_to_traverse; /// stack of HMsc to traverse
  DFSHMscFlatTraverser* m_traverser; /// traverser to use
  HMscPtr m_active_hmsc; /// hmsc thats traversing right now
  const std::string m_parent; /// list of parents
  std::stack<MscElement*> to_clean_up;
  void clean_up(MscElement*);
  SessionAttribute<std::list<ReferenceNode*> > m_attr_parent_msc;
  SessionAttribute<std::list<Msc*> > m_attr_parent_ref;

  TraverseHMscStartListenerPList m_hmsc_start_listeners;
  TraverseHMscFinishListenerPList m_hmsc_finish_listeners;

///////////////////// MARKING

  void add_parent(ReferenceNode*,Msc*); // ReferenceNode -> Msc
  void add_parent(Msc*,ReferenceNode*); // Msc -> ReferenceNode

public:
  std::list<ReferenceNode*> get_parents(Msc*);
  std::list<Msc*> get_parents(ReferenceNode*);
//////////////////// MARKING
public:
  DFSHMscsTraverse(DFSHMscFlatTraverser* t)
    :m_traverser(t)
    ,m_active_hmsc(NULL)
    ,m_attr_parent_msc("ref_parent",std::list<ReferenceNode*>())
    ,m_attr_parent_ref("msc_parent",std::list<Msc*>())
    { }

  ~DFSHMscsTraverse()
  {
    m_attr_parent_msc.clean_up();
    m_attr_parent_ref.clean_up();
  }

  void traverse(HMscPtr hmsc);

  void on_new_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* refNode);

  void on_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* refNode);
  void on_white_node_found(ReferenceNode*);
  void on_inner_bmsc_found(BMscPtr hmsc, ReferenceNode* refNode);


  void hmsc_start(HMscPtr);
  void hmsc_finished(HMscPtr);

   void add_hmsc_finish_listener(TraverseHMscFinishListener*);
   void remove_hmsc_finish_listeners();
   void add_hmsc_start_listener(TraverseHMscStartListener*);
   void remove_hmsc_start_listeners();

  


};

#endif //_DFS_HMSC_FLAT_TRAVERSER_H

// $Id: dfs_hmsc_flat_traverser.h 524 2009-12-16 13:12:08Z kocianon $
