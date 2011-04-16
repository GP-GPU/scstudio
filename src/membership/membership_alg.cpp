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

#include "membership/membership_alg.h"
#include "check/time/time_pseudocode.h"
#include "check/time/tightening.h"
#include <map>

/*
 *  Returns precondition list of the membership algorithm
 */
Searcher::PreconditionList MembershipAlg::get_preconditions(MscPtr msc) const
{
  Searcher::PreconditionList result;

  result.push_back(PrerequisiteCheck(L"Unique instance names", PrerequisiteCheck::PP_REQUIRED));
  result.push_back(PrerequisiteCheck(L"Correct Time Constraint Syntax", PrerequisiteCheck::PP_REQUIRED));
  result.push_back(PrerequisiteCheck(L"Time Consistent", PrerequisiteCheck::PP_REQUIRED));

  return result;
}

/*
 *  Tries finding bMsc in Msc 
 */
MscPtr MembershipAlg::find(MscPtr msc, MscPtr bmsc, std::vector<std::wstring> instances)
{
  HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc > (msc);
  BMscPtr bmsc_f = boost::dynamic_pointer_cast<BMsc > (bmsc);

  if (bmsc_f == NULL)
  {
    print_report(RS_ERROR, L"Flow MSC has to be bMSC.");
    return NULL;
  }

  MembershipContext* c = new MembershipContext();
  c->set_mem(this);
  c->set_focused_instances(instances);
  c->set_bmsc(bmsc_f);
  MscPtr result = NULL;

  if (hmsc != NULL)
    result = search_hmsc(c, hmsc, bmsc_f);
  else
  {
    BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc > (msc);
    if(bmsc != NULL)
      result = search_bmsc(c, bmsc, bmsc_f);
    else
    {
      print_report(RS_ERROR, L"Find flow algorithm cannot recognize a type of browsed MSC.");
      return NULL;
    }
  }

  delete c;
  return result;
}

MscPtr MembershipAlg::find(MscPtr msc, MscPtr bmsc)
{
  std::vector<std::wstring> instances;
  return this->find(msc, bmsc, instances);
}

MscPtr MembershipAlg::find(MscPtr msc, std::vector<MscPtr>& bmscs)
{
  MscPtr temp;
  bool found = false;

  for(unsigned int i = 0; i < bmscs.size(); i++)
  {
    temp = NULL;
    temp = find(msc, bmscs[i]);

    if(temp == NULL)
      print_report(RS_ERROR, L"HMsc specification does not contain \"" + bmscs[i]->get_label() + L"\" flow.");
    else
      print_report(RS_NOTICE, L"Flow \"" + bmscs[i]->get_label() + L"\" meets the specification.");
  }

  if(found)
    return msc;
  else
    return NULL;
}

/*
 *  Tries to find bMSC in HMSC
 */
MscPtr search_hmsc(MembershipContext* c, HMscPtr hmsc, BMscPtr bmsc_f)
{
  BMscGraphDuplicator duplicator;
  HMscPtr dup_hmsc;

  //TODO it is necessary to copy MSC because of path returning.
  //dup_hmsc should be MSC graph. Due to that duplicatior throws exception during copying time intervals, membership workes just with MSC graphs.
  dup_hmsc = hmsc; 
//  dup_hmsc = dup_hmsc = duplicator.duplicate(hmsc);

  //get time matrix
  BMscIntervalSetMatrix time_matrix2 = get_bmsc_matrix(bmsc_f);
  c->set_time_matrix(&time_matrix2);

  //creates configuration of defined bMSC
  ConfigurationPtr searched_conf = new Configuration(bmsc_f->get_instances());

  StartNodePtr start_node = dup_hmsc->get_start();

  //strt of searching
  if (check_branch(c, start_node, searched_conf))
    return make_result(c, dup_hmsc);
  else
    return NULL;

  return NULL;
}

/*
 * Checks branch if the communication is coressponding
 *
 * parameters : c - membership context 
 *              node - reference node 
 *              b - MSC configuration for search
 */
