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
 * Copyright (c) 2008 Matus Madzin <gotti@mail.muni.cz>
 *
 */

#include "membership/membership_time.h"

bool check_node_time(MembershipContext* c, ReferenceNodePtr node, ConfigurationPtr conf)
{
  if(c->get_relative_time())
    std::cout << "relative time" << std::endl;

//  if(c->get_absolut_time())
//      return check_absolut_time(c, node, conf);

  TimeRelationRefNodePtrSet top = node->get_time_relations_top();
  TimeRelationRefNodePtrSet bottom = node->get_time_relations_bottom();
  TimeRelationRefNodePtrSet::iterator it;

  //save relations where the current node is connected as the end of the interval (separated top/bottom)
  std::vector<TimeRelationRefNodePtr> top_vec, bottom_vec, themself_vec;

  for(it = top.begin(); it!=top.end(); it++)
  {
    if((*it)->get_ref_node_b() == node.get())
    {
      if((*it)->get_ref_node_b() == (*it)->get_ref_node_a())
        themself_vec.push_back(*it);
      else
        top_vec.push_back(*it);
    }
    else
      c->add_top_time_ref(*it, conf);
  }

  for(it = bottom.begin(); it!=bottom.end(); it++)
  {
    if((*it)->get_ref_node_b() == node.get())
    {
      if((*it)->get_ref_node_b() != (*it)->get_ref_node_a())
        bottom_vec.push_back(*it);
    }
    else
      c->add_bottom_time_ref(*it, conf);
  }

  //relation between the first and the second hmsc node
  Event* max_b = NULL; //! max_b - bottom of the second hmsc node
  Event* min_b = NULL; //! min_b - top of the second hmsc node

  if(!themself_vec.empty())
  {
    if(!check_node_time_themself(c, themself_vec, node, conf))
      return false;
  }

  if(!top_vec.empty())
    min_b = get_min_event(c, conf);

  if(!bottom_vec.empty())
    max_b = get_max_event(c, node, conf);

  //! found bottom time relation and decided the maximal event
  if(max_b != NULL)
  {
    if(!check_bottom_time_relations(c, bottom_vec, max_b))
      return false;
  }

  //! found top time relations and decided the minimal event
  if(min_b != NULL)
  {
    if(!check_top_time_relations(c, top_vec, min_b))
      return false;
  }

  return true;
}

bool check_node_time_themself(MembershipContext* c, const std::vector<TimeRelationRefNodePtr>& themself_vec,
                              ReferenceNodePtr node, ConfigurationPtr conf)
{
  Event* min_a = NULL; //! max_b - bottom of the second hmsc node
  Event* max_b = NULL; //! max_b - bottom of the second hmsc node

  if(themself_vec.size() > 1)
  {
    c->get_mem()->print_report(RS_ERROR, stringize() << L"Error: more time intervals between top and bottom of the "
      << node->get_msc()->get_label() << " node.");
    return false;
  }

  min_a = get_min_event(c, conf);
  max_b = get_max_event(c, node, conf);

  if(min_a == NULL || max_b == NULL)
  {
    if(min_a == NULL)
      c->get_mem()->print_report(RS_ERROR, stringize() << L"Error: not decidable which event is the minimal one of the "
        << node->get_msc()->get_label() << L" node.");
    else
      c->get_mem()->print_report(RS_ERROR, stringize() << "Error: not decidable which event is the maximal one of the "
        << node->get_msc()->get_label() << " node.");

    return false;
  }

  TimeRelationRefNodePtr relation = themself_vec.front();

  if(relation == NULL)
    throw std::runtime_error("Unexpected behaviour.");

  MscTimeIntervalSetD result, hmsc_inter, bmsc_inter;

  hmsc_inter = relation->get_interval_set();

  if(c->get_absolut_time())
  {
//TODO to nemoze byt list intervalov v evente pri absolutnom case, to nedava zmysel 
    MscTimeIntervalSetD a = *(min_a->get_absolut_times().begin());
    MscTimeIntervalSetD b = *(max_b->get_absolut_times().begin());
std::cout << "see 1    a " << a << " b " << b << std::endl;
    bmsc_inter = get_continous_interval(c, a, b);
  }
  else
    bmsc_inter = c->get_time_matrix()->operator()(min_a, max_b);

  result = MscTimeIntervalSetD::set_intersection(hmsc_inter, bmsc_inter);

  if(result.is_empty())
    return false;
  else
    if(result != bmsc_inter)
          c->add_not_covered_interval(relation);

  return true;
}

