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

#include "membership/diff_impl.h"

void process_diffrences(MembershipContext* c, BMscPtr dup_flow, const std::map<std::wstring, Difference*>& diff_map);
bool check_attributes(MembershipContext* c, BMscPtr specification, BMscPtr dup_flow);
bool check_matching_event_creation(std::map<std::wstring, std::list<Difference*> > insert_map, Difference* diff);
std::map<std::wstring, std::list<Difference*> > create_insert_map(MembershipContext* c, 
                                                                  const std::map<std::wstring, Difference*>& diff_map);
void diff_identification_settings(MembershipContext* c, const std::map<std::wstring, Difference*>& diff_map);
InstancePtr find_instance(BMscPtr bmsc, std::wstring name);

void print_result(BMscPtr msc)
{
  InstancePtrList insts = msc->get_instances();
  InstancePtrList::iterator it;
  bool setting;

  for(it = insts.begin(); it != insts.end(); it++)
  {
    std::wcerr << L"Instance " << (*it)->get_label() << std::endl;
    
    StrictOrderAreaPtr area = boost::dynamic_pointer_cast<StrictOrderArea> ((*it)->get_first());
    StrictEventPtr st = area->get_first();

    while(st != NULL)
    {
      std::wcerr << st->get_message()->get_label() << L" id: " << st->get_attribute("identification", -3, setting) << L" ";
      std::wcerr << L" removed: " << (st->get_marked()==REMOVED) << std::endl;
      st = st->get_successor();
    }
  }
}

/**
 * Find diffrences on instance between flow and specification
 * 
 * parameters: a - flow
 *             b - specification
 */
Difference* instance_diff(MembershipContext* c, InstancePtr a, InstancePtr b);

bool event_comparison(MembershipContext* c, StrictEventPtr& a, StrictEventPtr b);

BMscPtr bmsc_bmsc_diff(MembershipContext* c, BMscPtr specification, BMscPtr flow);
BMscPtr hmsc_bmsc_diff(MembershipContext* c, HMscPtr specification, BMscPtr flow);

/**
 * Find msc diffrences between flow and specification
 *
 * parameters: a - specification
 *             b - flow
 */
MscPtr MembershipAlg::diff(MscPtr specification, std::vector<MscPtr>& msc_flows)
{
  if(msc_flows.size() > 1)
  {
    print_report(RS_WARNING, L"unsupported function: Diff version of Find Flow algorithm does not support multiple flows.");
    print_report(RS_WARNING, L"Warning: Find flow algorithm takes the first of flows.");
  }

  if(msc_flows.size() == 0)
  {
    print_report(RS_ERROR, L"Error: No flow was chosen for checking.");
    return NULL;
  }

  BMscPtr flow = boost::dynamic_pointer_cast<BMsc> (msc_flows[0]);
  
  if(flow == NULL)
  {
    print_report(RS_ERROR, L"Error: A type of the flow MSC has to be BMSC.");
    return NULL; 
  }

  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc> (specification);
  
  MembershipContext* c = new MembershipContext();
  c->set_mem(this);
  c->set_diff_type(MESSAGE);

  if (bmsc == NULL)
  {
    HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc > (specification);

    return hmsc_bmsc_diff(c, hmsc, flow);
  }
  else
    return bmsc_bmsc_diff(c, bmsc, flow);
}

