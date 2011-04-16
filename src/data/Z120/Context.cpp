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
 * $Id: Context.cpp 1006 2010-12-08 20:34:49Z madzin $
 */

/*
 * we use ANTRL v3 (ANTLR 3.1.1)
 * see http://www.antlr.org
 */

/*
 * Maximal number of warning/error message is 33
 */

#ifndef __ParserStruct__
#define __ParserStruct__

#include "Context_Impl.h"

/*
 * Set name of Msc
 */
void set_msc_name_fun(struct Context* context, char* name)
{
  context->msc_name = name;
}

void my_print(char* a){
  std::cerr << a << std::endl;
}

/*
 * Set beginning values
 */
void init(struct Context* context)
{
  context->interval_label = "";
  context->time = "";

  context->myBmsc = new BMsc();
  context->myHmsc = new HMsc();
  context->myBmsc = NULL;
  context->myHmsc = NULL;
  context->element_name = "";
  context->reference_name = "";
  context->condition_name = "";
  context->node_type = connection;
  context->event_name = "";
  context->coregion_area_finished.clear();
  context->instances.clear();
  context->messages.clear();
  context->future_events.clear();
  context->future_time_events.clear();
  context->named_events.clear();
  context->current_event = NULL;
  context->not_create_event = 0;
  context->no_message_label = 0;
  context->open_instance = 0;
  context->data_parameter_decl = false;
  context->instance_parameter_decl = false;
  context->message_parameter_decl = false;
  context->timer_parameter_decl = false;
  context->error_inst_names.clear();
  context->error_nodes_names.clear();
  context->error_event_names.clear();
  context->end_msc = false;

  context->start_node = NULL;
  context->end_node = std::make_pair("", context->end_node.second);  //replaice context->end_node.second with NULL
  context->connect_name.clear();
  context->hmsc_nodes.clear();
  context->future_connections.clear();
  context->future_bottom_time_relations.clear();
  context->future_top_time_relations.clear();
  context->not_connect = 0;
  context->origin = false;
  context->time_dest = false;
  
  context->time_first = unknown;
  context->time_second = unknown;
  context->time_event = false;
  context->absolut_time = false;
  context->absolut_first_border = false;
  context->absolut_first_set = false;
  context->absolut_second_border = false;
}

/*
 * Return Msc structure.
 */
MscPtr get_msc_fun(struct Context* context)
{
  if (context->myBmsc == NULL)
  {
    return context->myHmsc;
  }
  else
  {
    return context->myBmsc;
  }

  return NULL;
}

struct s_Msc** get_total_msc_fun(struct Context* context)
{

  if(context->nonpointed.size() > 1)
  {
    std::string names = ""; 
    for(std::set<std::string>::iterator it = context->nonpointed.begin(); it != context->nonpointed.end(); it++)
    {
      if(it != context->nonpointed.begin())
      {
        names += ", ";
      }
      names += *it;
    }
    context->z->print_report(RS_WARNING, stringize() << L"Warning 02: Unreferenced MSC: " << TOWSTRING(names));
  }
  
  if(context->nonpointed.size() < 1 && context->mscs.size() > 0)
  {
    context->z->print_report(RS_WARNING, L"Warning 03: Infinite recursion among MSCs");
  }
  
  s_Msc** result = NULL;
  int position = 0;

  if(context->mscs.size() > 0){
    
    result = new s_Msc*[context->mscs.size()+1];

    std::vector<std::string>::iterator order_it;
    Context::MscPtrMap::const_iterator mscs_it;


    //add nonpointed HMSC    
    for(order_it = context->hmscs_order.begin(); order_it != context->hmscs_order.end(); order_it++)
    {
      Context::MscNameSet::const_iterator nonpointed_it = context->nonpointed.find(*order_it);
      
      if(nonpointed_it != context->nonpointed.end())
      { 
        for(mscs_it = context->mscs.lower_bound(*nonpointed_it);
          mscs_it != context->mscs.upper_bound(*nonpointed_it); mscs_it++)
        {
          Msc* my_msc = mscs_it->second.get();
          if(my_msc != NULL){
            intrusive_ptr_add_ref(my_msc);
            result[position++] = static_cast<s_Msc*>(my_msc);
          }
        }
      }
    }

    //add nonpointed BMSC
    for(order_it = context->bmscs_order.begin(); order_it != context->bmscs_order.end(); order_it++)
    {
      Context::MscNameSet::const_iterator nonpointed_it = context->nonpointed.find(*order_it);
      
      if(nonpointed_it != context->nonpointed.end())
      {
        for(Context::MscPtrMap::iterator mscs_it = context->mscs.lower_bound(*nonpointed_it);
          mscs_it != context->mscs.upper_bound(*nonpointed_it); mscs_it++)
        {
          Msc* my_msc = mscs_it->second.get();
          if(my_msc != NULL){
            intrusive_ptr_add_ref(my_msc);
            result[position++] = static_cast<s_Msc*>(my_msc);
          }
        }
      }
    }

    //add others MSC
    for(order_it = context->msc_file_order.begin(); 
        order_it != context->msc_file_order.end();
        order_it++)
    {
      // msc's listed nonpointed have already been processed above
      if(context->nonpointed.find(*order_it) != context->nonpointed.end())
        continue;
    
      mscs_it = context->mscs.find(*order_it);

      if(mscs_it == context->mscs.end())
        continue;

      Msc* my_msc = mscs_it->second.get();
      if(my_msc != NULL){
        intrusive_ptr_add_ref(my_msc);
        result[position++] = static_cast<s_Msc*>(my_msc);
      }
    }

    result[position] = NULL;
  }

  return result;

}

/*
 * Memory initialization.
 */
struct Context* new_context()
{
  Context* context = new Context;
  return context;
}

void add_z_fun(struct Context* context, struct s_Z120* z)
{
  context->z = static_cast<Z120*> (z);
}

/*
 * Free memory space
 */
void delete_context(struct Context* context)
{
  delete context;
}

