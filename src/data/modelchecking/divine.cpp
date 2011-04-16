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
 * Copyright (c) 2009 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: divine.cpp 1026 2011-01-09 13:14:07Z vacek $
 */



#include "data/modelchecking/divine.h"

const std::string Divine::node_id_attribute = "di_no_nu";
inline std::string to_string(const std::wstring &ws)
{
  return std::string(ws.begin(), ws.end());
}

ExportFormatter::PreconditionList Divine::get_preconditions(MscPtr msc) const
{
  ExportFormatter::PreconditionList result;
  //BMSC test preversion does not require anything

  result.push_back(PrerequisiteCheck(L"Universal Boundedness", PrerequisiteCheck::PP_RECOMMENDED));
  result.push_back(PrerequisiteCheck(L"Unique Instance Names", PrerequisiteCheck::PP_REQUIRED));
  result.push_back(PrerequisiteCheck(L"Local Choice", PrerequisiteCheck::PP_REQUIRED));

  return result;
}

void ProcessListener::process_successor_transitions(HMscNode *n, std::string first_node)
{  
  InstancePtr current_instance;
  NonemptyNodeFinder finder;
  HMscNodePListPtr succ =finder.find_successors(n);
  finder.cleanup_traversing_attributes();

  ReferenceNode *r;
  
  if(succ->size() == 1)
  {
    r = dynamic_cast<ReferenceNode*>(succ->front());
    if(!r)
      return;
    m_stream << "  " << first_node << " -> " << to_string(r->get_bmsc()->get_label()) << "_init," << std::endl;
    return;
  }
  bool send_found = false;

  for(HMscNodePList::const_iterator a = succ->begin(); a != succ->end(); a++)
  {
    r = dynamic_cast<ReferenceNode*>(*a);
    if(!r)
      continue;

    for(InstancePtrList::const_iterator it = r->get_bmsc()->get_instances().begin();
      it != r->get_bmsc()->get_instances().end();
      it++)
    {
      if(to_string((*it)->get_label()) == m_current_instance)
      {
        current_instance = *it;
        if(current_instance->has_events())
        {
          //Find a minimal event - is it a send event?
          EventArea *area = current_instance->get_first().get();

          while(area->is_empty())
            area = area->get_next().get();

          StrictOrderArea *strict = dynamic_cast<StrictOrderArea*>(area);

          if(strict)
          {
            if(strict->get_first()->is_send())
              send_found = true;
          }
          else
          {
            CoregionArea *coregion = dynamic_cast<CoregionArea*>(area);
            //This requires local choice - if the HMsc was not local choice,
            //there could be some other minimal receive event in the coregion.
            if(coregion->get_minimal_events().front()->is_send())
              send_found = true;
          }
        }//if the current instance has events
      } //if the current instance is found
    }//for instances in a successor
    if(send_found)
      break;
  }//for successors
  if(send_found)
  {
    for(HMscNodePList::const_iterator a = succ->begin(); a != succ->end(); a++)
    {
      r = dynamic_cast<ReferenceNode*>(*a);
      if(!r)
        continue;

      m_stream << "  " << first_node << " -> " << to_string(r->get_bmsc()->get_label()) << "_init," << std::endl;
    }//for successors 2 - enumerating transitions
  }//if send found
  else //no send found => polling
  {
    m_stream << "  " << first_node << " -> polling," << std::endl;
  } //no send found
}//if not states only

