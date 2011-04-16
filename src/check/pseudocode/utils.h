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
 * Copyright (c) 2009 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: utils.h 1067 2011-03-25 11:20:55Z lkorenciak $
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <stack>
#include "data/dfs_events_traverser.h"
#include "data/dfs_bmsc_graph_traverser.h"
#include "data/dfsb_hmsc_traverser.h"
#include "check/pseudocode/export.h"
#include "data/node_finder.h"
#include "check/pseudocode/visual_closure_initiator.h"
#include "check/pseudocode/causal_closure_initiator.h"

typedef std::map<std::wstring,size_t> StringSizeTMap;
typedef Event* EventP;
typedef std::set<Event*> EventPSet;


/**
 * \brief Returns event's next first successors
 * if event is a send one, returns also its match event
 */
class SCPSEUDOCODE_EXPORT EventFirstSuccessors
{
private:
  EventPSet m_succs;
  void get_first_area_events(EventArea*,Event*);
  void get_strict_successors(StrictEvent*);
  void get_coregion_successors(CoregionEvent*);
  void get_successors(Event *e);
public:
  static EventPSet get(Event* e)
  {
    EventFirstSuccessors first_successors;
    first_successors.get_successors(e);
    return first_successors.m_succs;
  }
}; // EventFirstSuccessors

/**
 * Marks Instances with apropriate identifiers
 */
class SCPSEUDOCODE_EXPORT InstanceIdMarker:public WhiteRefNodeFoundListener
{
private:

  std::string m_instance_id_attribute;

  StringSizeTMap m_identifiers;

  std::list<Instance*> m_modified_instances;

  void set_instance_id(Instance* i);

public:

  InstanceIdMarker(const std::string& instance_id_attribute="instance_id");

  virtual void on_white_node_found(ReferenceNode* node);

  size_t get_instance_id(Instance* i);

  void cleanup_attributes();

  size_t get_count();

};

class TopologyOrderListener:public EventFinishedListener
{
  EventPList* m_topology;

public:

  TopologyOrderListener(EventPList* topology)
  {
    m_topology = topology;
  }

  void on_event_finished(Event* e)
  {
    m_topology->push_front(e);
  }
};

class EventRecognizer
{
public:

  static bool is_matched_receive(Event* e)
  {
    return e->is_matched() && e->is_receive();
  }
};

/**
 * \brief Handles visual closure on bmsc
 * Init DFSEventsTraverser, TopologicalOrderListener
 * and VisualClosureInitiator. Makes easier to run is_leq method
 * on two events.
 */
class EventTopologyHandler
{
private:
  BMscPtr m_bmsc;
  EventPList m_event_topology;
  ChannelMapperPtr m_mapper;

  VisualClosureInitiator* m_p_visual_closure_initiator;
  CausalClosureInitiator* m_p_causal_closure_initiator;

  void init_visual()
  {
    TopologicalOrderListener topology_listener;
    DFSEventsTraverser traverser;

    traverser.add_event_finished_listener(&topology_listener);
    traverser.traverse(m_bmsc);
    m_event_topology = topology_listener.get_topology();

    EventPVector topology(topology_listener.get_topology().size());
    topology.assign(topology_listener.get_topology().begin(),topology_listener.get_topology().end());
    m_p_visual_closure_initiator = new VisualClosureInitiator();
    m_p_visual_closure_initiator->initialize(topology); //here should be topologically sorted elements

  }

  void init_causal()
  {
    m_p_causal_closure_initiator = new CausalClosureInitiator();
    m_p_causal_closure_initiator->initialize(get_topology(),*m_p_visual_closure_initiator,m_mapper);
  }

public:
  EventTopologyHandler(BMscPtr bmsc):m_bmsc(bmsc)
    ,m_p_visual_closure_initiator(NULL)
    ,m_p_causal_closure_initiator(NULL)
  {
    init_visual();
  }

  EventTopologyHandler(const EventTopologyHandler& handler)
  {
    m_bmsc = handler.m_bmsc;
    m_event_topology = handler.m_event_topology;

    m_p_visual_closure_initiator = new VisualClosureInitiator();
    m_p_visual_closure_initiator->initialize(get_topology());

    if(!handler.m_p_causal_closure_initiator)
    {
      m_mapper = handler.m_mapper;
      init_causal();
    }
  }

  ~EventTopologyHandler()
  {
    if(m_p_causal_closure_initiator)
      delete m_p_causal_closure_initiator;
    delete m_p_visual_closure_initiator;
  }

  void init_causal(ChannelMapperPtr mapper)
  {
    m_mapper = mapper;
    init_causal();
  }


  /**
   * \return true if a is less then b (a<b) or equal (a=b)
   */
  bool visual_is_leq(Event* a,Event *b)
  {
    BoolVector vector = m_p_visual_closure_initiator->get_visual_closure(a);

    return vector[m_p_visual_closure_initiator->get_topology_index(b)];
  }

  /**
   * \return true if a is less then b (a<b) or equal (a=b)
   */
  bool causal_is_leq(Event* a,Event *b)
  {
    if(!m_p_causal_closure_initiator)
      throw std::runtime_error("Causal closure hasnt been initialized!");

    BoolVector vector = m_p_causal_closure_initiator->get_causal_closure(a);

    return vector[m_p_visual_closure_initiator->get_topology_index(b)];
  }

