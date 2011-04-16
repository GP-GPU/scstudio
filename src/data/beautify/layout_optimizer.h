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
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>, 
 *                    Zuzana Pekarcikova <z.pekarcikova@gmail.com>
 *
 * $Id: layout_optimizer.h 1019 2011-01-04 07:15:39Z xpekarc $
 */

#include "data/msc.h"
#include "data/configurator.h"

class LayoutOptimizer : public ConfigReader
{
private:
	void load_registry_import(); 
	void load_registry_beautify();

public:
  LayoutOptimizer(ConfigProvider* config_provider);
  int process(BMscPtr bmsc);
	int after_import_process(BMscPtr bmsc);

  float m_instance_head_distance;
  float m_successor_distance;
  float m_send_receive_distance;
  float m_incomplete_message_length;
	float m_coregion_begin_event_distance;
	float m_event_coregion_end_distance;
	float m_instance_foot_distance;

};


// $Id: layout_optimizer.h 1019 2011-01-04 07:15:39Z xpekarc $