bool check_branch(MembershipContext* c, HMscNodePtr hmsc_node, ConfigurationPtr b)
{
  //add node to the path which will be printed if the flow meets specification
  c->push_path(hmsc_node);

  //checks end node
  EndNodePtr end = boost::dynamic_pointer_cast<EndNode > (hmsc_node);

  if (end != NULL)
  {
    //remove attributes from previous computation
    if(!c->get_attributed_events().empty()){
      c->clear_attributed_events();
    }

    if (b->is_null(c))
      return true;
    else
    {
      c->top_pop_path();
      return false;
    }
  }

  ReferenceNodePtr node = boost::dynamic_pointer_cast<ReferenceNode > (hmsc_node);

  //if the type of the node is not reference, look at the successors
  if (node == NULL)
    return check_next(c, hmsc_node, b);
  else
  {
    //when deosn't exist empty path to EndNode
    if (b->is_null(c) && !is_node_null(c, node))
    {
      c->top_pop_path();
      return false;
    }

    //checks if this node with this configuration was checked
    if (!look_at_checked_conf(c, node, b))
    {
      c->top_pop_path();
      return false;
    }

    ConfigurationPtr old_config_I, old_config_II;

    old_config_I = new Configuration(b);
    old_config_II = new Configuration(b);

    //tries to matching events between searched MSC and bMSC which is looked for
    if (!check_node(c, node, membership, b))
    {
      c->clear_attributed_events();
      c->top_pop_path();
      return false;
    }

    //if there are not time constraints or there are relative and absolut time constraints, skip the time checking
    if(c->get_absolut_time() != c->get_relative_time())
    {
      //checks time constraints for this node
      //time constraints are not checked in partial mebership algorithm
      if(c->get_focused_instances().empty())
      {
        if(!check_node_time(c, node, old_config_II))
        {
          c->clear_attributed_events();
          c->top_pop_path();
          return false;
        }
      }
    }

    //if there are absolut and relative time constraints, print error and warning
    if(c->get_absolut_time() && c->get_relative_time())
    {
      c->get_mem()->print_report(RS_ERROR, L"Error: both types of time constraints (relative, absolut) are not allowed in the flow MSC.");
      c->get_mem()->print_report(RS_WARNING, L"Warning: time checking was skipped.");
    }

    //checks the ordering of events (by attribute which was set at check_node(node, membership,b))
    if (!check_node(c, node, receive_ordering, old_config_I))
    {
      c->clear_attributed_events();
      c->top_pop_path();
      return false;
    }

    /* b and old_config_I have to be same at this moment because
    all events, which had attribute set, have to be checked
    
    old_config_II does not have to bee same as b because 
    during the time checking the algorithm has not to travers all events*/

    if(!b->compare(old_config_I)) 
    { 
      c->top_pop_path();
      return false;
    }

    NodeRelationPtrVector successors = node->get_successors();
    NodeRelationPtrVector::iterator it;
    SuccessorNode* succ;
    HMscNode* hmsc_succ;

    //continue with browsing node's seccessors
    for (it = successors.begin(); it != successors.end(); it++)
    {
      succ = (*it)->get_successor();
      hmsc_succ = dynamic_cast<HMscNode*> (succ);

      if (hmsc_succ != NULL)
      {
        ConfigurationPtr back_up = new Configuration(b);

        if (check_branch(c, hmsc_succ, b))
          return true;
        else
          b = back_up;
      }
    }
  }

  c->top_pop_path();
  return false;
}

/*
 *  Compare StrictOrderArea in bMSC from HMSC node and StrictOrderArea in bMSC from defined events
 */
bool strict_strict(MembershipContext* c, std::vector<Event*>& node_events, std::vector<Event*>& b_events, enum check_type type)
{
  if (node_events.size() != 1 || b_events.size() != 1)
    return false;

  StrictEvent* node_e = dynamic_cast<StrictEvent*> (node_events.front());
  StrictEvent* b_e = dynamic_cast<StrictEvent*> (b_events.front());

  while (node_e != NULL && b_e != NULL)
  {
    if (type == membership)
    {
      if (!compare_events(c, node_e, b_e))
        return false;

      //set identification to events
      set_identification(c, node_e, b_e);
      //check relative and absolut time constraints
      analyze_time_constraints(c, b_e);
    }
    else
    {
      if (!compare_events_attribute(c, node_e, b_e))
        return false;
    }

    node_e = node_e->get_successor().get();
    b_e = b_e->get_successor().get();
  }

  if (node_e == NULL)
    node_events.clear();

  if (b_e == NULL)
    b_events.clear();
  else
  {
    b_events.clear();
    b_events.push_back(b_e);
  }

  return true;
}

