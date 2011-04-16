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
 * Copyright (c) 2009 Zuzana Pekarcikova <207719@mail.muni.cz>
 *
 * $Id: length_optimizer.h 1017 2010-12-30 17:30:29Z xpekarc $
 */

#include "data/msc.h"
#include <set>
#include <list>
#include "data/configurator.h"
#include "data/dfs_instance_events_traverser.h"

class LengthOptimizer : public ConfigReader
{
public:
	long m_arrange;
	long m_use_const_length;
	float m_const_length_value;
	long m_use_first_length;
	long m_use_parts_length;



  LengthOptimizer(ConfigProvider* config_provider);
  int process(BMscPtr bmsc);
};

/*class SortByX
{
public:
  bool operator() (InstancePtr i, InstancePtr j)
  {
    return (i->get_line_begin().get_x() < j->get_line_begin().get_x());
  }
};*/

/*class GetLengthInstances : public WhiteEventFoundListener
{
private:
  Coordinate m_max_length_y;
public:
  void on_white_event_found(Event *e)
  {
		if(m_max_length_y < e->get_position().get_y())
      m_max_length_y = e->get_position().get_y();
  }

  GetLengthInstances():
    m_max_length_y(0)
  {}

  Coordinate get_max_length_y(void)
  {
    return m_max_length_y;
  }
};*/

// $Id: length_optimizer.h 1017 2010-12-30 17:30:29Z xpekarc $