//Instance "a" - flow
//Instance "b" - specification
Difference* instance_diff(MembershipContext* c, InstancePtr a, InstancePtr b)
{
  std::vector<StrictEventPtr> a_ordering;
  std::vector<StrictEventPtr> b_ordering;
  int row = 0;
  int col = 0;

  EventAreaPtr a_area = a->get_first();
  EventAreaPtr b_area = b->get_first();

  StrictOrderAreaPtr a_soa = boost::dynamic_pointer_cast<StrictOrderArea > (a_area);
  StrictOrderAreaPtr b_soa = boost::dynamic_pointer_cast<StrictOrderArea > (b_area);

  StrictEventPtr event_a = a_soa!=NULL ? a_soa->get_first() : NULL; 
  StrictEventPtr event_b = b_soa!=NULL ? b_soa->get_first() : NULL;

  StrictEventPtr start_a, start_b;

  if (event_a != NULL)
    start_a = event_a;
  if (event_b != NULL)
    start_b = event_b;

  while (event_a != NULL && event_b != NULL)
  {
    if (!event_comparison(c, event_a, event_b))
        break;

    row++;
    
//    if(c->get_diff_type() == MESSAGE)
//      set_identification(c, event_a.get(), event_b.get());
  
    event_a = event_a->get_successor();
    event_b = event_b->get_successor();

    //the algorithm skips REMOVED events in attributes checking part
    while(event_a != NULL && event_a->get_marked() == REMOVED)
    {
      event_a = event_a->get_successor();
    }
  }

  int lower, upper;

  if (event_a == NULL)
    lower = 1;
  else
    lower = -1;

  if (event_b == NULL)
    upper = -1;
  else
    upper = 1;

  if (lower > upper)
    //instances are same
    return NULL;

  int d; //current edit distance
  std::map<int, Distance*> last_d;
  std::map<int, Difference*> script;

  last_d[0] = new Distance(event_a, row);
  script[0] = NULL;
  col = row;

  for (d = 1; d < 100; d++)
  {
    for (int k = lower; k <= upper; k += 2)
    {
      Difference* diff = new Difference();
      std::map<int, Distance*>::iterator down, up;
      down = last_d.find(k - 1);
      up = last_d.find(k + 1);
      bool result = false;

      if (down == last_d.end() || (up != last_d.end() && up->second->getLine() >= down->second->getLine()))
        result = true;

      StrictEventPtr old_a;
      if ((k == -d || k != d) && result)
      {
        // moving down from the last d-1 on diagonal k+1
        // puts you farther along diagonal k than does
        // moving right from the last d-1 on diagonal k-1
        if (up != last_d.end())
        {
          old_a = up->second->getEvent();
          event_a = up->second->getEvent()->get_successor();
          row = up->second->getLine() + 1;
        }
        else
        {
          row = 1;

          if (start_a != NULL)
            event_a = start_a->get_successor();
          else
          {
            //TODO cover the error message
            throw std::runtime_error("Unexpected behaviour.");
          }
          old_a = event_a;
        }

        if (script.find(k + 1) != script.end())
          diff->setPrevious(script[k + 1]);

        diff->setOperation(REMOVE);
        diff->setLocation(old_a);
        diff->setMessage(old_a->get_message(), old_a->is_send());
      }
      else
      {
        // move right from the last d-1 on diagonal k-1
        if (down != last_d.end())
        {
          old_a = down->second->getEvent();
          event_a = down->second->getEvent();
          row = down->second->getLine();
        }
        else
        {
          row = 1;

          if (start_a != NULL)
            event_a = start_a->get_successor();
          else
          {
            //TODO cover the error message
            throw std::runtime_error("Unexpected behaviour.");
          }
          old_a = event_a;
        }

        if (script.find(k - 1) != script.end())
          diff->setPrevious(script[k - 1]);

        diff->setOperation(ADD);

        //TODO nemozem takto prechadzat eventy od zaciatku
        StrictEventPtr missing = start_b;
        for (int i = 0; i < row+k-1; i++)
        {
          missing = missing->get_successor();
        }

        //TODO nemozem takto prechadzat eventy od zaciatku
        //row must be decreased becasue location = start_a is the first iteration
        StrictEventPtr location = start_a;
        for (int i = 0; i < row-1; i++)
        {
          location = location->get_successor();

//TODO check this
          if(location->get_marked() == REMOVED)
            i--;
        }
        if(row <= 0)
          location = NULL;

        diff->setLocation(location);
        diff->setMessage(missing->get_message(), missing->is_send());

        if(c->get_diff_type() == ATTRIBUTE)
        {
          EventPtr missing_match_event = missing->get_matching_event();

          if(missing_match_event == NULL)
            throw std::runtime_error("Unexpected bahaviour.");

          bool set_value = false; 
          diff->setAttribute(missing_match_event->get_attribute("identification", -1, set_value));

          if(set_value)
            throw std::runtime_error("Unexpected bahaviour.");
        }
      }

      int old_col = col;
      col = row + k;
      int rest = col - old_col;

      if (rest > 0)
      {
        for (int i = 0; i < rest; i++)
        {
          if (event_b != NULL)
            event_b = event_b->get_successor();
        }
      }
      else
      {
        for (int i = rest; i < 0; i++)
        {
          if (event_b != NULL)
            event_b = event_b->get_predecessor();
          else
            event_b = b_soa->get_last();
        }
      }

      diff->setLine1(row);
      diff->setLine2(col);
      script[k] = diff;


      while (event_a != NULL && event_b != NULL && event_comparison(c, event_a, event_b))
      {
//        if(c->get_diff_type() == MESSAGE)
//          set_identification(c, event_a.get(), event_b.get());

        event_a = event_a->get_successor();
        event_b = event_b->get_successor();
        row++;
        col++;
      }

      last_d[k] = new Distance(event_a, row);

      if (event_a == NULL && event_b == NULL)
        // hit southeast corner, have the answer
        return script[k];

      if (event_a == NULL)
        // hit the last row, don't look to the left
        lower = k + 2;

      if (event_b == NULL)
        // hit the last column, don't look to the right
        upper = k - 2;
    }

    --lower;
    ++upper;
  }

  throw std::runtime_error("Unexpected behaviour.");
}