/*
 *  Compare StrictOrderArea in bMSC from HMSC node and CoretionArea in bMSC from defined events
 * 
 *  DEVELOPMENT WAS STOPPED
 */
bool strict_coregion(MembershipContext* c, StrictOrderAreaPtr node_strict, std::vector<Event*>& node_events,
                     CoregionAreaPtr b_coregion, std::vector<Event*>& b_events)
{
  c->get_mem()->print_report(RS_ERROR, L"unsuported function: Flow cannot contain coregion.");
  return false;
}

/*
 * checks attributes in guessed coregion message ordering
 */
bool check_coregion_attributes(MembershipContext* c, std::vector<Event*>& node_events, std::vector<Event*>& b_events)
{
  EventAreaPtr node_e_a;

  if(!node_events.empty())
    node_e_a = node_events[0]->get_general_area();
  else
    return false;

  CoregionAreaPtr cor = boost::dynamic_pointer_cast<CoregionArea> (node_e_a);

  if(cor == NULL)
    throw std::runtime_error("Unexpected behaviour.");

  CoregionOrderingPtr ordering = c->find_coregion_ordering(cor);

  if(ordering == NULL)
    throw std::runtime_error("Unexpected behaviour.");

  CoregionOrderingPtr back_up = new CoregionOrdering(ordering);
  Event* node_e = ordering->getFirst().get();
  StrictEvent* b_e = dynamic_cast<StrictEvent*> (b_events.front());

  while (node_e != NULL && b_e != NULL)
  {
    if (!compare_events_attribute(c, node_e, b_e))
    {
      c->add_checked_ordering(cor, new CoregionOrdering(back_up));
      return false;
    }
    ordering->removeFirst();

    if(!ordering->getOrdering().empty())
      node_e = ordering->getFirst().get();
    else
      node_e = NULL;

    b_e = b_e->get_successor().get();
  }

  node_events.clear();
  if (node_e != NULL)
     node_events.push_back(node_e);

  b_events.clear();
  if (b_e != NULL)
    b_events.push_back(b_e);

  return true;
}

/*
 * coretion_strict for receive_ordering type
 */
bool coregion_strict_receive_ordering(MembershipContext* c, std::vector<Event*>& node_events, std::vector<Event*>& b_events)
{
  if(!check_coregion_attributes(c, node_events, b_events))
  {
    Event* e = node_events.front();

    if(e == NULL)
      throw std::runtime_error("Unexpected behaviour.");

    EventAreaPtr a = e->get_general_area();
    CoregionAreaPtr cor = boost::dynamic_pointer_cast<CoregionArea>(a);
    SnapshotContextPtr snap = c->find_snapshot(cor);

    if(snap == NULL)
      throw std::runtime_error("Unexpected behaviour.");

    std::vector<Event*> snap_node_events = snap->getNodeEvents();
    std::vector<Event*> snap_b_events = snap->getBMSCEvents();

    if(c->find_coregion_ordering(cor) == NULL)
      throw std::runtime_error("Unexpected behaviour.");

    c->find_coregion_ordering(cor)->deleteOrdering();

    return coregion_strict(c, snap_node_events, snap_b_events, membership);
  }

  //check_coregion_attributes is OK
  return true;
}

/*
 *  Compare CoregionArea in bMSC from HMSC node and StrictOrderArea in bMSC from defined events
 *
 *  node_events - minimal events in coregion; 
 *  b_events - event in bmsc
 *  type - type of searching
 */
bool coregion_strict(MembershipContext* c, std::vector<Event*>& node_events, std::vector<Event*>& b_events,
                     enum check_type type)
{
  if(type == receive_ordering)
      return coregion_strict_receive_ordering(c, node_events, b_events);

  StrictEvent* b_e = dynamic_cast<StrictEvent*> (b_events.front());
  std::vector<Event*> acceptable_events;
  std::vector<Event*>::iterator node_it;
  std::vector<Event*>::iterator accept_it;