/*
 * Check if all collections are empty
 */
void check_collections_fun(struct Context* context)
{
  if(!context->coregion_area_opened.empty())
  {
    std::string names = "";
    for(std::set<std::string>::iterator it = context->coregion_area_opened.begin(); it != context->coregion_area_opened.end(); it++)
    {
      if(it != context->coregion_area_opened.begin())
      {
        names += ", ";
      }
      names += *it;
    }
    context->z->print_report(RS_WARNING, stringize() << "Warning 04: Instances with unfinished coregion: " << TOWSTRING(names));
    context->coregion_area_opened.clear();
  }

  if(!context->messages.empty()){ 
    std::multimap<std::string, CompleteMessagePtr>::iterator it;

    for(it = context->messages.begin(); it != context->messages.end(); ++it)
    {
      context->z->print_report(RS_WARNING, stringize() << L"Warning 05: Complete message (" << TOWSTRING(it->first) << ") with only one event");
    }
    
    IncompleteMessagePtr message;
    std::multimap<std::string, CompleteMessagePtr>::iterator msg_it;
    
    for(msg_it = context->messages.begin(); msg_it != context->messages.end(); msg_it++)
    {
      if(msg_it->second->get_send_event() == NULL){
        message = new IncompleteMessage(FOUND, msg_it->second->get_label());
        message->glue_event(msg_it->second->get_receive_event());
      }
      else
      {
        message = new IncompleteMessage(LOST, msg_it->second->get_label());
        message->glue_event(msg_it->second->get_send_event());
      }
    }
  }

  if(!context->future_events.empty()){ 
    std::map<std::string, EventPtr>::iterator future_it;
    CoregionEventPtr future_e;
    CoregionArea* area;

    for(future_it = context->future_events.begin(); future_it != context->future_events.end(); future_it++)
    {
	future_e = boost::dynamic_pointer_cast<CoregionEvent>(future_it->second);
        area = dynamic_cast<CoregionArea*> (future_e->get_area());

        if(area != NULL)
          area->remove_event(future_e);
    }
    context->future_events.clear();
    context->z->print_report(RS_WARNING, L"Warning 06: Dependency on nonexisting event\n"); 
  }

  if(!context->order_events.empty()){
    context->z->print_report(RS_WARNING, L"Warning 07: Reference to nonexisting event\n"); 
  }

  if(!context->future_connections.empty()){
    std::map<std::string, std::set<std::string> >::iterator it;
    std::string names;
 
    for(it = context->future_connections.begin(); it != context->future_connections.end(); ++it)
    {
      if(it != context->future_connections.begin())
      {
        names += ", ";
      }

      names += it->first; 
    }
    context->z->print_report(RS_WARNING, stringize() << L"Warning 08: MSC (" << TOWSTRING(context->msc_name) << ") has reference to nonexisting node: " << TOWSTRING(names));
    
   
    context->future_connections.clear();
  }

   if(context->open_instance > 0)
  {
    if(context->open_instance == 1)
    {
      context->z->print_report(RS_WARNING, stringize() << "Warning 20: MSC (" << TOWSTRING(context->msc_name) << ") has unterminated " << context->open_instance << " instance");
    }
    else
    {
      context->z->print_report(RS_WARNING, stringize() << "Warning 20: MSC (" << TOWSTRING(context->msc_name) << ") has unterminated " << context->open_instance << " instance");
    }
  }

  if(context->open_instance < 0)
  {
    context->z->print_report(RS_WARNING, stringize() << L"Warning 21: Instance in MSC (" << TOWSTRING(context->msc_name) << ") was terminated multiple times");
  }

  std::set<std::string> nodes_names;

  if(context->future_top_time_relations.size() > 0)
  {
    std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator it;
    std::set<TimeRelationRefNodePtr>::iterator set_it;    
    ReferenceNode* ref_node;

    for(it = context->future_top_time_relations.begin(); it != context->future_top_time_relations.end(); it++)
    {
      nodes_names.insert(it->first);

      for(set_it = it->second.begin(); set_it != it->second.end(); set_it++)
      {
        ref_node = (*set_it)->get_ref_node_a();
        if((*set_it)->is_bottom_node_a())
        {
          ref_node->remove_time_relation_bottom(*set_it);
        }
        else
        {
          ref_node->remove_time_relation_top(*set_it);
        }
      }
    }
    context->future_top_time_relations.clear();
  }

  if(context->future_bottom_time_relations.size() > 0)
  {
    std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator it;
    std::set<TimeRelationRefNodePtr>::iterator set_it;    
    ReferenceNode* ref_node;

    for(it = context->future_bottom_time_relations.begin(); it != context->future_bottom_time_relations.end(); ++it)
    {
      nodes_names.insert(it->first);

      for(set_it = it->second.begin(); set_it != it->second.end(); set_it++)
      {
        ref_node = (*set_it)->get_ref_node_a();
        if((*set_it)->is_bottom_node_a())
        {
          ref_node->remove_time_relation_bottom(*set_it);
        }
        else
        {
          ref_node->remove_time_relation_top(*set_it);
        }
      }
    }
    context->future_bottom_time_relations.clear();
  }

  if(!nodes_names.empty())
  {
    std::set<std::string>::iterator it;
    std::string names;

    for(it = nodes_names.begin(); it != nodes_names.end(); it++)
    {
      if(it != nodes_names.begin())
      {
        names += ", ";
      }

      names += (*it);
     }

     context->z->print_report(RS_WARNING, stringize() << L"Warning 24: Time information to nonexisting reference node: " << TOWSTRING(names));

   }

  context->error_inst_names.clear();
  context->error_nodes_names.clear();
  context->error_event_names.clear();

}

