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
 * Copyright (c) 2009 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: time_pseudocode.cpp 1074 2011-03-30 17:04:06Z lkorenciak $
 */
#include "time_pseudocode.h"

void BMscIntervalSetMatrix::build_up_matrix()
{
  m_builded = true;
  IntervalSetMatrix::resize(m_size);
  for (unsigned i=0;i<m_size;i++)
  {
    for (unsigned j=0;j<m_size;j++)
    {
      if (j==i)
      {
        operator()(i,j) = MscTimeIntervalD(0,0);
      }
      else
      {
        operator()(i,j)= MscTimeIntervalD(-D::infinity(),D::infinity());
      }
    }
  }

  std::map<std::pair<unsigned,unsigned>, MscTimeIntervalSetD>::const_iterator it;
  for(it=m_position_to_interval.begin();it!=m_position_to_interval.end();it++)
  {
    fill((it->first).first,(it->first).second,it->second);
  }
//  m_matrix_original = *this;
}

void BMscIntervalSetComponentMatrix::build_up_matrix()
{
  EventTopologyHandler event_topology(m_bmsc_original);
  EventPVector temporary_events = event_topology.get_topology();
  //in temporary_events should be topologically sorted elements according to visual_closure_initiator.h

  //initialising of comp operator
  bool (EventTopologyHandler::*comp) (Event* a,Event * b) = NULL;
  if(m_casually)
  {
    event_topology.init_causal(m_mapper); //if doesn't work using race_pos22.mpr for testing may help
    comp = &EventTopologyHandler::causal_is_leq;
  }
  else
  {
    comp = &EventTopologyHandler::visual_is_leq;
  }

  unsigned i; // column and row in matrix

  EventPVector::const_iterator e_v;
  EventPSet::const_iterator e_m;
  std::map<EventP,EventP> related_events;

  //this part of code is for filling realted_events map- used for finding articulation events  
  //making map event to event which is the largest event according to (event_topology.*comp) 
  //with witch has the original event interval in map m_position_to_interval, 
  //futhermore the second event should be larger than first event according to (event_topology.*comp)
  for (e_v=temporary_events.begin();e_v!=temporary_events.end();e_v++)
  {

    TimeRelationEventPtrList::const_iterator it;
    TimeRelationEventPtrList relations = (*e_v)->get_time_relations();

    for (it=relations.begin();it!=relations.end();it++)
    {
      EventP event_a = (*it)->get_event_a();
      EventP event_b;
      if(*e_v == event_a)
	event_b = (*it)->get_event_b();
      else
      {
	event_b = event_a;
	event_a = (*it)->get_event_b();
      }

      if (event_topology.visual_is_leq(event_b,event_a)) // b<=a
      {
	related_events[event_b]=event_a; //I can do this because from topological order follows 
	//that current_event is greater than any other event previously found
	
	// add time relation to the map
// 	m_rel_to_position[it->get()]=std::make_pair(event_b,event_a);
// 	std::pair<EventP,EventP> pair= std::make_pair(event_b,event_a);
// 	
// 	if(m_position_to_interval.find(pair)==m_position_to_interval.end())
// 	  m_position_to_interval[pair] = (*it)->get_interval_set();
// 	else
// 	  m_position_to_interval[pair] = 
// 	    MscTimeIntervalSetD::set_intersection((*it)->get_interval_set(),m_position_to_interval[pair]);
      }
    }
  }

  std::map<std::pair<EventP,EventP>, MscTimeIntervalSetD>::iterator it1;
  for(it1=m_position_to_interval.begin();it1!=m_position_to_interval.end();it1++)
  {
    EventP event_a = m_event_to_bmsc_event[it1->first.first];
    EventP event_b = m_event_to_bmsc_event[it1->first.second];
    if((event_topology.*comp)(event_b,event_a))
    {
      EventP temp = event_a;
      event_a = event_b;
      event_b = temp;
    }
    //event_a < event_b or event_a || event_b
    std::map<EventP,EventP>::iterator it2= related_events.find(event_a);
    if(it2 == related_events.end())
      related_events.insert(std::make_pair<EventP,EventP>(event_a,event_b));
    else 
      if(event_topology.visual_is_leq(it2->second,event_b))
	it2->second = event_b;
  }//end of filling of related_events

  //filling articulation_event_indices vector
  unsigned int offset = 0;
  unsigned int added_non_bmsc_events = 0;
  std::vector<unsigned int> articulation_event_indices;//contains articulation event indices
  EventP end_of_time_relation_span = NULL;
  bool in_some_time_relation = false;
  for(unsigned int i =0;i<temporary_events.size();i++)
  {
    if(in_some_time_relation)
    {
      //check whether we got to the end of the time relation
//      if(end_of_time_relation_span == temporary_events[i])
//	  in_some_time_relation = false; 
      in_some_time_relation = (end_of_time_relation_span != temporary_events[i]);
    }

    std::map<EventP,EventP>::iterator it=  m_bmsc_event_to_event.find(temporary_events[i]);

    if((!in_some_time_relation) && (it == m_bmsc_event_to_event.end())) //not in any relation and no event is added to it
    {
      //checking whether this event is articulation event
      bool is_articulation_event = true;
      for(unsigned int j= offset;j<i;j++)
	is_articulation_event = is_articulation_event && (event_topology.*comp)(temporary_events[j],temporary_events[i]);
      for(unsigned int j= i;is_articulation_event && j<temporary_events.size();j++)
	is_articulation_event = is_articulation_event && (event_topology.*comp)(temporary_events[i],temporary_events[j]);
      
      if(is_articulation_event)
      {
	articulation_event_indices.push_back(i+added_non_bmsc_events);
	offset = i;
      }
    }
    
    //adding events ||assigning numbers
    m_events.push_back(temporary_events[i]);
    
    //adding the events which were prepared for adding by method add_event_to_component
    if(it!=m_bmsc_event_to_event.end())
    {
      added_non_bmsc_events ++;
      EventP event = it->second;
      m_events.push_back(event);
    }
    
    //updating information whether we are in some time relation using related_events map
    std::map<EventP,EventP>::iterator it2= related_events.find(temporary_events[i]);
    if(it2 != related_events.end())
    {
      if(!in_some_time_relation || (event_topology.*comp)(end_of_time_relation_span,it2->second))
	end_of_time_relation_span = it2->second;
      in_some_time_relation = true;
    }
  }
  
  //assign numbers to events
  for (e_v=m_events.begin(),i=0;e_v!=m_events.end();e_v++,i++)
  {
    m_event_to_number[*e_v]=i; // assign number to the event
  }

  //creating components
  unsigned k = 1;
  unsigned last_index =0;
  unsigned temporary_events_index=0;
  if((articulation_event_indices.size() == 0 && m_events.size()) ||
    (articulation_event_indices.size()!=0 && articulation_event_indices.back() != m_events.size()-1))
      articulation_event_indices.push_back(m_events.size()-1);
  if(articulation_event_indices.size()==0 ||articulation_event_indices[0] != 0) k = 0;
  for(;k<articulation_event_indices.size();k++)
  {
    unsigned size = articulation_event_indices[k]-last_index+1;
    IntervalSetMatrix matrix1(size);

    //filling obvious interval sets
    for (unsigned i=0;i<size;i++)
    {
      for (unsigned j=0;j<size;j++)
      {
	if (j==i)
	{
	  matrix1.operator()(i,j) = MscTimeIntervalD(0,0);
	}
	else
	{
	  matrix1.operator()(i,j)= MscTimeIntervalD(-D::infinity(),D::infinity());
	}
      }
    }

    //filling interval sets
    for(unsigned j=last_index;j<articulation_event_indices[k];j++)
    {
      //to make sure we do not call try to find successors of events which are not in bmsc
      if(temporary_events[temporary_events_index] == m_events[j]) //maybe also dynamic cast may work
	temporary_events_index++;
      else 
      {
	continue;
      }

      //filling interval sets which are enforced by partial order (event_topology.*comp)
      EventPSet set = EventFirstSuccessors::get(m_events[j]); // assign to successors (0,inf)
      for (e_m=set.begin();e_m!=set.end();e_m++)
      {
	if((event_topology.*comp)(m_events[j],*e_m) && m_event_to_number[*e_m]-last_index <size)
	{
	  matrix1.operator()(j-last_index,m_event_to_number[*e_m]-last_index)=MscTimeIntervalD(0,D::infinity());
	  //maybe add interval inverse- now I thin it is not necessary
	}      
      }

      //TODO this part of code is possibe to do faster using map <open_relations,first_index>
      //filling interval sets from time relations
      TimeRelationEventPtrList::const_iterator it;
      EventP temp = m_events[j];
      TimeRelationEventPtrList relations = temp->get_time_relations();

      for (it=relations.begin();it!=relations.end();it++)
      {
	EventP event_a = (*it)->get_event_a();
	EventP event_b;
	if(m_events[j] == event_a)
	  event_b = (*it)->get_event_b();
	else
	{
	  event_b = event_a;
	  event_a = (*it)->get_event_b();
	}

	if (event_topology.visual_is_leq(event_a,event_b)) // b<=a
	{
	  // add time relation to the matrix
	  m_rel_to_position[it->get()]=std::make_pair(event_a,event_b);
	  MscTimeIntervalSetD int_set= MscTimeIntervalSetD::set_intersection((*it)->get_interval_set(),matrix1.operator()(j-last_index,m_event_to_number[event_b]-last_index));
	  matrix1.operator()(j-last_index,m_event_to_number[event_b]-last_index)= int_set;
	  matrix1.operator()(m_event_to_number[event_b]-last_index,j-last_index)= MscTimeIntervalSetD::interval_inverse(int_set);
	}
      }
      
    }
    
    //adding component to component matrix
    push_back_component(matrix1);
    last_index = articulation_event_indices[k];
  }

  m_builded = true;

//   //filling constraints which are enforced by partial order (event_topology.*comp)
//   //filling the constraints only to the events which are in same component
//   for (e_v=temporary_events.begin();e_v!=temporary_events.end();e_v++)
//   {
//     EventPSet set = EventFirstSuccessors::get(*e_v); // assign to successors (0,inf)
//     for (e_m=set.begin();e_m!=set.end();e_m++)
//     {
// //       	std::cout<<"event indices in matrix "<<m_event_to_number[*e_v]<<","<<m_event_to_number[*e_m]<<std::endl;
// //       std::cout<<"matrix size: "<<get_size()<<"should be: "<<m_events.size()<<"are events ordered: "<< (event_topology.*comp)(*e_v,*e_m)<<std::endl;
// // 	std::cout<<"filling ("<<get_component_index(m_event_to_number[*e_v]).first << ","
// // 			      <<get_component_index(m_event_to_number[*e_v]).second <<")("
// // 			      <<get_component_index(m_event_to_number[*e_m]).first<<","
// // 			      << get_component_index(m_event_to_number[*e_m]).second<<")"
// // 			      <<std::endl;
//       if((event_topology.*comp)(*e_v,*e_m)) std::cout<< m_event_to_number[*e_v]<< "<"<<m_event_to_number[*e_m]<<std::endl;
//       //i know that m_event_to_number[*e_v] <= m_event_to_number[*e_m]
//       unsigned index_a;
//       unsigned component_a;
//       int offset_b;
//       std::pair<unsigned int, unsigned int> pair = get_component_index(x);
//       index_a=pair.second; //index x in the correct component
//       component_a =pair.first;
//       offset_b =in_same_component(x,y,component_a,x-index_a);
//       if((event_topology.*comp)(*e_v,*e_m) && in_same_component(m_event_to_number[*e_v],m_event_to_number[*e_m]))
// //	get_component_index(m_event_to_number[*e_v]).first == get_component_index(m_event_to_number[*e_m]).first)
//       {
// 	fill(*e_v,*e_m,MscTimeIntervalD(0,D::infinity()));
// //	operator()(*e_v,*e_m)=MscTimeIntervalD(0,D::infinity());
//       }
//     }
//   }

  //filling constraints from HMSC path
  std::map<std::pair<EventP,EventP>, MscTimeIntervalSetD>::const_iterator it;
  for(it=m_position_to_interval.begin();it!=m_position_to_interval.end();it++)
  {
    fill((it->first).first,(it->first).second,it->second);
  }
}

