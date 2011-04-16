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
 * $Id: dfsb_hmsc_traverser.h 231 2009-04-26 19:48:01Z babicaj $
 */

#ifndef _DFSB_HMSC_TRAVERSER_H
#define _DFSB_HMSC_TRAVERSER_H

#include<stack>

#include "data/msc.h"
#include "data/dfs_bmsc_graph_traverser.h"

/**
 * Traverses all accessible HMscNodes in single HMsc in depth 
 * first search manner in backward (B in class name) direction.
 */
class SCMSC_EXPORT DFSBHMscTraverser:public DFSListenersContainer
{
  
public:
  
  /**
   * To change default name of attribute which holds color of HMscNodes use
   * color_attribute parameter.
   */
  DFSBHMscTraverser(const std::string& color_attribute = "dfsb:color")
  {
    m_color_attribute = color_attribute;
  }
  
  /**
   * Calls cleanup_traversing_attributes
   */
  virtual ~DFSBHMscTraverser();

  /**
   * Trigger of traversing
   */
  virtual void traverse(HMscPtr hmsc);

  virtual void traverse(HMscNode* node, bool cleanup=true);

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
  const MscElementPList& get_reached_elements()
  {
    return m_reached_elements;
  }
  
protected:
  
  /**
   * Holds nodes with set color attribute
   */
  HMscNodePList m_colored_nodes;

  /**
   * Holds currently reached HMscElements.
   */
  MscElementPList m_reached_elements;

  /**
   * Traverses single node
   */
  virtual bool traverse_node(HMscNode* node);

  /**
   * Traverses predecessors of succ
   */
  virtual void traverse_predecessors(SuccessorNode* succ);
  
  /**
   * Returns true iff node is already processed false otherwise
   */
  virtual bool is_processed(HMscNode* node);
  
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

  void white_node_found(HMscNode* n);

  void gray_node_found(HMscNode* n);

  void black_node_found(HMscNode* n);

  void node_finished(HMscNode* n);
  
};

#endif /* _DFSB_HMSC_TRAVERSER_H */

// $Id: dfsb_hmsc_traverser.h 231 2009-04-26 19:48:01Z babicaj $
