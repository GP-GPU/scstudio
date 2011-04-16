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
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *                    Zuzana Pekarcikova <z.pekarcikova@gmail.com>
 *
 * $Id: layout_optimizer.cpp 1019 2011-01-04 07:15:39Z xpekarc $
 */

#include <lp_lib.h>

#include "data/dfs_events_traverser.h"
#include "check/pseudocode/utils.h"
#include "layout_optimizer.h"
#include "instance_sequencer.h"
#include "length_optimizer.h"
#include <math.h>

#define PI 3.14159265

static lprec* initialize_solver(int events_size)
{
  lprec *lp = make_lp(0, events_size);
  if(lp == NULL)
    throw std::bad_alloc();

  // turn off verbose mode
  set_verbose(lp, NEUTRAL);

  // set the objective to minimize
  set_minim(lp);

  REAL* row = new REAL[events_size+1];
  // formulate the problem
  // note: element 0 of the array is not considered (i.e. ignored)
  for (int i = 1; i <= events_size; i++)
    row[i] = 0;
  // set the objective function: sum of e[i]
  set_obj_fn(lp, row);
  delete[] row;

  return lp;
}

// add constraint ex < ey, i.e. ex-->ey >= d
static int add_relative_distance_constraint(lprec *lp, int ex, int ey, REAL d)
{
  int columns = get_Ncolumns(lp);
  // we add new row: turn row entry mode on
  set_add_rowmode(lp, TRUE);

  REAL* row = new REAL[columns+1];
  // initialize the vector
  // note: element 0 of the array is not considered (i.e. ignored)
  for (int i = 1; i <= columns; i++)
    row[i] = 0;
  // add the constraint: -e[x] + e[y] >= d
  row[ex] = -1;
  row[ey] = 1;
  add_constraint(lp, row, GE, d);

  delete[] row;
  return 0;
}

// append new variable, i.e. new column to the matrix
static int add_binary_variable(lprec *lp)
{
  int rows = get_Nrows(lp);
  // we add new column: turn row entry mode off
  set_add_rowmode(lp, FALSE);

  REAL* column = new REAL[rows+1];
  // initialize the vector
  // note: element 0 of the array represents the objective value
  for (int i = 0; i <= rows; i++)
    column[i] = 0;

  add_column(lp, column);
  delete[] column;

  // the new column is added to the model at the end
  // current number of columns thus represents index of the new variable
  int idx = get_Ncolumns(lp);

  set_binary(lp, idx, TRUE);
  return idx;
}

// constraint ex < ey or ex > ey, i.e. |ex<-->ey| >= d
// see http://www.nabble.com/Absolute-value-constraint-td14841621.html
static int add_absolute_distance_constraint(lprec *lp, int ex, int ey, REAL d)
{
  static const REAL max_distance = 1000; // [millimeters]
  int z = add_binary_variable(lp);

  int columns = get_Ncolumns(lp);
  // we add new rows: turn row entry mode on
  set_add_rowmode(lp, TRUE);

  REAL* row = new REAL[columns+1];
  // initialize the vector
  // note: element 0 of the array is not considered (i.e. ignored)
  for (int i = 1; i <= columns; i++)
    row[i] = 0;
  // set the constraint: e[x] - e[y] + z*(M+d)
  row[ex] = 1;
  row[ey] = -1;
  row[z] = max_distance+d;
  // add constraint: e[x] - e[y] + z*(M+d) <= M
  add_constraint(lp, row, LE, max_distance);
  // add constraint: e[x] - e[y] + z*(M+d) >= d
  add_constraint(lp, row, GE, d);

  delete[] row;
  return 0;
}

typedef std::map<EventPtr, int> EventPtrMap;

class EventIndexer: public WhiteEventFoundListener
{
public:
  virtual void on_white_event_found(Event* e)
  {
    int index = m_events.size()+1;
    m_events.insert(std::make_pair(e, index));
  }

  size_t get_event_count() const
  {
    return m_events.size();
  }

