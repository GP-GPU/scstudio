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

#include "membership/membership_additional.h"

//parameters a - node event
//           b - bmsc event
bool compare_events(MembershipContext* c, Event* a, Event* b)
{
  if (b == NULL || a == NULL)
    return false;

  if (a->get_instance()->get_label() != b->get_instance()->get_label())
    return false;

  if (a->is_send() != b->is_send())
    return false;

  if ((a->get_complete_message() == NULL) != (b->get_complete_message() == NULL))
    return false;

  if (a->get_message()->get_label() != b->get_message()->get_label())
    return false;

  if (a->is_matched() != b->is_matched())
    return false;

  if (a->is_matched())
  {
    if (a->is_send())
    {
      if (a->get_receiver_label() != b->get_receiver_label())
        return false;
    }
    else
    {
      if (a->get_sender_label() != b->get_sender_label())
        return false;
    }
  }

  if(c->get_diff_type() != NOT_DIFF)
  {
    switch(a->is_attribute_set("identification") + b->is_attribute_set("identification"))
    {
      case 0: return true;
      case 1: return false;
      case 2: if(a->get_attribute("identification", -1) == b->get_attribute("identification", -1))
                return true;
              else 
                return false;
      default: throw std::runtime_error("Unexpected behaviour.");
    }
  }

  return true;
}

//parameters node_e - node event
//           b_e - bmsc event
bool compare_events_attribute(MembershipContext* c, Event* node_e, Event* b_e)
{
  IncompleteMessagePtr incom_node = boost::dynamic_pointer_cast<IncompleteMessage > (node_e->get_message());
  IncompleteMessagePtr incom_b = boost::dynamic_pointer_cast<IncompleteMessage > (b_e->get_message());

  switch ((incom_node != NULL) + (incom_b != NULL))
  {
  case 2:
    return true;
    break;

  case 1:
    return false;
    break;

  case 0:
    break;

  default:
    return false;
  }

  int node_id, b_id;
  bool node_set = true;

  node_id = node_e->get_attribute("identification", -1, node_set);

  if(node_set)
    c->add_attributed_event(node_e);

  bool b_set = true;

  b_id = b_e->get_attribute("identification", -2, b_set);

  if(b_set)
    c->add_attributed_event(b_e);

  //check if the partial membership algorithm is not running or the instance is among the focused instances
  if(c->get_focused_instances().empty()
     || c->contain_focused_instances(node_e->get_matching_event()->get_instance()->get_label()))
  {
    if (node_set || b_set || node_id != b_id)
      return false;

    bool relative = false;
    bool absolut = false;

    relative = compare_relative_time_constraints(c, node_e, b_e);

//TODO beginnig for absolut time support in BMSC parts of specification
//    absolut = compare_absolut_time_constraints(c, node_e, b_e);
    absolut = true;

    if(relative && absolut)
      return true;
    else
      return false;
  }
  else
    return true;
}

bool look_at_checked_conf(MembershipContext* c, ReferenceNodePtr node, ConfigurationPtr b)
{
  const std::set<ConfigurationPtr> checked_conf = c->find_checked_conf(node->get_msc()->get_label());

  if (!checked_conf.empty())
  {
    std::set<ConfigurationPtr>::const_iterator conf_it;

    for (conf_it = checked_conf.begin(); conf_it != checked_conf.end(); conf_it++)
      if (b->compare(*conf_it))
        return false;
  }

  return true;
}

//TODO I don't like the time of increasing max id 
void set_identification(MembershipContext* c, Event* node_e, Event* b_e)
{
  if (node_e->is_send())
  {
    node_e->set_attribute("identification", c->get_max_id());
    c->add_attributed_event(node_e);
    b_e->set_attribute("identification", c->get_max_id());
    c->add_attributed_event(b_e);
    c->increase_max_id();

    CompleteMessagePtr node_message = node_e->get_complete_message();

    if (node_message != NULL)
    {
      Event* event = node_message->get_receive_event();
      event->set_attribute("identification", c->get_max_id());
      c->add_attributed_event(event);
    }

    CompleteMessagePtr b_message = b_e->get_complete_message();

    if (b_message != NULL)
    {
      Event* event = b_message->get_receive_event();
      event->set_attribute("identification", c->get_max_id());
      c->add_attributed_event(event);
      c->increase_max_id();
    }
  }
}

bool is_node_null(MembershipContext* c, ReferenceNodePtr node)
{
  MscPtr msc = node->get_msc();

  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc> (msc);

  if(bmsc == NULL)
  {
    throw std::runtime_error("Unexpected behaviour.");
    return false;
  }

  InstancePtrList instances = bmsc->get_instances();
  InstancePtrList::iterator it;

  for(it = instances.begin(); it != instances.end(); it++)
  {
    if(c->get_focused_instances().empty()
       || c->contain_focused_instances((*it)->get_label()))
    {
      if(!is_instance_null(*it))
        return false;
    }
  }

  return true;
}

bool is_instance_null(InstancePtr instance)
{
  EventAreaPtr area;
  StrictOrderAreaPtr strict;
  CoregionAreaPtr cor;

  if(instance == NULL)
    return true;

  area = instance->get_first();

  while(area != NULL)
  {
    strict = boost::dynamic_pointer_cast<StrictOrderArea>(area);

    if(strict == NULL)
    {
      cor = boost::dynamic_pointer_cast<CoregionArea>(area);

      if(cor != NULL)
        if(!is_cor_area_null(cor))
          return false;
    }
    else
      if(!is_strict_area_null(strict))
        return false;

    area = area->get_next();
  }

  return true;
}