Event* get_min_event(MembershipContext* c, ConfigurationPtr conf)
{
  std::set<PositionPtr> positions = conf->get_positions();
  std::set<PositionPtr>::iterator it;
  //! vector of events which could be minimal event of matching HMSC node
  std::vector<Event*> pos_min;

  for(it = positions.begin(); it != positions.end(); it++)
  {
    std::vector<Event*> events = (*it)->get_events();

    if(events.size() == 0)
      continue;

    if(events.size() > 1)
    {
      CoregionAreaPtr cor = dynamic_cast<CoregionArea*> (events.front()->get_general_area());

      if(cor == NULL)
        throw std::runtime_error("Unexpected behaviour.");

      CoregionOrderingPtr cor_ordering = c->find_coregion_ordering(cor);

      if(cor_ordering != NULL)
        pos_min.push_back(cor_ordering->getOrdering().front().get());
    }
    else
      pos_min.push_back(events.front());
  }

  //set of the minimal events (in case more events, the algorithm is not able to decide the minimum one)
  std::set<Event*> result;

  result = eliminate_posible_minimal(c, pos_min);

  if(result.size() != 1)
    return NULL;
  else
    return *(result.begin());
}

Event* get_max_event(MembershipContext* c, ReferenceNodePtr node, ConfigurationPtr conf)
{
  std::set<PositionPtr> positions = conf->get_positions();
  std::set<PositionPtr>::iterator it;

  std::map<std::wstring, std::vector<int> > node_last_events;
  node_last_events = get_node_last_events(c, node);

  //! vector of posible events
  std::vector<Event*> pos_max;

  for(it = positions.begin(); it != positions.end(); it++)
  {
    std::vector<Event*> events = (*it)->get_events();

    if(events.empty())
      continue;

    //suppose one event due to coregions are not supported in the flow     
    Event* e = events.front();
    StrictEvent* s_e = dynamic_cast<StrictEvent*> (e);

    //find instance in the map of node last events
    std::map <std::wstring, std::vector<int> >::iterator node_last_it;
    node_last_it = node_last_events.find(e->get_instance()->get_label());

    if(node_last_it == node_last_events.end())
      continue;

    std::vector<int> node_events = node_last_it->second;

    Event* temp;
    for(unsigned int i = 0; i < node_events.size(); i++)
    {
      temp = find_event_on_instance_by_id(c, e->get_instance()->get_label(), node_events[i], s_e);

      if(temp == NULL)
        continue;

      pos_max.push_back(temp);
    }
  }

  //set of the maximal events (in case more events, the algorithm is not able to decide the maximum one)
  std::set<Event*> result;

  result = eliminate_posible_maximal(c, pos_max);

  if(result.size() != 1)
    return NULL;
  else
    return *(result.begin());
}

void update_maximum_set(MembershipContext* c, std::set<Event*>& max, Event* new_element)
{
  std::set<Event*> to_delete;
  std::set<Event*>::iterator it;

  for(it = max.begin(); it != max.end(); it++)
  {
    switch (compare_absolut_position(c, new_element, *it))
    {
      case 0: break;
      case 1: break;
      case 2: to_delete.insert(*it); break;
      default: throw std::runtime_error("Unexpected behaviour.");
    }
  }

  for(it = to_delete.begin(); it != to_delete.end(); it++)
  {
    max.erase(*it);
  }

  max.insert(new_element);
}

void update_minimum_set(MembershipContext* c, std::set<Event*>& min, Event* new_element)
{
  std::set<Event*> to_delete;
  std::set<Event*>::iterator it;

  for(it = min.begin(); it != min.end(); it++)
  {
    switch (compare_absolut_position(c, *it, new_element))
    {
      case 0: break;
      case 1: break;
      case 2: to_delete.insert(*it); break;
      default: throw std::runtime_error("Unexpected behaviour.");
    }
  }

  for(it = to_delete.begin(); it != to_delete.end(); it++)
  {
    min.erase(*it);
  }

  min.insert(new_element);
}