void check_references_fun(struct Context* context)
{
  if(!context->future_reference.empty()){ 
    std::map<std::string, std::set<ReferenceNodePtr> >::iterator map_it;
    std::set<ReferenceNodePtr>::iterator ref_it;
    std::string names = "";
   
    for(map_it=context->future_reference.begin(); map_it!=context->future_reference.end(); ++map_it){
      BMscPtr bmsc = new BMsc(TOWSTRING(map_it->first));
      if(map_it != context->future_reference.begin())
      {
        names += ", ";
      }
      names += map_it->first;
      
      for(ref_it=map_it->second.begin(); ref_it!=map_it->second.end(); ++ref_it){
        (*ref_it)->set_msc(bmsc);
      }
    }
    context->z->print_report(RS_WARNING, stringize() << L"Warning 09: Reference to nonexisting MSC: " << TOWSTRING(names));
  }
}

void msc_was_read_fun(struct Context* context)
{
  if(!context->end_msc)
  {
    context->z->print_report(RS_WARNING, stringize() << L"Warning 12: Incorrectly terminated MSC (" << TOWSTRING(context->msc_name) << "); missing 'endmsc;'" );
  }

  if(context->mscs.find(context->msc_name) != context->mscs.end()){
    if(context->error_mscs_names.find(context->msc_name) == context->error_mscs_names.end())
    {
      context->z->print_report(RS_WARNING, stringize() << "Warning 18: Multiple MSCs have the same name: " << TOWSTRING(context->msc_name) );
      context->error_mscs_names.insert(context->msc_name);
    }
  }

  if(context->mscs.find(context->msc_name) == context->mscs.end())
  {
     context->msc_file_order.push_back(context->msc_name);  

     if(context->myBmsc == NULL)
       context->hmscs_order.push_back(context->msc_name);
     else
       context->bmscs_order.push_back(context->msc_name);
  }

  context->mscs.insert(std::make_pair(context->msc_name, get_msc_fun(context)));
 
  std::map<std::string, std::set<ReferenceNodePtr> >::iterator future_ref_it;
  future_ref_it = context->future_reference.find(context->msc_name);

  if(future_ref_it == context->future_reference.end())
  {
    context->nonpointed.insert(context->msc_name);
  }
  else
  {
    MscPtr msc = get_msc_fun(context);
    std::set<ReferenceNodePtr>::iterator references = future_ref_it->second.begin();
    for(; references != future_ref_it->second.end(); ++references)
    {
        (*references)->set_msc(msc);
    }
    context->future_reference.erase(future_ref_it);
  
  }

}

/*
 * Bug report from grammar
 */
void bug_report_fun(struct Context* context, char* report){
  context->z->print_report(RS_ERROR, TOWSTRING(report));
}

void clear_time(struct Context* context)
{
  context->time = "";
  context->absolut_time = false;
  context->absolut_first_border = false;
  context->absolut_first_set = false;
  context->absolut_second_border = false; 
}

void add_time_fun(struct Context* context, char* time_part)
{
  context->time += time_part;
}

void duplicate_time_point(struct Context* context)
{
  if(context->absolut_first_set)
    context->absolut_second_border = context->absolut_first_border;
}

void set_time_fun(struct Context* context, char* time)
{
  context->time = time;
}

void set_absolut_time_true_fun(struct Context* context)
{
  context->absolut_time = true;
}

void set_absolut_first_or_second_true_fun(struct Context* context)
{
  context->absolut_time = true;

  if(!context->absolut_first_set)
  {
    context->absolut_first_set = true;
    context->absolut_first_border = true;
  }
  else
    context->absolut_second_border = true;
}

void set_origin_fun(struct Context* context)
{
  context->origin = true;
}

void set_time_dest_fun(struct Context* context)
{
  context->time_dest = true;
}

/*
 * End of MSC was occurred
 */
void set_end_msc_fun(struct Context* context)
{
  context->end_msc = true;
}

/*
 * FUNCTIONS FOR BMSC
 */

/*
 * Create new BMsc structure
 */
void new_bmsc_fun(struct Context* context)
{
  context->myBmsc = new BMsc(TOWSTRING(context->msc_name));
}

/*
 * Create incomplete message. Type of message is defined in parameter "kind" 
 *     output: LOST message
 *     input: FOUND message 
 */
void incomplete_message_fun(struct Context* context, char* msg_identifications, enum msg_kind kind)
{

  //event has been created before (variable not_create_event will be changed from grammar call (set_not_create_event)
  if(context->not_create_event == 1){
    return ;
  }

  std::string msg_identification(msg_identifications);
  std::string msg_name;
  if(context->no_message_label == 1)
  {
    msg_name = "";
    context->no_message_label = 0;
  }
  else
  {
    size_t pos = msg_identification.rfind(',');
    if(pos != 0)
    {
      msg_name = msg_identification.substr(0, pos);
    }
    else
    {
      msg_name = msg_identification;
    }
  }

  InstancePtr instance;

  std::map<std::string, InstancePtr>::iterator inst_it = context->instances.find(context->element_name);

  if(inst_it == context->instances.end())
  {
    instance = new Instance(TOWSTRING(context->element_name));
    context->instances.insert(std::make_pair(context->element_name, instance));
    context->myBmsc->add_instance(instance);
    context->open_instance++;
    context->z->print_report(RS_WARNING, 
      stringize() << "Warning 23: Instance (" << TOWSTRING(context->element_name) << ") has not been started (e.g. " << TOWSTRING(context->element_name) << ": instance;)");
  }
  else
  {
    instance = inst_it->second;
  }

  if (instance->get_last() == NULL ||
      context->coregion_area_finished.find(context->element_name) != context->coregion_area_finished.end())
  {
    StrictOrderAreaPtr strict(new StrictOrderArea());
    instance->add_area(strict);
    context->coregion_area_finished.erase(context->element_name);
  }

  std::map<std::string, EventPtr>::iterator event_it; 
  EventPtr event;

  //if the event was created prematurely
  if(context->event_name == "" || 
     (event_it = context->future_events.find(context->event_name)) == context->future_events.end())
  {
    event = instance->get_last()->add_event();
  }
  else
  {
    event = event_it->second;
    context->future_events.erase(event_it);
  }
  
  context->current_event = event;

  IncompleteMessagePtr message;

  if(kind == output)
  {
    message = new IncompleteMessage(LOST, TOWSTRING(msg_name));
  }
  else
  {
    message = new IncompleteMessage(FOUND, TOWSTRING(msg_name));
  }

  message->glue_event(event);
  
  //if event is named, it adds event to named_events
  if(context->event_name != "")
  {
      context->named_events.insert(std::make_pair(context->event_name, event));
  }

  create_future_time_relations(context); 
  context->event_name = "";
}

