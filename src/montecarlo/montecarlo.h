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
 * $Id: montecarlo.h 644 2010-03-01 23:22:38Z gotthardp $
 */

#include <boost/random.hpp>
#include <boost/random/lagged_fibonacci.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

#include "data/msc.h"
#include "data/simulator.h"
#include "montecarlo/export.h"

class SCMONTECARLO_EXPORT MonteCarlo : public Simulator
{
public:
  struct Histogram
  {
    Histogram():
      N(0)
    { }

    void add(size_t index, double value)
    {
      if(acc.size() < index+1)
        acc.resize(index+1);
      acc[index]++;

      N++;
    }

    Histogram& operator += (const Histogram& r)
    {
      if(r.acc.size() > acc.size())
        acc.resize(r.acc.size());
      for(size_t i = 0; i < r.acc.size(); i++)
        acc[i] += r.acc[i];

      N += r.N;
      return *this;
    }

    std::vector<size_t> acc;
    size_t N;
  };

  typedef std::map<std::string, Histogram> HistogramMap;

  struct SimulationState
  {
    SimulationState(double width):
      bin_width(width)
    { }

    void add_measurement(const std::string& name, double value)
    {
      histograms[name].add((size_t)floor(value/bin_width), value);
    }

    HistogramMap histograms;
    double bin_width;

    std::set<std::string> states;
  };

  typedef std::vector<std::pair<std::string, double> > MeasurementList;

  class Reject
  { };

  class BMscDealer
  {
  public:
    BMscDealer(SimulationState& state, boost::lagged_fibonacci607& seeder, float max_message_delay) :
      m_state(state), m_seeder(seeder),
      m_delay_rng(seeder, boost::uniform_real<>(0,max_message_delay))
    { }

    double traverse(double time_begin, const BMsc* bmsc);

    typedef std::map<EventPtr, double> EventTimeMap;
    const EventTimeMap& get_timestamps() const
    { return m_timestamps; }

  private:
    SimulationState& m_state;
    boost::lagged_fibonacci607& m_seeder;
    //! random number generator used to generate message delay
    boost::variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<> > m_delay_rng;

    double traverse_area(double time_begin, EventArea* area);
    double traverse_event(double time_begin, Event* event);
    double traverse_strict_event(double time_begin, StrictEvent* event);
    double traverse_coregion_event(double time_begin, CoregionEvent* event);
    void traverse_matching_event(double time_begin, Event* event);

    EventTimeMap m_timestamps;

    typedef std::set<EventPtr> EventPtrSet;
    EventPtrSet m_completed_events;
  };

  class HMscDealer
  {
  public:
    HMscDealer(SimulationState& state, boost::lagged_fibonacci607& seeder, float max_message_delay) :
      m_state(state), m_seeder(seeder),
      m_rng(seeder, boost::uniform_real<>(0,1)),
      m_max_message_delay(max_message_delay)
    { }

    double traverse(double time_begin, const HMsc* hmsc);

    typedef std::multimap<ReferenceNodePtr, std::pair<double,double> > ReferenceNodeTimeMap;
    const ReferenceNodeTimeMap& get_timestamps() const
    { return m_timestamps; }

  private:
    SimulationState& m_state;
    boost::lagged_fibonacci607& m_seeder;
    //! random number generator used for path selection
    boost::variate_generator<boost::lagged_fibonacci607&, boost::uniform_real<> > m_rng;

    void check_relations(MeasurementList& measurements,
      const HMscDealer::ReferenceNodeTimeMap::const_iterator tpos,
      const TimeRelationRefNodePtrSet& relations) const;
    double traverse_node(double time_begin, HMscNode* node);
    double traverse_reference(double time_begin, ReferenceNode* node);
    double traverse_condition(double time_begin, ConditionNode* node);

    float m_max_message_delay;
    ReferenceNodeTimeMap m_timestamps;
  };

  MonteCarlo();

  int do_sample(SimulationState& state, boost::lagged_fibonacci607& seeder, const Msc* msc);

  //! Start simulation and measurement calculation.
  virtual void start(const MscPtr& msc);
  //! Continue simulation.
  virtual void next();
  //! Stop simulation.
  virtual void stop();
  //! Returns true if the simulation is started.
  virtual bool is_running() const
  { return !m_stoprequested; }

  void thread_main(unsigned int seed, const Msc* msc);

private:
  MscPtr m_msc;

  volatile bool m_stoprequested;
  typedef boost::shared_ptr<boost::thread> ThreadPtr;
  std::vector<ThreadPtr> m_threads;

  boost::mutex m_mutex;
  boost::condition m_callback_condition;
  bool m_callback_active;

  HistogramMap m_histograms;
  float m_bin_width;
  float m_max_message_delay;
};

// $Id: montecarlo.h 644 2010-03-01 23:22:38Z gotthardp $
