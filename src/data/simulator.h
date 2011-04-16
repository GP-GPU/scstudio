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
 * $Id: simulator.h 632 2010-02-28 16:38:28Z gotthardp $
 */

#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include <boost/shared_ptr.hpp>

#include "data/msc.h"
#include "data/configurator.h"
#include "data/reporter.h"

#if defined(_MSC_VER)
// FIXME: to be removed once the Simulator has some implementation in a .cpp file
#pragma warning(disable: 4275)
#endif

struct MeasurementMap
{
  typedef std::map<std::string, std::vector<double> > DensityMap;

  DensityMap density;
  double bin_width;
};

class SimulationListener;
typedef boost::shared_ptr<SimulationListener> SimulationListenerPtr;

class Simulator : public ConfigReader, public Reporter
{
public:
  virtual ~Simulator() {}

  void set_process_count(int i)
  { m_process_count = i; }

  void set_listener(SimulationListener* listener)
  { m_listener = listener; }

  //! Start simulation and measurement calculation.
  virtual void start(const MscPtr& msc) = 0;
  //! Continue simulation.
  virtual void next() = 0;
  //! Stop simulation.
  virtual void stop() = 0;

  virtual bool is_running() const = 0;

protected:
  int m_process_count;
  SimulationListener* m_listener;
};

typedef boost::shared_ptr<Simulator> SimulatorPtr; 

class SimulationListener
{
public:
  virtual ~SimulationListener() {}

  virtual void on_result(const MeasurementMap& measurements) = 0;
  virtual void on_error(const std::string& what) = 0;
};

//! module initialization function
typedef Simulator* (*FInitSimulator)();

#endif /* _SIMULATOR_H */

// $Id: simulator.h 632 2010-02-28 16:38:28Z gotthardp $
