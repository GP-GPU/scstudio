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
 * $Id: montecarlo_test.cpp 646 2010-03-02 10:50:56Z gotthardp $
 */

#include <iostream>

#include "data/Z120/z120.h"
#include "montecarlo/montecarlo.h"

class TestSimulator : public SimulationListener
{
public:
  TestSimulator()
  {
    m_simulator.set_listener(this);
    m_was_result = false;
    m_was_error = false;
  }

  virtual void on_result(const MeasurementMap& measurements)
  {
    std::cout << "value";
    size_t bins = 0;
    // walk through the measurements
    for(MeasurementMap::DensityMap::const_iterator mpos = measurements.density.begin();
      mpos != measurements.density.end(); mpos++)
    {
      // print header
      std::cout << ", " << mpos->first;
      // get the maximal number of bins
      bins = std::max(bins, mpos->second.size());
    }
    std::cout << std::endl;

    // print the measurements
    for(size_t bidx = 0; bidx < bins; bidx++)
    {
      std::cout << measurements.bin_width*bidx;

      for(MeasurementMap::DensityMap::const_iterator mpos = measurements.density.begin();
        mpos != measurements.density.end(); mpos++)
      {
        std::cout << ", " << mpos->second[bidx];
      }
      std::cout << std::endl;
    }

    {
      boost::mutex::scoped_lock lock(m_result_mutex);
      m_was_result = true;
    }
    m_result_condition.notify_all();
  }

  virtual void on_error(const std::string& what)
  {
    std::cerr << "EXCEPTION: Simulation failed: " << what << std::endl;
    {
      boost::mutex::scoped_lock lock(m_result_mutex);
      m_was_error = true;
    }
    m_result_condition.notify_all();
  }

  MonteCarlo m_simulator;

  boost::mutex m_result_mutex;
  boost::condition m_result_condition;
  bool m_was_result;
  bool m_was_error;
};

class DefaultConfigProvider : public ConfigProvider
{
public:
  virtual long get_config_long(const std::wstring& section, const std::wstring& parameter, long def = 0) const
  { return def; }
  virtual float get_config_float(const std::wstring& section, const std::wstring& parameter, float def = 0.0f) const
  { return def; }
};

int main(int argc, char** argv)
{
  if(argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " <filename> <correct>" << std::endl;
    return 1;
  }

  Z120 z120;

  StreamReportPrinter printer(std::wcerr);
  z120.set_printer(&printer);

  std::vector<MscPtr> msc;

  try
  {
    msc = z120.load_msc(argv[1]);
  }
  catch(std::exception& exc)
  {
    std::cerr << "ERROR: Cannot open '" << argv[1] << "': " << exc.what() << std::endl;
    return 1;
  }

  if(msc.empty() || msc[0] == NULL)
  {
    std::cerr << "ERROR: Cannot open " << argv[1] << std::endl;
    return 1;
  }

  char *endptr;
  int correct = strtol(argv[2], &endptr, 10);
  if(*argv[2] == '\0' || *endptr != '\0')
  {
    std::cerr << "ERROR: Not a boolean value: " << argv[2] << std::endl;
    return 1;
  }

  TestSimulator test;

  DefaultConfigProvider config;
  test.m_simulator.set_config_provider(&config);

  test.m_simulator.start(msc[0]);
  // wait until simulation finishes
  boost::mutex::scoped_lock lock(test.m_result_mutex);
  while(!test.m_was_result && !test.m_was_error)
    test.m_result_condition.wait(lock);

  test.m_simulator.stop();

  if(test.m_was_result && !test.m_was_error)
    return correct;
  else
    return !correct;
}

// $Id: montecarlo_test.cpp 646 2010-03-02 10:50:56Z gotthardp $
