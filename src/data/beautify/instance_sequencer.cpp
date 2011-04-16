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
 * $Id: instance_sequencer.cpp 1023 2011-01-08 11:44:57Z xpekarc $
 */

#include "instance_sequencer.h"

InstanceSequencer::InstanceSequencer(ConfigProvider *config_provider) :
  ConfigReader(config_provider)
{
}

void InstanceSequencer::load_registry_beautify()
{
	// distance between two instances [mm]
	m_space = (float)get_config_float(L"Beautify",L"SpaceValue", 30.0);
	// space instances and set the order of them 
	m_setting_space = (long)get_config_long(L"Beautify", L"SettingSpace", 0);
	// the order of instances does not change
	m_original_order = (long)get_config_long(L"Beautify", L"OriginalOrder", 0);
	// the order of instances changes, crossing messages and instances are eliminated 
	m_crossing_order = (long)get_config_long(L"Beautify", L"CrossingOrder", 0);
	// the order of instances changes, going back messages are eliminated 
	m_going_back_order = (long)get_config_long(L"Beautify", L"GoingBackOrder", 0);

	// set the width of heads
	m_setting_head_width = (long)get_config_long(L"Beautify", L"SettingHead", 1);
	// set width of head/foot to the constant value (m_const_head_value)
	m_use_const_head = (long)get_config_long(L"Beautify",L"ConstHead", 1);
	// constant of width of head/foot [mm]
	m_const_head_value = (float)get_config_float(L"Beautify",L"HeadValue", 10.0);

	//set width of coregions
	m_setting_coregion_width = (long)get_config_long(L"Beautify", L"SettingCoregion", 1);
	// set width of coregions to the constant value (m_const_coregion_value)
	m_use_const_coregion = (long)get_config_long(L"Beautify",L"ConstCoregion", 1);
	// constant of width of coregions [mm]
	m_const_coregion_value = (float)get_config_float(L"Beautify", L"CoregionValue", 7);
}
void InstanceSequencer::load_registry_import() 
{
	// distance between two instances [mm]
	m_space = (float)get_config_float(L"Beautify",L"SpaceValueImport", 30.0);
	// constant of width of head/foot [mm]
	m_const_head_value = (float)get_config_float(L"Beautify",L"HeadValueImport", 10.0);
	// constant of width of coregions [mm]
	m_const_coregion_value = (float)get_config_float(L"Beautify", L"CoregionValueImport", 7);
}

int InstanceSequencer::sequence(BMscPtr bmsc, unsigned crossing, unsigned going_back)
{
  // crossing and message going back is not mistake
  // note: msc do not change
  //if(crossing == 0 && going_back == 0)
  //  return 1; //success

  

  // for each instance set an attribute from 0 to number of instances - 1 and
  //make set of indexes of instances, which has not been ordered yet
  //at first there are indexes of all instances
  unsigned id = 0;
  for(InstancePtrList::const_iterator it = bmsc->get_instances().begin(); it != bmsc->get_instances().end(); it++)
  {
    (*it)->set_attribute("be_id",id);
    m_index_order.push_back(*it);
    m_unordered.insert(id);
    id++;
  }

  m_graph.create_from_bmsc_with_param(bmsc, "be_id");


  //find two indexes, which instances have the most arrows between them
  unsigned int i = 0;
  unsigned max_val = 0, max_i = 0, max_j = 0;
  if(m_index_order.size() != 1)
  {
    for (i = 0; i<(m_index_order.size()-1); i++)
    {
      for (unsigned j = i+1; j<m_index_order.size(); j++)
      {
        if(m_graph.get_graph()[i][j] + m_graph.get_graph()[j][i] >= max_val)
        {
          max_i = i;
          max_j = j;
          max_val = m_graph.get_graph()[i][j] + m_graph.get_graph()[j][i];
        }
      }
    }
	}
  else
  {
      max_i = i;
  }

  m_array.push_back(max_i);
  m_unordered.erase(max_i);
  m_ordered.insert(max_i);

  if(m_index_order.size() != 1) 
  {
    add(max_j, crossing, going_back);
    m_unordered.erase(max_j);
    m_ordered.insert(max_j);
  }

  //until there are some unordered instances
  while(!m_unordered.empty())
  {
    max_val = 0;

    //find index of instance, which has the most arrows with ordered instances
    for(std::set<int>::iterator it_unord = m_unordered.begin(); it_unord != m_unordered.end(); it_unord++)
    {
      for(std::set<int>::iterator it_ord = m_ordered.begin(); it_ord != m_ordered.end(); it_ord++)
      { 
        if(max_val <= m_graph.get_graph()[*it_unord][*it_ord] + m_graph.get_graph()[*it_ord][*it_unord])
        {
          max_val = m_graph.get_graph()[*it_unord][*it_ord] + m_graph.get_graph()[*it_ord][*it_unord];
          max_i = *it_unord;
        }
      }
    }

    add(max_i, crossing, going_back);
    m_unordered.erase(max_i);
    m_ordered.insert(max_i);
  }

	if(crossing && going_back)
	{
		int forward=0;
		int back=0;

		for(std::vector<int>::const_iterator index1=m_array.begin(); index1!=m_array.end(); index1++)
		{
			std::vector<int>::const_iterator index2;
			for(index2=m_array.begin(); index2!=index1; index2++)
			{
				back += m_graph.get_graph()[*index1][*index2];
			}
			index2++;
			for(;index2!=m_array.end();index2++)
			{
				forward += m_graph.get_graph()[*index1][*index2];
			}
		}
		if(back > forward)
		{
			std::vector<int> oposite_array;
			for(std::vector<int>::reverse_iterator index=m_array.rbegin(); index!=m_array.rend(); index++)
				oposite_array.push_back(*index);
			m_array.clear();
			m_array = oposite_array;

		}	
	}

  //remove attributes of instances
  for(InstancePtrList::const_iterator it = bmsc->get_instances().begin(); it != bmsc->get_instances().end(); it++)
  {
    (*it)->remove_attribute<unsigned>("be_id");
  }
  return 0;
}