BMscPtr bmsc_bmsc_diff(MembershipContext* c, BMscPtr specification, BMscPtr flow)
{
  BMscDuplicator duplicator;
  BMscPtr dup_flow;

  dup_flow = duplicator.duplicate(flow);
  c->set_bmsc(dup_flow);
  c->set_specification(specification);

  InstancePtrList spec_inst = specification->get_instances();
  InstancePtrList flow_inst = dup_flow->get_instances();

  std::map<std::wstring, Difference*> diff_map;

  InstancePtrList::iterator spec_it;
  InstancePtrList::iterator flow_it;
  
  std::set<std::wstring> all_instances;
  std::set<std::wstring>::iterator all_inst_it;

  for(spec_it = spec_inst.begin(); spec_it != spec_inst.end(); spec_it++)
  {
    all_inst_it = all_instances.find((*spec_it)->get_label());
    
    if(all_inst_it == all_instances.end())
      all_instances.insert((*spec_it)->get_label());
  }

  for (flow_it = flow_inst.begin(); flow_it != flow_inst.end(); flow_it++)
  {
    all_inst_it = all_instances.find((*flow_it)->get_label());

    if(all_inst_it == all_instances.end())
      all_instances.insert((*flow_it)->get_label());
  }

  bool flow_found = false;
  bool spec_found = false;

for(int i = 0; i < 2; i++)
{
  diff_map.clear();

  for(all_inst_it = all_instances.begin(); all_inst_it != all_instances.end(); all_inst_it++)
  {
    spec_found = false; 
    flow_found = false; 

    //spec_it is pointing to the specification instance which has the proper name
    for(spec_it = spec_inst.begin(); spec_it != spec_inst.end(); spec_it++)
    {
      if((*spec_it)->get_label() == *all_inst_it)
      {
        spec_found = true;
        break;
      }
    }

    //flow_it is pointing to the flow instance which has the proper name
    for (flow_it = flow_inst.begin(); flow_it != flow_inst.end(); flow_it++)
    {
      if((*flow_it)->get_label() == *all_inst_it)
      {
        flow_found = true;
        break;
      }
    }

    //when some instance of the flow is not comntained in the specification
    if(!spec_found)
    {
      StrictOrderAreaPtr s_e_a = boost::dynamic_pointer_cast<StrictOrderArea> ((*flow_it)->get_first());
      StrictEventPtr s_e = s_e_a==NULL ? NULL : s_e_a->get_first();
      Difference* old_diff = NULL;

      while(s_e != NULL)
      {
        Difference* diff = new Difference();

        diff->setPrevious(old_diff);
        diff->setOperation(REMOVE);
        diff->setLocation(s_e);
        diff->setMessage(s_e->get_message(), s_e->is_send());

        old_diff = diff;

        s_e = s_e->get_successor();

        if(s_e == NULL)
        {
          s_e_a = boost::dynamic_pointer_cast<StrictOrderArea> (s_e_a->get_next());
          s_e = s_e_a==NULL ? NULL : s_e_a->get_first();
        }
      }

      if(old_diff != NULL)
        diff_map.insert(std::make_pair(*all_inst_it, old_diff));
    }

    //when some instance of the specification is not contained in the flow
    if(!flow_found)
    {
      StrictOrderAreaPtr s_e_a = boost::dynamic_pointer_cast<StrictOrderArea> ((*spec_it)->get_first());
      StrictEventPtr s_e = s_e_a==NULL ? NULL : s_e_a->get_first();
      Difference* old_diff = NULL;
 
      while(s_e != NULL)
      {
        Difference* diff = new Difference();

        diff->setPrevious(old_diff);
        diff->setOperation(ADD);
        diff->setLocation(NULL);
        diff->setMessage(s_e->get_message(), s_e->is_send());

        old_diff = diff;

        s_e = s_e->get_successor();
        
        if(s_e == NULL)
        {
          s_e_a = boost::dynamic_pointer_cast<StrictOrderArea> (s_e_a->get_next());
          s_e = s_e_a==NULL ? NULL : s_e_a->get_first();
        }
      }

      if (old_diff != NULL)
        diff_map.insert(std::make_pair(*all_inst_it, old_diff));
    }

    if(!spec_found || !flow_found)
      continue;
    
    //when the instance of proper name was found in both MSCs (specification, flow) 
    Difference* diff = instance_diff(c, *flow_it, *spec_it);

    if(diff != NULL)
      diff_map.insert(std::make_pair((*flow_it)->get_label(), diff));
  }

  if(i == 0)
    diff_identification_settings(c, diff_map);
}


  if(!diff_map.empty())
  {
    process_diffrences(c, dup_flow, diff_map);
    check_attributes(c, specification, dup_flow);

    c->clear_attributed_events();
    return dup_flow;
  }
  else
  {
    if(check_attributes(c, specification, dup_flow))
    {
      c->clear_attributed_events();
      return dup_flow;
    }
    else
    {
      c->clear_attributed_events();
      return NULL;
    }
  }
}