std::set<Event*> eliminate_posible_minimal(MembershipContext* c, std::vector<Event*> pos_min)
{
  std::set<Event*> min;

  if(pos_min.size() == 0)
    return min;

  min.insert(pos_min[0]);

  //try to eliminate pos_min to one event
  for(unsigned int i = 0; i < pos_min.size(); i++)
  {
    switch (compare_absolut_position(c, (*min.begin()), pos_min[i]))
    {
      case 0: min.insert(pos_min[i]); break;
      case 1: break;
      case 2: update_minimum_set(c, min, pos_min[i]); break;
      default: throw std::runtime_error("Unexpected behaviour.");
    }
  }

  return min;
}

std::set<Event*> eliminate_posible_maximal(MembershipContext* c, std::vector<Event*> pos_max)
{
  std::set<Event*> max;

  if(pos_max.size() == 0)
    return max;

  max.insert(pos_max[0]);

  //try to eliminate pos_min to one event
  for(unsigned int i = 0; i < pos_max.size(); i++)
  {
    switch (compare_absolut_position(c, pos_max[i], (*max.begin())))
    {
      case 0: max.insert(pos_max[i]); break;
      case 1: break;
      case 2: update_maximum_set(c, max, pos_max[i]); break;
      default: throw std::runtime_error("Unexpected behaviour.");
    }
  } 

  return max;
}

int compare_absolut_position(MembershipContext* c, Event* first, Event* second)
{
//TODO There can be checking for first == second 

  MscTimeIntervalSetD inter;

  //in case the absolut time constraints, it is necessary to compute interval
//TODO event has not to have list of the absolut times just one time constraints
  if(c->get_absolut_time())
{
    MscTimeIntervalSetD a = *(first->get_absolut_times().begin());
    MscTimeIntervalSetD b = *(second->get_absolut_times().begin());

    inter = get_continous_interval(c, a, b);    
//    inter = get_continous_interval(c, *(first->get_absolut_times().begin()), *(second->get_absolut_times().begin()));
std::cout << "time interval inter " << inter << std::endl;
}
  else 
    inter = c->get_time_matrix()->operator()(first, second);

  //compute which event is before
  const IntervalList intervals = inter.get_set();
  bool positive = false;
  bool negative = false;

  IntervalList::const_iterator it;

  for(it = intervals.begin(); it != intervals.end(); it++)
  {
    if(abs(it->get_begin_value()) == 0 && it->get_begin_closed())
      return 1;

    if(abs(it->get_end_value()) == 0 && it->get_end_closed())
      return 2;

    if(it->get_begin_value() >= 0)
      positive = true;

    if(it->get_begin_value() < 0)
      negative = true;

    if(it->get_end_value() > 0)
      positive = true;

    if(it->get_end_value() <= 0)
    {
      negative = true;
    }
  }

  if(negative == positive)
    return 0;

  if(negative)
    return 2;

  if(positive)
    return 1;

  return 3;
}

bool compare_relative_time_constraints(MembershipContext* c, Event* node_a, Event* bmsc_a)
{
  TimeRelationEventPtrList node_intervals = node_a->get_time_relations();
  TimeRelationEventPtrList::iterator it;
  int time_node_b_id;

  for(it = node_intervals.begin(); it != node_intervals.end(); it++)
  {
    if((*it)->get_event_a() == node_a)
    {
      bool time_node_b_id_set = true;

//TODO i don't like it, probably better would be is_attribute_set or something similar
      time_node_b_id = (*it)->get_event_b()->get_attribute("identification", -1, time_node_b_id_set);

      if(time_node_b_id_set)
        throw std::runtime_error("Unexpected behaviour.");

      Event* bmsc_b = find_event_on_instance_by_id(c, (*it)->get_event_b()->get_instance()->get_label(), time_node_b_id);

      if(bmsc_b == NULL)
        return false;

      MscTimeIntervalSetD result, hmsc_inter, bmsc_inter;

      hmsc_inter = (*it)->get_interval_set();

      if(c->get_absolut_time())
      {
        MscTimeIntervalSetD a = *(bmsc_a->get_absolut_times().begin());
        MscTimeIntervalSetD b = *(bmsc_b->get_absolut_times().begin());
        bmsc_inter = get_continous_interval(c, a, b);
      }
      else
        bmsc_inter = c->get_time_matrix()->operator()(bmsc_a, bmsc_b);

      result = MscTimeIntervalSetD::set_intersection(hmsc_inter, bmsc_inter);

      if(result.is_empty())
          return false;
      else
        if(result != bmsc_inter)
          c->add_not_covered_interval(*it);
    }
  }

  return true;
}