  /**
   * \return vector of topologically sorted events according to visual order
   */
  const EventPVector get_topology()
  {
    EventPVector topology(m_event_topology.size());
    topology.assign(m_event_topology.begin(),m_event_topology.end());
    return topology;
  }
};

template<class MscElementContainer,class AttributeType>
void remove_attributes(MscElementContainer& container, const std::string& name)
{
  typename MscElementContainer::const_iterator i;
  for(i=container.begin();i!=container.end();i++)
  {
    MscElement* e = (*i);
    e->remove_attribute<AttributeType>(name);
  }
}

/**
 * \brief Eliminates from hmsc nodes which are not reachable from
 * end nodes of hmsc.
 */
void eliminate_noending_nodes(HMscPtr& hmsc);

/**
 * \brief Marks white nodes by boolean true
 */
class SCPSEUDOCODE_EXPORT WhiteNodeMarker: public WhiteNodeFoundListener
{
protected:

  HMscNodePList m_modified;

  std::string m_mark;

public:

  WhiteNodeMarker(const std::string& mark="mark");

  ~WhiteNodeMarker();

  void cleanup_attributes();

  virtual void on_white_node_found(HMscNode* n);

  bool& get_mark(HMscNode* n);
};

/**
 * \brief Eliminates nodes which are not reachable from end node.
 *
 * Currently supports only BMsc graph, please extend implementation if
 * neccessary.
 */
class NoendingNodesEliminator
{
protected:

  WhiteNodeMarker m_marker;

  DFSBHMscTraverser m_back_traverser;

public:

  NoendingNodesEliminator();

  void eliminate(HMscPtr& hmsc);

};

/**
 * \brief Check whether the HMscNode is ending node
 * (one of the straight successors is End node)
 * due to connection node theather
 *
 */
class IsEndingNode
{
private:

public:
  IsEndingNode()
  {
  }
  static EndNode* check(HMscNode* node)
  {
    EndNode* end;
    //isnt it EndNode itself?
    if((end=dynamic_cast<EndNode*>(node)))
      return end;

    HMscNodePListPtr succs = NodeFinder::successors(node,"IsEndingNode");
    HMscNodePList::iterator it;
    for(it=succs->begin();it!=succs->end();it++)
      if((end=dynamic_cast<EndNode*>(*it)))
        return end;
    return end;

  }
};

/**
 * \brief Vector of all reachable events in bmsc
 * Using DFSEventsTraverser goes through the BMsc and
 * pick up all events
 * (WhiteNodeFoundListener)
 */
class AllReachableEventPVector:public std::vector<EventP>, public WhiteEventFoundListener
{
private:
  DFSEventsTraverser m_traverser;
public:
  AllReachableEventPVector(BMscPtr bmsc)
  {
    m_traverser.add_white_event_found_listener(this);
    m_traverser.traverse(bmsc);
    m_traverser.cleanup_traversing_attributes();
    m_traverser.remove_white_event_found_listeners();
  }

  AllReachableEventPVector()
  {}

  void set_new_bmsc(BMscPtr bmsc)
  {
    clear();
    m_traverser.add_white_event_found_listener(this);
    m_traverser.traverse(bmsc);
    m_traverser.cleanup_traversing_attributes();
    m_traverser.remove_white_event_found_listeners();
  }

  void on_white_event_found(Event* e)
  {
    this->push_back(e);
  }
};

/**
 * \brief List of minimal events
 * Go through the vector of all events using AllReachableEventPVector
 * and using EventTopologyHandler choose minimal events
 */
class MinimalEventPList: public std::list<EventP>
{
private:
  AllReachableEventPVector m_events;
  EventTopologyHandler m_event_top;

public:
  MinimalEventPList(BMscPtr bmsc):m_events(bmsc),m_event_top(bmsc)
  {
    EventPVector::iterator it;
    EventPVector::iterator it_b;
    for (it=m_events.begin();it!=m_events.end();it++)
    {
      bool is_minimal = true;
      for (it_b=m_events.begin();it_b!=m_events.end();it_b++)
      {
        if (it_b != it && m_event_top.visual_is_leq(*it_b,*it))
        {
          is_minimal=false;
          break;
        }
      }
      if (is_minimal)
        push_back(*it);
    }
  }
};

/**
 * \brief list of maximal events
 * Go through the vector of all events using AllReachableEventPVector
 * and using EventTopologyHandler choose maximal events
 */
class MaximalEventPList: public std::list<EventP>
{
private:
  AllReachableEventPVector m_events;
  EventTopologyHandler m_event_top;

public:
  MaximalEventPList(BMscPtr bmsc):m_events(bmsc),m_event_top(bmsc)
  {
    EventPVector::iterator it;
    EventPVector::iterator it_b;
    for (it=m_events.begin();it!=m_events.end();it++)
    {
      bool is_maximal = true;
      for (it_b=m_events.begin();it_b!=m_events.end();it_b++)
      {
        if (it_b != it && m_event_top.visual_is_leq(*it,*it_b))
        {
          is_maximal=false;
          break;
        }
      }
      if (is_maximal)
        push_back(*it);
    }
  }
};
#endif /* _UTILS_H */

// $Id: utils.h 1067 2011-03-25 11:20:55Z lkorenciak $
