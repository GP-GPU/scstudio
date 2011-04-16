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
 * Copyright (c) 2008 Matus Madzin <gotti@mail.muni.cz>
 *
 */

#ifndef __MEMBERSHIP_BASE__
#define __MEMBERSHIP_BASE__

#include <set>
#include <vector>
#include <string>
#include "data/msc.h"
#include "data/searcher.h"
#include "membership/export.h"
#include "check/pseudocode/msc_duplicators.h"
#include "check/time/constraint_syntax.h"

bool contain_focused_instances(std::wstring instance);

class Position;
class Configuration;
class MembershipAlg;
class SnapshotContext;
class CoregionOrdering;
enum check_type {membership, receive_ordering};
enum TopBottom {top, bottom};
enum DiffType {NOT_DIFF, MESSAGE, ATTRIBUTE};

typedef boost::intrusive_ptr<Position> PositionPtr;
typedef boost::intrusive_ptr<Configuration> ConfigurationPtr;
typedef boost::intrusive_ptr<SnapshotContext> SnapshotContextPtr;
typedef boost::intrusive_ptr<CoregionOrdering> CoregionOrderingPtr;

//! saves internal information for algorithm
class MembershipContext
{
private:
  MembershipAlg* mem; //! enable print_report 
  int max_id; //! identification for receive events
  BMscPtr bmsc; //! bmsc which represents the flow
  MscPtr msc; //! msc which represents the specification
  std::vector<std::wstring> focused_instances; //! name of instances on which a user is focused on
  std::map<CoregionAreaPtr, SnapshotContextPtr> snapshots; //! snapshots of context for each coregion
  std::vector<Event*> attributed_events; //! events with attribute
  std::map<std::wstring, std::set<ConfigurationPtr> > checked_conf; //! map of bmsc names and configurations which were checked
  std::map<TimeRelationRefNodePtr, ConfigurationPtr> top_time_references; //! the start of time interval is connected on top 
  std::map<TimeRelationRefNodePtr, ConfigurationPtr> bottom_time_references; //! the start of time interval is connected on bottom
  std::map<CoregionAreaPtr, std::vector<CoregionOrderingPtr> > checked_orderings; //! checked possibilities of coregion ordering
  std::map<CoregionAreaPtr, CoregionOrderingPtr> coregion_ordering; //! save ordering of coregion events in appropriate coregion
  BMscIntervalSetMatrix* time_matrix; //! matrix of all time intervals among events in bmsc
  std::vector<TimeRelationPtr> not_full_covered_intervals; //! intervals in specification which are not full covered
  std::stack<HMscNodePtr> path; //! store information about the path if the flow meets the specification
  bool print_path; //! store whether the path will be printed
  bool relative_time;
  bool absolut_time; 
  
  enum DiffType diff_type;

public:
  MembershipContext() 
  {
    max_id = 0;
    print_path = true;
    relative_time = false;
    absolut_time = false;
    diff_type = NOT_DIFF;
  }

  void set_mem(MembershipAlg* m)
  {
    mem = m;
  }

  MembershipAlg* get_mem() 
  {
    return mem;
  }

  const int get_max_id() const
  {
    return max_id;
  }

  void increase_max_id()
  {
    max_id++;
  }

  void set_bmsc(BMscPtr b)
  {
    bmsc = b;
  }

  const BMscPtr get_bmsc() const
  {
    return bmsc;
  }
  
  void set_specification(MscPtr m)
  {
    msc = m;
  }

  const MscPtr get_specification() const
  {
    return msc;
  }

  void set_focused_instances(std::vector<std::wstring> instances)
  {
    focused_instances.insert(focused_instances.begin(), instances.begin(), instances.end());
  }

  const std::vector<std::wstring> get_focused_instances() const
  {
    return focused_instances;
  }

  //! find name of the instance among fosuces instances
  bool contain_focused_instances(std::wstring instance)
  {
    for(unsigned int i = 0; i < focused_instances.size(); i++)
      if(focused_instances[i] == instance)
        return true;

    return false;
  }