  CoregionEvent* node_e = NULL;

  if (node_events.empty())
    return true;

  Event* e = node_events.front();
  EventAreaPtr e_a = e->get_general_area();
  CoregionAreaPtr cor = boost::dynamic_pointer_cast<CoregionArea>(e_a);

  if(cor == NULL)
    throw std::runtime_error("Unexpected behaviour.");

  SnapshotContextPtr snap = c->find_snapshot(cor);
 
  if(snap == NULL) 
  {
    SnapshotContextPtr snap = new SnapshotContext(node_events, b_events);
    c->add_snapshot(cor, snap);
  }

  //it is supposed that the bMSC contains just one strict event area on one instance
  //one coregion area in HMSC cannot be sepparated into two or more event areas in bMSC
  if (b_events.front() == NULL)
  {
    b_events.clear();
    return false;
  }

  for (node_it = node_events.begin(); node_it != node_events.end(); node_it++)
  {
    if (compare_events(c, *node_it, b_e))
    {
      node_e = dynamic_cast<CoregionEvent*> (*node_it);

      if (node_e == NULL)
        return false;

      acceptable_events.push_back(*node_it);
    }
  }

  if (acceptable_events.size() == 0)
    return false;


  for (accept_it = acceptable_events.begin(); accept_it != acceptable_events.end(); accept_it++)
  {
    CoregionOrderingPtr map_ordering = c->find_coregion_ordering(cor);
    bool result = compare_events(c, *accept_it, b_e);

    if(result)
    {
      //set identification to events
      set_identification(c, *accept_it, b_e);
      //check relative and absolut time constraints
      analyze_time_constraints(c, b_e);

      CoregionEvent* cor_e = dynamic_cast<CoregionEvent*> (*accept_it);

      //save current coregion ordering 
      if(map_ordering == NULL)
      {
        std::vector<CoregionEventPtr> cor_vec;

        if(cor_e == NULL)
          throw std::runtime_error("Unexpected behaviour.");

        cor_vec.push_back(cor_e);
        c->add_coregion_ordering(cor, new CoregionOrdering(cor_vec));
      }
      else
      {
        map_ordering->addLast(cor_e);
      }

      node_e = dynamic_cast<CoregionEvent*> (*accept_it);

      //back up bMSC position
      StrictEvent* old_b_e = b_e;
      std::vector<Event*> old_node_events;

      //back up positions in coregion
      std::vector<Event*>::iterator it_o;
      for (it_o = node_events.begin(); it_o != node_events.end(); it_o++)
        old_node_events.push_back(*it_o);

      //remove eqivalent events
      for(unsigned int i = 0; i < node_events.size(); i++)
      {
        if(node_events[i] == *accept_it)
        {
          node_events.erase(node_events.begin() + i);
          break;
        }
      }

      //when the coregion area describes more that the appropriate strict area
      if(b_e->get_successor() == NULL)
      {
        b_events.clear();
        return true;
      }

      b_e = b_e->get_successor().get();

      CoregEventRelPtrVector succ = node_e->get_successors();
      CoregEventRelPtrVector::iterator succ_it;

      //add node event successors among the acceptable
      for (succ_it = succ.begin(); succ_it != succ.end(); succ_it++)
        node_events.push_back((*succ_it)->get_successor());

      b_events.clear();
      b_events.push_back(b_e);

      if (coregion_strict(c, node_events, b_events, type))
      {
        if (b_events.size() == 1 && *(b_events.begin()) == NULL)
          b_events.clear();
        
        std::vector<CoregionOrderingPtr> checked_orderings = c->find_checked_ordering(cor);

	//when it tries first ordering
        if(checked_orderings.empty())
          return true;

	if(map_ordering == NULL)
        {
          throw std::runtime_error("Unexpected behaviour.");
	  return false;
        }

        //checks whether the ordering was not consider in previous computation
        for(unsigned int i = 0; i < checked_orderings.size(); i++)
        {
          if(map_ordering->compare(checked_orderings[i]))
	  {
	    map_ordering->deleteOrdering();
            return false;
          }
        }
        return true;
      }
      else
      {
        b_e = old_b_e;
        node_events.clear();

        EventAreaPtr e_a = (*accept_it)->get_general_area();
        CoregionAreaPtr cor = boost::dynamic_pointer_cast<CoregionArea> (e_a);

        CoregionOrderingPtr map_ordering = c->find_coregion_ordering(cor);

        if(map_ordering != NULL )
          map_ordering->removeLast();

        for (it_o = old_node_events.begin(); it_o != old_node_events.end(); it_o++)
          node_events.push_back(*it_o);
      }
    }
  }