  int get_event_index(EventPtr e) const
  {
    EventPtrMap::const_iterator pos = m_events.find(e);
    if(pos == m_events.end())
      throw std::range_error("Unknown event");

    return pos->second;
  }

  const EventPtrMap& get_events() const
  {
    return m_events;
  }

private:
  EventPtrMap m_events;
};

class UnoverlayingMessages : public WhiteEventFoundListener
{
public:
	UnoverlayingMessages(lprec *lp, const EventIndexer& indexer, Event* f, double cross) :
		m_lp(lp), m_indexer(indexer), m_f(f), m_cross(cross) {}

	float m_successor_distance;

  void on_white_event_found(Event *e)
	{
		
		int e1 = m_indexer.get_event_index(m_f);
		int e2 = m_indexer.get_event_index(e);
		add_absolute_distance_constraint(m_lp, e1, e2, m_successor_distance+m_cross);
	}

private:
	EventPtr m_f;
	lprec *m_lp;
	const EventIndexer& m_indexer;
	double m_cross;
};

class EventConstraintCreator: public EventSuccessorListener, public SendReceivePairListener, public WhiteEventFoundListener
{
public:
	EventConstraintCreator(lprec *lp, const EventIndexer& indexer, int coregions_size, std::vector<InstancePtr> instances) : 
    m_size(coregions_size), m_lp(lp), m_indexer(indexer), m_instances(instances) {}

  float m_successor_distance;
  float m_send_receive_distance;
	float m_coregion_begin_event_distance;
	float m_event_coregion_end_distance;
	float m_incomplete_message_length;

  virtual void on_event_successor(Event* event, Event* successor)
  {
    int e1, e2;
    if(successor->get_attribute<NodeColor>("cc_color",WHITE) == GRAY)
    {
      // cycle detected: reverse direction
      e1 = m_indexer.get_event_index(successor);
      e2 = m_indexer.get_event_index(event);
    }
    else
    {
      e1 = m_indexer.get_event_index(event);
      e2 = m_indexer.get_event_index(successor);
    }		
    add_relative_distance_constraint(m_lp, e1, e2, m_successor_distance);
}

  virtual void on_send_receive_pair(Event* send, Event* receive)
  {
    int e1, e2;
    if(receive->get_attribute<NodeColor>("cc_color",WHITE) == GRAY)
    {
      // cycle detected: reverse direction
      e1 = m_indexer.get_event_index(receive);
      e2 = m_indexer.get_event_index(send);
    }
    else
    {
      e1 = m_indexer.get_event_index(send);
      e2 = m_indexer.get_event_index(receive);
    }

		add_relative_distance_constraint(m_lp, e1, e2, m_send_receive_distance);
    // update the objective function for matched events:
    // minimize length and y-coordinate, i.e. 1*y(receive) - y(send) + (1/all_evnets)*y(receive)
    set_obj(m_lp, e1, -1.0);
    set_obj(m_lp, e2, (1.0+1.0/(m_indexer.get_event_count()+2*m_size)));
  }

