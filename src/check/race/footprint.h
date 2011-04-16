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
 * $Id: footprint.h 188 2009-02-03 09:46:50Z babicaj $
 */

#ifndef _FOOTPRINT_H
#define _FOOTPRINT_H

#include <map>
#include <set>
#include <list>

// see http://www.boost.org/doc/libs/1_37_0/libs/smart_ptr/shared_ptr.htm
#include <boost/shared_ptr.hpp>

#include "data/msc.h"


class Footprint;
class EventDependentInstances;

typedef std::vector<bool> BoolVector;
typedef boost::shared_ptr<EventDependentInstances> EDInstancesPtr;
typedef boost::shared_ptr<Footprint> FootprintPtr;
typedef std::list<MscElement*> MscElementPList;

class EDInstancesPtrComparator
{
public:
  bool operator()(const EDInstancesPtr& a, const EDInstancesPtr& b);
};

class FootprintPtrComparator
{
public:
  bool operator()(const FootprintPtr& a, const FootprintPtr& b) const;
};

typedef std::set<EDInstancesPtr,EDInstancesPtrComparator> EDInstancesPtrSet;
typedef std::vector<EDInstancesPtrSet> EDInstancesPtrSetVector;

/**
 * Represents dependent Instances -- contain lesser/greater (depends on chosen semantic) 
 * Event then the Event with specified m_event in this class. 
 */
class EventDependentInstances
{
  
private:
  
  /**
   * This instance of DependetInstances represents dependet instances of an Event
   * of this attribute 
   */
  Event* m_event;
  
  /**
   * Each Instance i must have its own number. For this vector m_instances holds: 
   * m_instances[i]==true iff i contains any greater/lesser Event than the Event 
   * m_event. 
   */
  BoolVector m_instances;
  
public:
  
  /**
   *
   */
  EventDependentInstances(Event* event,size_t instances_count);
  
  /**
   * Used in std::set as comparision method
   *
   * The di is supposed to have m_instances as large as this one has.
   */
  bool operator<(const EventDependentInstances& di) const;
  
  /**
   * Returns -1 for this<di, 0 for this==di and 1 for this>di
   */
  int compare(const EventDependentInstances& di) const;
  
  void set_dependent(size_t instance);
  
  const BoolVector& get_instances();
  
  Event* get_event();
  
};

class ExtremeEvents
{
  
protected:
  
  /**
   * Holds greater/lesser Instances of Events accessible under id of Event's
   * Instance.
   */
  EDInstancesPtrSetVector m_events_instances;
  
  /**
   * Compares first and second like they would be a strings.
   * 
   * Compares element by element, if any less/greater element in first is found
   * than an element of second at the same position, first is declared to be 
   * less/greater than second.
   *
   * Returns x<0 if first < second, 0 if first == second, x>0 if first > second
   */
  int compare(
    const EDInstancesPtrSet& first,
    const EDInstancesPtrSet& second) const;
  
public:
  
  ExtremeEvents();
  
  ExtremeEvents(size_t instances_count);
  
  /**
   * Compares ExtremeEvents item by item.
   *
   * Items are supposed to be separate EDInstancesPtrSets which are elements
   * of m_events_instances. Empty item i (empty EDInstancesPtrSet) is supposed 
   * to be less than anything else which is not empty.
   */
  bool operator<(const ExtremeEvents& other) const;
  
  void add_extreme_event(size_t instance, EDInstancesPtr& edis);
  
  const EDInstancesPtrSetVector& get_events_instances() const;
};

/**
 * Footprint of path in BMsc-graph.
 *
 * Footprint is implementation of footprint (with lowercase f) suggested in 
 * article Decidable Race Condition in High-Level Message Sequence Charts by
 * Jan Fousek, Vojtech Rehak and Petr Slovak.
 */
class Footprint:public ExtremeEvents
{
  
private:
  
  /**
   * Path in HMsc which built this Footprint.
   *
   * This path must contain exactly one ReferenceNode - the last one element of
   * the m_path or must contain only one element - StartNode (initial Footprint). 
   * All remaining members of this m_path are supposed to be 
   * ConnectionNodes.
   */
  MscElementPList m_path;
  
  /**
   * Previous Footprint which was this one created from.
   */
  FootprintPtr m_previous;
  
public:
  
  /**
   * Creates new initial Footprint
   *
   * Created Footprint doesn't reference neither ReferenceNode nor Footprint and 
   * is supposed to be predecessor (not neccessary direct) of the others 
   * Footprints.
   */
  Footprint(StartNode* start, size_t instances_count);
  
  /**
   * Creates new Footprint from previous Footprint and maximal events (denoted 
   * as MaxP in the article) of particular BMsc b.
   *
   * The new Footprint is created likewise construction of footprint (with 
   * lowercase f) in the article. 
   * 
   * @param path - members of this list are copied into m_path, see m_path property for details about neccessary properties of this list
   * @param max_events_less - ExtremeEvents of b whose m_instances attribute is supposed to have lesser semantic. 
   * @param max_events_greater - ExtremeEvents of b whose m_instances attribute is supposed to have greater semantic.
   */
  Footprint(
    FootprintPtr previous, 
    const MscElementPList& path,
    const ExtremeEvents& max_events_less,
    const ExtremeEvents& max_events_greater);
  
  HMscNode* get_node();

  const MscElementPList& get_path() const;

  FootprintPtr get_previous();
};

#endif /* _FOOTPRINT_H */

// $Id: footprint.h 188 2009-02-03 09:46:50Z babicaj $