  return false;
}

/*
 *  Compare CoregionArea in bMSC from HMSC node and CoregionArea in bMSC from defined events
 *
 *  DEVELOPMENT WAS STOPPED
 */
bool coregion_coregion(MembershipContext* c, CoregionAreaPtr node_coregion, std::vector<Event*>& node_events,
                       CoregionAreaPtr b_coregion, std::vector<Event*>& b_events)
{
  c->get_mem()->print_report(RS_ERROR, L"unsuported function: Flow cannot contain coregion.");
  return false;
}

/*
 * finds instance by the name
 */ 
InstancePtr find_instance(InstancePtrList instances, std::wstring name)
{

  InstancePtrList::iterator it;

  for (it = instances.begin(); it != instances.end(); it++)
  {
    if (name == (*it)->get_label())
      return *it;
  }

  return NULL;
}

/*
 * Add checked branch to map of checked branches
 */
void add_checked_branch(MembershipContext* c, ReferenceNodePtr ref_node, ConfigurationPtr conf)
{
  BMscPtr bmsc = ref_node->get_bmsc();
  
  if(bmsc == NULL)
  {
    throw std::runtime_error("Unexpected behaviour.");
    return;
  }

  c->add_checked_conf(bmsc->get_label(), conf);  
}

/*
 * Tries finding bMSC in bMSC
 *
 * bmsc - where the bmsc_f is looking for
 * bmsc_f - pattern 
 */
MscPtr search_bmsc(MembershipContext* c, BMscPtr bmsc, BMscPtr bmsc_f)
{
  c->print_msc_path(false);

  //creates HMSC
  HMscPtr hmsc = new HMsc(L"membership");

  //creates start node
  StartNodePtr start_n = new StartNode();
  hmsc->set_start(start_n);

  //creates reference node
  ReferenceNodePtr ref_node = new ReferenceNode();
  ref_node->set_msc(bmsc);

  hmsc->add_node(ref_node);
  PredecessorNode* node = dynamic_cast<PredecessorNode*> (start_n.get());
  SuccessorNode* succ = dynamic_cast<SuccessorNode*> (ref_node.get());

  if (node != NULL && succ != NULL)
    node->add_successor(succ);
  else
  {
    throw std::runtime_error("Unexpected behaviour.");
    return NULL;
  }

  //creates end node
  EndNodePtr end_n = new EndNode();

  hmsc->add_node(end_n);
  node = dynamic_cast<PredecessorNode*> (ref_node.get());
  succ = dynamic_cast<SuccessorNode*> (end_n.get());

  if (node != NULL && succ != NULL)
    node->add_successor(succ);
  else
  {
    throw std::runtime_error("Unexpected behaviour.");
    return NULL;
  }

  return search_hmsc(c, hmsc, bmsc_f);
}

/*
 * Checks instance
 */