  std::map<CoregionAreaPtr, SnapshotContextPtr> get_snapshots()
  {
    return snapshots;
  }

  SnapshotContextPtr find_snapshot(CoregionAreaPtr cor)
  {
    std::map<CoregionAreaPtr, SnapshotContextPtr>::iterator it;
    it = snapshots.find(cor);

    if(it == snapshots.end())
      return NULL;
    else
      return it->second;
  }
 
  void add_snapshot(CoregionAreaPtr cor, SnapshotContextPtr snap)
  {
    snapshots[cor] = snap;
  }

  const std::vector<Event*> get_attributed_events() const
  {
    return attributed_events;
  }

  //! add event to #attributed_events
  void add_attributed_event(Event* e)
  {
    attributed_events.push_back(e);
  }

  //! removes event attribute "indentification" and clear #attributed_events
  void clear_attributed_events()
  {
    for(unsigned int i = 0; i < attributed_events.size(); i++)
      attributed_events[i]->remove_attribute<int>("identification");

    attributed_events.clear();
  }

  const std::set<ConfigurationPtr> find_checked_conf(std::wstring name) 
  {
    return checked_conf[name];
  }

  void add_checked_conf(std::wstring name, ConfigurationPtr conf)
  {
    checked_conf[name].insert(conf);
  }

  void add_top_time_ref(TimeRelationRefNodePtr rel, ConfigurationPtr conf)
  {
    top_time_references[rel] = conf;
  }

  ConfigurationPtr find_top_time_ref(TimeRelationRefNodePtr rel)
  {
    std::map<TimeRelationRefNodePtr, ConfigurationPtr>::iterator it;
    it = top_time_references.find(rel);    

    if(it == top_time_references.end())
      return NULL;
    else
      return it->second;
  }

  void add_bottom_time_ref(TimeRelationRefNodePtr rel, ConfigurationPtr conf)
  {
    bottom_time_references[rel] = conf;
  }

  ConfigurationPtr find_bottom_time_ref(TimeRelationRefNodePtr rel)
  {
    std::map<TimeRelationRefNodePtr, ConfigurationPtr>::iterator it;
    it = bottom_time_references.find(rel);

    if(it == bottom_time_references.end())
      return NULL;
    else
      return it->second;
  }

  void add_checked_ordering(CoregionAreaPtr cor, CoregionOrderingPtr ordering)
  {
    checked_orderings[cor].push_back(ordering);
  }

  std::vector<CoregionOrderingPtr> find_checked_ordering(CoregionAreaPtr cor)
  {
    return checked_orderings[cor];
  }

  void add_coregion_ordering(CoregionAreaPtr cor, CoregionOrderingPtr ordering)
  {
    coregion_ordering[cor] = ordering;
  }

  CoregionOrderingPtr find_coregion_ordering(CoregionAreaPtr cor)
  {
    std::map<CoregionAreaPtr, CoregionOrderingPtr>::iterator it;
    it = coregion_ordering.find(cor);

    if(it == coregion_ordering.end())
      return NULL;
    else
      return it->second; 
  }

  void set_time_matrix(BMscIntervalSetMatrix* matrix)
  {
    time_matrix = matrix;
  }

  BMscIntervalSetMatrix* get_time_matrix()
  {
    return time_matrix;
  }

  void add_not_covered_interval(TimeRelationPtr hmsc_inter)
  {
    not_full_covered_intervals.push_back(hmsc_inter);
  }

  const std::vector<TimeRelationPtr> get_not_covered_intervals()
  {
    return not_full_covered_intervals;
  }

  void push_path(HMscNodePtr node)
  {
    path.push(node);
  }

  HMscNodePtr top_pop_path()
  {
    HMscNodePtr node = path.top();
    path.pop();
    return node;
  }

  HMscNodePtr top_path()
  {
    return path.top();
  }

  int get_path_size()
  {
    return path.size();
  }

  void print_msc_path(bool value)
  {
    print_path = value;
  }