BMscIntervalSetMatrix get_bmsc_matrix(BMscPtr bmsc_f)
{
  //matrix context inicialization 
  BMscIntervalSetMatrix matrix(bmsc_f);
  //create matrix
  matrix.build_up();

  MscSolveTCSP solve;
  //tight the matrix 
  MscSolveTCSPReport report = solve.solveTCSP(matrix);
  //get the result of the tightening 
  matrix = report.m_matrix_result;

  return matrix;
}

std::map<std::wstring, std::vector<int> > get_node_last_events(MembershipContext* c, ReferenceNodePtr node)
{
  MscPtr msc = node->get_msc();
  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);

  if(bmsc == NULL)
    throw std::runtime_error("Unexpected behaviour.");

  InstancePtrList instances = bmsc->get_instances();
  InstancePtrList::iterator inst_it;

  //name of instance, event identification
  std::map<std::wstring, std::vector<int> > node_last_events;
  EventPtr start = NULL;
  EventArea* area;
  StrictOrderArea* strict;
  CoregionArea* cor;

  for(inst_it = instances.begin(); inst_it != instances.end(); inst_it++)
  {
    area = (*inst_it)->get_first().get();

    if(area == NULL)
      continue;

    strict = dynamic_cast<StrictOrderArea*>(area);
    cor = dynamic_cast<CoregionArea*>(area);

    if(cor == NULL && strict == NULL)
      throw std::runtime_error("Unexpected behaviour.");

    if(strict != NULL)
      start = strict->get_first();
    else
    {
      CoregionOrderingPtr cor_order = c->find_coregion_ordering(cor);

      if(cor_order == NULL)
        throw std::runtime_error("Unexpected behaviour.");

      start = cor_order->getOrdering().front();
    }

    std::vector<int> identifications;
    Event* e = get_last_instance_event(c, start.get());

    if(e == NULL)
      continue;

    int it_id;
    bool it_set = true;

    it_id = e->get_attribute("identification", -1, it_set);

    if(!it_set)
      identifications.push_back(it_id);
    else 
      throw std::runtime_error("Unexpected behaviour.");

    node_last_events.insert(std::make_pair((*inst_it)->get_label(), identifications));
  }
  return node_last_events;
}

bool check_bottom_time_relations(MembershipContext* c, std::vector<TimeRelationRefNodePtr> bottom_vec, Event* max_b)
{
  //relation between the first and the second hmsc node
  Event* max_a = NULL; //! max_a - bottom of the first hmsc node
  Event* min_a = NULL; //! min_a - top of the first hmsc node

  std::vector<TimeRelationRefNodePtr>::iterator it_vec;
  ConfigurationPtr map_top_conf, map_bottom_conf;

  for(it_vec = bottom_vec.begin(); it_vec != bottom_vec.end(); it_vec++)
  {
    MscTimeIntervalSetD hmsc_inter = (*it_vec)->get_interval_set();

    map_top_conf = c->find_top_time_ref(*it_vec);

    if(map_top_conf != NULL)
    {
      MscTimeIntervalSetD bmsc_inter, result;

      min_a = get_min_event(c, map_top_conf);

      if(c->get_absolut_time())
      {
        MscTimeIntervalSetD a = *(min_a->get_absolut_times().begin());
        MscTimeIntervalSetD b = *(max_b->get_absolut_times().begin());
//TODO to nemoze byt list intervalov v evente pri absolutnom case, to nedava zmysel 
std::cout << "see 2" << std::endl;
        bmsc_inter = get_continous_interval(c, a, b);
      }
      else
        bmsc_inter = c->get_time_matrix()->operator()(min_a, max_b);
  
      result = MscTimeIntervalSetD::set_intersection(hmsc_inter, bmsc_inter);

      if(result.is_empty())
        return false;
      else
        if(result != bmsc_inter)
          c->add_not_covered_interval(*it_vec);
    }

    map_bottom_conf = c->find_bottom_time_ref(*it_vec);

    if(map_bottom_conf != NULL)
    {
      MscTimeIntervalSetD bmsc_inter, result;

      max_a = get_max_event(c, (*it_vec)->get_ref_node_a(), map_bottom_conf);

      if(c->get_absolut_time())
      {
        MscTimeIntervalSetD a = *(min_a->get_absolut_times().begin());
        MscTimeIntervalSetD b = *(max_b->get_absolut_times().begin());
//TODO to nemoze byt list intervalov v evente pri absolutnom case, to nedava zmysel 
std::cout << "see 3" << std::endl;
        bmsc_inter = get_continous_interval(c, a, b);
      }
      else
        bmsc_inter = c->get_time_matrix()->operator()(max_a, max_b);

      result = MscTimeIntervalSetD::set_intersection(hmsc_inter, bmsc_inter);

      if(result.is_empty())
        return false;
      else
        if(result != bmsc_inter)
          c->add_not_covered_interval(*it_vec);
    }
  }

  return true;
}