void ProcessListener::on_white_node_found(HMscNode *n)
{
  InstancePtr current_instance;
  if(dynamic_cast<StartNode*>(n))
  {
    if(m_states_only)
      return;
    else
      process_successor_transitions(n, "start");
    return;
  }//if start node

  ReferenceNode *ref = dynamic_cast<ReferenceNode*>(n);
  if(!ref)
    return;

  bool any_events = false, current_found = false;

  for(InstancePtrList::const_iterator it = ref->get_bmsc()->get_instances().begin();
    it != ref->get_bmsc()->get_instances().end();
    it++)
  {
    if((*it)->has_events())
      any_events = true;
    if(to_string((*it)->get_label()) == m_current_instance)
    {
      current_instance = *it;
      if(current_instance->has_events())
        current_found = true;
    }
  }

  if(!any_events)
    return;

  if(!m_states_only)
    process_successor_transitions(n, to_string(ref->get_bmsc()->get_label()) + "_end");



  ////////////////////////////
  if(!current_found)
  {
    if(m_states_only)
    {
      m_stream << ", " << to_string(ref->get_bmsc()->get_label()) << "_init"; 
      m_stream << ", " << to_string(ref->get_bmsc()->get_label()) << "_end"; 
    }
    else
    {
      m_stream << "  " << to_string(ref->get_bmsc()->get_label()) << "_init -> "
        << to_string(ref->get_bmsc()->get_label()) << "_end," << std::endl;
    }
    return;
  }

  std::string current_name = to_string(ref->get_bmsc()->get_label());
  std::string current_state = current_name + "_init";
  std::string next_state;
  unsigned state_number = 0;
  if(m_states_only)
  {
    m_stream << ", " << current_state; 
    m_stream << ", " << current_name + "_end";
  }

  EventArea* current_area;
  StrictOrderArea* strict_area;
  CoregionArea* coregion_area;
  current_area = current_instance->get_first().get();
  std::stringstream ss;

  unsigned node_number, dummy;
  node_number = n->get_attribute(Divine::node_id_attribute, dummy);

  bool minimal_events = true;

  while(current_area)
  {
    strict_area = dynamic_cast<StrictOrderArea*>(current_area);
    if(strict_area)
    {
      StrictEventPtr strict_event = strict_area->get_first();
      while(strict_event)
      {
        ss.str("");
        ss << current_name << "_" << state_number++;
        next_state = ss.str();
        if(m_states_only)
        {
          if(strict_event->is_receive())
            m_stream << ", " << next_state << "_test";

          m_stream << ", " << next_state;
        }
        else
        {
          if(strict_event->is_receive())
          {
            m_stream << "  " << current_state << " -> " << next_state << "_test" << std::endl;
            m_stream << "    {";
            m_stream << "sync to_" << to_string(strict_event->get_instance()->get_label())
              << "?{node, from, message};";
            m_stream << "}," << std::endl;

            m_stream << "  " << next_state << "_test -> " << next_state << std::endl;
            m_stream << "    {";
            m_stream << "guard node == " << node_number 
              << " and from == " << m_instance_map[strict_event->get_matching_event()->get_instance()->get_label()]
              << " and message == " << m_message_map[strict_event->get_message()->get_label()];

            m_stream << "}," << std::endl;
            if(minimal_events)
            {
              m_stream << "  polling_test -> " << next_state <<  std::endl;
              m_stream << "    {";
              m_stream << "guard node == " << node_number 
                << " and from == " << m_instance_map[strict_event->get_matching_event()->get_instance()->get_label()]
                << " and message == " << m_message_map[strict_event->get_message()->get_label()];

            m_stream << "}," << std::endl;
            }
          }
          else
          {
            m_stream << "  " << current_state << " -> " << next_state << std::endl;
            m_stream << "    {";
            m_stream << "sync to_" << to_string(strict_event->get_matching_event()->get_instance()->get_label())
              << "!{" << node_number << ", " << m_instance_map[strict_event->get_instance()->get_label()] 
              << ", " << m_message_map[strict_event->get_message()->get_label()];              
              m_stream << "};}," << std::endl;
          }
        }
        strict_event = strict_event->get_successor();
        current_state = next_state;
        minimal_events = false;
      }
    }
    else  //area is a coregion area
    {
      coregion_area = dynamic_cast<CoregionArea*>(current_area);
      unsigned events_count = coregion_area->get_events().size();

      if(events_count == 0)
        continue;

      unsigned event_index = 0;
      std::list<std::string> state_queue;
      std::set<std::string> in_queue;
      std::vector<CoregionEventPtr> event_vector;
      std::string state_string = "";
      for(unsigned i = 0; i < events_count; i++)
        state_string += "0";

      state_queue.push_back(state_string);
      in_queue.insert(state_string);

      for(CoregionEventPtrSet::const_iterator it = coregion_area->get_events().begin();
        it != coregion_area->get_events().end(); it++)
      {
        event_vector.push_back(*it);
        event_vector.back()->set_attribute("event_index", event_index++);
      }

      next_state = current_state + "_" + state_string;
      if(!m_states_only)
        m_stream << "  " << current_state << " -> " << next_state << "," << std::endl;

      while(!state_queue.empty())
      {
        state_string = state_queue.front();
        in_queue.erase(in_queue.find(state_string));
        state_queue.pop_front();


        if(m_states_only)
        {
          m_stream << ", " << current_state + "_" + state_string;
        }

        unsigned i;
        bool preconditions;
        bool first_receive;
        for(i = 0; i < state_string.size(); i++)
        {
          preconditions = true;
          first_receive = true;
          if(state_string.at(i) == '0')
          {
            for(unsigned j = 0; j < event_vector.at(i)->get_predecessors().size(); j++)
            {
              unsigned pred_index, dummy;

              pred_index = event_vector.at(i)->get_predecessors().at(j)->get_predecessor()->get_attribute("event_index", dummy);

              if(state_string.at(pred_index) == '0')
              {
                preconditions = false;
                break;
              }
            }
            if(!preconditions)
              continue;

            //event i may be executed in the current state
            //careful - only one reading from the channel is allowed in each state
            std::string new_state_string = state_string;
            new_state_string.at(i) = '1';
            if(in_queue.find(new_state_string) == in_queue.end())
            {
              state_queue.push_back(new_state_string);
              in_queue.insert(new_state_string);
            }
            if(event_vector.at(i)->is_send())
            {
              if(!m_states_only)
              {
                m_stream << "  " << current_state << "_" << state_string << " -> " << current_state << "_" << new_state_string << std::endl;
                m_stream << "    {";
                m_stream << "sync to_" << to_string(event_vector.at(i)->get_matching_event()->get_instance()->get_label())
                << "!{" << node_number << ", " << m_instance_map[event_vector.at(i)->get_instance()->get_label()] 
                << ", " << m_message_map[event_vector.at(i)->get_message()->get_label()];              
                m_stream << "};}," << std::endl;
              }
            }
            else //event is receive
            {
              if(first_receive && m_states_only)
              {
                m_stream << ", " << current_state << "_" << state_string << "_test";
                continue;
              }

              //Processing transitions
              if(first_receive)
              {
                m_stream << "  " << current_state << "_" << state_string << " -> " << current_state << "_" << state_string << "_test" << std::endl;
                m_stream << "    {";
                m_stream << "sync to_" << to_string(event_vector.at(i)->get_instance()->get_label())
                  << "?{node, from, message};";
                m_stream << "}," << std::endl;

                first_receive = false;
              }

              m_stream << "  " << current_state << "_" << state_string << "_test -> " << current_state << "_" << new_state_string << std::endl;
              m_stream << "    {";
              m_stream << "guard node == " << node_number 
                << " and from == " << m_instance_map[event_vector.at(i)->get_matching_event()->get_instance()->get_label()]
                << " and message == " << m_message_map[event_vector.at(i)->get_message()->get_label()];

              m_stream << "}," << std::endl;
              if(minimal_events && event_vector.at(i)->is_minimal())
              {
                m_stream << "  polling_test -> " << current_state << "_" << new_state_string << std::endl;
                m_stream << "    {";
                m_stream << "guard node == " << node_number 
                  << " and from == " << m_instance_map[event_vector.at(i)->get_matching_event()->get_instance()->get_label()]
                  << " and message == " << m_message_map[event_vector.at(i)->get_message()->get_label()];

                m_stream << "}," << std::endl;
              }
            }//receive event
          }//if event i is 0
        }//for all events
        minimal_events = false; //Only in state "0...0" a transition from polling may be performed
      }//while queue not empty



      for(CoregionEventPtrSet::const_iterator it = coregion_area->get_events().begin();
        it != coregion_area->get_events().end(); it++)
        (*it)->remove_attribute<unsigned>("event_index");
      minimal_events = false;

      current_state = current_state + "_" + state_string;
    }//coregion area

    current_area = current_area->get_next().get();
    minimal_events = false;
  }//all areas
  if(!m_states_only)
  {
    m_stream << "  " << current_state << " -> " << current_name << "_end," << std::endl;
  }
}