  bool get_print_path()
  {
    return print_path;
  }

  void found_relative_time()
  {
    relative_time = true;
  }

  bool get_relative_time()
  {
    return relative_time;
  }

  void found_absolut_time()
  {
    absolut_time = true;
  }

  bool get_absolut_time()
  {
    return absolut_time;
  }

  enum DiffType get_diff_type()
  {
    return diff_type;
  }

  void set_diff_type(enum DiffType type)
  {
    diff_type = type;
  }
};

//! store information about the position of checking algorithm on instance
class Position
{

private:
  std::wstring name;  //! name of instance
  std::vector<Event*> events;  //!start searching events

  //! Number of references to this object.
  mutable size_t m_counter;

// see http://www.boost.org/doc/libs/1_37_0/libs/smart_ptr/intrusive_ptr.html
friend void intrusive_ptr_add_ref(const Position *ptr);
friend void intrusive_ptr_release(const Position *ptr);

public:
  Position(std::wstring name, EventPtr position)
  {
    this->name = name;
    this->events.push_back(position.get());
    this->m_counter = 0;
  }

  Position(std::wstring name, CoregionEventPVector vector)
  {
    this->m_counter = 0;

    CoregionEventPVector::iterator it;

    for(it = vector.begin(); it!=vector.end(); it++)
    {
      events.push_back(*it);
    }
  }

  Position(PositionPtr origin)
  {
    this->m_counter = 0;
    this->name = origin->get_name();
    
    std::vector<Event*>::iterator it;
    std::vector<Event*> origin_events = origin->get_events();

    for(it = origin_events.begin(); it != origin_events.end(); it++)
    {
      events.push_back(*it);
    }
  }

  std::wstring get_name()
  {
    return name;
  }

  std::vector<Event*>  get_events()
  {
    return events;
  }

  bool compare(PositionPtr a)
  {
    if(name != a->get_name()) return false;
    
    if(events.size() != a->get_events().size()) return false;

    std::vector<Event*>::iterator it;
    Event* b;    

    for(it=events.begin(); it!=events.end(); it++)
    {
      b = a->find_event(*it);
      
      if(b == NULL || !this->compare_events(*it, b))
        return false;
    }

    return true;
  }

  Event* find_event(Event* event)
  {
    std::vector<Event*>::iterator it;
    for(it=events.begin(); it!=events.end(); it++)
    {
      if(*it == event) 
        return *it;
    }
    
    return NULL;
  }

  bool compare_events(Event* a, Event* b)
  {
    if(a->get_instance()->get_label() != b->get_instance()->get_label())
      return false;

    if(a->is_send() != b->is_send())
      return false;

    if((a->get_complete_message() == NULL) != (b->get_complete_message() == NULL))
      return false;

    if(a->get_message()->get_label() != b->get_message()->get_label())
      return false;

    if(a->is_matched() != b->is_matched())
      return false;

    if(a->is_matched())
    {
      if(a->is_send())
      { 
        if(a->get_receiver_label() != b->get_receiver_label())
          return false;
      }
      else
      {
        if(a->get_sender_label() != b->get_sender_label())
          return false;
      }
    }  

    return true;
  }

  void set_events(std::vector<Event*> new_events)
  {
    events = new_events;
  }

  bool is_empty()
  {
    std::vector<Event*>::iterator it;

    for(it = events.begin(); it != events.end(); it++)
    {
      if(*it != NULL)
        return false;
    }
 
    return true;
  }
};

inline void intrusive_ptr_add_ref(const Position *ptr)
{
  if(ptr != NULL)
    ++ptr->m_counter;
}

inline void intrusive_ptr_release(const Position *ptr)
{
  if(ptr != NULL)
  {
    if(--ptr->m_counter <= 0)
      delete ptr;
  }
}


//! store information about cheking algorithm in the flow
class Configuration
{

private: 
  InstancePtrList instance_list;  
  std::set<PositionPtr> positions;