bool check_instance(MembershipContext* c, InstancePtr node_instance, enum check_type type, PositionPtr old_position)
{
  if(!c->get_focused_instances().empty() && !c->contain_focused_instances(node_instance->get_label()))
    return true;

  std::vector<Event*> node_events, b_events;
  EventAreaPtr node_area, b_area;
  std::wstring name;

  b_events = old_position->get_events();

  if (!b_events.size() == 1)
  {
    if(!is_instance_null(node_instance))
      return false;
    else
      return true;
  }

  if (node_instance != NULL)
    node_area = node_instance->get_first();

  //return true if the node has not event on the instance
  //because matching event to pattern bMSC can be defined in successor of node
  if (node_area == NULL && !old_position->is_empty())
    return true;

  StrictEvent* b_event = dynamic_cast<StrictEvent*> (b_events[0]);
  StrictOrderAreaPtr b_strict;
  CoregionAreaPtr node_coregion, b_coregion;

  if (b_event != NULL)
    b_area = b_strict = b_event->get_area();
  else
  {
    CoregionEvent* b_cor = dynamic_cast<CoregionEvent*> (b_events[0]);
    b_area = b_coregion = b_cor->get_area();
  }

  StrictOrderAreaPtr node_strict = boost::dynamic_pointer_cast<StrictOrderArea > (node_area);

  if (node_strict == NULL)
  {
    node_coregion = boost::dynamic_pointer_cast<CoregionArea > (node_area);

    if (node_coregion != NULL)
      node_events.insert(node_events.begin(), node_coregion->get_minimal_events().begin(),
                         node_coregion->get_minimal_events().end());
    else
      throw std::runtime_error("Unexpected behaviour.");
  }
  else
  {
    if (node_events.empty())
      node_events.push_back(node_strict->get_first().get());
    else
      throw std::runtime_error("Unexpected behaviour.");
  }

  bool result;

  while (node_area != NULL && b_area != NULL)
  {
    switch ((node_strict != NULL) + (b_strict != NULL))
    {
    case 2:
      result = strict_strict(c, node_events, b_events, type);
      break;

    case 1:
      if (node_strict != NULL)
        result = strict_coregion(c, node_strict, node_events, b_coregion, b_events);
      else
        result = coregion_strict(c, node_events, b_events, type);
      break;

    case 0:
      result = coregion_coregion(c, node_coregion, node_events, b_coregion, b_events);
      break;

    default:
      throw std::runtime_error("Unexpected behaviour.");
    }

    if (result)
    {
      if (node_events.empty())
      {
        node_area = node_area->get_next();

        if (node_area != NULL)
        {
          node_strict = boost::dynamic_pointer_cast<StrictOrderArea > (node_area);

          if (node_strict == NULL)
          {
            node_coregion = boost::dynamic_pointer_cast<CoregionArea > (node_area);
            node_events.insert(node_events.begin(), node_coregion->get_minimal_events().begin(),
                               node_coregion->get_minimal_events().end());
          }
          else
            node_events.push_back(node_strict->get_first().get());
        }
      }

      if (b_events.empty())
      {
        b_area = b_area->get_next();

        if (b_area != NULL)
        {
          b_strict = boost::dynamic_pointer_cast<StrictOrderArea > (b_area);

          if (b_strict == NULL)
          {
            b_coregion = boost::dynamic_pointer_cast<CoregionArea > (b_area);
            b_events.insert(b_events.begin(), b_coregion->get_minimal_events().begin(),
                            b_coregion->get_minimal_events().end());
          }
          else
            b_events.push_back(b_strict->get_first().get());
        }
      }
    }
    else
      return false;
  }

  if ((node_area != NULL) && (b_area == NULL))
    return false;

  if (node_area == NULL)
    old_position->set_events(b_events);

  return true;
}

/*
 * Check bMsc on which ReferenceNode references. Check_type defindes search settings 
 *
 * check_type = membership defines that the function traverses node and bMSC, 
 *              checks matching events and set attribute to receive events
 *
 * check_type = receive_ordering checks attribute of receive events whether matching is correct. 
 *
 * Membership mode: bMSCs are traversed first time, old_conf defines points 
 *                  from where it starts traverse,
 *
 * Receive_ordering mode: bMSCs are traversed second time, old_conf defines points 
 *                        from where it starts traverse,
 */