BMscPtr hmsc_bmsc_diff(MembershipContext* c, HMscPtr specification, BMscPtr flow)
{
  c->get_mem()->print_report(RS_ERROR, L"unsupported function: For Diff version of Find Flow algorithm, the type of specification MSC has to be BMSC.");
  return NULL;
}

StrictOrderAreaPtr get_proper_area(BMscPtr flow, Difference* diff, std::wstring inst_name)
{
  InstancePtrList instances = flow->get_instances();
  InstancePtrList::iterator instances_it;
  
  for(instances_it = instances.begin(); instances_it != instances.end(); instances_it++)
  {
    if((*instances_it)->get_label() == inst_name)
      break;
  }

  EventAreaPtr e_a;
  StrictOrderAreaPtr s_e_a;

  if(instances_it == instances.end())
  { 
    InstancePtr original; 

    CompleteMessagePtr com_msg = boost::dynamic_pointer_cast<CompleteMessage>(diff->getMessage());

    if(com_msg != NULL)
    {
      if(diff->getDirection() == SEND)
        original = com_msg->get_sender();
      else
        original = com_msg->get_receiver();
    }
    else
    {
      IncompleteMessagePtr incom_msg = boost::dynamic_pointer_cast<IncompleteMessage>(diff->getMessage());
      original = incom_msg->get_event()->get_instance();
    }

    InstancePtr inst = new Instance(inst_name);
    inst->set_line_begin(original->get_line_begin());
    inst->set_line_end(original->get_line_end());

    flow->add_instance(inst);
    s_e_a = new StrictOrderArea();
    s_e_a->set_instance(inst.get());
    inst->set_first(s_e_a);
  }
  else
  {
    EventAreaPtr e_a = (*instances_it)->get_first();
  
    if(e_a == NULL)
    {
      s_e_a = new StrictOrderArea();
      s_e_a->set_instance(instances_it->get());
      (*instances_it)->set_first(s_e_a);
    }
    else
      s_e_a = boost::dynamic_pointer_cast<StrictOrderArea> (e_a);
  }

  if(s_e_a == NULL)
    throw std::runtime_error("Unexpected behaviour, check whether flow MSC does not contain coregion.");

  return s_e_a;
}

void set_event_position(StrictEventPtr e, StrictEventPtr location, std::wstring inst_name, BMscPtr flow)
{
  InstancePtrList insts = flow->get_instances();
  InstancePtrList::iterator it;
  InstancePtr inst = NULL;

  for(it = insts.begin(); it != insts.end(); it++)
  {
    if((*it)->get_label() == inst_name)
    {
      inst = *it; 
      break;
    }
  }

  MscPoint position, start_p, end_p;

  if(inst == NULL)
    throw std::runtime_error("Unexpected behaviour.");

  if(location == NULL)
  {
    start_p = MscPoint(0,0);

    //TODO coregion area
    StrictOrderAreaPtr s_e_a = boost::dynamic_pointer_cast<StrictOrderArea>(inst->get_first());

    if(s_e_a == NULL || s_e_a->get_first() == NULL)
      end_p = MscPoint(0, inst->get_line_end().get_y()-inst->get_line_begin().get_y());
    else
      end_p = s_e_a->get_first()->get_position();
  }
  else
  {
    start_p = location->get_position();
    StrictEventPtr original_successor = location->get_successor();

    if(original_successor != NULL)
      end_p = original_successor->get_position();
    else
      end_p = MscPoint(0, inst->get_line_end().get_y()-inst->get_line_begin().get_y());
  }

  double x = start_p.get_x() + (end_p.get_x() - start_p.get_x())/2;
  double y = start_p.get_y() + (end_p.get_y() - start_p.get_y())/2;
  position = MscPoint(x,y);
  e->set_position(position);
}