int InstanceSequencer::add(int index, unsigned crossing, unsigned going_back)
{
  double min_value = 10e10;
  unsigned min_i = 0;

  m_array.push_back(index);

  //for all possible position of index in m_array count the value of this order 
  //and if it is lower the minimal value, remember the position
  for(int i = m_array.size()-1; i>0; i--)
  {
    if (min_value > count_value(crossing, going_back))
    {
      min_value = count_value(crossing, going_back);
      min_i = i;
    }
    std::swap(m_array.at(i),m_array.at(i-1));
  }

  if(min_value > count_value(crossing, going_back))
  {
    min_value = count_value(crossing, going_back);
    min_i = 0;
  }

  //put index on remembered position 
  for(unsigned i = 0; i < min_i; i++)
    std::swap(m_array.at(i), m_array.at(i+1));

  return 0;
}

double InstanceSequencer::count_value(unsigned x, unsigned y){
  double value = 0;
  for(unsigned i = 0; i < m_array.size(); i++)
  {
    for(unsigned j = i+1; j < m_array.size(); j++)
    {
      if(j > i + 1)
        value += (m_graph.get_graph()[m_array[i]][m_array[j]] + m_graph.get_graph()[m_array[j]][m_array[i]])
        * (int)(j-i-1) * (double)(x);
      value += m_graph.get_graph()[m_array[j]][m_array[i]] * y;
    }
  }
  return value;
}

double InstanceSequencer::finding_max_width_on_instance(InstancePtr inst)
{
	double max_width = inst->get_width();
	EventAreaPtr area;
	area = inst->get_first();
	while(area)
	{
		CoregionArea* coregion;
		coregion = dynamic_cast<CoregionArea*>(area.get());
		if(coregion)
			max_width = (coregion->get_width() > max_width) ? coregion->get_width() : max_width;	
		area = area->get_next();
	}

	return max_width;
}

int InstanceSequencer::after_import_process(BMscPtr bmsc)
{
	m_array.clear();
	m_unordered.clear();
	m_ordered.clear();
	m_index_order.clear();

	// load parameters from registry
	load_registry_import();

	// set the width of all coregions and instances
	for(InstancePtrList::const_iterator it=bmsc->get_instances().begin(); it!= bmsc->get_instances().end(); it++)
	{
		(*it)->set_width((Size)m_const_head_value);

		EventAreaPtr area;
		area = (*it)->get_first();
		while(area)
		{
			CoregionArea* coregion;
			coregion = dynamic_cast<CoregionArea*>(area.get());
			if(coregion)
				coregion->set_width((Size)m_const_coregion_value);
			area = area->get_next();
		}
	}

	sequence(bmsc, 1, 1);
	// resize the space if overlapping the instances heads
	if(m_space < m_const_coregion_value)
		m_const_coregion_value = m_space;
	if(m_space < m_const_head_value)
		m_space = m_const_coregion_value;

	// point of start sequention
	double start_x = 10;
	double start_y = 10;
	for(std::vector<int>::const_iterator it_vect = m_array.begin(); it_vect != m_array.end(); it_vect++)
	{
		m_index_order[*it_vect]->set_line_begin(MscPoint(start_x, start_y));
		m_index_order[*it_vect]->set_line_end(MscPoint(start_x, start_y+1));
		start_x+=m_space;
	}

	return 0;
}

