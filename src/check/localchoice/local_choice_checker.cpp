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
 * Copyright (c) 2008 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: local_choice_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "check/localchoice/local_choice_checker.h"

LocalChoiceCheckerPtr LocalChoiceChecker::m_instance;
const std::string LocalChoiceChecker::lc_mep_attribute = "lcmep";
const std::string LocalChoiceChecker::lc_nip_attribute = "lcnip";

void InitListener::on_white_node_found(HMscNode *n)
{
  StartNode *start_node;
  ReferenceNode *current_node;
  std::set<std::wstring> nip, mep;
  current_node = dynamic_cast<ReferenceNode*>(n);
  if(current_node == NULL)
  {
    start_node = dynamic_cast<StartNode*>(n);
    if(start_node == NULL)
      return;

    //start node is considered empty reference node
    start_node->set_attribute(LocalChoiceChecker::lc_mep_attribute, mep);
    start_node->set_attribute(LocalChoiceChecker::lc_nip_attribute, nip);
    return;
  }

  InstancePtrList::const_iterator it;

  BMscInitListener bmsc_init;
  DFSInstanceEventsTraverser bmsc_traverser;
  bmsc_traverser.add_white_event_found_listener(&bmsc_init);

  for(it = current_node->get_bmsc()->get_instances().begin();
      it != current_node->get_bmsc()->get_instances().end();
      it++)
  {
    bmsc_traverser.traverse(it->get());
    if(!bmsc_init.is_idle())
      nip.insert((*it)->get_label());

    if(bmsc_init.is_mep())
      mep.insert((*it)->get_label());

    bmsc_init.reset();
    bmsc_traverser.cleanup_traversing_attributes();
  }

  current_node->set_attribute(LocalChoiceChecker::lc_mep_attribute, mep);

  current_node->set_attribute(LocalChoiceChecker::lc_nip_attribute, nip);
}

void BMscInitListener::on_white_event_found(Event *e)
{
  CoregionEventPtr cor_e;
  if(m_first) //Still looking for a possibly first event in the instance
  {
    cor_e = dynamic_cast<CoregionEvent*>(e);
    if(cor_e)
    {
      if(cor_e->is_minimal())
      {
        if(cor_e->is_send())
          m_mep = true;
      }
      else //the event is not minimal
        m_first = false;
    }
    else //e is strict event
    {
      m_first = false;
      if(e->is_send())
        m_mep = true;
    }
  }
  m_idle = false;  //any event found -> instance is not idle
}

void IterationListener::on_white_node_found(HMscNode *n)
{
  if(!dynamic_cast<ReferenceNode*>(n) && !dynamic_cast<StartNode*>(n))
      return;
  HMscNodePListPtr successors;
  HMscNodePList::iterator it;
  std::set<std::wstring>::iterator str_it;
  
  std::set<std::wstring> empty; //neutral value for get_attribute
  
  successors = NodeFinder::successors(n, "rnf_inner"); //the string is the name of the dynamic attribute

  std::set<std::wstring> &curr_nip = n->get_attribute(LocalChoiceChecker::lc_nip_attribute, empty);
  //no reference here as it causes error
  std::set<std::wstring> curr_mep = n->get_attribute(LocalChoiceChecker::lc_mep_attribute, empty);

  for(it = successors->begin();it != successors->end(); it++)
  {
    if(!dynamic_cast<ReferenceNode*>(*it))
      continue;
    std::set<std::wstring> &succ_mep = (*it)->get_attribute(LocalChoiceChecker::lc_mep_attribute, empty);
    for(str_it = succ_mep.begin(); str_it != succ_mep.end(); str_it++)
    {
      if(curr_nip.find(*str_it) != curr_nip.end())
        continue; //no merging - the process is not idle in node n
      //else
      if(curr_mep.find(*str_it) == curr_mep.end())
      {
        curr_mep.insert(*str_it);
        m_change = true;
      }
    }
  }
  n->remove_attribute<std::set<std::wstring> >(LocalChoiceChecker::lc_mep_attribute);
  n->set_attribute(LocalChoiceChecker::lc_mep_attribute, curr_mep);
}