/*
 * create new event and incomplete message 
 * then add it to the proper ordering
 *
 * parameters - bmsc 
 *            - diff, difference to process
 *            - msg, original message which is supposed to add
 *            - ins_name 
 */
void add_incomplete_message(MembershipContext* c, BMscPtr dup_flow, Difference* diff, IncompleteMessagePtr msg, std::wstring ins_name)
{
  StrictEventPtr new_e = new StrictEvent();
  set_event_position(new_e, diff->getLocation(), ins_name, dup_flow);

  IncompleteMessagePtr new_msg = new IncompleteMessage(
                                       msg->is_lost() ? LOST : FOUND,
                                       msg->get_label(),
                                       msg->get_instance_label());

  new_msg->glue_event(new_e);

  if(diff->getLocation() != NULL)
    diff->getLocation()->set_successor(new_e);
  else
    get_proper_area(dup_flow, diff, ins_name)->set_first(new_e);

  set_identification(c, new_e.get(), msg->get_event());
}

StrictEventPtr create_event(BMscPtr dup_flow, Difference* diff, std::wstring inst_name)
{
  StrictEventPtr predecessor = diff->getLocation();
  StrictOrderAreaPtr strict_e_a;

  if(predecessor == NULL)
    strict_e_a = get_proper_area(dup_flow, diff, inst_name);
  else
    strict_e_a = predecessor->get_area();

  StrictEventPtr new_e = new StrictEvent();
  set_event_position(new_e, diff->getLocation(), inst_name, dup_flow);

  new_e->set_area(strict_e_a.get());
  new_e->set_marked(ADDED);

  if(predecessor == NULL)
    strict_e_a->set_first(new_e);
  else
    predecessor->set_successor(new_e);

  return new_e;
}