  virtual void on_white_event_found(Event* e)
  {
    int e1;
    e1 = m_indexer.get_event_index(e);
    if(!e->is_matched())
    {
      // update the objective function for unmatched events:
      set_obj(m_lp, e1, 1.0/(m_indexer.get_event_count()+2*m_size));

			
				unsigned i = e->get_instance()->get_attribute("id",i);
				i++;
				//double e_pos = e->get_instance()->get_line_begin().get_x();
				float horizontal_space = std::sqrt(std::pow(m_incomplete_message_length,2) - std::pow(m_send_receive_distance,2));
				while(i < m_instances.size())
				{
					double next_inst_space = m_instances.at(i)->get_line_begin().get_x() - e->get_instance()->get_line_begin().get_x(); 
					if(next_inst_space <= horizontal_space)
					{
						double crossing = m_send_receive_distance* (next_inst_space / horizontal_space);
						if(e->get_incomplete_message()->is_found())
							crossing = -crossing;
						UnoverlayingMessages unoverlaying(m_lp, m_indexer, e, crossing);
						unoverlaying.m_successor_distance = m_successor_distance;
						DFSInstanceEventsTraverser traverser;
						traverser.add_white_event_found_listener(&unoverlaying);
						traverser.traverse((m_instances.at(i)).get());
						horizontal_space = horizontal_space - (float)next_inst_space;
						i++;
					}
					else 
						break;
				}

			
    }
		else
		{
			// solving the unoverlaying two messages or unoverlaying message and coregion edge
			if(e->is_send())
			{
				unsigned i, j;
				i = e->get_instance()->get_attribute("id", i);
				j = e->get_matching_event()->get_instance()->get_attribute("id", j);
				if(j<i)
					std::swap(i,j);

				//distance between instance of e and instance of e->matching_event
				double distance_event_matching = m_instances.at(j)->get_line_begin().get_x() - m_instances.at(i)->get_line_begin().get_x();

				// instances between send and receive event
				for(++i; i<j && i<m_instances.size(); ++i)
				{
					// distance between instance of e and instance i
					double distance_event_i = m_instances.at(j)->get_line_begin().get_x() - m_instances.at(i)->get_line_begin().get_x();
					// crossing e->message and instance i 
					double crossing = m_send_receive_distance * distance_event_i / distance_event_matching; 

					if(e->get_matching_event()->get_attribute<NodeColor>("cc_color",WHITE) == GRAY)
						crossing = -crossing;
					UnoverlayingMessages unoverlaying(m_lp, m_indexer, e, crossing);
					unoverlaying.m_successor_distance = m_successor_distance;
					DFSInstanceEventsTraverser traverser;
					traverser.add_white_event_found_listener(&unoverlaying);



					// traverse events and set the constraints between events
					traverser.traverse((m_instances.at(i)).get());

					// traverse coregions and set the constraints between event e and coregion edges
					EventAreaPtr area;
					area = (m_instances.at(i)).get()->get_first();
					while(area)
					{
						CoregionArea* coregion = dynamic_cast<CoregionArea*>(area.get());
						if(coregion)
						{
							int begin;
							int e1 = m_indexer.get_event_index(e);
							begin = coregion->get_attribute("id",begin);
							add_absolute_distance_constraint(m_lp, e1, begin,  m_successor_distance+crossing);
							add_absolute_distance_constraint(m_lp, (begin+1), e1, m_successor_distance+crossing);
						}			     
						area = area->get_next();
					}
				}
			}
		}
  }

private:
  lprec *m_lp;
  const EventIndexer& m_indexer;
	int m_size;
	std::vector<InstancePtr> m_instances;
};

