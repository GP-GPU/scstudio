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
 * $Id: dfs_bmsc_graph_traverser.h 408 2009-10-08 09:35:30Z kocianon $
 */

#ifndef _DFS_MSC_GRAPH_TRAVERSER_H
#define _DFS_MSC_GRAPH_TRAVERSER_H

#include<stack>

#include "data/msc.h"
#include "data/checker.h"

typedef std::list<HMscNode*> HMscNodePList;
typedef std::list<HMscNodePList> HMscNodePListList;
typedef std::list<MscElement*> MscElementPList;
typedef std::list<MscElementPList> MscElementPListList;


/**
 * Listener of founded white HMscNode
 */
class SCMSC_EXPORT WhiteNodeFoundListener
{
public:

  virtual ~WhiteNodeFoundListener()
  {

  }

  virtual void on_white_node_found(HMscNode* n)=0;
};

/**
 * Listener of founded gray HMscNode
 */
class SCMSC_EXPORT GrayNodeFoundListener
{
public:
  
  virtual ~GrayNodeFoundListener()
  {
    
  }
  
  virtual void on_gray_node_found(HMscNode* n)=0;
};

/**
 * Listener of founded black HMscNode
 */
class SCMSC_EXPORT BlackNodeFoundListener
{
public:
  
  virtual ~BlackNodeFoundListener()
  {
    
  }
  
  virtual void on_black_node_found(HMscNode* n)=0;
};

/**
 * Listener of finished HMscNode
 */
class SCMSC_EXPORT NodeFinishedListener
{
public:
  
  virtual ~NodeFinishedListener()
  {
    
  }
  
  virtual void on_node_finished(HMscNode* node)=0;
};

/**
 * Auxiliary listener of founded white HMscNode.
 *
 * Extend this listener to process only ReferenceNodes during traversing.
 */
class SCMSC_EXPORT WhiteRefNodeFoundListener:public WhiteNodeFoundListener
{
public:
  
  virtual ~WhiteRefNodeFoundListener()
  {
    
  }
  
  virtual void on_white_node_found(HMscNode* n){
    ReferenceNode* ref = dynamic_cast<ReferenceNode*>(n);
    if(ref!=NULL)
    {
      on_white_node_found(ref);
    }
  }
  
  virtual void on_white_node_found(ReferenceNode* n)=0;
};

/**
 * Auxiliary listener of founded gray HMscNode.
 *
 * Extend this listener to process only ReferenceNodes during traversing.
 */
class SCMSC_EXPORT GrayRefNodeFoundListener:public GrayNodeFoundListener
{
public:
  
  virtual ~GrayRefNodeFoundListener()
  {
    
  }
  
  virtual void on_gray_node_found(HMscNode* n){
    ReferenceNode* ref = dynamic_cast<ReferenceNode*>(n);
    if(ref!=NULL)
    {
      on_gray_node_found(ref);
    }
  }
  
  virtual void on_gray_node_found(ReferenceNode* n)=0;
};

/**
 * Auxiliary listener of founded black HMscNode.
 *
 * Extend this listener to process only ReferenceNodes during traversing.
 */
class SCMSC_EXPORT BlackRefNodeFoundListener:public BlackNodeFoundListener
{
public:
  
  virtual ~BlackRefNodeFoundListener()
  {
    
  }
  
  virtual void on_black_node_found(HMscNode* n){
    ReferenceNode* ref = dynamic_cast<ReferenceNode*>(n);
    if(ref!=NULL)
    {
      on_black_node_found(ref);
    }
  }
  
  virtual void on_black_node_found(ReferenceNode* n)=0;
};

/**
 * Auxiliary listener of finished HMscNode.
 *
 * Extend this listener to process only ReferenceNodes during traversing.
 */
class SCMSC_EXPORT RefNodeFinishedListener:public NodeFinishedListener
{
public:
  
  virtual ~RefNodeFinishedListener()
  {
    
  }
  