void process_diffrences(MembershipContext* c, BMscPtr dup_flow, const std::map<std::wstring, Difference*>& diff_map)
{
  std::map<std::wstring, std::list<Difference*> > insert_map;
  insert_map = create_insert_map(c, diff_map);

  std::map<std::wstring, std::list<Difference*> >::iterator ins_map_it;
  std::map<std::wstring, std::set<Unmatched*> > unmatched_map;
  std::map<std::wstring, std::set<Unmatched*> >::iterator unmatched_map_it;

  for (ins_map_it = insert_map.begin(); ins_map_it != insert_map.end(); ins_map_it++)
  {
    std::list<Difference*> insert_list = ins_map_it->second;
    std::list<Difference*>::iterator ins_list_it;

    for (ins_list_it = insert_list.begin(); ins_list_it != insert_list.end(); ins_list_it++)
    {
      MscMessagePtr message = (*ins_list_it)->getMessage();
      CompleteMessagePtr com_msg = boost::dynamic_pointer_cast<CompleteMessage> (message);

      if (com_msg == NULL)
      {
        IncompleteMessagePtr in_msg = boost::dynamic_pointer_cast<IncompleteMessage > (message);

	add_incomplete_message(c, dup_flow, (*ins_list_it), in_msg, ins_map_it->first);
        continue;
      }


      //checks whether exists events which is supposed to be connected with instance
      unmatched_map_it = unmatched_map.find(ins_map_it->first);

      //create new record in unmatched_map
      if(unmatched_map_it == unmatched_map.end())
      {
        std::wstring match_inst;

        if ((*ins_list_it)->getDirection() == SEND)
          match_inst = com_msg->get_receiver()->get_label();
        else
          match_inst = com_msg->get_sender()->get_label();
        
        //check the matching instance, whether has a diffrence for creation of the matching event
        //if the diffrence exist, create unmatched record
        Unmatched* unmatched;

        if(check_matching_event_creation(insert_map, *ins_list_it))
        {
          StrictEventPtr new_e = create_event(dup_flow, *ins_list_it, ins_map_it->first);
          unmatched = new Unmatched(new_e, message);  
        }
        else
          continue;

        std::map<std::wstring, std::set<Unmatched*> >::iterator match_it = unmatched_map.find(match_inst);
 
        //add to unmatched
        if(match_it == unmatched_map.end())
        {
          std::set<Unmatched*> unmatched_set;
          unmatched_set.insert(unmatched);
          unmatched_map.insert(std::make_pair(match_inst, unmatched_set));
        }
        else
          match_it->second.insert(unmatched);
        continue;
      }

      bool was_created = false;
      std::set<Unmatched*> unmatched_set = unmatched_map_it->second;
      std::set<Unmatched*>::iterator unmatched_set_it;
      
      for(unmatched_set_it = unmatched_set.begin(); unmatched_set_it != unmatched_set.end(); unmatched_set_it++)
      {
        if(com_msg == (*unmatched_set_it)->getMessage())
        {
          StrictEventPtr new_e = create_event(dup_flow, *ins_list_it, ins_map_it->first);
          StrictEventPtr match_e = (*unmatched_set_it)->getEvent();

          CompleteMessagePtr new_msg = new CompleteMessage(com_msg->get_label());
          new_msg->set_marked(ADDED);

          if((*ins_list_it)->getDirection() == SEND)
            new_msg->glue_events(new_e, match_e);
          else
            new_msg->glue_events(match_e, new_e);
 
          if(new_e->is_send())
          {
            set_identification(c, new_e.get(), com_msg->get_send_event());
            set_identification(c, match_e.get(), com_msg->get_receive_event());
          }
          else
          {
            set_identification(c, match_e.get(), com_msg->get_send_event());
            set_identification(c, new_e.get(), com_msg->get_receive_event());
          }

          was_created = true;
          break;
        }
      }

      if(was_created)
      {
        unmatched_set.erase(unmatched_set_it);
        unmatched_map_it->second = unmatched_set;
      }
      else
      {
        std::wstring match_inst;

        if ((*ins_list_it)->getDirection() == SEND)
          match_inst = com_msg->get_receiver()->get_label();
        else
          match_inst = com_msg->get_sender()->get_label();

        Unmatched* unmatched;

        if(check_matching_event_creation(insert_map, *ins_list_it))
        {
          StrictEventPtr new_e = create_event(dup_flow, *ins_list_it, ins_map_it->first);
          unmatched = new Unmatched(new_e, message);
        }
        else
          continue; 

        std::map<std::wstring, std::set<Unmatched*> >::iterator match_it = unmatched_map.find(match_inst);

        if(match_it == unmatched_map.end())
        {
          std::set<Unmatched*> match_inst_unmatched_set;
          match_inst_unmatched_set.insert(unmatched);
          unmatched_map.insert(std::make_pair(match_inst, match_inst_unmatched_set));
        }
        else
          match_it->second.insert(unmatched);
      }
    }
  }

  //check unmatched diffrences in unmatched map
  for (unmatched_map_it = unmatched_map.begin(); unmatched_map_it != unmatched_map.end(); unmatched_map_it++)
  {
    std::set<Unmatched*> unmatched_set = unmatched_map_it->second;
    std::set<Unmatched*>::iterator unmatched_set_it;

    if(!unmatched_set.empty())
      throw std::runtime_error("Unexpected behaviour.");
 
    unmatched_set.clear();
  }

}