  //! Number of references to this object.
  mutable size_t m_counter;

friend void intrusive_ptr_add_ref(const Configuration *ptr);
friend void intrusive_ptr_release(const Configuration *ptr);

public: 
 
  Configuration(InstancePtrList instances)
  {
    this->m_counter = 0;

    this->instance_list = instances;
    InstancePtrList::iterator it;
    EventAreaPtr area;
    StrictOrderAreaPtr st_area;

    for(it = instances.begin(); it!= instances.end(); ++it)
    {
      area = (*it)->get_first();
      st_area = boost::dynamic_pointer_cast<StrictOrderArea>(area);
      
      if(st_area != NULL)
        positions.insert(new Position((*it)->get_label(), st_area->get_first()));
      else
        if((*it)->get_first())
          std::cerr << "Error: Coregions in pattern bMSC are not supported" << std::endl;
    }
  }

  Configuration(ConfigurationPtr origin) 
  {
    this->m_counter = 0;

    InstancePtrList origin_inst = origin->get_instances();
    this->instance_list.insert(instance_list.begin(), origin_inst.begin(), origin_inst.end());

    std::set<PositionPtr> origin_positions = origin->get_positions();

    std::set<PositionPtr>::iterator it;
   
    for(it = origin_positions.begin(); it != origin_positions.end(); it++)
    {
      this->positions.insert(new Position(*it));
    }
  }


  PositionPtr find(PositionPtr p)
  {
    std::set<PositionPtr>::iterator it;

    for(it = positions.begin(); it != positions.end(); it++)
    {
      if((*it)->compare(p)) return *it;
    }

    return NULL;
  }

  PositionPtr find(std::wstring name)
  {
    std::set<PositionPtr>::iterator it;

    for(it = positions.begin(); it != positions.end(); it++)
    {
      if((*it)->get_name() == name)
        return *it;
    }
   
    return NULL;
  }

  bool is_null(MembershipContext* c)
  {
    std::set<PositionPtr>::iterator it;

    for(it = positions.begin(); it != positions.end(); it++)
    {
      if(!(*it)->get_events().empty())
      {
        if(c->get_focused_instances().empty() || c->contain_focused_instances((*it)->get_name()))
          return false;
      }
    }

    return true;
  }

  bool compare(ConfigurationPtr a)
  {
    if(instance_list.size() != a->get_instances().size() || positions.size() != a->get_positions().size())
      return false;

    std::set<PositionPtr>::const_iterator set_it;
    std::set<PositionPtr> p_set = a->get_positions();
    for(set_it = p_set.begin(); set_it != p_set.end(); set_it++)
    {
      if(!this->find(*set_it)) return false;
    }
    return true;
  }

  InstancePtrList get_instances()
  {
    return instance_list;
  }

  const std::set<PositionPtr>& get_positions() const
  {
    return positions;
  }

  void set_position_events(std::wstring name, std::vector<Event*> events)
  {
    std::set<PositionPtr>::iterator pos_it;

    for(pos_it = positions.begin(); pos_it != positions.end(); pos_it++)
    {
      if((*pos_it)->get_name() == name)
      {
        (*pos_it)->set_events(events);
        break;
      }
    }
  }

};

inline void intrusive_ptr_add_ref(const Configuration *ptr)
{
  if(ptr != NULL)
    ++ptr->m_counter;
}

inline void intrusive_ptr_release(const Configuration *ptr)
{
  if(ptr != NULL)
  {
    if(--ptr->m_counter <= 0)
      delete ptr;
  }
}

//! store specification and flow events where checking algorithm was in some timestamp
class SnapshotContext
{

private:
  std::vector<Event*> node_e;
  std::vector<Event*> b_events;

  //! Number of references to this object.
  mutable size_t m_counter;

friend void intrusive_ptr_add_ref(const SnapshotContext *ptr);
friend void intrusive_ptr_release(const SnapshotContext *ptr);

public:
  SnapshotContext(std::vector<Event*> node_events, std::vector<Event*> bmsc_events)
  {
    this->m_counter = 0;

    std::vector<Event*>::iterator it;

    for(it = node_events.begin(); it != node_events.end(); it++)
    {
      node_e.push_back(*it);
    }
    for(it = bmsc_events.begin(); it != bmsc_events.end(); it++)
    {
      b_events.push_back(*it);
    }
  }