static int create_absolute_constraint(BMscPtr bmsc, lprec *m_lp, const EventIndexer& indexer, float m_successor_distance, 
																			float m_coregion_begin_event_distance, float m_event_coregion_end_distance,
																			std::list<CoregionArea*> coregions)
{
  EventAreaPtr area;
	for(std::list<CoregionArea*>::const_iterator coregion=coregions.begin(); coregion!=coregions.end(); coregion++)
	{
    CoregionEventPtrSet left_side, right_side;
    std::list<CoregionEventPtrSet> sides;
    CoregionEventPtrSet::const_iterator it_e2;
    // separate events with messages going to the right and to the left 
    for(CoregionEventPtrSet::const_iterator it_e=(*coregion)->get_events().begin(); it_e!=(*coregion)->get_events().end(); it_e++)
		{
		  if((*it_e)->is_matched())
			{
				if((*it_e)->get_matching_event()->get_instance()->get_line_begin().get_x() > 
					(*it_e)->get_instance()->get_line_begin().get_x())
					right_side.insert(*it_e);
				else
					left_side.insert(*it_e);
      }
      else
      {
        if((*it_e)->get_incomplete_message()->is_found())
          left_side.insert(*it_e);
        else
          right_side.insert(*it_e);
      }
    }
    sides.push_back(left_side);
    sides.push_back(right_side);
    // set constraints for events from left and right side separatelly
		for(std::list<CoregionEventPtrSet>::const_iterator side=sides.begin(); side!=sides.end(); side++)
		{
      for(CoregionEventPtrSet::const_iterator it_e=side->begin(); it_e!=side->end(); it_e++)
      {
				int e1, e2;
				e1 = indexer.get_event_index(*it_e);

				// add absolute constraints for all two events on the same side
        it_e2 = it_e;
        it_e2++;
        if(it_e2 != side->end())
        {
          for(;it_e2!=side->end(); it_e2++)
          {
            e2 = indexer.get_event_index(*it_e2);
            add_absolute_distance_constraint(m_lp, e1, e2, m_successor_distance);
          }
        }

				int begin=0;
				begin = (*coregion)->get_attribute("id",begin);
				// add constrain for all events to be bigger than begin of coregion and lower then end of coregion
				add_relative_distance_constraint(m_lp, begin, e1, m_coregion_begin_event_distance);
				add_relative_distance_constraint(m_lp, e1, (begin+1), m_event_coregion_end_distance);
    
        set_obj(m_lp, begin, -1.0);
        set_obj(m_lp, (begin+1), 1.0 + 1.0/(indexer.get_event_count()+2*coregions.size()));
      }
    }
		int begin=0;
		begin = (*coregion)->get_attribute("id",begin);
		add_relative_distance_constraint(m_lp, begin, (begin+1), m_coregion_begin_event_distance + m_event_coregion_end_distance);
  }

  int level;
  for(InstancePtrList::const_iterator instanc=bmsc->get_instances().begin(); instanc!=bmsc->get_instances().end(); instanc++)
	{
    level = 0;
		area = (*instanc)->get_first();
    while(area)
    {
      CoregionArea* coregion = dynamic_cast<CoregionArea*>(area.get());
      if(coregion)
      {
        int begin;
        begin = coregion->get_attribute("id",begin);
        if(level != 0)
          add_relative_distance_constraint(m_lp, level, begin,  m_successor_distance);

        level = begin+1;    	
			}	
      else
      {
        StrictOrderArea* strict = dynamic_cast<StrictOrderArea*>(area.get());
        if(!strict->is_empty())
        {
          if(level != 0)
            add_relative_distance_constraint(m_lp, level, indexer.get_event_index(strict->get_first()), m_successor_distance); 
          level = indexer.get_event_index(strict->get_last());
        }
      }
      area = area->get_next();
    }
  }

	return 0;
}

typedef std::list<CoregionArea*> CoregionAreaList;
typedef std::vector<InstancePtr> InstancePtrVector;

InstancePtrVector index_instances(InstancePtrList instances)
{
	SortByX comparator;
	instances.sort(comparator);

	InstancePtrVector instances_vector;
 	for(InstancePtrList::const_iterator it=instances.begin(); it!=instances.end(); it++)
	{
		(*it)->set_attribute("id",instances_vector.size());
		instances_vector.push_back((*it));
	}
	
	return instances_vector;
}

CoregionAreaList index_coregions(InstancePtrList instances, int start_index)
{
	CoregionAreaList coregions;
	CoregionArea* coregion;
	EventAreaPtr area;

	for(InstancePtrList::const_iterator it=instances.begin(); it!=instances.end(); it++)
	{
		area = (*it)->get_first();
		while(area)
		{
			coregion = dynamic_cast<CoregionArea*>(area.get());
			if(coregion)
			{
				coregions.push_back(coregion);
				coregion->set_attribute("id",start_index);
				start_index +=2;
			}	
			area = area->get_next();
		}
	}
	

	return coregions;
}

