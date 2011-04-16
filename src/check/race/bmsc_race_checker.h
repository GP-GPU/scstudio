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
 * $Id: bmsc_race_checker.h 123 2008-11-28 23:39:47Z gotthardp $
 */

#ifndef _BMSC_RACE_CHECKER_H
#define _BMSC_RACE_CHECKER_H

#include "data/msc.h"
#include "data/dfs_events_traverser.h"
#include "check/pseudocode/visual_closure_initiator.h"
#include "check/pseudocode/causal_closure_initiator.h"

typedef std::list<Instance*> InstancePList;

class BMscRaceChecker:
public BMscChecker,
public TopologicalOrderListener
{
  
protected:
  
  VisualClosureInitiator m_visual_initiator;
  
  CausalClosureInitiator m_causal_initiator;
  
  static const std::string m_instance_events_attribute;
  
  InstancePList m_modified_instaces;
  
  EventPList& get_instance_events(Instance* i)
  {
    static EventPList empty;
    return i->get_attribute<EventPList>(m_instance_events_attribute,empty);
  }
  
  /**
   * It was found that e1<e2 but not e1<<e2
   */
  BMsc* create_counter_example(Event* e1, Event* e2);
  
public:
  
  BMscRaceChecker()
  {
    
  }
  
  BMscPtr check(BMscPtr bmsc, ChannelMapperPtr mapper);
  
  virtual void on_event_finished(Event* event)
  {
    TopologicalOrderListener::on_event_finished(event);
    //used to compare only ordering of events at the same instance
    get_instance_events(event->get_instance()).push_front(event);
  }
  
  void cleanup_attributes();
  
};

#endif /* _BMSC_RACE_CHECKER_H */

// $Id: bmsc_race_checker.h 123 2008-11-28 23:39:47Z gotthardp $
