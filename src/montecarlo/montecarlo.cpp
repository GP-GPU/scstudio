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
 * Copyright (c) 2010 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: montecarlo.cpp 644 2010-03-01 23:22:38Z gotthardp $
 */

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "montecarlo.h"

double MonteCarlo::BMscDealer::traverse(double time_begin, const BMsc* bmsc)
{
  double time_end = time_begin;

  // step 1: create timestamps of all events
  for(InstancePtrList::const_iterator instance = bmsc->get_instances().begin();
    instance != bmsc->get_instances().end(); instance++)
  {
    // get instance duration
    double itime = traverse_area(time_begin, (*instance)->get_first().get());
    // calculate maximal duration of all instances
    time_end = std::max(time_end, itime);
  }

  MeasurementList measurements;

  // step 2: walk through the timestamps and verify constraints
  for(BMscDealer::EventTimeMap::const_iterator tpos = m_timestamps.begin();
    tpos != m_timestamps.end(); tpos++)
  {
    const TimeRelationEventPtrList& relations = tpos->first->get_time_relations();
    for(TimeRelationEventPtrList::const_iterator rpos = relations.begin();
      rpos != relations.end(); rpos++)
    {
      if((*rpos)->get_event_a() != tpos->first)
        continue;

      BMscDealer::EventTimeMap::const_iterator tpos2 = m_timestamps.find((*rpos)->get_event_b());
      assert(tpos2 != m_timestamps.end());

      double delay = fabs(tpos2->second - tpos->second);
      // directed constraint applies only to events
      // when they dynamically occur in the direction indicated
      if(!(*rpos)->is_directed() ||
        /* (*rpos)->is_directed() && */ tpos->second < tpos2->second)
      {
        // if the constraint is violated, reject this trace
        if(!(*rpos)->get_interval_set().includes(delay))
          throw Reject();
      }

      if(!(*rpos)->get_measurement().empty())
        measurements.push_back(std::make_pair((*rpos)->get_measurement(), delay));
    }
  }
  // trace was accepted

  // add measurements to histogram
  for(MeasurementList::const_iterator mpos = measurements.begin();
    mpos != measurements.end(); mpos++)
  {
    m_state.add_measurement(mpos->first, mpos->second);
  }

  return time_end;
}

double MonteCarlo::BMscDealer::traverse_area(double time_begin, EventArea* area)
{
  if(!area)
    return time_begin;

  StrictOrderArea* strict = dynamic_cast<StrictOrderArea*>(area);
  if(strict)
  {
    if(strict->is_empty())
      // get area duration
      return traverse_area(time_begin, strict->get_next().get());
    else
      // get time of the last event
      return traverse_event(time_begin, strict->get_first().get());
  }

  CoregionArea* coregion = dynamic_cast<CoregionArea*>(area);
  if(coregion)
  {
    if(coregion->is_empty())
      // get area duration
      return traverse_area(time_begin, coregion->get_next().get());
    else
    {
      double time_end = time_begin;

      for(CoregionEventPVector::const_iterator min = coregion->get_minimal_events().begin();
        min != coregion->get_minimal_events().end(); min++)
      {
        // get time of the last event
        double etime = traverse_event(time_begin, *min);
        // find the latest event in this coregion
        time_end = std::max(time_end, etime);
      }

      return time_end;
    }
  }

  // this point should never be reached
  return time_begin;
}

double MonteCarlo::BMscDealer::traverse_event(double time_begin, Event* event)
{
  double event_time = time_begin;

  EventTimeMap::iterator itime = m_timestamps.find(event);
  if(itime != m_timestamps.end())
  {
    if(m_completed_events.find(event) == m_completed_events.end())
      throw std::runtime_error("Cycle detected");

    event_time = std::max(time_begin, itime->second);
  }

  // store time of this event
  m_timestamps[event] = event_time;

  double time_end = event_time;

  StrictEvent* strict = dynamic_cast<StrictEvent*>(event);
  if(strict)
    // find the latest event
    time_end = traverse_strict_event(event_time, strict);

  CoregionEvent* coregion = dynamic_cast<CoregionEvent*>(event);
  if(coregion)
    // find the latest event in this coregion
    time_end = traverse_coregion_event(event_time, coregion);

  m_completed_events.insert(event);
  return time_end;
}

