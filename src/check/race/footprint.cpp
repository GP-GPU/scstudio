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
 * $Id: footprint.cpp 188 2009-02-03 09:46:50Z babicaj $
 */

#include "check/race/footprint.h"

bool EDInstancesPtrComparator::operator()(const EDInstancesPtr& a, const EDInstancesPtr& b)
{
  return a->operator <(*b.get());
}

bool FootprintPtrComparator::operator()(const FootprintPtr& a, const FootprintPtr& b) const
{
  return a->operator <(*b.get());
}

EventDependentInstances::EventDependentInstances(Event* event,size_t instances_count)
{
  m_event = event;
  m_instances = BoolVector(instances_count,false);
}

bool EventDependentInstances::operator<(const EventDependentInstances& di) const
{
  return compare(di)<0;
}
  
int EventDependentInstances::compare(const EventDependentInstances& di) const
{
  if(m_event<di.m_event) return -1;
  if(di.m_event<m_event) return 1;
  //compare bool vector as binary number
  for(size_t i=0; i<m_instances.size(); i++)
  {
    if(m_instances[i]!=di.m_instances[i])
      return m_instances[i]-di.m_instances[i];
  }
  //otherwise this is same as di
  return 0;
}

void EventDependentInstances::set_dependent(size_t instance)
{
  m_instances[instance] = true;
}

const BoolVector& EventDependentInstances::get_instances()
{
  return m_instances;
}

Event* EventDependentInstances::get_event()
{
  return m_event;
}

int ExtremeEvents::compare(
  const EDInstancesPtrSet& first,
  const EDInstancesPtrSet& second) const
{
  EDInstancesPtrSet::const_iterator first_i = first.begin();
  EDInstancesPtrSet::const_iterator second_i = second.begin();
  while(first_i!=first.end() && second_i!=second.end())
  {
    const EDInstancesPtr first_edis = *first_i;
    const EDInstancesPtr second_edis = *second_i;
    int comparision = first_edis->compare(*second_edis.get());
    if(comparision!=0) return comparision;
    first_i++;
    second_i++;
  }
  return first.size()-second.size();
}

ExtremeEvents::ExtremeEvents()
{
  
}

ExtremeEvents::ExtremeEvents(size_t instances_count)
{
  m_events_instances = EDInstancesPtrSetVector(instances_count);
}

bool ExtremeEvents::operator<(const ExtremeEvents& other) const
{
  const EDInstancesPtrSetVector& other_events = other.get_events_instances();
  for(size_t i=0;i<m_events_instances.size();i++)
  {
    int comparision = compare(m_events_instances[i],other_events[i]);
    if(comparision!=0)
    {
      return comparision<0;
    }
  }
  //otherwise this is supposed to be same as other
  return false;
}

void ExtremeEvents::add_extreme_event(size_t instance, EDInstancesPtr& edis)
{
  m_events_instances[instance].insert(edis);
}

const EDInstancesPtrSetVector& ExtremeEvents::get_events_instances() const
{
  return m_events_instances;
}


Footprint::Footprint(StartNode* start, size_t instances_count):ExtremeEvents(instances_count)
{
  m_path.push_back(start);
}

Footprint::Footprint(
  FootprintPtr previous, 
  const MscElementPList& path,
  const ExtremeEvents& max_events_less,
  const ExtremeEvents& max_events_greater)
  :ExtremeEvents(previous->get_events_instances().size())
{
  const EDInstancesPtrSetVector& previous_events = previous->get_events_instances();
  const EDInstancesPtrSetVector& events_less = max_events_less.get_events_instances();
  const EDInstancesPtrSetVector& events_greater = max_events_greater.get_events_instances();

  m_previous = previous;
  m_path.insert(m_path.begin(),++path.begin(),path.end());
  size_t instances_count = previous_events.size();

  for(size_t i=0; i<instances_count; i++)
  {
    /*
     * If events_greater[i] contains any EDInstances these will become part
     * of new Footprint - doesn't care about previous_events[i] 
     */
    if(events_greater[i].size()>0)
    {
      m_events_instances[i] = events_greater[i];
    }
    else if(previous_events[i].size()>0)
    {
      /*
       * Let previous_edi be a EvendDependntInstaces in previous_events[i]. Find
       * less_edi (EventDependentInstances) from events_less and instance 
       * index j such that (less_edi.m_instances[j]==true and 
       * previous_edi.m_instances[j]==true).
       *
       * Let new_edi be copied previous_edi (the new_edi will be inserted into
       * m_events_instances[i]). If there is any less_edi and index j as 
       * described in previous paragraph then new_edi.m_instances[j]==true.
       */
      EDInstancesPtrSet::const_iterator previous_edi;
      for(previous_edi=previous_events[i].begin();previous_edi!=previous_events[i].end();previous_edi++)
      {
        EDInstancesPtr new_edi(new EventDependentInstances(**previous_edi));
        const BoolVector& previous_edi_instances = (*previous_edi)->get_instances();
        for(size_t j=0;j<instances_count;j++)
        {
          EDInstancesPtrSet::const_iterator less_edi;
          for(less_edi=events_less[j].begin();less_edi!=events_less[j].end();less_edi++)
          {
            const BoolVector& less_edi_instances = (*less_edi)->get_instances();
            size_t y=0;
            for(;y<instances_count;y++)
            {
              if(previous_edi_instances[y]&&less_edi_instances[y])
              {
                break;
              }
            }
            if(y!=instances_count)
            {
              break;
            }
          }
          if(less_edi!=events_less[j].end())
          {
            new_edi->set_dependent(j);
            break;
          }
        }
        m_events_instances[i].insert(new_edi);
      }
    }
  }
}

HMscNode* Footprint::get_node()
{
  return dynamic_cast<HMscNode*>(m_path.back());
}

const MscElementPList& Footprint::get_path() const
{
  return m_path;
}

FootprintPtr Footprint::get_previous()
{
    return m_previous;
}

// $Id: footprint.cpp 188 2009-02-03 09:46:50Z babicaj $