/*
 * Add name to order_events
 */
void add_order_event_fun(struct Context* context, char* name){

  //event has been created before (variable not_create_event will be changed from grammar call (set_not_create_event)
  if(context->not_create_event == 1){
    return ;
  }

  context->order_events.insert(name);
}

/*
 * Create new instance in BMsc
 */
void new_instance_fun(struct Context* context)
{
  InstancePtr instance(new Instance(TOWSTRING(context->element_name)));
  if(context->instances.find(context->element_name) != context->instances.end())
  {
    if(context->error_inst_names.find(context->element_name) == context->error_inst_names.end())
    {
      context->z->print_report(RS_WARNING, stringize() << L"Warning 16: Multiple instances with the same name: " << TOWSTRING(context->element_name));
      context->error_inst_names.insert(context->element_name);
    }
  }
  else{
    context->instances.insert(std::make_pair(context->element_name, instance));
  }
  context->myBmsc->add_instance(instance);
  context->open_instance++;
}

/*
 * Create new CorregionArea on instance
 */
void start_coregion_fun(struct Context* context)
{
  std::set<std::string>::iterator it = context->coregion_area_finished.find(context->element_name);
  if(it != context->coregion_area_finished.end())
  {
    context->coregion_area_finished.erase(it);
  }

  if(context->coregion_area_opened.find(context->element_name) == context->coregion_area_opened.end())
  {
    InstancePtr instance = context->instances.find(context->element_name)->second;
    CoregionAreaPtr coregion(new CoregionArea());
    instance->add_area(coregion);
    context->coregion_area_opened.insert(context->element_name);
  }
  else
  {
    context->z->print_report(RS_WARNING, stringize() << "Warning 10: Instance (" << TOWSTRING(context->element_name) << ") has overlapping coregions");
  }
}

/*
 * Finished CorregionArea on instance (add instance to set of currently finished instances)
 */
void end_coregion_fun(struct Context* context)
{
  if(context->coregion_area_opened.find(context->element_name) != context->coregion_area_opened.end())
  {
    InstancePtr instance = context->instances.find(context->element_name)->second;
    context->coregion_area_finished.insert(context->element_name);
    context->coregion_area_opened.erase(context->element_name);
  
}
  else
  {
    context->z->print_report(RS_WARNING, stringize() << "Warning 11: Instance (" << TOWSTRING(context->element_name) << ") does not have any open coregion\n");      
  }
}

/*
 * Connect input message
 */
void message_fun(struct Context* context, char* msg_identifications, enum msg_kind kind)
{
  
  //event has been created before (variable not_create_event will be changed from grammar call (set_not_create_event)
  if(context->not_create_event == 1){
    return ;
  }
  //message identification
  std::string msg_identification(msg_identifications);
  std::string msg_name;
  
  if(context->no_message_label == 1)
  {
    msg_name = "";
    context->no_message_label = 0;
  }
  else
  {
    size_t pos = msg_identification.rfind(',');
    if(pos != 0)
    {
      msg_name = msg_identification.substr(0, pos);
    }
    else
    {
      msg_name = msg_identification;
    }
  }

  InstancePtr instance;

  std::map<std::string, InstancePtr>::iterator inst_it = context->instances.find(context->element_name);

  if(inst_it == context->instances.end())
  {
    instance = new Instance(TOWSTRING(context->element_name));
    context->instances.insert(std::make_pair(context->element_name, instance));
    context->myBmsc->add_instance(instance);
    context->open_instance++;
    context->z->print_report(RS_WARNING, 
      stringize() << "Warning 23: Instance (" << TOWSTRING(context->element_name) << ") has not been started (e.g. " << TOWSTRING(context->element_name) << ": instance;)");
  }
  else
  {
    instance = inst_it->second;
  }

  //add new strict area to instance if it is needed
  if (instance->get_last() == NULL || 
      context->coregion_area_finished.find(context->element_name) != context->coregion_area_finished.end())
  {
    StrictOrderAreaPtr strict(new StrictOrderArea());
    instance->add_area(strict);
    context->coregion_area_finished.erase(context->element_name);
  }

  std::map<std::string, EventPtr>::iterator event_it; 
  EventPtr event;
  
  //if the event was created prematurely
  if(context->event_name == "" || 
     (event_it = context->future_events.find(context->event_name)) == context->future_events.end())
  {
    event = instance->get_last()->add_event();
  }
  else
  {
    event = event_it->second;
    context->future_events.erase(event_it);
  }

  context->current_event = event;

  std::multimap<std::string, CompleteMessagePtr>::iterator message_it;
  message_it = context->messages.find(msg_identification);

  if(kind == input){
    if(message_it != context->messages.end() && message_it->second->get_receive_event() == NULL){
      message_it->second->glue_receive_event(event);
      context->messages.erase(message_it);
    }
    else{
      CompleteMessagePtr message = new CompleteMessage(TOWSTRING(msg_name));
      message->glue_receive_event(event);
      context->messages.insert(std::make_pair(msg_identification, message));
    }
  }
  else{
    if (message_it != context->messages.end() && message_it->second->get_send_event() == NULL)
    {
      message_it->second->glue_send_event(event);
      context->messages.erase(message_it);
    }
    else{
      CompleteMessagePtr message = new CompleteMessage(TOWSTRING(msg_name));
      message->glue_send_event(event);
      context->messages.insert(std::make_pair(msg_identification, message));
    }
  }

  //if event is named, it adds event to named_events
  if(context->event_name != "")
  {
      context->named_events.insert(std::make_pair(context->event_name, event));
  }
 
  create_future_time_relations(context); 
  context->event_name = "";
}