double MonteCarlo::BMscDealer::traverse_strict_event(double time_begin, StrictEvent* event)
{
  // store matching event time in m_timestamps
  traverse_matching_event(time_begin, event);

  if(event->get_successor().get())
    // get time of the last event
    return traverse_event(time_begin, event->get_successor().get());
  else
    // get area duration
    return traverse_area(time_begin, event->get_area()->get_next().get());
}

double MonteCarlo::BMscDealer::traverse_coregion_event(double time_begin, CoregionEvent* event)
{
  // store matching event time in m_timestamps
  traverse_matching_event(time_begin, event);

  if(event->get_successors().size()!=0)
  {
    double time_end = time_begin;

    const CoregEventRelPtrVector& successors = event->get_successors();
    CoregEventRelPtrVector::const_iterator successor;
    for(successor=successors.begin(); successor!=successors.end(); successor++)
    {
      // get time of the last event
      double etime = traverse_event(time_begin, (*successor)->get_successor());
      // find the latest event in this coregion
      time_end = std::max(time_end, etime);
    }

    return time_end;
  }
  else
    // get area duration
    return traverse_area(time_begin, event->get_area()->get_next().get());
}

void MonteCarlo::BMscDealer::traverse_matching_event(double time_begin, Event* event)
{
  if(event->is_send() && event->get_matching_event())
  {
    // generate delay for this message transmission
    double delay = m_delay_rng();

    traverse_event(time_begin+delay, event->get_matching_event());
    // event time is stored in m_timestamps
  }
}

double MonteCarlo::HMscDealer::traverse(double time_begin, const HMsc* hmsc)
{
  // step 1: create timestamps of all reference nodes
  double time_end = traverse_node(time_begin, hmsc->get_start().get());

  MeasurementList measurements;

  // step 2: walk through the timestamps and verify constraints
  for(HMscDealer::ReferenceNodeTimeMap::const_iterator tpos = m_timestamps.begin();
    tpos != m_timestamps.end(); tpos++)
  {
    check_relations(measurements, tpos, tpos->first->get_time_relations_top());
    check_relations(measurements, tpos, tpos->first->get_time_relations_bottom());
  }
  // trace was accepted

  // add measurements to histogram
  for(MeasurementList::const_iterator mpos = measurements.begin();
    mpos != measurements.end(); mpos++)
  {
    m_state.add_measurement(mpos->first, mpos->second);
  }

  return time_end;
}

void MonteCarlo::HMscDealer::check_relations(MeasurementList& measurements,
  const HMscDealer::ReferenceNodeTimeMap::const_iterator tpos,
  const TimeRelationRefNodePtrSet& relations) const
{
  for(TimeRelationRefNodePtrSet::const_iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    if((*rpos)->get_ref_node_a() != tpos->first)
      continue;

    double time_a;
    if((*rpos)->is_top_node_a())
      time_a = tpos->second.first;
    else
      time_a = tpos->second.second;

    std::pair<HMscDealer::ReferenceNodeTimeMap::const_iterator,
      HMscDealer::ReferenceNodeTimeMap::const_iterator> all_peers = m_timestamps.equal_range((*rpos)->get_ref_node_b());
    // walk though all occurences of "node B"
    // as HMSC may contain cycles, the node might have been traversed multiple times
    for(HMscDealer::ReferenceNodeTimeMap::const_iterator tpos2 = all_peers.first;
      tpos2 != all_peers.second; tpos2++)
    {
      double time_b;
      if((*rpos)->is_bottom_node_b())
        time_b = tpos2->second.second;
      else
        time_b = tpos2->second.first;

      double delay = fabs(time_b - time_a);
      // directed constraint applies only to events
      // when they dynamically occur in the direction indicated
      if(!(*rpos)->is_directed() ||
        /* (*rpos)->is_directed() && */ time_a < time_b)
      {
        // if the constraint is violated, reject this trace
        if(!(*rpos)->get_interval_set().includes(delay))
          throw Reject();
      }

      if(!(*rpos)->get_measurement().empty())
        measurements.push_back(std::make_pair((*rpos)->get_measurement(), delay));
    }
  }
}