  std::vector<Event*> getNodeEvents()
  { 
    return node_e;
  }

  std::vector<Event*> getBMSCEvents()
  {
    return b_events;
  }
};

inline void intrusive_ptr_add_ref(const SnapshotContext *ptr)
{
  if(ptr != NULL)
    ++ptr->m_counter;
}

inline void intrusive_ptr_release(const SnapshotContext *ptr)
{
  if(ptr != NULL)
  {
    if(--ptr->m_counter <= 0)
      delete ptr;
  }
}

//! store information about the coregion linearization
class CoregionOrdering
{
private:
  std::vector<CoregionEventPtr> ordering;

  //! Number of references to this object.
  mutable size_t m_counter;

friend void intrusive_ptr_add_ref(const CoregionOrdering *ptr);
friend void intrusive_ptr_release(const CoregionOrdering *ptr);

public:
  CoregionOrdering(std::vector<CoregionEventPtr> ord)
  {
    m_counter = 0;

    for(unsigned int i = 0; i < ord.size(); i++)
    { 
      this->ordering.push_back(ord[i]);
    }
  }

  CoregionOrdering(CoregionOrderingPtr origin)
  {
    m_counter = 0;
    
    std::vector<CoregionEventPtr> ord = origin->getOrdering();
    
    for(unsigned int i = 0; i < ord.size(); i++)
    {
      this->ordering.push_back(ord[i]);
    }
  }
  
  void deleteOrdering()
  {
    ordering.clear();
  }

  void removeFirst()
  {
    if(!ordering.empty())
      ordering.erase(ordering.begin());
  }

  void removeLast()
  {
    if(!ordering.empty())
      ordering.pop_back();
  }

  void addLast(CoregionEventPtr event)
  {
    ordering.push_back(event);
  }

  CoregionEventPtr getFirst()
  {
    return ordering.front();
  }

  std::vector<CoregionEventPtr> getOrdering()
  {
    return ordering;
  }

  bool compare(CoregionOrderingPtr a)
  { 
    std::vector<CoregionEventPtr> a_ordering = a->getOrdering();

    if(this->ordering.size() != a_ordering.size())
      exit (4); //return false;

    for(unsigned int i = 0; i < this->ordering.size(); i++)
    {
      if(this->ordering.at(i) != a_ordering.at(i))
        return false;
    }
    return true;
  }
};

inline void intrusive_ptr_add_ref(const CoregionOrdering *ptr)
{
  if(ptr != NULL)
    ++ptr->m_counter;
}

inline void intrusive_ptr_release(const CoregionOrdering *ptr)
{
  if(ptr != NULL)
  {
    if(--ptr->m_counter <= 0)
      delete ptr;
  }
}


class SCMEMBERSHIP_EXPORT MembershipAlg: public Searcher
{
  
public:

  MembershipAlg(){};

  virtual ~MembershipAlg(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Membership"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L""; }

  //! Returns a list of preconditions for this search.
  virtual PreconditionList get_preconditions(MscPtr msc) const;

  //! Finds the bmsc flow in hmsc specification
  virtual MscPtr find(MscPtr hmsc, MscPtr bmsc);

  //! Make diff between specification and flow
  virtual MscPtr diff(MscPtr specification, std::vector<MscPtr>& flows);
  
  //! Finds each bmsc flow from the vector in hmsc specification
  virtual MscPtr find(MscPtr hmsc, std::vector<MscPtr>& bmscs);

  //! Finds the bmsc flow in hmsc specification, focusing on specified instances, DO NOT CHECKING TIME CONSTRAINTS
  virtual MscPtr find(MscPtr hmsc, MscPtr bmsc, std::vector<std::wstring> instances);

MscPtr get_dp_msc(MscPtr msc);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes() {}

};

#endif