bool check_top_time_relations(MembershipContext* c, std::vector<TimeRelationRefNodePtr> top_vec, Event* min_b)
{
  //relation between the first and the second hmsc node
  Event* max_a = NULL; //! max_a - bottom of the first hmsc node
  Event* min_a = NULL; //! min_a - top of the first hmsc node

  std::vector<TimeRelationRefNodePtr>::iterator it_vec;
  ConfigurationPtr map_bottom_conf, map_top_conf;

  for(it_vec = top_vec.begin(); it_vec != top_vec.end(); it_vec++)
  {
    MscTimeIntervalSetD hmsc_inter = (*it_vec)->get_interval_set();

    map_bottom_conf = c->find_bottom_time_ref(*it_vec);

    if(map_bottom_conf != NULL)
    {
      MscTimeIntervalSetD result, bmsc_inter;

      max_a = get_max_event(c, (*it_vec)->get_ref_node_a(), map_bottom_conf);

      if(c->get_absolut_time())
      {
        MscTimeIntervalSetD a = *(min_a->get_absolut_times().begin());
        MscTimeIntervalSetD b = *(min_b->get_absolut_times().begin());
//TODO to nemoze byt list intervalov v evente pri absolutnom case, to nedava zmysel 
std::cout << "see 4" << std::endl;
        bmsc_inter = get_continous_interval(c, a, b);
      }
      else
        bmsc_inter = c->get_time_matrix()->operator()(max_a, min_b);

      result = MscTimeIntervalSetD::set_intersection(hmsc_inter, bmsc_inter);

      if(result.is_empty())
        return false;
      else
        if(result != bmsc_inter)
          c->add_not_covered_interval(*it_vec);
    }

    map_top_conf = c->find_top_time_ref(*it_vec);

    if(map_top_conf != NULL)
    {
      MscTimeIntervalSetD result, bmsc_inter;

      min_a = get_min_event(c, map_top_conf);
      bmsc_inter = c->get_time_matrix()->operator()(min_a, min_b);
      result = MscTimeIntervalSetD::set_intersection(hmsc_inter, bmsc_inter);

      if(result.is_empty())
        return false;
      else
        if(result != bmsc_inter)
          c->add_not_covered_interval(*it_vec);
     }
  }

  return true;
}

void analyze_time_constraints(MembershipContext* c, Event* b_e)
{
  if(b_e->get_absolut_times().size() > 0)
    c->found_absolut_time();

  if(b_e->get_time_relations().size() > 0)
    c->found_relative_time();
}

MscTimeIntervalSetD get_continous_interval(MembershipContext* c, MscTimeIntervalSetD& a, MscTimeIntervalSetD& b)
{
  std::list< MscTimeInterval<double> > a_set = a.get_set();
  std::list< MscTimeInterval<double> > b_set = b.get_set();

  std::list< MscTimeInterval<double> >::iterator a_it, b_it;

  MscTimeIntervalSet<double> new_set;
  MscTimeInterval<double> new_interval;
  MscIntervalCouple<double> start;
  MscIntervalCouple<double> end;

  for(a_it = a_set.begin(); a_it != a_set.end(); a_it++)
  {
    for(b_it = b_set.begin(); b_it != b_set.end(); b_it++)
    {
      //compute beginning of the interval
      start = MscIntervalCouple<double> (b_it->get_begin().get_closed() && a_it->get_end().get_closed(),
                                         b_it->get_begin().get_value() - a_it->get_end().get_value());
      
      //compute ending of the interval 
      end = MscIntervalCouple<double> (b_it->get_end().get_closed() && a_it->get_begin().get_closed(),
                                       b_it->get_end().get_value() - a_it->get_begin().get_value());
 
      new_interval = MscTimeInterval<double>(start, end);
      new_set = MscTimeIntervalSetD::set_union(new_set, new_interval);
    }
  }
  
  return new_set;
}