// void BMscIntervalSetMatrix::build_up_matrix_causally() //add acyclic checker as precondition
// {
//   IntervalSetMatrix::resize(m_events.size());
//   for (unsigned i=0;i<m_events.size();i++)
//   {
//     for (unsigned j=0;j<m_events.size();j++)
//     {
//       if (j==i)
//       {
//         operator()(i,j)=MscTimeIntervalSetD();
//         operator()(i,j).insert(MscTimeIntervalD(0,0));
//       }
//       else
//       {
//         operator()(i,j) =MscTimeIntervalSetD();
//         operator()(i,j).insert(MscTimeIntervalD(-D::infinity(),D::infinity()));
//       }
//     }
//   }
//
//
//   CausalClosureInitiator causal_closure_initiator;
//   causal_closure_initiator.initialize(m_topology,m_visual_closure_initiator,m_mapper);
//
//   EventPVector::iterator e_before;
//   EventPVector::iterator e_after;
//   BoolVector causal_closure;
//   for (e_before=m_events.begin();e_before!=m_events.end();e_before++)
//   {
//     causal_closure = causal_closure_initiator.get_causal_closure(*e_before);
//     for (e_after=m_events.begin();e_after!=m_events.end();e_after++)
//     {
//       if(e_before == e_after ) continue; //on diagonal in the matrix remain constrains [0]
//       if (causal_closure[m_visual_closure_initiator.get_topology_index(*e_after)]) //pozor za sebou si to mozno prepisujem!!!
//       {
// 	fill(*e_before,*e_after,MscTimeIntervalD(0,D::infinity()));
//       }
//     }
//   }
//
// /*
//   const InstancePtrList& instances = m_original_bmsc->get_instances(); //strange that we don't use smart pointers instead normal ones - if memory leaks happen- check this
//   InstancePtrList::const_iterator inst_it;
//
//
//   EventArea* area;
//   StrictOrderArea* strict_area;
//   CoregionArea* coregion_area;
//   EventPSet events_to_do; //new set of EventPtr
//   CoregionEventPtrSet coregion_events_to_do;
//   CoregionEventPtrSet coregion_events;
//   EventPSet::iterator event_it;
//   CoregionEventPtrSet::iterator cor_event_it;
//   CoregionEventPtrSet::iterator cor_event_it2;
//   CoregEventRelPtrVector predcessors;
//   CoregEventRelPtrVector::iterator predces_it;
//
//   StrictEvent* strict_event;
//   Event* match_event;
//   CoregionEvent coregion_event;
//
//   bool contains_send;
//
//   //for every instance execute the body
//   for(inst_it=instances.begin();inst_it!=instances.end();inst_it++) //be careful about how many instances may different bMSCs have
//   {
//     //traverse all areas of the process
//     area = (*inst_it)->get_first().get();
//     while(area){
//       strict_area = dynamic_cast<StrictOrderArea*>(area);
//       if(strict_area)
//       { //the area is a strict order area
//
// 	//traverse all events in the strict order area
//         strict_event = strict_area->get_first().get();
//         while(strict_event){
//
//           if(strict_event->is_send()){ //event is send => add causal constraints
//             for(event_it=events_to_do.begin();event_it!=events_to_do.end();event_it++){
//               //add constraint and its inverse
// 	      fill(strict_event,*event_it,MscTimeIntervalD(0,D::infinity()));
// 	    }
//             for(cor_event_it=coregion_events_to_do.begin();cor_event_it!=coregion_events_to_do.end();cor_event_it++){
//               //add constraint and its inverse
// 	      fill(strict_event,(*cor_event_it).get(),MscTimeIntervalD(0,D::infinity()));
//             }
//             match_event = strict_event->get_matching_event();
//             //add constraint and its ineverse
// 	    fill(strict_event,match_event,MscTimeIntervalD(0,D::infinity()));
//             coregion_events_to_do.clear();
//             events_to_do.clear();
//           }
//           events_to_do.insert(strict_event);
//           strict_event = strict_event->get_successor().get();
//         }
//       }else {
//         coregion_area = dynamic_cast<CoregionArea*>(area);
//         if(coregion_area){
// 	  //the area is a coregion area
//           contains_send = false;
//           coregion_events= coregion_area->get_events();
// 	  //execute the body for all events in the coregion area
//           for(cor_event_it2=coregion_events.begin();cor_event_it2!=coregion_events.end();cor_event_it2++){
//             coregion_event = cor_event_it2->get();
//             if(coregion_event.is_send()){ //event is send => add causal constraints
//               contains_send = true;
//               for(event_it=events_to_do.begin();event_it!=events_to_do.end();event_it++){
//                 //add constraint and its inverse
// 		fill(&coregion_event,*event_it,MscTimeIntervalD(0,D::infinity()));
//               }
//               for(cor_event_it=coregion_events_to_do.begin();cor_event_it!=coregion_events_to_do.end();cor_event_it++){
//                 //add constraint and its inverse
// 		fill(&coregion_event,(*cor_event_it).get(),MscTimeIntervalD(0,D::infinity()));
//               }
//               predcessors = coregion_event.get_predecessors();
//               for(predces_it=predcessors.begin();predces_it!=predcessors.end();predces_it++){
//                 //add constraint and its inverse
// 		fill(&coregion_event,(*predces_it).get()->get_predecessor(),MscTimeIntervalD(0,D::infinity()));
// 	      }
//               match_event = strict_event->get_matching_event();
//               //add constraint and its inverse
// 	      fill(&coregion_event,match_event,MscTimeIntervalD(0,D::infinity()));
//             }
//           }
//           if(contains_send){
//             coregion_events_to_do.clear();
//             events_to_do.clear();
// 	  }
// 	  coregion_events_to_do.insert(coregion_events.begin(),coregion_events.end()); //add new events from last coregion
//
//         }else{
// 	  throw std::runtime_error("found area which is not strict order nor coregion");
//         }
//       }
//       area = area->get_next().get();
//     }
//   }*/
//
// //  std::cout << *this <<std::endl;
//
// //  EventPVector::iterator e_v;
//
//
//   for (e_before=m_events.begin();e_before!=m_events.end();e_before++)
//   {
//     TimeRelationEventPtrList::const_iterator it;
//     TimeRelationEventPtrList relations = (*e_before)->get_time_relations();
//
//     for (it=relations.begin();it!=relations.end();it++)
//     {
//       if(e_before == e_after ) continue; //on diagonal in the matrix remain constrains [0]
//  //     m_time_rel_set.insert(*it); //whats this ??
//       EventP a = (*it)->get_event_a();
//       EventP b = (*it)->get_event_b();
//
//       if (m_visual_closure_initiator.get_visual_closure(a)[m_visual_closure_initiator.get_topology_index(b)]) //TODO use visual closure instead of topology handler
//       {
// //        fill(a,b,MscTimeIntervalSetD::set_intersection((*it)->get_interval_set(),operator()(a,b)));
//         fill(a,b,(*it)->get_interval_set());
//       }
//       else
//       {
// //        fill(b,a,MscTimeIntervalSetD::set_intersection((*it)->get_interval_set(),operator()(b,a)));
//         fill(b,a,(*it)->get_interval_set());
//       }
//     }
//   }
// }

