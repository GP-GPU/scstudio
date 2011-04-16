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
 * $Id: node_finder.h 455 2009-11-13 17:17:17Z kocianon $
 */

#ifndef _REFNODE_FINDER_H
#define _REFNODE_FINDER_H

#include <list>
#include "data/dfs_bmsc_graph_traverser.h"
#include "check/pseudocode/export.h"

typedef std::list<HMscNode*> HMscNodePList;
typedef boost::shared_ptr<HMscNodePList> HMscNodePListPtr;

class ReferenceNodeFinderListener:
  public WhiteNodeFoundListener,public GrayNodeFoundListener
{

  HMscNodePList* m_successors;

  void insert_successor(HMscNode* n)
  {
    //Do not insert ConnectionNodes
    if(!dynamic_cast<ConnectionNode*>(n))
    {
      m_successors->push_back(n);
    }
  }

public:

  ReferenceNodeFinderListener(HMscNodePList* successors):
      WhiteNodeFoundListener(),GrayNodeFoundListener(),
      m_successors(successors)
  {
  }

  void on_white_node_found(HMscNode* n)
  {
    insert_successor(n);
  }

  //HMscNode can be accessible from itself
  void on_gray_node_found(HMscNode *n)
  {
    if(n->is_attribute_set("first_node"))
    {
      insert_successor(n);
      n->remove_attribute<bool>("first_node");
    }
  }

};

class FindNodeListener:public WhiteNodeFoundListener
{
private:

  HMscNode* m_desired;

public:

  FindNodeListener(HMscNode* desired);

  void on_white_node_found(HMscNode* n);

};

/**
 * \brief The class provides finding reference nodes in the neighbourhood of a node. See find_successors()
 *        and find_predecessors() for details.
 *
 *
 */
class SCMSC_EXPORT NodeFinder: public DFSBMscGraphTraverser
{

public:



  NodeFinder(const std::string& color_attribute = "rnf_color");

  /**
   * \brief Finds nearest
   * (possibly indirect) successors of a node which satisfy the is_terminal() condition.
   *
   * As a default, only ConnectionNodes are skipped. Write an inherited class and
   * redefine is_terminal() to change the behavior. Connection nodes are never present
   * in the resulting list.
   *
   * References in nodes are not followed, neighbouring nodes are not searched in
   * referenced MSCs.
   *
   * Note that it is neccessary to call cleanup_traversing_attributes()
   * in case you want to reuse this traverser while traversing HMsc.
   */
  HMscNodePListPtr find_successors(HMscNode* node);

  /**
   * \brief Dual function to find_successors().
   */
  HMscNodePListPtr find_predecessors(HMscNode* node);



  /**
   * \brief A predicate telling whether searching for successors should continue from
   * the given node. It continues only if terminal(node) == false.
   */
   virtual bool is_terminal(HMscNode *node);

   //! Returns the result of find_successors() or find_predecessors().
   HMscNodePListPtr get_result(void)
   {
     return m_result;
   }

   //! Returns nodes skipped by find_successors() or find_predecessors(). Connection nodes are not stored.
   HMscNodePListPtr get_skipped(void)
   {
     return m_skipped_nodes;
   }


  /**
   * \brief Creates ReferenceNodeFinder and returns result of find_successors(). For derrived classes
   *        use the template version.
   */
  static HMscNodePListPtr successors(HMscNode* node, const std::string& color_attribute = "rnf_color");


  /**
   * \brief Creates ReferenceNodeFinder and returns result of find_predecessors(). For derrived classes
   *        use the template version.
   */
  static HMscNodePListPtr predecessors(HMscNode* node, const std::string& color_attribute = "rnf_color");

  /**
   * \brief Creates a finder of the type T and returns its result of find_successors().
   */
  template<class T>
  static HMscNodePListPtr successors(HMscNode* node, const std::string& color_attribute = "rnf_color");

  /**
   * \brief Creates a finder of the type T and returns its result of find_predecessors().
   */
  template<class T>
  static HMscNodePListPtr predecessors(HMscNode* node, const std::string& color_attribute = "rnf_color");

  /**
   * \brief Finds path from start to desired node. It always proceeds forward.
   *
   * Found path can be retrieved by get_reached_elements()
   */
  void find_node(HMscNode* start, HMscNode* desired);

protected:
  //! True when find_successors() is in progress, false if find_predecessors() works.
  bool m_running_forward;

  /**
   * \brief Traverses from node to nearest StartNode,EndNodes or ReferenceNodes.
   */
  bool traverse_node(HMscNode* node);

  /**
   * \brief Traverses predecessors of a given node
   */
  void traverse_predecessors(SuccessorNode *succ);

  //! Stores the result of find_successors() or find_predecessors()
  HMscNodePListPtr m_result;

  //! Stores nodes skipped by find_successors() or find_predecessors()
  HMscNodePListPtr m_skipped_nodes;

};

//! find next RefNode successors of HMscNode
class RefNodeFinder: public NodeFinder
{
private:

public:
bool is_terminal(HMscNode *node)
{
  if(dynamic_cast<ReferenceNode*>(node))
    return true;

  return false;
}

};

#endif /* _REFNODE_FINDER_H */

// $Id: node_finder.h 455 2009-11-13 17:17:17Z kocianon $