bool is_cor_area_null(CoregionAreaPtr cor)
{
  if(cor == NULL)
    return true;

  if(!cor->get_minimal_events().empty())
    return false;

  return true;
}

bool is_strict_area_null(StrictOrderAreaPtr strict)
{
  if(strict == NULL)
    return true;

  if(strict->get_first() != NULL)
    return false;

  return true;
}

bool is_empty_instance(MembershipContext* c, InstancePtrList node_instances, InstancePtrList b_instances)
{
  InstancePtrList::iterator node_instence_it, b_instance_it;

  bool in_bmsc_f;
  EventAreaPtr event_area;

  for (node_instence_it = node_instances.begin(); node_instence_it != node_instances.end(); node_instence_it++)
  {
    //in case the user is not focused on this instance the algorithm skip the checking of them
    bool skip = false;

    if(!c->get_focused_instances().empty())
    {
      skip = true;

      for(unsigned int i = 0; i < c->get_focused_instances().size(); i++)
      {
        if(c->get_focused_instances().at(i) == (*node_instence_it)->get_label())
          skip = false;
      }
    }

    if(skip)
      continue;

    in_bmsc_f = false;

    for (b_instance_it = b_instances.begin(); b_instance_it != b_instances.end(); b_instance_it++)
    {
      if ((*node_instence_it)->get_label() == (*b_instance_it)->get_label())
      {
        in_bmsc_f = true;
        break;
      }
    }

    if (!in_bmsc_f)
    {
      event_area = (*node_instence_it)->get_first();

      if (event_area != NULL)
      {
        StrictOrderAreaPtr strict = boost::dynamic_pointer_cast<StrictOrderArea > (event_area);

        if (strict == NULL)
        {
          CoregionAreaPtr coregion = boost::dynamic_pointer_cast<CoregionArea > (event_area);
          if (coregion != NULL)
          {
            if (coregion->get_minimal_events().size() != 0)
              return false;
          }
        }
        else
        {
          if (strict->get_first() != NULL)
            return false;
        }
      }
    }
  }

  return true;
}

Event* get_last_instance_event(MembershipContext* c, Event* start)
{

  if(start == NULL)
    return NULL;

  EventArea* area = start->get_general_area();
  EventArea* not_null_area = area;

  StrictOrderArea* str;
  CoregionArea* cor;

  while(area->get_next() != NULL)
  {
    area = area->get_next().get();
    str = dynamic_cast<StrictOrderArea*>(area);
    cor = dynamic_cast<CoregionArea*>(area);

    if(str == NULL)
    {
      if(!is_strict_area_null(str))
        not_null_area = str;
    }
    else
      if(!is_cor_area_null(cor))
        not_null_area = cor;
  }

  str = dynamic_cast<StrictOrderArea*>(not_null_area);
  cor = dynamic_cast<CoregionArea*>(not_null_area);

  if(str != NULL)
    return str->get_last().get();
  if(cor != NULL)
  {
    CoregionOrderingPtr cor_ord = c->find_coregion_ordering(cor);

    if(cor_ord != NULL)
      return cor_ord->getOrdering().back().get();
    else
      return NULL;
  }

  return NULL;
}

Event* find_event_on_instance_by_id(MembershipContext* c, std::wstring label, int id, StrictEventPtr start_event)
{
  EventAreaPtr first;

  if(start_event == NULL)
  {
    InstancePtrList instances = c->get_bmsc()->get_instances();
    InstancePtrList::iterator it;
    InstancePtr instance;

    for(it = instances.begin(); it != instances.end(); it++)
    {
      if((*it)->get_label() == label)
      {
        instance = *it;
        break;
      }
    }

    first = instance->get_first();
  }
  else
  {
    first = start_event->get_general_area();
  }

  StrictOrderAreaPtr strict = boost::dynamic_pointer_cast<StrictOrderArea>(first);

  //TODO check whether it could fall down (SEGFAULT)
  StrictEventPtr str = strict->get_first();

  //function finds event only in bMSC which is looked for, so coregion area is not allowed
  if(str == NULL)
  {
    c->get_mem()->print_report(RS_ERROR, L"unsupported function: Flow BMSC contains coregion.");
    return NULL;
  }

  int event_id;
  bool event_set = true;

  event_id = str->get_attribute("identification", -1, event_set);

  if(event_set)
    c->add_attributed_event(str.get());

  StrictEventPtr old;

  while(event_id != id)
  {
    old = str;
    str = str->get_successor();

    if(str == NULL)
    {
      EventAreaPtr area = old->get_general_area();
      area = area->get_next();

      strict = boost::dynamic_pointer_cast<StrictOrderArea>(area);

      if(strict == NULL)
      {
        c->get_mem()->print_report(RS_ERROR, L"Error: Node was not found.");
        return NULL;
      }

      str = strict->get_first();

      if(str == NULL)
      {
        c->get_mem()->print_report(RS_ERROR, L"Error: Node was not found.");
        return NULL;
      }
    }

    event_id = str->get_attribute("identification", -1, event_set);

    if(event_set)
      c->add_attributed_event(str.get());
  }

  return str.get();
}

