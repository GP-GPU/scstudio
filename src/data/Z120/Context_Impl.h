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
 * $Id: Context_Impl.h 1006 2010-12-08 20:34:49Z madzin $
 */

/*
 * we use ANTRL v3 (ANTLR 3.1.1)
 * see http://www.antlr.org
 */

#ifndef _Context_Impl_
#define _Context_Impl_

#include "data/Z120/Context.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <set>
#include "data/msc.h"
#include "z120.h"

enum hmsc_node_type {reference, connection, condition};

struct Context
{
  std::string msc_name;

  /*
   * BMsc: name of instance
   * HMsc: name of node
   */
  std::string element_name;

  /*
   * Textual file
   */
  typedef std::multimap<std::string, MscPtr> MscPtrMap;
  MscPtrMap mscs; // map of msc in the file

  std::vector<std::string> hmscs_order; //HMSCs order in the file
  std::vector<std::string> bmscs_order; //BMSCs order in the file
  std::vector<std::string> msc_file_order; //msc ordering in the file

  typedef std::set<std::string> MscNameSet;
  MscNameSet nonpointed; // msc which is not refered
  std::map<std::string, std::set<ReferenceNodePtr> > future_reference; //map of name of nodes on which was refered and does not exist
  Z120* z;

  /*
  * Time interval 
  */
  std::string interval_label;
  std::string time;
  bool origin;
  bool time_dest;
  bool absolut_time;
  bool absolut_first_border;
  bool absolut_first_set;
  bool absolut_second_border;

  /*
   * Error handling
   */
  std::set<std::string> error_inst_names; //names of instances which were declared more times (two or more instances with the same name)
  std::set<std::string> error_mscs_names; //names of mscs which were declared more times
  std::set<std::string> error_event_names; //names of events which were declared more times
  std::set<std::string> error_nodes_names; //names of nodes which were declared more times
 
  bool end_msc;

  /*
   * BMsc
   */
  BMscPtr myBmsc;
  std::set<std::string> coregion_area_finished; // set of instances which has currently finished coregion area
  std::set<std::string> coregion_area_opened; //set of instances which has currently opend coregion area
  std::map<std::string, InstancePtr> instances; // map of instances (key: name of instance, value: smart pointer to instance)
  std::multimap<std::string, CompleteMessagePtr> messages; // map of future complete messages  
  std::map<std::string, EventPtr> future_events; // map of name of events on which was pointed before they were created
  std::map<std::string, std::set<TimeRelationEventPtr> > future_time_events; // map of name of events on which was created time relation before they were created
  std::map<std::string, EventPtr> named_events; // map of name of events
  std::set<std::string> order_events; //set of name of events which takes place after keywords 'before', 'after'
  EventPtr current_event; //event which was currently created
  std::string event_name; //name of event
  std::string time_event_name; //name of event on which points a reference in the time relation
  bool time_event;
  int not_create_event; //flag for event in case msc has two labeled event with the same name
  int no_message_label; //flag for message which does not have label
  int open_instance; //counter of open instances


   //variables for syntactic grammar solutions
  bool data_parameter_decl;
  bool instance_parameter_decl;
  bool message_parameter_decl;
  bool timer_parameter_decl;


  /*
   * HMsc
   */
  HMscPtr myHmsc;
  StartNodePtr start_node;
  std::pair<std::string, EndNodePtr> end_node;
  std::list<std::string> connect_name; // name of HMsc nodes which are successors of actual node
  std::string reference_name;

  std::string condition_name;
  HMscNodePtr current_node;

  std::map<std::string, HMscNodePtr> hmsc_nodes; // map of hmsc nodes (key: name of node, value: smart pointer to node)
  std::map<std::string, std::set<std::string> > future_connections; // map of nodes to which node will be connected
  std::map<std::string, std::set<TimeRelationRefNodePtr> > future_top_time_relations;
  std::map<std::string, std::set<TimeRelationRefNodePtr> > future_bottom_time_relations;

  enum hmsc_node_type node_type; //flag to recognize HMsc nodes (ConnectionNode, ReferenceNode, ConditionNode). 
  std::string time_node_name;
  int not_connect;

  enum time_relation_kind time_first;
  enum time_relation_kind time_second;

  ~Context() {}
};

void create_future_connections_fun(struct Context* context, SuccessorNode* succ);

#endif // _Context_Impl_

// $Id: Context_Impl.h 1006 2010-12-08 20:34:49Z madzin $
