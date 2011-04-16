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
 * $Id: length_optimizer.cpp 1017 2010-12-30 17:30:29Z xpekarc $
 */

#include "length_optimizer.h"
#include "layout_optimizer.h"
#include "instance_sequencer.h"

LengthOptimizer::LengthOptimizer(ConfigProvider *config_provider) :
  ConfigReader(config_provider)
{
	// arrange the messages and corregions and set the length of instances
	m_arrange = (long)get_config_long(L"Beautify", L"Arrange", 1);
	// set length of instances to constant (m_const_length_value)
	m_use_const_length = (long)get_config_long(L"Beautify",L"ConstInstanceLength", 1);
	// constant of length instances [mm]
	m_const_length_value = (float)get_config_float(L"Beautify",L"InstanceValue", 20.0);
	// set length of instances to the length of the first instance in BMSC
	m_use_first_length = (long)get_config_long(L"Beautify",L"FirstInstanceLength", 0);
	// set length of instances according to the arrangement of events and coregions
	m_use_parts_length = (long)get_config_long(L"Beautify",L"PartsInstanceLength", 0);
}

int LengthOptimizer::process(BMscPtr bmsc)
{	
	// do not change the length or do not arrange the messages and coregions or 
	// setting length according to the individual distances on instances (solved in layout_optimizer)
	if(!m_arrange || m_use_parts_length || (!m_use_const_length && !m_use_parts_length && !m_use_first_length))
		return 0; //success

	if(m_use_first_length)
	{
		// find the instance with the smallest x coordinate and save its length to the variable m_const_length_value
		InstancePtrList::const_iterator it=bmsc->get_instances().begin();
		InstancePtr leftmost_instance = (*it);
		it++;
		for(; it != bmsc->get_instances().end(); it++)
			leftmost_instance = ((*it)->get_line_begin().get_x() < leftmost_instance->get_line_begin().get_x()) ? (*it) : leftmost_instance;
		m_const_length_value = (float)(leftmost_instance->get_height());
	}
	for(InstancePtrList::const_iterator it = bmsc->get_instances().begin(); it != bmsc->get_instances().end(); it++)
		(*it)->set_line_end(MscPoint((*it)->get_line_begin().get_x(), (*it)->get_line_begin().get_y()+ m_const_length_value));

  return 0;
}

// $Id: length_optimizer.cpp 1017 2010-12-30 17:30:29Z xpekarc $