  virtual void on_node_finished(HMscNode* n){
    ReferenceNode* ref = dynamic_cast<ReferenceNode*>(n);
    if(ref!=NULL)
    {
      on_node_finished(ref);
    }
  }
  
  virtual void on_node_finished(ReferenceNode* n)=0;
};

typedef NodeFinishedListener* NodeFinishedListenerP;
typedef std::list<NodeFinishedListenerP> NodeFinishedListenerPList;
typedef WhiteNodeFoundListener* WhiteNodeFoundListenerP;
typedef std::list<WhiteNodeFoundListenerP> WhiteNodeFoundListenerPList;
typedef GrayNodeFoundListener* GrayNodeFoundListenerP;
typedef std::list<GrayNodeFoundListenerP> GrayNodeFoundListenerPList;
typedef BlackNodeFoundListener* BlackNodeFoundListenerP;
typedef std::list<BlackNodeFoundListenerP> BlackNodeFoundListenerPList;

class SCMSC_EXPORT DFSListenersContainer
{
public:
  /**
   * Adds NodeFinishedListener
   */
  void add_node_finished_listener(NodeFinishedListenerP l)
  {
    m_node_finished_listeners.push_back(l);
  }
  
  void remove_node_finished_listeners()
  {
    m_node_finished_listeners.erase(
      m_node_finished_listeners.begin(),m_node_finished_listeners.end());
  }
  
  /**
   * Adds WhiteNodeFoundListener
   */
  void add_white_node_found_listener(WhiteNodeFoundListenerP l)
  {
    m_white_node_found_listeners.push_back(l);
  }
  
  void remove_white_node_found_listeners()
  {
    m_white_node_found_listeners.erase(
      m_white_node_found_listeners.begin(),m_white_node_found_listeners.end());
  }
  
  /**
   * Adds GrayNodeFoundListener
   */
  void add_gray_node_found_listener(GrayNodeFoundListenerP l)
  {
    m_gray_node_found_listeners.push_back(l);
  }
  
  void remove_gray_node_found_listeners()
  {
    m_gray_node_found_listeners.erase(
      m_gray_node_found_listeners.begin(),m_gray_node_found_listeners.end());
  }
  
  /**
   * Adds BlackNodeFoundListener
   */
  void add_black_node_found_listener(BlackNodeFoundListenerP l)
  {
    m_black_node_found_listeners.push_back(l);
  }
  
  void remove_black_node_found_listeners()
  {
    m_black_node_found_listeners.erase(
      m_black_node_found_listeners.begin(),m_black_node_found_listeners.end());
  }
  
  void remove_all_listeners()
  {
    remove_node_finished_listeners();
    remove_white_node_found_listeners();
    remove_gray_node_found_listeners();
    remove_black_node_found_listeners();
  }
  
  virtual ~DFSListenersContainer();
  
protected:
    
  /**
   * Holds listeners
   */
  NodeFinishedListenerPList m_node_finished_listeners;
  
  /**
   * Holds listeners
   */
  WhiteNodeFoundListenerPList m_white_node_found_listeners;
  
  /**
   * Holds listeners
   */
  GrayNodeFoundListenerPList m_gray_node_found_listeners;
  
  /**
   * Holds listeners
   */
  BlackNodeFoundListenerPList m_black_node_found_listeners;
  
  /**
   * Holds listeners
   */
  GrayNodeFoundListenerPList m_grey_node_found_listeners;
  
  /**
   * Called when white HMscNode is found.
   */
  virtual void white_node_found(HMscNode* n);
  
  /**
   * Called when gray HMscNode is found.
   */
  virtual void gray_node_found(HMscNode* n);
  
  /**
   * Called when black HMscNode is found.
   */
  virtual void black_node_found(HMscNode* n);
  
  /**
   * Called when all successors of e are processed.
   */
  virtual void node_finished(HMscNode* n);
};