void FindNonLocalListener::on_white_node_found(HMscNode *n)
{
  if(!dynamic_cast<ReferenceNode*>(n) && !dynamic_cast<StartNode*>(n))
      return;
  HMscNodePListPtr successors = NodeFinder::successors(n, "rnf_inner"); //the string is the name of the dynamic attribute
  HMscNodePList::iterator succ_it;
  std::set<std::wstring> empty; //neutral value for get_attribute
  std::wstring valid_mep;
  bool first = true;
  bool nonlocal = false;
  unsigned succ_count = 0;
  for(succ_it = successors->begin(); succ_it != successors->end(); succ_it++)
    if(dynamic_cast <ReferenceNode*>(*succ_it))
      succ_count++;
  if(succ_count < 2)
    return;

  bool was_empty = false, was_full = false;
  unsigned mep_count;
  for(succ_it = successors->begin(); succ_it != successors->end(); succ_it++)
  {
    if(!dynamic_cast<ReferenceNode*>(*succ_it))
      continue;
    mep_count = (*succ_it)->get_attribute(LocalChoiceChecker::lc_mep_attribute, empty).size();
    if(mep_count > 1)
    {
      nonlocal = true;
      break;
    }
    if(mep_count == 1)
    {
      if(was_empty)
      {
        nonlocal = true;
        break;
      }
      was_full = true;
    }
    else if(mep_count == 0)
    {
      if(was_full)
      {
        nonlocal = true;
        break;
      }
      was_empty = true;
    }

    if(was_empty)
      continue;

    if(first)
    {
      valid_mep = *((*succ_it)->get_attribute(LocalChoiceChecker::lc_mep_attribute, empty).begin());
        //the expression returns the first (and only) entry in the current successor's MEP
      first = false;
    }
    if(*((*succ_it)->get_attribute(LocalChoiceChecker::lc_mep_attribute, empty).begin()) != valid_mep)
      nonlocal = true;
  }
  if(nonlocal)
  {
    m_nonlocal_found = true;
    n->get_original()->set_marked(MARKED);
  }


}

void CleanupListener::on_white_node_found(HMscNode *n)
{
  n->remove_attribute<std::set<std::wstring> >(LocalChoiceChecker::lc_mep_attribute);
  n->remove_attribute<std::set<std::wstring> >(LocalChoiceChecker::lc_nip_attribute);

}

std::list<HMscPtr> LocalChoiceChecker::check(HMscPtr hmsc, ChannelMapperPtr chm)
{
  HMscPtr transformed = m_graph_duplicator.duplicate_hmsc(hmsc);
  DFSRefNodeHMscTraverser ref_traverser;
  InitListener in_list;
  CleanupListener cl_list;
  IterationListener it_list;
  FindNonLocalListener find_list;
  ref_traverser.add_white_node_found_listener(&in_list);
  ref_traverser.traverse(transformed);
  ref_traverser.cleanup_traversing_attributes();
  ref_traverser.remove_all_listeners();

  ref_traverser.add_white_node_found_listener(&it_list);
  while(it_list.changed())
  {
    it_list.reset();
    ref_traverser.traverse(transformed);
    ref_traverser.cleanup_traversing_attributes();

  }
  ref_traverser.remove_all_listeners();
  ref_traverser.cleanup_traversing_attributes();
  ref_traverser.add_white_node_found_listener(&find_list);

  ref_traverser.traverse(transformed);
  ref_traverser.remove_all_listeners();
  ref_traverser.cleanup_traversing_attributes();
  ref_traverser.add_white_node_found_listener(&cl_list);
  ref_traverser.traverse(transformed);
  m_graph_duplicator.cleanup_attributes();

  std::list<HMscPtr> result;
  if(!find_list.nonlocal_found())
    return result;


  DFSHMscTraverser hmsc_traverser;
  ResultFinder res_find;
  hmsc_traverser.add_white_node_found_listener(&res_find);

  bool result_found = true;
  while(result_found)
  {
    result_found = false;
    try
    {
      hmsc_traverser.traverse(hmsc);
    }
    catch(NonlocalNode)
    {
      HMscPathDuplicator duplicator;
      HMscPtr example = duplicator.duplicate_path(hmsc_traverser.get_reached_elements());
      MscElementPListList::const_iterator h;
      MscElement* last = hmsc_traverser.get_reached_elements().back().back();
      duplicator.get_copy(last)->set_marked(MARKED);
      result.push_back(example);
      hmsc_traverser.cleanup_traversing_attributes();
      duplicator.cleanup_attributes();
      result_found = true;
    }
  }
  return result;
}

void LocalChoiceChecker::cleanup_attributes(void)
{
}

Checker::PreconditionList LocalChoiceChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  result.push_back(PrerequisiteCheck(L"Nonrecursivity", PrerequisiteCheck::PP_REQUIRED));
  result.push_back(PrerequisiteCheck(L"Unique Instance Names", PrerequisiteCheck::PP_RECOMMENDED));

  return result;
}

//  $Id: local_choice_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
