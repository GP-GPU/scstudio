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
 * $Id: dfs_events_traverser.h 719 2010-04-14 21:26:46Z vacek $
 */

#ifndef _DFS_EVENTS_TRAVERSER_H
#define _DFS_EVENTS_TRAVERSER_H

#include "data/msc.h"
#include "data/checker.h"

typedef std::list<Event*> EventPList;
typedef boost::shared_ptr<EventPList> EventPListPtr;
typedef std::list<MscElement*> MscElementPList;

/**
 * Listener of detected white Event
 */
class SCMSC_EXPORT WhiteEventFoundListener
{
public:
  
  virtual ~WhiteEventFoundListener()
  {
    
  }
  
  virtual void on_white_event_found(Event* e)=0;
};

/**
 * Listener of detected white Event
 */
class SCMSC_EXPORT GrayEventFoundListener
{
public:
  
  virtual ~GrayEventFoundListener()
  {
    
  }
  
  virtual void on_gray_event_found(Event* e)=0;
};

/**
 * Listener of detected black Event
 */
class SCMSC_EXPORT BlackEventFoundListener
{
public:
  
  virtual ~BlackEventFoundListener()
  {
    
  }
  
  virtual void on_black_event_found(Event* e)=0;
};

/**
 * Listener of finished Event
 */
class SCMSC_EXPORT EventFinishedListener
{
public:
  
  virtual ~EventFinishedListener()
  {
    
  }
  
  virtual void on_event_finished(Event* event)=0;
};

/**
 * Listener for event successor relations.
 */
class SCMSC_EXPORT EventSuccessorListener
{
public:
  
  virtual ~EventSuccessorListener()
  {
    
  }
  
  virtual void on_event_successor(Event* event, Event* successor)=0;
};

/**
 * Listener for sender-receiver relations.
 */
class SCMSC_EXPORT SendReceivePairListener
{
public:
  
  virtual ~SendReceivePairListener()
  {
    
  }
  
  virtual void on_send_receive_pair(Event* send, Event* receive)=0;
};

typedef std::list<EventFinishedListener*> EventFinishedListenerPList;
typedef std::list<WhiteEventFoundListener*> WhiteEventFoundListenerPList;
typedef std::list<GrayEventFoundListener*> GrayEventFoundListenerPList;
typedef std::list<BlackEventFoundListener*> BlackEventFoundListenerPList;
typedef std::list<EventSuccessorListener*> EventSuccessorListenerPList;
typedef std::list<SendReceivePairListener*> SendReceivePairListenerPList;

/**
 * Processes Events during traversing BMsc's Events in depth first search manner.
 *
 * Events of BMsc are during traversing in different states - with different 
 * color. WHITE color means that event wasn't traversed before. GRAY events
 * are those which have been already traversed and they are just on the stack - 
 * not all successors of these events were processed. BLACK events are those 
 * which have been already traversed but aren't yet on the stack - all 
 * successors have been processed.
 *
 * Listeners' methods are called at particular positions in code of traversing.
 */
class SCMSC_EXPORT DFSEventsTraverser
{
  
public:
  
  DFSEventsTraverser(const std::string& color="color");
  
  virtual ~DFSEventsTraverser();
  
  /**
   * Traverses events in bmsc.
   *
   * Events are traversed in depth first search manner.
   */  
  virtual void traverse(BMscPtr bmsc);
  
  /**
   * Adds EventFinishedListener
   */
  void add_event_finished_listener(EventFinishedListener* l)
  {
    event_finished_listeners.push_back(l);
  }
  
  /**
   * Adds WhiteEventFoundListener
   */
  void add_white_event_found_listener(WhiteEventFoundListener* l)
  {
    white_event_found_listeners.push_back(l);
  }
  
  /**
   * Adds GrayEventFoundListener
   */
  void add_gray_event_found_listener(GrayEventFoundListener* l)
  {
    gray_event_found_listeners.push_back(l);
  }
  
  /**
   * Adds BlackEventFoundListener
   */
  void add_black_event_found_listener(BlackEventFoundListener* l)
  {
    black_event_found_listeners.push_back(l);
  }
  
  /**
   * Adds EventSuccessorListener
   */
  void add_event_successor_listener(EventSuccessorListener* l)
  {
    event_successor_listeners.push_back(l);
  }
  
  /**
   * Adds SendReceivePairListenerPList
   */
  void add_send_receive_pair_listener(SendReceivePairListener* l)
  {
    send_receive_pair_listeners.push_back(l);
  }
  
  /**
   * Cleans up set attributes.
   */
  virtual void cleanup_traversing_attributes();
  
  const MscElementPList& get_reached_elements()
  {
    return m_reached_elements;
  }
  
  static EventPListPtr topology_order(BMscPtr b);

  void remove_white_event_found_listeners();
  void remove_gray_event_found_listeners();
  void remove_black_event_found_listeners();
  void remove_event_finished_listeners();
  void remove_event_successor_listeners();
  void remove_send_receive_pair_listeners();
  void remove_all_listeners();
  
protected:
  
  /**
   * Holds path to currently reached event
   */
  MscElementPList m_reached_elements;
  
  /**
   * List of colored events during traversing.
   */
  EventPList m_colored_events;

  std::string m_color;
  
  /**
   * Holds listeners
   */
  EventFinishedListenerPList event_finished_listeners;
  WhiteEventFoundListenerPList white_event_found_listeners;
  GrayEventFoundListenerPList gray_event_found_listeners;
  BlackEventFoundListenerPList black_event_found_listeners;
  GrayEventFoundListenerPList grey_event_found_listeners;
  EventSuccessorListenerPList event_successor_listeners;
  SendReceivePairListenerPList send_receive_pair_listeners;

  virtual void traverse_area(EventArea* area, Event* predecessor = NULL);
  
  virtual void traverse_strict_event(StrictEvent* event);
  
  virtual void traverse_coregion_event(CoregionEvent* event);
  
  virtual void traverse_matching_event(Event* event);
  
  bool is_processed(Event* event);
 
  /**
   * Called when white Event is found.
   */
  void white_event_found(Event* e);
  
  /**
   * Called when gray Event is found.
   */
  void gray_event_found(Event* e);
  
  /**
   * Called when black Event is found.
   */
  void black_event_found(Event* e);
  
  /**
   * Called when all successors of e are processed.
   */
  void event_finished(Event* e);
  
  /**
   * Called when a relation event-->successor is found.
   */
  void event_successor(Event* event, Event* successor);

  /**
   * Called when a message send-->receive is found.
   */
  void send_receive_pair(Event* send, Event* receive);

  /**
   * Sets color attribute of e to c value .
   */
  void set_color(Event* e, NodeColor c)
  {
    e->set_attribute<NodeColor>(m_color,c);
  }
  
  /**
   * Returns value of color attribute.
   *
   * If attribute isn't set it is set to default value WHITE.
   */
  NodeColor get_color(Event* e);
  
};

#endif /* _DFS_EVENTS_TRAVERSER_H */

// $Id: dfs_events_traverser.h 719 2010-04-14 21:26:46Z vacek $