double set_events_coregions_positions(CoregionAreaList coregions, float instance_head_distance, double proportion, EventIndexer indexer, REAL* var,
																		float incomplete_message_length, float send_receive_distance)
{
	double max_y = 0;
	for(CoregionAreaList::const_iterator cor_it=coregions.begin(); cor_it!=coregions.end(); cor_it++)
	{
		int begin=0;
		begin = (*cor_it)->get_attribute("id", begin);
		(*cor_it)->set_begin_height((instance_head_distance+var[begin-1])*proportion);
		(*cor_it)->set_end_height((instance_head_distance+var[begin])*proportion);
		if(var[begin]*proportion+instance_head_distance > max_y) 
			max_y = var[begin]*proportion+instance_head_distance;
	}
 
	
  for(EventPtrMap::const_iterator epos = indexer.get_events().begin();
    epos != indexer.get_events().end(); epos++)
  {
		CoregionArea *coregion;
    coregion = dynamic_cast<CoregionArea*>(epos->first->get_general_area());
    if(coregion)
    {
      if(epos->first->is_matched())
      {
        // messages going to right put on the right side of coregion
        if((epos->first->get_matching_event()->get_instance()->get_line_begin().get_x() > 
          epos->first->get_instance()->get_line_begin().get_x()))
          epos->first->set_position(MscPoint(0+coregion->get_width(), 
					(instance_head_distance+var[epos->second-1])*proportion-coregion->get_begin_height()));
        else
					epos->first->set_position(MscPoint(0, (instance_head_distance+var[epos->second-1])*proportion-coregion->get_begin_height()));
      }
      else
      {
				IncompleteMessagePtr mess;
        mess = epos->first->get_incomplete_message();
        // found message put on the left side of coregion
        if(mess->is_found())
          epos->first->set_position(MscPoint(0,  (instance_head_distance+var[epos->second-1])*proportion-coregion->get_begin_height()));
        // lost message put on the right side of coregion
        else if(mess->is_lost())
          epos->first->set_position(MscPoint(0+coregion->get_width(),  
					(instance_head_distance+var[epos->second-1])*proportion-coregion->get_begin_height()));
      }
		}
    else
    {
      // note: var element 0 contains the first variable, etc.
      epos->first->set_position(MscPoint(0, (instance_head_distance+var[epos->second-1])*proportion));
    }
		if(var[epos->second-1]*proportion+instance_head_distance > max_y) 
			max_y = var[epos->second-1]*proportion+instance_head_distance;
	}

	// aligning of incomplete messages
  for(EventPtrMap::const_iterator epos = indexer.get_events().begin();
    epos != indexer.get_events().end(); epos++)
  {
    IncompleteMessagePtr in_mess = epos->first->get_incomplete_message();
    if(in_mess.get())
    {
      if(in_mess->is_lost())
      { 
        in_mess->set_dot_position(MscPoint(incomplete_message_length, send_receive_distance));
      }
      else if(in_mess->is_found())
      {
        in_mess->set_dot_position(MscPoint(-incomplete_message_length, send_receive_distance));
      }
    }
  }
	return max_y;
}

void remove_inst_cor_attributes(InstancePtrList instances)
{
	//remove attributes of instances
  for(InstancePtrList::const_iterator it = instances.begin(); it != instances.end(); it++)
  {
    (*it)->remove_attribute<unsigned>("be_id");

		EventAreaPtr area;
		area = (*it)->get_first();
		while(area)
		{
			CoregionArea* coregion;
			coregion = dynamic_cast<CoregionArea*>(area.get());
			if(coregion)
				coregion->remove_attribute<unsigned>("id");
			area = area->get_next();
		}
	}
}