/*
 * Set successors or predecessors of coregion event
 *     successors: kind = before
 *     predecessors: kind = after
 */
void add_relation_fun(struct Context* context, enum relation_kind kind){
  
  //event has been created before (variable not_create_event will be changed from grammar call (set_not_create_event)
  if(context->not_create_event == 1){
    return ;
  }

  std::set<std::string>::iterator it;
  std::map<std::string, EventPtr>::iterator named_event_it;
  CoregionEventPtr event = boost::dynamic_pointer_cast<CoregionEvent>(context->current_event);

  if(event == NULL){
    std::wstring warning = L"Warning 13: Event of " + context->current_event->get_message()->get_label();
    warning = warning + L" message is not in coregion";
    context->z->print_report(RS_WARNING, stringize() << warning);
    return;
  }

  for(it = context->order_events.begin(); it != context->order_events.end(); ++it)
  {
    CoregionEventPtr event1;

    if((named_event_it = context->named_events.find(*it)) != context->named_events.end())
    {
      event1 = boost::dynamic_pointer_cast<CoregionEvent>(named_event_it->second);
    }
    else
    {
      std::map<std::string, EventPtr>::iterator future_it = context->future_events.find(*it);

      if(future_it != context->future_events.end())
      {
        event1 = boost::dynamic_pointer_cast<CoregionEvent>(future_it->second);
      }
      else
      {
        InstancePtr instance = context->instances.find(context->element_name)->second;
        EventPtr e = instance->get_last()->add_event();
        event1 = boost::dynamic_pointer_cast<CoregionEvent>(e);

        context->future_events.insert(std::make_pair(*it, event1));
      }
    }

    if(event1 == event)
    {
      context->z->print_report(RS_WARNING, stringize() << L"Warning 33: wrong before/after relation");
      context->order_events.clear();
      return ;
    }

    if(event1 != NULL)
    {
      if(kind == before)
        event->add_successor(event1.get());
      else
        event1->add_successor(event.get());
    }
  }

  context->order_events.clear();
}

/*
 * Set name of instance
 */
void set_instance_name_fun(struct Context* context, char* name)
{
  context->element_name = name;
}

void set_event_name_fun(struct Context* context, char* name)
{
  if(context->named_events.find(name) == context->named_events.end()){
    context->event_name = name;
  }
  else{
    context->not_create_event = 1;
    if(context->error_event_names.find(name) == context->error_event_names.end())
    {
      context->z->print_report(RS_WARNING, stringize() << L"Warning 17: Multiple events with the same name: " << TOWSTRING(name));
      context->error_event_names.insert(name);
    }
  }
}

void set_not_create_event_fun(struct Context* context)
{
  context->not_create_event = 0;
}

void missing_message_label_fun(struct Context* context)
{
  context->no_message_label = 1;
  context->z->print_report(RS_WARNING, stringize() << L"Warning 19: Message without label on the instance (" << TOWSTRING(context->element_name) << ")");
}

void end_instance_fun(struct Context* context){
    context->open_instance--;
}

void set_time_event_fun(struct Context* context)
{
   context->time_event = true;
}

void set_time_event_false_fun(struct Context* context)
{
    context->time_event = false;
}

void add_time_relation_event_fun(struct Context* context)
{
  if(context->absolut_time)
  {
    add_event_absolut_time_fun(context);
    return ;
  }

  TimeRelationEventPtr relation;
  try
  {
    relation = new TimeRelationEvent(context->time);
    relation->set_directed(context->origin);
    context->origin = false;
  }
  catch(std::exception &exc)
  {
    context->z->print_report(RS_ERROR, stringize() << L"Warning: " << exc.what());
    return ;
  }

  std::map<std::string, EventPtr>::iterator named_events_it = context->named_events.find(context->time_event_name);

  if(named_events_it != context->named_events.end())
  {
    relation->glue_events(context->current_event.get(), named_events_it->second.get());
  }
  else
  {
    relation->glue_event_a(context->current_event.get());

    std::map<std::string, std::set<TimeRelationEventPtr> >::iterator it = context->future_time_events.find(context->time_event_name);
 
    if(it != context->future_time_events.end())
    {
      it->second.insert(relation);
    }
    else
    {
      std::set<TimeRelationEventPtr> my_set;
      my_set.insert(relation);
      context->future_time_events.insert(std::make_pair(context->time_event_name, my_set));
    }
  }
}

void  add_event_absolut_time_fun(struct Context* context)
{
  //checking the corectness of syntax of the absolut time
  if(!(context->absolut_first_border == context->absolut_second_border))
  {
    context->z->print_report(RS_ERROR, stringize() << L"Warning: wrong definition of absolut time"); 
  }
  
  context->current_event->add_absolut_time(*(new MscTimeIntervalSet<double>(context->time)));

  context->absolut_first_border = false;
  context->absolut_second_border = false;
  context->absolut_first_set = false;
  context->absolut_time = false;  
}

/*
 * Set name of event which is in time relation with the event
 */
void set_time_reference_event_fun(struct Context* context, char* name)
{
  context->time_event_name = name;
  context->time_second = unknown;
}

void create_future_time_relations(struct Context* context)
{
  std::map<std::string, std::set<TimeRelationEventPtr> >::iterator it = context->future_time_events.find(context->event_name);
  std::set<TimeRelationEventPtr>::iterator set_it;

  if(it != context->future_time_events.end())
  {
    for(set_it = it->second.begin(); set_it != it->second.end(); ++set_it)
    {
      (*set_it)->glue_event_b(context->current_event.get());
    }
  }
}