double MonteCarlo::HMscDealer::traverse_node(double time_begin, HMscNode* node)
{
  double next_time = time_begin;

  ReferenceNode* reference = dynamic_cast<ReferenceNode*>(node);
  if(reference)
    next_time = traverse_reference(time_begin, reference);

  ConditionNode* condition = dynamic_cast<ConditionNode*>(node);
  if(condition)
    next_time = traverse_condition(time_begin, condition);

  PredecessorNode* predecessor = dynamic_cast<PredecessorNode*>(node);
  if(predecessor)
  {
    int ways = predecessor->get_successors().size();
    if(ways > 0)
    {
      // randomly select the path to follow
      int way = (int)floor(m_rng()*ways);
      NodeRelationPtr relation = predecessor->get_successors().at(way);
      // traverse the successor node
      HMscNode* next_node = dynamic_cast<HMscNode*>(relation->get_successor());
      return traverse_node(next_time, next_node);
    }
    else
      // disconnected node reached
      throw Reject();
  }
  else
    // end node reached
    return next_time;
}

double MonteCarlo::HMscDealer::traverse_reference(double time_begin, ReferenceNode* node)
{
  double time_end = time_begin;

  const BMsc* bmsc = node->get_bmsc().get();
  if(bmsc)
  {
    BMscDealer dealer(m_state, m_seeder, m_max_message_delay);
    time_end = dealer.traverse(time_begin, bmsc);
  }

  const HMsc* hmsc = node->get_hmsc().get();
  if(hmsc)
  {
    HMscDealer dealer(m_state, m_seeder, m_max_message_delay);
    time_end = dealer.traverse(time_begin, hmsc);
  }

  // store time of this reference node
  // as HMSC may contain cycles, the node may be traversed multiple times
  m_timestamps.insert(ReferenceNodeTimeMap::value_type(node,
    std::pair<double,double>(time_begin,time_end)));

  return time_end;
}

double MonteCarlo::HMscDealer::traverse_condition(double time_begin, ConditionNode* node)
{
  switch(node->get_type())
  {
  case ConditionNode::SETTING:
    // setting condition will reset all previous states
    /* Z.120: If the guard is a set of condition names, the last setting condition
     * on that set of instances must have a non-empty intersection with the guard. */
    m_state.states.clear();
    // set new states
    for(std::vector<std::string>::const_iterator npos = node->get_names().begin();
      npos != node->get_names().end(); npos++)
    {
      m_state.states.insert(*npos);
    }
    break;

  case ConditionNode::GUARDING:
    for(std::vector<std::string>::const_iterator npos = node->get_names().begin();
      npos != node->get_names().end(); npos++)
    {
      // pass if at least one state is set
      if(m_state.states.find(*npos) != m_state.states.end())
        return time_begin;
    }
    throw Reject();

  case ConditionNode::RANDOM:
    // reject if the probability condition is not satisfied
    if(m_rng() > node->get_probability())
      throw Reject();
    break;

  case ConditionNode::OTHERWISE:
  default:
    break;
  }

  return time_begin;
}

MonteCarlo::MonteCarlo()
{
  m_stoprequested = true;
  m_callback_active = false;
  m_process_count = 1;

  m_bin_width = 1.0f;
  m_max_message_delay = 10.0f;
}

int MonteCarlo::do_sample(SimulationState& state, boost::lagged_fibonacci607& seeder, const Msc* msc)
{
  try
  {
    const BMsc* bmsc = dynamic_cast<const BMsc*>(msc);
    if(bmsc)
    {
      BMscDealer dealer(state, seeder, m_max_message_delay);
      dealer.traverse(0, bmsc);
      return 1; // accepted
    }

    const HMsc* hmsc = dynamic_cast<const HMsc*>(msc);
    if(hmsc)
    {
      HMscDealer dealer(state, seeder, m_max_message_delay);
      dealer.traverse(0, hmsc);
      return 1; // accepted
    }
  }
  catch(Reject&)
  { }

  return 0; // rejected
}