BMscPtr BMscIntervalSetMatrix::get_modified_bmsc(IntervalMatrix* matrix)
{
  IntervalSetMatrix matrix2;
  if(matrix) matrix2 = *matrix;
  else matrix2 = *this;

  // go through all relations and set new values
  std::map<TimeRelation*, std::pair<unsigned,unsigned> >::const_iterator it;
  for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
  {
    (it->first)->set_interval_set(matrix2.operator()(it->second.first,it->second.second));
  }
  return m_bmsc_original;
}

BMscPtr BMscIntervalSetMatrix::get_modified_bmsc()
{
  // duplicate bmsc
  BMscPtr copy_bmsc;
  BMscDuplicator duplicator;
  copy_bmsc = duplicator.duplicate_bmsc(m_bmsc_original);

  // go through all relations and set copy new values
  std::map<TimeRelation*, std::pair<unsigned,unsigned> >::const_iterator it;
  for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
  {
//    if(duplicator.get_copy(it->first))
      ((TimeRelation*)duplicator.get_copy(it->first))->set_interval_set(operator()(it->second.first,it->second.second));
//    else
//      (it->first)->set_interval_set(operator()(it->second.first,it->second.second));
  }
  return copy_bmsc;
}

BMscPtr BMscIntervalSetComponentMatrix::get_modified_bmsc(unsigned component,IntervalMatrix matrix)
{
//   if(matrix)
//   {
//     // go through all relations and set new values
//     std::map<TimeRelation*, std::pair<EventP,EventP> >::const_iterator it;
//     for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
//     {
//       //both events have to be in same component
//       (it->first)->set_interval_set(operator()(it->second.first,it->second.second));
//     }
//   }
//   else
//   {
    unsigned offset = get_offset(component);
    // go through all relations and set new values
    std::map<TimeRelation*, std::pair<EventP,EventP> >::const_iterator it;
    for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
    {
      //both events have to be in same component(follows from contruction of the component matrix)
      if((0<= get_number(it->second.first)-offset) && 
	(get_number(it->second.first)-offset< get_component(component).size()))
	//both events in component with index "component"
	(it->first)->set_interval_set(matrix.operator()(get_number(it->second.first)-offset,get_number(it->second.second)-offset));
      else
	(it->first)->set_interval_set(operator()(it->second.first,it->second.second));
    }
//  }
  return m_bmsc_original;
}

