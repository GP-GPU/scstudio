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
 * $Id: instance_sequencer.h 1019 2011-01-04 07:15:39Z xpekarc $
 */

#include "data/msc.h"
#include "data/dfs_instance_events_traverser.h"
#include "check/pseudocode/communication_graph.h"
#include "data/configurator.h"


class InstanceSequencer : public ConfigReader
{
private:
  std::vector<int> m_array;
  std::vector<InstancePtr> m_index_order;
  CommunicationGraph m_graph;
  std::set<int> m_unordered, m_ordered;
  int add(int index, unsigned crossing, unsigned going_back);
  double count_value(unsigned crossing, unsigned going_back); 
	double finding_max_width_on_instance(InstancePtr inst);
	void load_registry_import(); 
	void load_registry_beautify();
	//ConfigProvider* m_config_provider;
	//unsigned m_crossing_penalization;
  //unsigned m_going_back_penalization;

public:
	InstanceSequencer(ConfigProvider* config_provider);

	long m_setting_space;
	long m_original_order;
	long m_crossing_order;
	long m_going_back_order;
	
	long m_setting_head_width;
	long m_use_const_head;
	float m_const_head_value;
	long m_setting_coregion_width;
	long m_use_const_coregion;
	float m_const_coregion_value;

	float m_space;

  int sequence(BMscPtr bmsc, unsigned crossing = 0, unsigned going_back = 0);
  int process(BMscPtr bmsc);
	int after_import_process(BMscPtr bmsc);


};

class SortByX
{
public:
  bool operator() (InstancePtr i, InstancePtr j)
  {
    return (i->get_line_begin().get_x() < j->get_line_begin().get_x());
  }
};



// $Id: instance_sequencer.h 1019 2011-01-04 07:15:39Z xpekarc $