void MonteCarlo::start(const MscPtr& msc)
{
  // store the data structure being analyzed
  m_msc = msc;

  m_histograms.clear();
  m_bin_width = get_config_float(L"Simulator", L"BinWidth", 1.0f);
  m_max_message_delay = get_config_float(L"Simulator", L"MaxMessageDelay", 10.0f);

  boost::variate_generator<boost::lagged_fibonacci607, boost::uniform_int<> > seed(
    boost::lagged_fibonacci607((boost::int32_t)time(NULL)),
    boost::uniform_int<>(0, INT_MAX));

  m_stoprequested = false;
  m_callback_active = false;
  // execute threads
  for(int i = 0; i < m_process_count; i++)
  {
    ThreadPtr thread(new boost::thread(boost::bind(&MonteCarlo::thread_main, this, seed(), m_msc.get())));
#ifdef WIN32
    HANDLE th = thread->native_handle();
    // be nice and keep visio alive
    SetThreadPriority(th, THREAD_PRIORITY_BELOW_NORMAL);
#endif
    m_threads.push_back(thread);
  }
}

void MonteCarlo::next()
{
  // critical section protected by m_mutex
  {
    boost::mutex::scoped_lock lock(m_mutex);
    m_callback_active = false;
  }
  // wake up the threads
  m_callback_condition.notify_all();
}

void MonteCarlo::stop()
{
  // critical section protected by m_mutex
  {
    boost::mutex::scoped_lock lock(m_mutex);
    m_stoprequested = true;
    m_callback_active = false;
  }
  // wake up the threads
  m_callback_condition.notify_all();

  // wait for the executed threads
  for(std::vector<ThreadPtr>::const_iterator tpos = m_threads.begin();
    tpos != m_threads.end(); tpos++)
  {
    (*tpos)->join();
  }

  m_threads.clear();
  // release analyzed data structure
  m_msc = NULL;
}

void MonteCarlo::thread_main(unsigned int seed, const Msc* msc)
{
  // pseudo-random number generators should not be initialized frequently
  // to assure reentrance, we have one generator per thread
  boost::lagged_fibonacci607 seeder(seed);

  while(!m_stoprequested)
  {
    try
    {
      SimulationState state(m_bin_width);
    
      boost::posix_time::ptime const stop_time = boost::posix_time::microsec_clock::local_time()
        + boost::posix_time::seconds(1);
      // calculate for a given time period
      while(!m_stoprequested && stop_time > boost::posix_time::microsec_clock::local_time())
      {
        do_sample(state, seeder, msc);
      }

      boost::mutex::scoped_lock lock(m_mutex);

      MeasurementMap measurements;
      measurements.bin_width = m_bin_width;

      for(HistogramMap::const_iterator hpos = state.histograms.begin();
        hpos != state.histograms.end(); hpos++)
      {
        m_histograms[hpos->first] += hpos->second;

        for(std::vector<size_t>::const_iterator bpos = m_histograms[hpos->first].acc.begin();
          bpos != m_histograms[hpos->first].acc.end(); bpos++)
        {
          measurements.density[hpos->first].push_back(double(*bpos)/m_histograms[hpos->first].N);
        }
      }

      // wait until the previous callback is processed
      while(m_callback_active)
        m_callback_condition.wait(lock);

      if(m_listener && !m_stoprequested)
      {
        m_callback_active = true;
        m_listener->on_result(measurements);

        // wait for next() or stop() command
        while(m_callback_active)
          m_callback_condition.wait(lock);
      }
    }
    catch(std::exception& exc)
    {
      boost::mutex::scoped_lock lock(m_mutex);

      if(m_listener)
      {
        m_callback_active = true;
        m_listener->on_error(exc.what());

        // wait for next() or stop() command
        while(m_callback_active)
          m_callback_condition.wait(lock);
      }
    }
    catch(...)
    {
      boost::mutex::scoped_lock lock(m_mutex);

      if(m_listener)
      {
        m_callback_active = true;
        m_listener->on_error("unexpected exception");

        // wait for next() or stop() command
        while(m_callback_active)
          m_callback_condition.wait(lock);
      }
    }
  }
}

// $Id: montecarlo.cpp 644 2010-03-01 23:22:38Z gotthardp $