bool check_attributes(MembershipContext* c, BMscPtr specification, BMscPtr dup_flow)
{
  std::map<std::wstring, Difference*> diff_map;

  InstancePtrList spec_insts = specification->get_instances();
  InstancePtrList flow_insts = dup_flow->get_instances();
  InstancePtrList::iterator flow_it;
  InstancePtrList::iterator spec_it;
  bool found = false;

  c->set_diff_type(ATTRIBUTE);

  //TODO prerobit ako pri prvej kontrole all_instances
  for(spec_it = spec_insts.begin(); spec_it != spec_insts.end(); spec_it++)
  {
    for(flow_it = flow_insts.begin(); flow_it != flow_insts.end(); flow_it++)
    {
      if((*flow_it)->get_label() == (*spec_it)->get_label())
      {
        Difference* diff =  instance_diff(c, *flow_it, *spec_it);

        if(diff != NULL)
          diff_map.insert(std::make_pair((*flow_it)->get_label(), diff));

        found = true;
        break;
      }
    }

    if(!found)
      throw std::runtime_error("Unexpected behaviour.");
  }

  //process_diffrences
  if(!diff_map.empty())
  {
//TODO much of the following code was copied from proccess diffrences 
  std::map<std::wstring, std::list<Difference*> > insert_map;
  insert_map = create_insert_map(c, diff_map);

  std::map<std::wstring, std::list<Difference*> >::iterator ins_map_it;

  for (ins_map_it = insert_map.begin(); ins_map_it != insert_map.end(); ins_map_it++)
  {
    std::list<Difference*> insert_list = ins_map_it->second;
    std::list<Difference*>::iterator ins_list_it;

    for (ins_list_it = insert_list.begin(); ins_list_it != insert_list.end(); ins_list_it++)
    {
      MscMessagePtr message = (*ins_list_it)->getMessage();
      CompleteMessagePtr com_msg = boost::dynamic_pointer_cast<CompleteMessage> (message);

      if (com_msg == NULL)
        throw std::runtime_error("Unexpected behaviour.");

      std::wstring match_inst;
      std::wstring inst;

      if((*ins_list_it)->getDirection() == SEND)
      {
        inst = com_msg->get_sender()->get_label();
        match_inst = com_msg->get_receiver()->get_label();
      }
      else
      {
        match_inst = com_msg->get_sender()->get_label();
        inst = com_msg->get_receiver()->get_label();
      }

      MscMessagePtr m = (*ins_list_it)->getMessage();
      CompleteMessagePtr commplete = boost::dynamic_pointer_cast<CompleteMessage>(m);
      EventPtr e = commplete->get_send_event();
      Event* match_e = find_event_on_instance_by_id(c, match_inst, (*ins_list_it)->getAttribute());

      StrictEvent* match_se = dynamic_cast<StrictEvent*>(match_e);

      if(match_se == NULL)
        throw std::runtime_error("Unexpected behaviour.");

      StrictEventPtr new_match_e = new StrictEvent();
      set_event_position(new_match_e, match_se, match_inst, dup_flow);

      new_match_e->set_area(match_se->get_area());
      new_match_e->set_marked(ADDED);

      match_se->set_successor(new_match_e);

      StrictEventPtr new_e = create_event(dup_flow, *ins_list_it, inst);

      CompleteMessagePtr new_msg = new CompleteMessage((*ins_list_it)->getMessage()->get_label());
      new_msg->set_marked(ADDED);

      new_msg->glue_events(new_match_e, new_e);
     }    
   }

//-------------------------------------------------------------------    
    return true;
  }
  return false;
}

bool event_comparison(MembershipContext* c, StrictEventPtr& event_a, StrictEventPtr event_b)
{
  if(c->get_diff_type() == MESSAGE)
    return compare_events(c, event_a.get(), event_b.get());
  else
  {
    while(event_a != NULL && event_a->get_marked() == REMOVED)
    {
      event_a = event_a->get_successor();
    }

    int sum = (event_a == NULL) + (event_b == NULL);

    switch (sum)
    {
      case 0: break;
      case 1: return false;
      case 2: return true;
    } 

    return compare_events_attribute(c, event_a.get(), event_b.get());
  }
}

bool check_matching_event_creation(std::map<std::wstring, std::list<Difference*> > insert_map, Difference* diff)
{
  std::wstring match_inst_name;

  CompleteMessagePtr complete = boost::dynamic_pointer_cast<CompleteMessage>(diff->getMessage());

  if(complete == NULL)
    throw std::runtime_error("Unexpected behaviour.");

  if(diff->getDirection() == SEND)
    match_inst_name = complete->get_receiver()->get_label();
  else
    match_inst_name = complete->get_sender()->get_label();

  std::map<std::wstring, std::list<Difference*> >::iterator it;
  it = insert_map.find(match_inst_name);

  if(it != insert_map.end())
  {
    std::list<Difference*> insert_list = it->second;
    std::list<Difference*>::iterator insert_list_it;
    
    for(insert_list_it = insert_list.begin(); insert_list_it != insert_list.end(); insert_list_it++)
    {
      if((*insert_list_it)->getMessage() == diff->getMessage())
        return true;
    }
  }

  return false;
}