void set_data_parameter_decl(struct Context* context)
{
  if(!context->data_parameter_decl) context->data_parameter_decl = true;
  else context->z->print_report(RS_WARNING, L"Warning 26: Data parameters were declared multiple times");
}

void set_instance_parameter_decl(struct Context* context)
{
  if(!context->instance_parameter_decl) context->instance_parameter_decl = true;
  else context->z->print_report(RS_WARNING, L"Warning 27: Instance parameters were declared multiple times");
}

void set_message_parameter_decl(struct Context* context)
{
  if(!context->message_parameter_decl) context->message_parameter_decl = true;
  else context->z->print_report(RS_WARNING, L"Warning 28: Message parameters were declared multiple times");
}

void set_timer_parameter_decl(struct Context* context)
{
  if(!context->timer_parameter_decl) context->timer_parameter_decl = true;
  else context->z->print_report(RS_WARNING, L"Warning 29: Timer parameters were declared multiple times");
}


/*
 * FUNCTIONS FOR HMSC
 */

/*
 * Create new HMsc
 */
void new_hmsc_fun(struct Context* context)
{
  context->myHmsc = new HMsc(TOWSTRING(context->msc_name));
}

/*
 * Create start node of HMsc
 */
void new_start_node_fun(struct Context* context)
{
  StartNodePtr node = new StartNode();
  context->myHmsc->set_start(node);
  context->start_node = node;
  future_connection_fill_in_fun(context);
}

/*
 * Create end node of HMsc
 */
void new_end_node_fun(struct Context* context)
{
    EndNodePtr node = new EndNode();
    context->myHmsc->add_node(node);
    context->end_node = std::make_pair(context->element_name, node);
    create_future_connections_fun(context, node.get());
}

/*
 * Set reference name
 */
void set_reference_name_fun(struct Context* context, char* name)
{
  context->reference_name = name;
}

/*
 * Set condition name
 */
void set_condition_name_fun(struct Context* context, char* name)
{
  context->condition_name += name;
}

void clear_condition_name_fun(struct Context* context)
{
  context->condition_name = "";
}

/*
 * Set name of node
 */
void set_node_name_fun(struct Context* context, char* name)
{
  context->element_name = name;
}

/*
 * Add name of node to connect to current node
 */
void add_connect_name_fun(struct Context* context, char* name)
{
  context->connect_name.push_back(name);
}

/*
 * Create reference node
 */
void new_reference_node_fun(struct Context* context)
{
  if(TOWSTRING(context->reference_name) == get_msc_fun(context)->get_label())
  {
    context->z->print_report(RS_WARNING, stringize() << L"Warning 31: Infinite recursion at node: " << TOWSTRING(context->element_name));
  }

  std::map<std::string, HMscNodePtr>::iterator it = context->hmsc_nodes.find(context->element_name);
  std::multimap<std::string, MscPtr>::iterator msc_it = context->mscs.find(context->reference_name);
  ReferenceNodePtr node = NULL;

  if (it == context->hmsc_nodes.end())
  {
    node = new ReferenceNode();
    std::multimap<std::string, MscPtr>::iterator msc_it = context->mscs.find(context->reference_name);
  
    if(msc_it == context->mscs.end())
    {
        std::map<std::string, std::set<ReferenceNodePtr> >::iterator ref_it = context->future_reference.find(context->reference_name);
      
        if(ref_it != context->future_reference.end())
        {
          ref_it->second.insert(node);
        }
        else
        {
          std::set<ReferenceNodePtr> my_set;
          my_set.insert(node);
          context->future_reference.insert(std::make_pair(context->reference_name, my_set));
        }
    }
    else
    {
      node->set_msc(msc_it->second);
  
      std::set<std::string>::iterator erase_it = context->nonpointed.find(context->reference_name);
      if(erase_it != context->nonpointed.end())
      {
        context->nonpointed.erase(erase_it);
      }
    }

    context->hmsc_nodes.insert(make_pair(context->element_name, node));
    context->myHmsc->add_node(node);
    context->current_node = node;
    context->node_type = reference;

    std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator top_it;
    std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator bottom_it;

    top_it = context->future_top_time_relations.find(context->element_name);
    bottom_it = context->future_bottom_time_relations.find(context->element_name);

    if(top_it != context->future_top_time_relations.end())
    {
      std::set<TimeRelationRefNodePtr>::iterator set_it;
      
      for(set_it = top_it->second.begin(); set_it != top_it->second.end(); ++set_it)
      {
        (*set_it)->glue_ref_node_b(false, node.get());
      }
      
      context->future_top_time_relations.erase(top_it);
    }

    if(bottom_it != context->future_bottom_time_relations.end())
    {
      std::set<TimeRelationRefNodePtr>::iterator set_it;

      for(set_it = bottom_it->second.begin(); set_it != bottom_it->second.end(); ++set_it)
      {
        (*set_it)->glue_ref_node_b(true, node.get());
      }

      context->future_bottom_time_relations.erase(bottom_it);
    }
  }
  else
  {
    if(context->error_nodes_names.find(context->element_name) == context->error_nodes_names.end())
    {
      context->z->print_report(RS_WARNING, stringize() << L"Warning 15: Multiple nodes with the same name: " << TOWSTRING(context->element_name));
      context->error_nodes_names.insert(context->element_name);
    }

    context->not_connect = 1;
  }
}

/*
 * Create connection node
 */
void new_connection_node_fun(struct Context* context)
{
  std::map<std::string, HMscNodePtr>::iterator it = context->hmsc_nodes.find(context->element_name);

  if (it == context->hmsc_nodes.end())
  {
    ConnectionNodePtr node = new ConnectionNode();
    context->hmsc_nodes.insert(std::make_pair(context->element_name, node));
    context->myHmsc->add_node(node);
    context->current_node = node;
    context->node_type = connection;
  }
  else
  {
    if(context->error_nodes_names.find(context->element_name) == context->error_nodes_names.end())
    {
      context->z->print_report(RS_WARNING, stringize() << L"Warning 15: Multiple nodes with the same name: " << TOWSTRING(context->element_name));
      context->error_nodes_names.insert(context->element_name);
    }

    context->not_connect = 1;
  }
}

