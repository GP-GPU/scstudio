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
 * $Id: causal_closure_initiator.h 124 2008-11-29 11:22:31Z gotthardp $
 */

#ifndef _CAUSAL_CLOSURE_INITIATOR_H
#define _CAUSAL_CLOSURE_INITIATOR_H

#include <stack>
#include <vector>

#include "data/msc.h"
#include "data/dfs_instance_events_traverser.h"
#include "check/pseudocode/visual_closure_initiator.h"
#include "check/pseudocode/export.h"

typedef std::stack<Event*> EventPStack;

class SCPSEUDOCODE_EXPORT CausalClosureInitiator
{
  
protected:
  
  /**
   * Used for cleaning up attributes.
   */
  EventPStack m_modified_events;
  
  /**
   * Name of causal closure attribute.
   */
  std::string m_causal_closure_attribute;
  
  /**
   * Logically multiply closure by itself.
   */
  void square_closure(std::vector<BoolVector*>& closure);
  
  /**
   * Logical multiplication of row and column (indeces) of square matrix closure.
   */
  bool multiply(std::vector<BoolVector*>& closure, size_t row, size_t column);

public:

  /**
   * Sets explicit names of attributes
   */
  CausalClosureInitiator(
    const std::string& causal_closure_attribute = "causal_closure")
  {
    m_causal_closure_attribute = causal_closure_attribute;
  }

  
  /**
   * Cleans up set attributes.
   */
  ~CausalClosureInitiator()
  {
    cleanup_attributes();
  }
    
  /**
   * Getter of causal closure attribute of e.
   */
  BoolVector& get_causal_closure(Event* e)
  {
    static BoolVector empty(1,false);
    return e->get_attribute<BoolVector>(m_causal_closure_attribute,empty);
  }
  
  /**
   * Computes causal closure.
   *
   * A visual_closure_init must be initialized - initialize method must be 
   * called before.
   *
   * @param events - topologically sorted events used for visual_closure_init's initialization
   * @param visual_closure_init - initialized VisualClosureInitiator
   */
  void initialize(const EventPVector& events, 
    VisualClosureInitiator& visual_closure_init, ChannelMapperPtr mapper);

  /**
   * Cleans up set attributes.
   */
  void cleanup_attributes();
  
};

#endif /* _CAUSAL_CLOSURE_INITIATOR_H */

// $Id: causal_closure_initiator.h 124 2008-11-29 11:22:31Z gotthardp $