std::map<std::wstring, std::list<Difference*> > create_insert_map(MembershipContext* c,
                                                                  const std::map<std::wstring, Difference*>& diff_map)
{
  std::map<std::wstring, Difference*>::const_iterator it;
  std::map<std::wstring, std::list<Difference*> > insert_map;
  Difference* diff;

  for (it = diff_map.begin(); it != diff_map.end(); it++)
  {
    std::list<Difference*> insert_list;
    diff = it->second;

    if (diff == NULL)
      continue;

    while (diff != NULL)
    {
      if (diff->getOperation() == REMOVE)
      {
        if(c->get_diff_type() == ATTRIBUTE || diff->getLocation()->is_send())
        {
          diff->getLocation()->set_marked(REMOVED);
          EventPtr match_e = diff->getLocation()->get_matching_event();

          if(match_e != NULL)
            match_e->set_marked(REMOVED);

          diff->getLocation()->get_message()->set_marked(REMOVED);
        }
      }
      else
        insert_list.push_back(diff);

      diff = diff->getPrevious();
    }

    insert_map.insert(std::make_pair(it->first, insert_list));
  }

  return insert_map;
}

void diff_identification_settings(MembershipContext* c, const std::map<std::wstring, Difference*>& diff_map)
{
  std::map<std::wstring, Difference*>::const_iterator map_it;
  std::vector<Difference*> inst_diff; 
  InstancePtr spec_inst, flow_inst;
  StrictEventPtr spec_e, flow_e;
  Difference* diff;

  BMscPtr specification = boost::dynamic_pointer_cast<BMsc> (c->get_specification());

  if(specification == NULL)
    return ;

  InstancePtrList instances = specification->get_instances();
  InstancePtrList::iterator instances_it;

  for(instances_it = instances.begin(); instances_it != instances.end(); instances_it++)
  {
    inst_diff.clear();
    diff = NULL;

    map_it = diff_map.find((*instances_it)->get_label());
    
    if(map_it != diff_map.end())
      diff = map_it->second;

    while(diff != NULL)
    {
      inst_diff.push_back(diff);
      diff = diff->getPrevious();
    }

    spec_inst = find_instance(boost::dynamic_pointer_cast<BMsc> (c->get_specification()), (*instances_it)->get_label());
    flow_inst = find_instance(c->get_bmsc(), (*instances_it)->get_label());

    if(spec_inst == NULL || flow_inst == NULL)
      continue;

    StrictOrderAreaPtr spec_ea = boost::dynamic_pointer_cast<StrictOrderArea>(spec_inst->get_first());
    StrictOrderAreaPtr flow_ea = boost::dynamic_pointer_cast<StrictOrderArea>(flow_inst->get_first());

    if(spec_ea == NULL || flow_ea == NULL)
      continue;

    spec_e = spec_ea->get_first();
    flow_e = flow_ea->get_first();

    //set identification to proper events
    std::vector<Difference*>::reverse_iterator vec_it = inst_diff.rbegin();
    while(spec_e != NULL && flow_e != NULL)
    {
     if(vec_it != inst_diff.rend())
     {
       if((*vec_it)->getOperation() == REMOVE)
       {
	 if((*vec_it)->getLocation() == flow_e)
	 {
         //due to some differences (REMOVE and ADD) can have same location
         //in case the following diffrence has the same location flow_e is not changing
         if(vec_it + 1 == inst_diff.rend() ||
            (vec_it + 1 != inst_diff.rend() && (*(vec_it+1))->getLocation() != (*vec_it)->getLocation()))
         {
              flow_e = flow_e->get_successor();
              vec_it++;
              continue;
         }
	 }
       }
       else
       {
         if((*vec_it)->getLocation() == NULL || (*vec_it)->getLocation() == flow_e->get_predecessor())
         {
           spec_e = spec_e->get_successor();

           //due to in some cases the flow_e was not changed
           //in case that flow_e was not changed and the location of the following diffrence is not same
           //it is neccessary to change the flow_e too
           if(vec_it != inst_diff.rbegin() &&
               (*(vec_it-1))->getLocation() == (*vec_it)->getLocation() &&
               (*vec_it)->getLocation() != NULL &&
               (vec_it+1 == inst_diff.rend() || (*vec_it)->getLocation() != (*(vec_it+1))->getLocation()))
           {
              flow_e = flow_e->get_successor();
           }
           vec_it++;
           continue; 
         }
       }
     }


      set_identification(c, spec_e.get(), flow_e.get());
      spec_e = spec_e->get_successor();
      flow_e = flow_e->get_successor();
    }
  }
}

InstancePtr find_instance(BMscPtr bmsc, std::wstring name)
{
  if(bmsc == NULL)
    return NULL;

  InstancePtrList insts = bmsc->get_instances();
  InstancePtrList::iterator it;

  for(it = insts.begin(); it != insts.end(); it++)
  {
    if((*it)->get_label() == name)
      return *it;
  }

  return NULL;
}