void LayoutOptimizer::load_registry_beautify()
{
	// distance between the begin of instance and the first event on this instance [mm]
	m_instance_head_distance = (float)get_config_float(L"Beautify", L"HeadDistValue", 5);
	// distance between the last event on instance and the end of instance
	m_instance_foot_distance = (float)get_config_float(L"Beautify", L"FootDistValue", 5);
	// distance between two succeeding events on the same instance [mm]
	m_successor_distance = (float)get_config_float(L"Beautify", L"SuccDistValue", 5);
	// distance between send and receive events of the same message [mm]
	m_send_receive_distance = (float)get_config_float(L"Beautify", L"SlopeValue", 0);
	// distance between coregion begin and the first event attached to it [mm]
	m_coregion_begin_event_distance = (float)get_config_float(L"Beautify", L"BeginCorDistValue", 5);
	// distance between the last event attached to coregion and the end of this coregion [mm]
	m_event_coregion_end_distance = (float)get_config_float(L"Beautify", L"EndCorDistValue", 5);
  // length of lost and found messages [mm]
  m_incomplete_message_length = (float)get_config_float(L"Beautify", L"IncompleteMsgValue", 20);
}

void LayoutOptimizer::load_registry_import()
{
	// distance between the begin of instance and the first event on this instance [mm]
	m_instance_head_distance = (float)get_config_float(L"Beautify", L"HeadDistValueImport", 5);
	// distance between the last event on instance and the end of instance
	m_instance_foot_distance = (float)get_config_float(L"Beautify", L"FootDistValueImport", 5);
	// distance between two succeeding events on the same instance [mm]
	m_successor_distance = (float)get_config_float(L"Beautify", L"SuccDistValueImport", 5);
	// distance between send and receive events of the same message [mm]
	m_send_receive_distance = (float)get_config_float(L"Beautify", L"SlopeValueImport", 0);
	// distance between coregion begin and the first event attached to it [mm]
	m_coregion_begin_event_distance = (float)get_config_float(L"Beautify", L"BeginCorDistValueImport", 5);
	// distance between the last event attached to coregion and the end of this coregion [mm]
	m_event_coregion_end_distance = (float)get_config_float(L"Beautify", L"EndCorDistValueImport", 5);
  // length of lost and found messages [mm]
  m_incomplete_message_length = (float)get_config_float(L"Beautify", L"IncompleteMsgValueImport", 20);
}

LayoutOptimizer::LayoutOptimizer(ConfigProvider* config_provider):
  ConfigReader(config_provider)
	{}
	
int LayoutOptimizer::after_import_process(BMscPtr bmsc)
{
	load_registry_import();

	EventIndexer indexer;
  DFSEventsTraverser indexing_traverser;
  indexing_traverser.add_white_event_found_listener(&indexer);
  indexing_traverser.traverse(bmsc.get());

	InstancePtrVector instances = index_instances(bmsc->get_instances());	
	CoregionAreaList coregions;
	coregions = index_coregions(bmsc->get_instances(), indexer.get_event_count()+1);
	
	lprec *lp = initialize_solver(indexer.get_event_count()+ 2*coregions.size());

  EventConstraintCreator constraint_creator(lp, indexer, coregions.size(), instances);
  constraint_creator.m_successor_distance = m_successor_distance;
  constraint_creator.m_send_receive_distance = m_send_receive_distance;
	constraint_creator.m_coregion_begin_event_distance = m_coregion_begin_event_distance;
	constraint_creator.m_event_coregion_end_distance = m_event_coregion_end_distance;
	constraint_creator.m_incomplete_message_length = m_incomplete_message_length;

  DFSEventsTraverser traverser("cc_color");
  traverser.add_event_successor_listener(&constraint_creator);
  traverser.add_send_receive_pair_listener(&constraint_creator);
  traverser.add_white_event_found_listener(&constraint_creator);
  // create the constraints
  create_absolute_constraint(bmsc, lp, indexer, m_successor_distance, m_coregion_begin_event_distance, m_event_coregion_end_distance, coregions);
  traverser.traverse(bmsc.get());

  // (2) solve the problem
  // before solving the problem we have to turn the row entry mode off
  // note: don't call other API functions while in row entry mode
  set_add_rowmode(lp, FALSE);

	// check, if there there is something to solve
	if(indexer.get_event_count() == 0 && coregions.size() == 0)
  { 
		for(InstancePtrList::const_iterator it=bmsc->get_instances().begin(); it!=bmsc->get_instances().end();it++)
			(*it)->set_line_end(MscPoint((*it)->get_line_end().get_x(), (*it)->get_line_begin().get_y()+m_instance_head_distance+m_instance_foot_distance));
    delete_lp(lp);
    return 1; // success
  }

	if(solve(lp) != 0)
    return 0; // no solution found

  int columns = get_Ncolumns(lp);
  // process the results
  REAL* var = new REAL[columns];
  get_variables(lp, var);

	double max_y = set_events_coregions_positions(coregions, m_instance_head_distance, 1, indexer, var, 
		m_incomplete_message_length, m_send_receive_distance);


	for(InstancePtrList::const_iterator it=bmsc->get_instances().begin(); it!=bmsc->get_instances().end(); it++)
	{
		(*it)->set_line_end(MscPoint((*it)->get_line_end().get_x(), (*it)->get_line_begin().get_y()+max_y+m_instance_foot_distance));
	}

	remove_inst_cor_attributes(bmsc->get_instances());

  delete[] var;

  delete_lp(lp);

	return 1; // success
}