int Divine::save_msc(std::ostream& stream, const std::wstring &name,
    const MscPtr& selected_msc, const std::vector<MscPtr>& msc)
{
  reset();
  BMscPtr bmsc;
  HMscPtr hmsc;
  bmsc = boost::dynamic_pointer_cast<BMsc>(selected_msc);
  if(bmsc)
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
    hmsc = hmsc1;
  }
  else
  {
    HMscPtr hmsc1 = boost::dynamic_pointer_cast<HMsc>(selected_msc);
    if(!hmsc1)
    {
      return 1;
    }
    BMscGraphDuplicator duplicator;
    hmsc = duplicator.duplicate(hmsc1);
    duplicator.cleanup_attributes();
  }

  //processing hmsc (or bmsc transformed to a simple hmsc

  DFSBMscGraphTraverser hmsc_traverser;
  PreprocessListener init_listener(m_message_map, m_instance_map, m_instance_names, m_receive_counts);

  hmsc_traverser.add_white_node_found_listener(&init_listener);

  try
  {
    hmsc_traverser.traverse(hmsc);  //preprocessing
  }
  catch(...)
  {
    print_report(RS_ERROR, L"Lost/found messages are not supported.");
    hmsc_traverser.cleanup_traversing_attributes();
    hmsc_traverser.remove_all_listeners();
    CleanupListener cl;
    hmsc_traverser.add_white_node_found_listener(&cl);
    hmsc_traverser.traverse(hmsc);
    return 1;
  }
  hmsc_traverser.remove_all_listeners();
  for(unsigned i = 0; i < m_instance_names.size(); i++)
    stream << "channel {int, int, int} to_" << m_instance_names.at(i) << "[" << m_receive_counts.at(i) << "];" << std::endl;

  ProcessListener bmscpl(stream, m_message_map, m_instance_map);
  DFSRefNodeHMscTraverser ref_traverser;
  ref_traverser.add_white_node_found_listener(&bmscpl);
  for(unsigned i = 0; i < m_instance_names.size(); i++)
  {
    stream << "process " << m_instance_names.at(i) << std::endl;
    stream << "{" << std::endl;
    stream << "int node, from, message;" << std::endl;

    stream << "state start, polling, polling_test";
    bmscpl.set_instance(m_instance_names.at(i));
    bmscpl.set_states();
    ref_traverser.traverse(hmsc);
    stream << ";" <<std::endl;
    stream << "init start;" << std::endl;

    bmscpl.set_transitions();
    stream << "trans" << std::endl;
    ref_traverser.traverse(hmsc);

    stream << "  polling -> polling_test" << std::endl;
    stream << "    {sync to_" << m_instance_names.at(i) << "?{node,from,message};};" << std::endl;

    stream << "}\n" << std::endl;
  }
  stream << "system async;" << std::endl;

  hmsc_traverser.remove_all_listeners();
  CleanupListener cl;
  hmsc_traverser.add_white_node_found_listener(&cl);
  hmsc_traverser.traverse(hmsc);
  return 0;
}