/**
 * Traverses all accessible HMscNodes in HMsc and referenced HMsc in depth 
 * first search manner.
 *
 * HMsc is traversed like BMsc-graph (flattened version of HMsc - each 
 * ReferenceNode references only BMsc not HMsc), i.e. each HMsc is traversed as 
 * many times as it is referenced. If ReferenceNode references HMsc, this HMsc 
 * is traversed before successors of referencing node are processed.
 *
 * HMscNodes of HMsc are during traversing in different states - with different 
 * color. WHITE color means that node wasn't traversed before. GRAY nodes
 * are those which have been already traversed and they are just on the stack - 
 * not all successors of these events were processed. BLACK nodes are those 
 * which have been already traversed but aren't yet on the stack - all 
 * successors have been processed.
 *
 * Note that not all HMscNodes must be traversed by this DFSBMscGraphTraverser, 
 * imagine HMscNode B which is accessible only from ReferenceNode A but A's
 * HMsc hasn't got any end node.
 *
 * @warning Non-recursive HMsc is expected.
 */
class SCMSC_EXPORT DFSBMscGraphTraverser:public DFSListenersContainer
{
  
public:
  
  /**
   * To change default name of attribute which holds color of HMscNodes use
   * color_attribute parameter.
   */
  DFSBMscGraphTraverser(const std::string& color_attribute = "msc_graph_traverse_color")
  {
    m_color_attribute = color_attribute;
  }
  
  /**
   * Calls cleanup_traversing_attributes
   */
  virtual ~DFSBMscGraphTraverser();

  /**
   * Trigger of traversing
   */
  virtual void traverse(HMscPtr hmsc);

  virtual void traverse(HMscNode* node);

  /**
   * Cleans up traversing attributes
   */
  virtual void cleanup_traversing_attributes();
  
  
  
  /**
   * Returns reached HMscNodes in particular phase of traversing.
   *
   * The result corresponds to call stack on particular HMscNodes.
   *
   * Warning: The result is emptied when cleanup_traversing_attributes() is called
   */
  const MscElementPListList& get_reached_elements()
  {
    return m_reached_elements;
  }
  
protected:
  
  /**
   * Holds nodes with set color attribute
   */
  HMscNodePListList m_colored_nodes;

  /**
   * Holds currently reached HMscElements.
   */
  MscElementPListList m_reached_elements;
  
  /**
   * Cleans up traversing attributes from the top of m_colored_nodes
   */
  virtual void cleanup_top_attributes();

  /**
   * Traverses single node
   */
  virtual bool traverse_node(HMscNode* node);

  /**
   * Traverses successors
   */
  virtual bool traverse_successors(PredecessorNode* predecessor);
  
  /**
   * Returns true iff node is just processed false otherwise
   */
  virtual bool is_processed(HMscNode* node);
  
  virtual bool traverse_reference_node(ReferenceNode* node);
  
  /**
   * Color attribute's name
   */
  std::string m_color_attribute;
  
  /**
   * Sets color attribute of e to c value .
   */
  void set_color(HMscNode* n, NodeColor c);
  
  /**
   * Returns value of color attribute.
   *
   * If attribute isn't set it is set to default value WHITE.
   */
  NodeColor& get_color(HMscNode* n);
  
  virtual void push_top_attributes();
  
  /**
   * Called when white HMscNode is found.
   */
  virtual void white_node_found(HMscNode* n);
  
  /**
   * Called when gray HMscNode is found.
   */
  virtual void gray_node_found(HMscNode* n);
  
  /**
   * Called when black HMscNode is found.
   */
  virtual void black_node_found(HMscNode* n);
  
  /**
   * Called when all successors of e are processed.
   */
  virtual void node_finished(HMscNode* n);
};

#endif /* _DFS_MSC_GRAPH_TRAVERSER_H */

// $Id: dfs_bmsc_graph_traverser.h 408 2009-10-08 09:35:30Z kocianon $