int InstanceSequencer::process(BMscPtr bmsc)
{
	m_array.clear();
  m_unordered.clear();
  m_ordered.clear();
  m_index_order.clear();

	//load parameters from registry
	load_registry_beautify();

	// sort them by X (do not change order) 
	InstancePtrList instances;
	instances = bmsc->get_instances();
	SortByX compare;
	instances.sort(compare);

	// set the start of sequence
	double start_x = (*instances.begin())->get_line_begin().get_x(); 
	double start_y = (*instances.begin())->get_line_begin().get_y();

	if(start_x < 10)
		start_x = 10;
	if(start_y < 10)
		start_y = 10;

	
  
	bool no_elements = true;			// predicate if BMSC contains any coregion or event
	// find the width of the first coregion in BMSC

		bool coregion_found = false;
		InstancePtrList::const_iterator it = instances.begin();
		while(!coregion_found && it!=instances.end())
		{
			EventAreaPtr area;
			area = (*it)->get_first();
			while(area)
			{
				CoregionArea* coregion;
				coregion = dynamic_cast<CoregionArea*>(area.get());
				if(coregion)
				{
					coregion_found = true;
					if(m_setting_coregion_width && !m_use_const_coregion)
						m_const_coregion_value = (float)coregion->get_width();
				}	
				area = area->get_next();
			}
			if(no_elements && (coregion_found || !(*it)->is_empty()))
			{
				no_elements = false;
				if(!m_setting_coregion_width || m_use_const_coregion)
					break;
			}
			it++;
		}


	// find the width of the first instance in BMSC
	if(m_setting_head_width && !m_use_const_head)
		m_const_head_value = (float)(*instances.begin())->get_width();

	// setting the width of heads/foots and coregions
	if(m_setting_head_width || m_setting_coregion_width)
		for(InstancePtrList::const_iterator it=instances.begin(); it!=instances.end(); it++)
		{
			if(m_setting_head_width)
					(*it)->set_width((Size)m_const_head_value);
			if(m_setting_coregion_width)
			{
				EventAreaPtr area;
				area = (*it)->get_first();
				while(area)
				{
					CoregionArea* coregion;
					coregion = dynamic_cast<CoregionArea*>(area.get());
					if(coregion)
						coregion->set_width((Size)m_const_coregion_value);
					area = area->get_next();
				}
			}
	}

	double shift = 0;

	// does not setting space, only if heads/foots or coregions of neighbouring instances are overlaying
	if(!m_setting_space)
	{
		for(InstancePtrList::const_iterator it = instances.begin(); it != instances.end(); it++)
		{
			if(it!=instances.begin())
			{
				InstancePtrList::const_iterator it_previous = it;
				it_previous--;
				double m_space = (*it)->get_line_begin().get_x() - (*it_previous)->get_line_begin().get_x();
				double width_of_it_and_it_previous = finding_max_width_on_instance(it->get())/2 + finding_max_width_on_instance(it_previous->get())/2;
				shift = (width_of_it_and_it_previous > m_space) ? 
					shift+width_of_it_and_it_previous-m_space : shift;
			}

			double instance_length = (*it)->get_height();
			(*it)->set_line_begin(MscPoint((*it)->get_line_begin().get_x()+shift, start_y));
			(*it)->set_line_end(MscPoint((*it)->get_line_begin().get_x(), (*it)->get_line_begin().get_y()+instance_length));
		}
	}
	else // set the space
	{
		// check if the value of m_space is big enoutht to no heads/foods no coregions overlay each other 
		for(InstancePtrList::const_iterator it = instances.begin(); it != instances.end(); it++)
		{
			if(it!=instances.begin())
			{
				InstancePtrList::const_iterator it_previous = it;
				it_previous--;
				double width_of_it_and_it_previous = finding_max_width_on_instance(it->get())/2 + finding_max_width_on_instance(it_previous->get())/2;
				shift = (width_of_it_and_it_previous > shift) ? width_of_it_and_it_previous : shift;
			}
		}
		if(shift > m_space)
			m_space = (float)shift;

		if(m_original_order || no_elements)
		{
			for(InstancePtrList::const_iterator it = instances.begin(); it != instances.end(); it++)
			{
				double instance_length = (*it)->get_height();
				(*it)->set_line_begin(MscPoint(start_x, start_y));
				(*it)->set_line_end(MscPoint(start_x, (*it)->get_line_begin().get_y()+instance_length));
				start_x+=m_space;
			}
		}
		else if(!m_original_order) //change the sequence
		{
			if(!m_crossing_order && !m_going_back_order)
				sequence(bmsc, !m_crossing_order, !m_going_back_order);
			else
				sequence(bmsc, m_crossing_order, m_going_back_order);
			for(std::vector<int>::const_iterator it_vect = m_array.begin(); it_vect != m_array.end(); it_vect++)
			{
				double instance_length = m_index_order[*it_vect]->get_height();
				m_index_order[*it_vect]->set_line_begin(MscPoint(start_x, start_y));
				m_index_order[*it_vect]->set_line_end(MscPoint(start_x, m_index_order[*it_vect]->get_line_begin().get_y()+instance_length));
				start_x+=m_space;
			}
		}
	}

  return 0;
}

// $Id: instance_sequencer.cpp 1023 2011-01-08 11:44:57Z xpekarc $