BMscPtr BMscIntervalSetComponentMatrix::get_modified_bmsc()
{
  // duplicate bmsc
  BMscPtr copy_bmsc;
  BMscDuplicator duplicator;
  copy_bmsc = duplicator.duplicate_bmsc(m_bmsc_original);

  // go through all relations and set new values
  std::map<TimeRelation*, std::pair<EventP,EventP> >::const_iterator it;
  for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
  {
      ((TimeRelation*)duplicator.get_copy(it->first))->set_interval_set(operator()(it->second.first,it->second.second));
  }
  return copy_bmsc;
}

BMscPtr BMscIntervalSetComponentMatrix::get_modified_bmsc(IntervalSetComponentMatrix matrix)
{
  // duplicate bmsc
  BMscPtr copy_bmsc;
  BMscDuplicator duplicator;
  copy_bmsc = duplicator.duplicate_bmsc(m_bmsc_original);

  // go through all relations and set new values
  std::map<TimeRelation*, std::pair<EventP,EventP> >::const_iterator it;
  for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
  {
      ((TimeRelation*)duplicator.get_copy(it->first))->set_interval_set(matrix.operator()(get_number(it->second.first),get_number(it->second.second)));
  }
  return copy_bmsc;
}
// $Id: time_pseudocode.cpp 1074 2011-03-30 17:04:06Z lkorenciak $