bool check_node(MembershipContext* c, ReferenceNodePtr node, enum check_type type, ConfigurationPtr conf)
{
  //get msc from the node
  ConfigurationPtr old_conf = new Configuration(conf);

  MscPtr msc = node->get_msc();
  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc > (msc);

  if (bmsc == NULL)
  {
    c->get_mem()->print_report(RS_ERROR, stringize() << L"Error: Reference node \"" << msc->get_label() << "\"does not refers to bMSC.");
    return false;
  }

  InstancePtrList node_instances = bmsc->get_instances();
  InstancePtrList b_instances = conf->get_instances();

  InstancePtrList::iterator node_instence_it;

  std::set<PositionPtr> positions = conf->get_positions();
  std::set<PositionPtr>::iterator position_it;

  if (c->get_focused_instances().empty() && node_instances.size() > b_instances.size())
  {
    if (!is_empty_instance(c, node_instances, b_instances))
      return false;
  }

  InstancePtr node_instance = NULL;
  std::wstring name;

  //checks each instances from the position to the end of bMSC or the end of the HMSC node
  for (position_it = positions.begin(); position_it != positions.end();
          position_it++)
  {
    name = (*position_it)->get_name();

    node_instance = NULL;

    for (node_instence_it = node_instances.begin();
            node_instence_it != node_instances.end(); node_instence_it++)
    {
      if (name == (*node_instence_it)->get_label())
      {
        node_instance = *node_instence_it;
        break;
      }
    }

    if (node_instance == NULL)
      continue; //remember position and continue in cycle

    if (!check_instance(c, node_instance, type, *position_it))
    {
      add_checked_branch(c, node, old_conf);
      return false;
    }
  }

  return true;
}

bool check_next(MembershipContext* c, HMscNodePtr hmsc_node, ConfigurationPtr b)
{
  PredecessorNode* pred = dynamic_cast<PredecessorNode*> (hmsc_node.get());

  if (pred == NULL)
    return false;

  NodeRelationPtrVector successors = pred->get_successors();
  NodeRelationPtrVector::iterator it;

  for (it = successors.begin(); it != successors.end(); it++)
  {
    HMscNode* succ = dynamic_cast<HMscNode*> ((*it)->get_successor());
    ConfigurationPtr old = new Configuration(b);
    if (check_branch(c, succ, b))
      return true;
    else
      b = old;
  }

  return false;
}

MscPtr make_result(MembershipContext* c, HMscPtr msc)
{
  if(c->get_not_covered_intervals().size() > 0)
  {
    if(c->get_not_covered_intervals().size() > 1)
      c->get_mem()->print_report(RS_ERROR, L"Marked time intervals in the specification is not full covered.");
    else
      c->get_mem()->print_report(RS_ERROR, L"Marked time interval in the specification is not full covered.");

    color_intervals(c);
  }

  if(c->get_print_path())
    return color_path(c, msc);
  else
  {
    HMscNodePtr start = msc->get_start();

    if(start == NULL)
      throw std::runtime_error("Unexpected behaviour.");

    PredecessorNode* predecessor = dynamic_cast<PredecessorNode*>(start.get());
    
    if(predecessor == NULL)
      throw std::runtime_error("Unexpected behaviour.");

    SuccessorNode* suc = predecessor->get_successors().front()->get_successor();
    ReferenceNode* ref = dynamic_cast<ReferenceNode*>(suc);

    if(ref == NULL)
      throw std::runtime_error("Unexpected behaviour.");

    return ref->get_msc();
  }
}

HMscPtr color_path(MembershipContext* c, HMscPtr msc)
{
  HMscNodePtr node;
  PredecessorNode* predecessor;
  SuccessorNode* successor;
  NodeRelationPtrVector predecessors_rel; 

  while(c->get_path_size() > 0)
  {
    node = c->top_pop_path();
    node->set_marked();

    //in case the node is reference, 
    //this increase the value of attribute membership_counter
    //membership counter - store amount of node memberships (occurrences) in the flow
    if(dynamic_cast<ReferenceNode*>(node.get()) != NULL)
      node->set_attribute("membership_counter" , node->get_attribute("membership_counter", 0)+1);

    successor = dynamic_cast<SuccessorNode*>(node.get());

    //start node is not successor 
    if(successor == NULL)
      continue;

    predecessors_rel = successor->get_predecessors();
    predecessor = dynamic_cast<PredecessorNode*>(c->top_path().get());

    //end node is not predecessor
    if(predecessor == NULL)
      continue;
    
    //mark the connections
    for(unsigned int i = 0; i < predecessors_rel.size(); i++)
      if(predecessors_rel[i]->get_predecessor() == predecessor)
        predecessors_rel[i]->set_marked();
  }

  return msc;
}

void color_intervals(MembershipContext* c)
{
  std::vector<TimeRelationPtr> relations = c->get_not_covered_intervals();
  
  for(unsigned int i = 0; i < relations.size(); i++)
    relations[i]->set_marked();
}