/*
 * Create condition node
 */
void new_condition_node_fun(struct Context* context)
{
  std::map<std::string, HMscNodePtr>::iterator it = context->hmsc_nodes.find(context->element_name);

  if (it == context->hmsc_nodes.end())
  {
    try
    {
      ConditionNodePtr node = new ConditionNode(context->condition_name);
      context->hmsc_nodes.insert(std::make_pair(context->element_name, node));
      context->myHmsc->add_node(node);
      context->current_node = node;
      context->node_type = condition;
    }
    catch(std::exception &exc)
    {
      context->z->print_report(RS_ERROR, stringize() << L"Warning: " << exc.what());
      context->error_nodes_names.insert(context->element_name);
    }
  }
  else
  {
    if(context->error_nodes_names.find(context->element_name) == context->error_nodes_names.end())
    {
      context->z->print_report(RS_WARNING, stringize() << L"Warning 15: Multiple nodes with the same name: " << TOWSTRING(context->element_name));
      context->error_nodes_names.insert(context->element_name);
    }

    context->not_connect = 1;
  }
}

void create_connections_fun(struct Context* context)
{
  if(context->not_connect == 1)
  {
    context->connect_name.clear();
    context->not_connect = 0;
    return ;
  }

  future_connection_fill_in_fun(context);
  
  if(context->node_type == reference)
  {
    ReferenceNodePtr node = boost::dynamic_pointer_cast<ReferenceNode>(context->current_node);
    create_future_connections_fun(context, node.get());
  }
 
  if(context->node_type == connection)
  {
    ConnectionNodePtr node = boost::dynamic_pointer_cast<ConnectionNode>(context->current_node);
    create_future_connections_fun(context, node.get());
  }

  if(context->node_type == condition)
  {
    ConditionNodePtr node = boost::dynamic_pointer_cast<ConditionNode>(context->current_node);
    create_future_connections_fun(context, node.get());
  }

  context->connect_name.clear();
}

/*
 * Create connections among curently created node and existed nodes or create record in future_connections 
 */
void future_connection_fill_in_fun(struct Context* context)
{
  if(context->element_name == "")
  {
    context->element_name = "start";
  }
  
  while (!context->connect_name.empty())
  {
    std::map<std::string, std::set<std::string> >::iterator future_it;
    std::map<std::string, HMscNodePtr>::iterator hmsc_it;

    future_it = context->future_connections.find(context->connect_name.front());
    hmsc_it = context->hmsc_nodes.find(context->connect_name.front());             //node which has to be connected to current node

    if (hmsc_it == context->hmsc_nodes.end() && context->connect_name.front() != context->end_node.first)
    {
      if (future_it == context->future_connections.end())
      { //check of record existence in future connections for this node
        std::set<std::string> my_set;
        my_set.insert(context->element_name);
        context->future_connections.insert(std::make_pair(context->connect_name.front(), my_set));
        context->connect_name.pop_front();
      }
      else
      {
        future_it->second.insert(context->element_name);
        context->connect_name.pop_front();
      }
    }
    else
    {
      std::map<std::string, HMscNodePtr>::iterator element_node_it = context->hmsc_nodes.find(context->element_name);
      PredecessorNode* node = dynamic_cast<PredecessorNode*> (element_node_it->second.get());
      if (hmsc_it != context->hmsc_nodes.end())
      {
        SuccessorNode* succ = dynamic_cast<SuccessorNode*> (hmsc_it->second.get());
        node->add_successor(succ);
        context->connect_name.pop_front();
      }
      else
      {
        if (context->connect_name.front() == context->end_node.first)
        {
          node->add_successor(context->end_node.second.get());
          context->connect_name.pop_front();
        }
      }
    }
  }
}

/*
 * Create connections among node and others hmsc nodes by future_connections
 */
void create_future_connections_fun(struct Context* context, SuccessorNode* succ)
{
  std::map<std::string, std::set<std::string> >::iterator future_it;
  future_it = context->future_connections.find(context->element_name);

  if (future_it != context->future_connections.end())
  {
    std::set<std::string>::iterator set_it;
    for (set_it = future_it->second.begin(); set_it != future_it->second.end(); ++set_it)
    {
      std::map<std::string, HMscNodePtr>::iterator hmsc_node_it = context->hmsc_nodes.find(*set_it);
      if (hmsc_node_it != context->hmsc_nodes.end())
      {
        PredecessorNode* node = dynamic_cast<PredecessorNode*> (hmsc_node_it->second.get());
        node->add_successor(succ);
      }
      else
      {
        PredecessorNode* node = dynamic_cast<PredecessorNode*> (context->start_node.get());
	node->add_successor(succ);
      }
    }
    
    context->future_connections.erase(future_it);
  }
}


/*
 * Set time_ to define where the time interval should be connected
 */
void set_time_reference_node_fun(struct Context* context, char* name)
{
  context->time_node_name = name;
}

/*
 * Set time interval from top to bottom on the same node
 */
void add_time_relation_ref_time_fun(struct Context* context)
{
  if(context->time_dest)
  {
    std::string report = "";
    
    if(context->time_second != unknown) 
      report = (context->time_second ? "bottom " : "top ") + context->time_node_name;
    else
      report = context->time_event_name;

    if(context->origin)
      report += " origin";
    
    context->z->print_report(RS_WARNING,  stringize() << "Warning 32: Unexpected (" << TOWSTRING(report) << ") at time definition: " 
    << TOWSTRING(context->element_name));
  } 

  TimeRelationRefNodePtr relation;
  try
  {
    relation = new TimeRelationRefNode(context->time);
  }
  catch(std::exception &exc)
  {
    context->z->print_report(RS_ERROR, stringize() << L"Warning: " << exc.what());
    return ;
  }

  ReferenceNodePtr node = boost::dynamic_pointer_cast<ReferenceNode>(context->current_node);    

  if(node != NULL)
  {
    relation->glue_ref_nodes(false, node.get(), true, node.get());
  }
  else
  {
//    std::cout << " warning " << std::endl;
  }
}