void NodeCounterListener::on_white_event_found(Event *e)
{
  if(!e->is_matched())
    throw std::runtime_error("Lost/found messages not supported.");

  m_total++;
  if(e->is_receive())
    m_received++;
}


void MessageIdListener::on_white_event_found(Event *e)
{
  if(m_message_map.find(e->get_message()->get_label()) == m_message_map.end())
    m_message_map.insert(std::make_pair(e->get_message()->get_label(), m_message_map.size()));
}


void PreprocessListener::on_white_node_found(HMscNode *node)
{
  ReferenceNode *ref  = dynamic_cast<ReferenceNode*>(node);
  if(!ref)
    return;

  InstancePtrList::const_iterator it;

  DFSInstanceEventsTraverser instance_traverser;
  //counts number of receive and all events
  NodeCounterListener node_counter;
  instance_traverser.add_white_event_found_listener(&node_counter);

  for(it = ref->get_bmsc()->get_instances().begin();
      it !=ref->get_bmsc()->get_instances().end();
      it++)
  {
    if(m_instance_map.find((*it)->get_label()) == m_instance_map.end())
    {
      m_instance_map[(*it)->get_label()] = m_instance_count++;
      m_instance_names.push_back(to_string((*it)->get_label()));
      m_receive_counts.push_back(0);
    }

    node_counter.reset();
    instance_traverser.traverse(it->get());  //counting nodes
    m_receive_counts[m_instance_map[(*it)->get_label()]] += node_counter.get_received_count();
  }

  instance_traverser.remove_all_listeners();

  MessageIdListener mes_id(m_message_map);

  instance_traverser.add_white_event_found_listener(&mes_id);

  instance_traverser.traverse(ref->get_bmsc());

    
  node->set_attribute(Divine::node_id_attribute, m_node_number);

  std::wstringstream ss;

  BMscPtr bmsc =ref->get_bmsc();
  ss << "N" << m_node_number++ << "_" << bmsc->get_label();
  bmsc->set_label(ss.str());
}


// $Id: divine.cpp 1026 2011-01-09 13:14:07Z vacek $
