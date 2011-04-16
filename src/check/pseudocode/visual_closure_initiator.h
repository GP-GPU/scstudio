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
 * $Id: visual_closure_initiator.h 124 2008-11-29 11:22:31Z gotthardp $
 */

#ifndef _VISUAL_CLOSURE_INITIATOR_H
#define _VISUAL_CLOSURE_INITIATOR_H

#include <list>
#include <vector>

#include "data/msc.h"
#include "data/dfs_events_traverser.h"
#include "check/pseudocode/export.h"

typedef std::list<Event*> EventPList;
typedef std::vector<Event*> EventPVector;
typedef std::vector<bool> BoolVector;
typedef std::list<CoregionArea*> CoregionAreaPList;

/**
 * Stores topological order of events in external list.
 */
class TopologicalOrderListener:public EventFinishedListener
{
  
protected:
  
  EventPList m_topology;
    
public:
  
  const EventPList& get_topology()
  {
    return m_topology;
  }
  
  virtual void on_event_finished(Event* event)
  {
    m_topology.push_front(event);
  }
  
};

class SCPSEUDOCODE_EXPORT VisualClosureInitiator
{
  
protected:
  
  /**
   * Events with modified attributes
   */
  EventPList m_modified_events;
 
  /**
   * Name of topology attribute.
   */
  std::string m_topology_index_attribute;
  
  /**
   * Name of visual closure attribute.
   */
  std::string m_visual_closure_attribute;
  
  /**
   * Completes closure_matrix between succ_index event and pred_index event
   */  
  void make_closure(std::vector<BoolVector*>& closure_matrix, size_t succ_index, size_t pred_index);
  
  /**
   * Completes closure_matrix between succ_index event and last events in pred_area
   */
  void make_closure(std::vector<BoolVector*>& closure_matrix, size_t succ_index, EventArea* pred_area);

  /**
   * Completes closure_matrix for topologicaly sorted event - event_topology
   */
  void make_closure(std::vector<BoolVector*>& closure_matrix, const EventPVector& event_topology);
  
  /**
   * Completes closure_matrix for event_e with index e
   */
  void make_closure(std::vector<BoolVector*>& closure_matrix, size_t e, Event* event_e);
  
public:
  
  /**
   * Sets explicit names of attributes
   */
  VisualClosureInitiator(
    const std::string& visual_closure_attribute = "visual_closure",
    const std::string& topology_index_attribute = "visual_closure_topology_index")
  {
    m_visual_closure_attribute = visual_closure_attribute;
    m_topology_index_attribute = topology_index_attribute;
  }

  
  /**
   * Calls cleanup_attributes().
   */
  ~VisualClosureInitiator();
    
  /**
   * Computes visual closure.
   *
   * @param event_topology - topologically sorted events of BMsc 
   * (i<j <=> (event_topology[i]<event_topology[j] or event_topology[i]||event_topology[j]).
   */
  void initialize(const EventPVector& event_topology);
  
  /**
   * Getter of visual closure attribute of e.
   */
  BoolVector& get_visual_closure(Event* e)
  {
    static BoolVector empty(1,false);
    return e->get_attribute<BoolVector>(m_visual_closure_attribute,empty);
  }

  /**
   * Getter of topology index attribute of e.
   */
  size_t get_topology_index(Event* e);
  
  /**
   * Setter of topology index attribute of e.
   */
  void set_topology_index(Event* e,size_t i);
  
  /**
   * Cleans up set attributes.
   */
  void cleanup_attributes();

};

#endif /* _VISUAL_CLOSURE_INITIATOR_H */

// $Id: visual_closure_initiator.h 124 2008-11-29 11:22:31Z gotthardp $
