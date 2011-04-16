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
 * $Id: name_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "check/structure/name_checker.h"

NameCheckerPtr NameChecker::m_instance;

void FindFirstNodeListener::on_white_node_found(HMscNode *n)
{
  ReferenceNode* refnode = dynamic_cast<ReferenceNode*>(n);
  if(refnode != NULL)
  {
    BMscPtr bmsc = refnode->get_bmsc();
    if(bmsc != NULL)
    {
      throw FirstNodeFoundException();
    }
        
  }
}
void NameListener::on_white_node_found(HMscNode *n)
{
  size_t num_of_instances;
  std::vector<std::wstring> current_labels;
  
  InstancePtrList::const_iterator instance_iterator;
  ReferenceNode* refnode = dynamic_cast<ReferenceNode*>(n);
  if(refnode != NULL)
  {
    BMscPtr bmsc = refnode->get_bmsc();
    if(bmsc != NULL)
    {
      const InstancePtrList& instances = bmsc->get_instances();
      for(instance_iterator = instances.begin(); instance_iterator != instances.end(); instance_iterator++)
        current_labels.push_back(instance_iterator->get()->get_label());
      num_of_instances = current_labels.size();

      std::sort(current_labels.begin(), current_labels.end());
      for(unsigned i = 1; i < current_labels.size(); i++)
        if(current_labels[i] == current_labels[i - 1])
        {
          throw DuplicateNamesException(current_labels[i]);
        }
      if(m_first_node)
      {
        m_instance_names = current_labels;
        m_first_node = false;
      }
      else
      {/*  different processes are no more considered an error
        if(current_labels != m_instance_names)
          throw InconsistentNamesException(); 
        */
      }
    }
        
  }
}

void NodeAdder::on_white_node_found(HMscNode *n)
{
  m_where_to_add->add_node(n);
}


Checker::PreconditionList NameChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}

HMscPtr NameChecker::create_duplicate_counter_example(const MscElementPListList& path, const std::wstring& label)
{
  HMscPathDuplicator duplicator;
  HMscPtr example = duplicator.duplicate_path(path);
  MscElementPListList::const_iterator h;
  ReferenceNode *refnode;
  InstancePtrList::const_iterator instance_iterator;
  for(h=path.begin();h!=path.end();h++)
  {
    MscElement* last = (*h).back();
    duplicator.get_copy(last)->set_marked(MARKED);
  }
  refnode=dynamic_cast<ReferenceNode*>(duplicator.get_copy(path.back().back()));
  if(refnode != NULL)
  {
    for(instance_iterator = refnode->get_bmsc()->get_instances().begin();
        instance_iterator != refnode->get_bmsc()->get_instances().end();
        instance_iterator++)
      if(label == (instance_iterator->get()->get_label()))
        (*instance_iterator)->set_marked(MARKED);
  }
  return example;
}

HMscPtr NameChecker::create_inconsistent_counter_example(const MscElementPListList& path1, const MscElementPListList& path2)
{
  HMscPtr p, q;
  HMscPathDuplicator duplicator;
  
  MscElementPListList::const_iterator h;

  p = duplicator.duplicate_path(path1);
  for(h=path1.begin();h!=path1.end();h++)
  {
    MscElement* last = (*h).back();
    duplicator.get_copy(last)->set_marked(MARKED);
  }

  for(h=path2.begin();h!=path2.end();h++)
  {
    MscElement* last = (*h).back();
    if(duplicator.get_copy(last))
      duplicator.get_copy(last)->set_marked(MARKED);
  }

  duplicator.cleanup_attributes();


  q = duplicator.duplicate_path(path2);

  for(h=path2.begin();h!=path2.end();h++)
  {
    MscElement* last = (*h).back();
    duplicator.get_copy(last)->set_marked(MARKED);
  }

  for(h=path1.begin();h!=path1.end();h++)
  {
    MscElement* last = (*h).back();
    if(duplicator.get_copy(last))
      duplicator.get_copy(last)->set_marked(MARKED);
  }

  NodeAdder no_a(p);
  DFSBMscGraphTraverser node_adder;
  node_adder.add_white_node_found_listener(&no_a);
  node_adder.traverse(q);
  const NodeRelationPtr& rel = *(q->get_start()->get_successors().begin());
  p->get_start()->add_successor(rel->get_successor());
  return p;
}


std::list<BMscPtr> NameChecker::check(BMscPtr bmsc, ChannelMapperPtr chm)
{
  HMscPtr hmsc1(new HMsc(L"HMsc1"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1_1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1_1);
  hmsc1->add_node(end1);
  start1->add_successor(r1_1.get());
  r1_1->add_successor(end1.get());
  r1_1->set_msc(bmsc);
  std::list<BMscPtr> result;

  std::list<HMscPtr> hresult = check(hmsc1, chm);
  if(hresult.empty())
    return result;

  HMscPtr r = hresult.front();
  if(r)
  {
    DFSHMscTraverser first_node_traverser;
    FindFirstNodeListener ffnl;
    first_node_traverser.add_white_node_found_listener(&ffnl);
    try
    {
      first_node_traverser.traverse(r);
    }
    catch(FirstNodeFoundException)
    {
      result.push_back(dynamic_cast<ReferenceNode*>(first_node_traverser.get_reached_elements().back().back())->get_bmsc());
      return result;
    }
    return result; //just to avoid compiler warning - this line is actually not reachable
  }
  else 
  {
    return result;
  }
}
std::list<HMscPtr> NameChecker::check(HMscPtr hmsc, ChannelMapperPtr chm)
{
  DFSHMscTraverser name_traverser, first_node_traverser;
  NameListener n_ch;
  FindFirstNodeListener ffnl;
  HMscPtr p(NULL);
  MscElementPListList path_to_first;
  name_traverser.add_white_node_found_listener(&n_ch);
  first_node_traverser.add_white_node_found_listener(&ffnl);
  std::list<HMscPtr> result;
  try
  {
    first_node_traverser.traverse(hmsc);
  }
  catch(FirstNodeFoundException)
  {
    path_to_first = first_node_traverser.get_reached_elements();
    first_node_traverser.cleanup_traversing_attributes();
  }

  try
  {
    name_traverser.traverse(hmsc);
  }
  catch(DuplicateNamesException &err)
  {
    p = create_duplicate_counter_example(name_traverser.get_reached_elements(), err.get_name());
    name_traverser.cleanup_traversing_attributes();
    m_graph_duplicator.cleanup_attributes();
	result.push_back(p);
    return result;
  }

  catch(InconsistentNamesException)
  {
    p = create_inconsistent_counter_example(name_traverser.get_reached_elements(), path_to_first);
    name_traverser.cleanup_traversing_attributes();
    m_graph_duplicator.cleanup_attributes();
    result.push_back(p);
    return result;
  }
  return result;
}

void NameChecker::cleanup_attributes()
{
}

// $Id: name_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
