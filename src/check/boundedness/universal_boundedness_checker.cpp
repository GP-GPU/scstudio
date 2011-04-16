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
 * $Id: universal_boundedness_checker.cpp 710 2010-03-30 16:54:45Z vacek $
 */

#include "check/boundedness/universal_boundedness_checker.h"


UniversalBoundednessCheckerPtr UniversalBoundednessChecker::m_instance;

const std::string UniversalBoundednessChecker::com_graph_attribute = "cograt";

void NameCollector::on_white_node_found(HMscNode *n)
{
  InstancePtrList::const_iterator instance_iterator;
  ReferenceNode* refnode = dynamic_cast<ReferenceNode*>(n);
  unsigned number;
  if(refnode != NULL)
  {
    BMscPtr bmsc = refnode->get_bmsc();
    if(bmsc != NULL)
    {
      const InstancePtrList& instances = bmsc->get_instances();
      for(instance_iterator = instances.begin(); instance_iterator != instances.end(); instance_iterator++)
        if(m_instance_map.find(instance_iterator->get()->get_label()) == m_instance_map.end())
        {
          number = m_instance_map.size();
          m_instance_map[instance_iterator->get()->get_label()] = number;
        }
    }

  }
}

void BoundednessAssignListener::on_white_node_found(HMscNode *n)
{
  CommunicationGraph comgraph;
  ReferenceNode* refnode;
  refnode = dynamic_cast<ReferenceNode*>(n);
  if(refnode)
  {
    comgraph.create_from_bmsc_with_map(refnode->get_bmsc(), m_label_map);
    refnode->set_attribute<CommunicationGraph>(UniversalBoundednessChecker::com_graph_attribute, comgraph);
  }
  m_node_number++;
}


void BoundednessCleanupListener::on_white_node_found(HMscNode *n)
{
  n->remove_attribute<CommunicationGraph>(UniversalBoundednessChecker::com_graph_attribute);
}

void BoundednessListener::on_elementary_cycle_found(const MscElementPList &cycle)
{
  CommunicationGraph cycle_graph, helper;
  MscElementPList::const_iterator items;
  bool first_reference = true;

  ReferenceNode *current_node;
  for(items = cycle.begin(); items != cycle.end(); items++)
  {
    current_node = dynamic_cast<ReferenceNode*>(*items);

    if(current_node == NULL)
    {
      continue;
    }

    if(first_reference)
    {
      first_reference = false;
      cycle_graph = current_node->get_attribute(UniversalBoundednessChecker::com_graph_attribute, helper);
    }
    else cycle_graph.merge(current_node->get_attribute(UniversalBoundednessChecker::com_graph_attribute, helper));
  }
  if(!cycle_graph.is_strongly_connected())
  {
    MscElementPListList result, dummy;
    if(cycle.front()->is_attribute_set("boundedness_result"))
    {
      result = cycle.front()->get_attribute("boundedness_result", dummy);
      cycle.front()->remove_attribute<MscElementPListList>("boundedness_result");
    }
    result.push_back(cycle);
    cycle.front()->set_attribute("boundedness_result", result);
  }
}

Checker::PreconditionList UniversalBoundednessChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  result.push_back(PrerequisiteCheck(L"Nonrecursivity", PrerequisiteCheck::PP_REQUIRED));
  result.push_back(PrerequisiteCheck(L"Unique Instance Names", PrerequisiteCheck::PP_RECOMMENDED));

  return result;
}

HMscPtr UniversalBoundednessChecker::create_counter_example(const MscElementPList& to_cycle, const MscElementPList& cycle)
{
  HMscPtr p;
  HMscPathDuplicator duplicator;
  MscElementPList::const_iterator iter;
  MscElementPListList path;
  path.push_back(MscElementPList());
  for(iter = to_cycle.begin(); iter!=to_cycle.end(); iter++)
    path.back().push_back(*iter);
  for(iter = cycle.begin(), iter++; iter!=cycle.end(); iter++) //first node is already stored in the previous step
    path.back().push_back(*iter);
  

  p = duplicator.duplicate_path(path);


  for(iter=cycle.begin();*iter!=cycle.back();iter++)
  {
    duplicator.get_copy(*iter)->set_marked();
  }
  duplicator.cleanup_attributes();

  return p;
}

std::list<HMscPtr> UniversalBoundednessChecker::check(HMscPtr hmsc, ChannelMapperPtr chm)
{
  DFSHMscTraverser name_traverser("UB_color");
  NameCollector name_collector;
  HMscPtr p(NULL);
  name_traverser.add_white_node_found_listener(&name_collector);
  name_traverser.traverse(hmsc);

  /* Here the main part of the checker begins */
  BMscGraphDuplicator graph_duplicator;

  HMscPtr transformed = graph_duplicator.duplicate_hmsc(hmsc);
  graph_duplicator.cleanup_attributes();
  
  DFSHMscTraverser msc_traverser;
  BoundednessAssignListener assigner(name_collector.get_instance_map());
  BoundednessCleanupListener cleaner;

  //First, communication graphs and numbers are assigned to all the vertices.
  msc_traverser.add_white_node_found_listener(&assigner);
  msc_traverser.traverse(transformed);
  //Notice that the checking traverser takes the number of vertices as a parameter.
  BoundednessListener cycles(assigner.get_vertex_count());  
  msc_traverser.remove_all_listeners();

  std::list<HMscPtr> result;

  ElementaryCyclesTraverser cycle_traverser;

  //This is the part performing the actual checking procedure.
  cycle_traverser.add_cycle_listener(&cycles);

  cycle_traverser.traverse(transformed);

  ResultCatcher result_listener;
  msc_traverser.add_white_node_found_listener(&result_listener);

  
  bool was_result = true;
  while(was_result)
  {
    was_result = false;
    try
    {
      msc_traverser.traverse(transformed);
    }
    catch(UnboundedCycleException &e)
    {
      was_result = true;
      MscElementPListList::iterator it;
      MscElementPListList unb_cycles;
      unb_cycles = e.get_unbounded_cycles();
      for(it = unb_cycles.begin(); it != unb_cycles.end(); it++)
      {
        result.push_back(create_counter_example(msc_traverser.get_reached_elements().back(), *it));
      }
      msc_traverser.cleanup_traversing_attributes();

    }
  }
  msc_traverser.remove_all_listeners();

  //Finally, dynamic attributes are removed.
  msc_traverser.add_white_node_found_listener(&cleaner);
  msc_traverser.traverse(transformed);

  
  return  result;
}

void UniversalBoundednessChecker::cleanup_attributes()
{
}
// $Id: universal_boundedness_checker.cpp 710 2010-03-30 16:54:45Z vacek $