int LayoutOptimizer::process(BMscPtr bmsc)
{	
	load_registry_beautify();

	LengthOptimizer length_optimizer(this->get_config_provider());
	if(!length_optimizer.m_arrange)
		return 1; //success

	// if the instances length is already set, distances between elements will have the same size 
	if(!length_optimizer.m_use_parts_length)
	{
		m_successor_distance = m_coregion_begin_event_distance = m_event_coregion_end_distance = 
			m_instance_head_distance = m_instance_foot_distance = 1;
		m_send_receive_distance = 0;
	}

  EventIndexer indexer;
  DFSEventsTraverser indexing_traverser;
  indexing_traverser.add_white_event_found_listener(&indexer);
  indexing_traverser.traverse(bmsc.get());
	InstancePtrVector instances = index_instances(bmsc->get_instances());
	CoregionAreaList coregions = index_coregions(bmsc->get_instances(), indexer.get_event_count()+1);

	lprec *lp = initialize_solver(indexer.get_event_count()+ 2*coregions.size());

  EventConstraintCreator constraint_creator(lp, indexer, coregions.size(), instances);
  constraint_creator.m_successor_distance = m_successor_distance;
  constraint_creator.m_send_receive_distance = m_send_receive_distance;
	constraint_creator.m_coregion_begin_event_distance = m_coregion_begin_event_distance;
	constraint_creator.m_event_coregion_end_distance = m_event_coregion_end_distance;
	constraint_creator.m_incomplete_message_length = m_incomplete_message_length;

  DFSEventsTraverser traverser("cc_color");
  traverser.add_event_successor_listener(&constraint_creator);
  traverser.add_send_receive_pair_listener(&constraint_creator);
  traverser.add_white_event_found_listener(&constraint_creator);
  // create the constraints
  create_absolute_constraint(bmsc, lp, indexer, m_successor_distance, m_coregion_begin_event_distance, m_event_coregion_end_distance, coregions);
  traverser.traverse(bmsc.get());

  // (2) solve the problem
  // before solving the problem we have to turn the row entry mode off
  // note: don't call other API functions while in row entry mode
  set_add_rowmode(lp, FALSE);

  // check, if there there is something to solve
	if(indexer.get_event_count() == 0 && coregions.size() == 0)
  { 
		if(length_optimizer.m_use_parts_length)
			for(InstancePtrList::const_iterator it=bmsc->get_instances().begin(); it!=bmsc->get_instances().end();it++)
				(*it)->set_line_end(MscPoint((*it)->get_line_end().get_x(), (*it)->get_line_begin().get_y()+m_instance_head_distance+m_instance_foot_distance));
    delete_lp(lp);
    return 1; // success
  }

  if(solve(lp) != 0)
    return 0; // no solution found

  int columns = get_Ncolumns(lp);
  // process the results
  REAL* var = new REAL[columns];
  get_variables(lp, var);

  IncompleteMessagePtr mess;

	double proportion = 1;
	if(length_optimizer.m_use_const_length || length_optimizer.m_use_first_length)
	{
		Coordinate max_y = 0;
		for (int i = 0; i < columns; ++i)
			max_y = (var[i] > max_y) ? var[i] : max_y;
		max_y = max_y + m_instance_head_distance + m_instance_foot_distance;
		proportion = (*bmsc->get_instances().begin())->get_height() / max_y;
	}

	double max_instance_length = 0;
	if(!length_optimizer.m_use_const_length && !length_optimizer.m_use_first_length)
	{
		double proportion_enlarging_distances = 1;
		double proportion_shortening_distances = 1;
		for(InstancePtrList::const_iterator it=bmsc->get_instances().begin(); it!=bmsc->get_instances().end(); it++)
		{
			EventAreaPtr area;
			area = (*it)->get_last();
			Coordinate max_y = 0;
			while(area)
			{
				StrictOrderArea* strict_area = dynamic_cast<StrictOrderArea*>(area.get());
				if(strict_area)
				{
					if(strict_area->get_last())
						max_y = var[indexer.get_event_index(strict_area->get_last())-1] + m_instance_head_distance;
				}
				else
				{
					CoregionArea* coregion_area = dynamic_cast<CoregionArea*>(area.get());
					int end = 0;
					end = coregion_area->get_attribute("id", end);
					max_y = var[end] + m_instance_head_distance;
				}
				if(max_y == 0)
					area = area->get_previous();
				else
					area = NULL;
			}
			// if max_y == 0, no event neither coregion laying on it
			if(max_y)
			{
				max_y = (*it)->get_line_begin().get_y()+max_y+m_instance_foot_distance;
				if(length_optimizer.m_use_parts_length)
				{
					(*it)->set_line_end(MscPoint((*it)->get_line_end().get_x(), max_y));
					max_instance_length = (double)(*it)->get_height() > max_instance_length ? (double)(*it)->get_height() : max_instance_length;
				}
				else
				{
					// any event or coregion is laying over the end of instance => shorten distances between elements on instances
					if(max_y > (*it)->get_line_end().get_y())
						proportion_shortening_distances = (((double)(*it)->get_height()) / (max_y-(*it)->get_line_begin().get_y())) < proportion_shortening_distances ? 
							(((double)(*it)->get_height()) / max_y) : proportion_shortening_distances;
					// on the bottom of instance there is unused space => enlarge distances between elements on instances
					// no event no coregion is laying over the end of instance 
					if(max_y < (*it)->get_line_end().get_y() && (proportion_shortening_distances==1))
						if(proportion_enlarging_distances == 1)
							proportion_enlarging_distances = ((double)(*it)->get_height()) / (max_y-(*it)->get_line_begin().get_y());
						else
							proportion_enlarging_distances = (((double)(*it)->get_height()) / (max_y-(*it)->get_line_begin().get_y()) 
							< proportion_enlarging_distances) ? ((double)(*it)->get_height()) / (max_y-(*it)->get_line_begin().get_y()): proportion_enlarging_distances;
				}
			}
		}
		proportion = (proportion_shortening_distances != 1) ? proportion_shortening_distances : proportion_enlarging_distances;
	}

	double max_y = set_events_coregions_positions(coregions, m_instance_head_distance, proportion, indexer, var, 
		m_incomplete_message_length, m_send_receive_distance);

	if(length_optimizer.m_use_parts_length)
		for(InstancePtrList::const_iterator it=bmsc->get_instances().begin(); it!=bmsc->get_instances().end(); it++)
			(*it)->set_line_end(MscPoint((*it)->get_line_end().get_x(), (*it)->get_line_begin().get_y()+max_y+m_instance_foot_distance));
	
	remove_inst_cor_attributes(bmsc->get_instances());

  delete[] var;

  delete_lp(lp);
  return 1; // success
}

// $Id: layout_optimizer.cpp 1019 2011-01-04 07:15:39Z xpekarc $