/*
 * Set time interval between current node and another node. 
 */
void add_time_relation_ref_fun(struct Context* context)
{
  if(context->time_event)
  {
    add_time_relation_event_fun(context);
    context->time_dest = false;
    return ;
  }

  if(context->time_first == time_def)
  {
    add_time_relation_ref_time_fun(context);
    context->time_dest = false;
    return ;
  }

  if(context->time_second == unknown || context->time_second == time_def)
  {
    context->z->print_report(RS_WARNING,  stringize() << "Warning 25: Time definition in " << TOWSTRING(context->element_name) << " has only one connection point");
    context->time_second = context->time_first;
    context->time_node_name = context->time_event_name;
  }

  TimeRelationRefNodePtr relation;
  try
  {
    relation = new TimeRelationRefNode(context->time);
    relation->set_directed(context->origin);
    context->origin = false;
  }
  catch(std::exception &exc)
  {
    context->z->print_report(RS_ERROR, stringize() << L"Warning: " << exc.what());
    context->time_dest = false;
    return ;
  }

  std::map<std::string, HMscNodePtr>::iterator ref_it; 
  ReferenceNodePtr node; 
  std::set<TimeRelationRefNodePtr> my_set;

  ref_it = context->hmsc_nodes.find(context->time_node_name);
  node = boost::dynamic_pointer_cast<ReferenceNode>(context->current_node);

  if(node == NULL)
  {
    context->z->print_report(RS_ERROR, L"Warning 23: Time information can be defined only to a reference node");
    context->time_dest = false;
    return ;
  }

    if(ref_it != context->hmsc_nodes.end())
    {
      ReferenceNodePtr node2 = boost::dynamic_pointer_cast<ReferenceNode>(ref_it->second);
      if(node2 == NULL)
      {
//        context->z->print_report(RS_ERROR, L"Warning 23: Time information can be defined only to a reference node");
//        return ;
//--------------------------------------------------------------------------------------------------------------------
// added due to the same warning with 24 to print     
      relation->glue_ref_node_a(context->time_first == bottom, node.get());

      if(context->time_second == top)
      {
        std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator top_it;
        top_it = context->future_top_time_relations.find(context->time_node_name);

        if(top_it == context->future_top_time_relations.end())
        {
          my_set.insert(relation);
          context->future_top_time_relations.insert(std::make_pair(context->time_node_name, my_set));
        }
        else
        {
          top_it->second.insert(relation);
        }
      }
      else{
        std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator bottom_it;
        bottom_it = context->future_bottom_time_relations.find(context->time_node_name);

        if(bottom_it == context->future_bottom_time_relations.end())
        {
          my_set.insert(relation);
          context->future_bottom_time_relations.insert(std::make_pair(context->time_node_name, my_set));
        }
        else
        {
          bottom_it->second.insert(relation);
        }
      }
      context->time_dest = false;
      return;
//----------------------------------------------------------------------------------------------------------------------------          
      }

      if(context->time_first == context->time_second && context->time_first != unknown && node2 == node)
      {
        context->z->print_report(RS_ERROR, L"Warning 30: Time interval was defined to one point");
        context->time_dest = false;
        return ;
      }

      relation->glue_ref_nodes(context->time_first == bottom, node.get(), context->time_second == bottom, node2.get());
    }
    else
    {
      relation->glue_ref_node_a(context->time_first == bottom, node.get());
      
      if(context->time_second == top)
      {
        std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator top_it;
        top_it = context->future_top_time_relations.find(context->time_node_name);

        if(top_it == context->future_top_time_relations.end())
        {
          my_set.insert(relation);
          context->future_top_time_relations.insert(std::make_pair(context->time_node_name, my_set));
        }
        else
        {
          top_it->second.insert(relation);
        }
      }
      else{
        std::map<std::string, std::set<TimeRelationRefNodePtr> >::iterator bottom_it;
        bottom_it = context->future_bottom_time_relations.find(context->time_node_name);

        if(bottom_it == context->future_bottom_time_relations.end())
        {  
          my_set.insert(relation);
          context->future_bottom_time_relations.insert(std::make_pair(context->time_node_name, my_set));
        }
        else
        {
          bottom_it->second.insert(relation);
        }
      }
    }
    
  context->time_second = unknown;
  context->time_dest = false;
}

/*
 * Set time_first to define where the time interval should be connected
 */
void set_first_time_rel_kind_fun(struct Context* context, enum time_relation_kind kind)
{
   context->time_first = kind;
}

/*
 * Set time_second to define where the time interval should be connected
 */
void set_second_time_rel_kind_fun(struct Context* context, enum time_relation_kind time_relation_type)
{
  context->time_second = time_relation_type;
}

void add_global_comment(struct Context* context, char* text)
{
  std::string complete = text; 

  complete = complete.substr(complete.find("'")+1, complete.size());
  complete = complete.substr(0, complete.rfind("'"));

  CommentPtr comment = new Comment(TOWSTRING(complete));

  if(context->myBmsc != NULL)
    context->myBmsc->add_comment(comment);

  if(context->myHmsc != NULL)
    context->myHmsc->add_comment(comment);
}

void add_element_comment(struct Context* context, char* text)
{
  std::string complete = text;

  complete = complete.substr(complete.find("'")+1, complete.size());
  complete = complete.substr(0, complete.rfind("'"));

  CommentPtr comment = new Comment(TOWSTRING(complete));

  if(context->myBmsc != NULL && context->current_event != NULL)
    context->current_event->add_comment(comment);

  if(context->myHmsc != NULL && context->current_node != NULL)
    context->current_node->add_comment(comment);

}

#endif

// $Id: Context.cpp 1006 2010-12-08 20:34:49Z madzin $
