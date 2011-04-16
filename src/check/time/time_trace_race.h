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
 * $Id: time_trace_race.h 1029 2011-02-02 22:17:59Z madzin $
 */

#ifndef _TIME_TRACE_RACE_H_
#define _TIME_TRACE_RACE_H_

#include "data/time.h"
#include "data/dfs_hmsc_traverser.h"
#include "data/dfs_hmsc_flat_traverser.h"
#include "data/dfs_events_traverser.h"
#include "data/Z120/z120.h"

#include "check/pseudocode/utils.h"
#include "check/time/export.h"
#include "check/time/time_consistency.h"
#include "check/time/time_pseudocode.h"
#include "check/time/tightening.h"
#include "check/time/hmsc_all_paths.h"
#include "check/pseudocode/msc_duplicators.h"
#include <list>

#include <iostream>

class TimeRaceChecker;
class CheckTimeTraceRaceListener;
class TimeRaceInHMscException;

//////////////////////////////////////////////////////////
  /**
   * \brief 
   *
   * @param name - 
   * @returns 
   * @Warning 
   */
class TimeRaceInHMscException:public std::exception
{
private:
  

  HMscPtr m_counterexample_hmsc;
  
public:
  
  TimeRaceInHMscException(HMscPtr hmsc){
    m_counterexample_hmsc = hmsc;
  }
  
  ~TimeRaceInHMscException() throw ()
  {
    
  }

  /**
   * \brief 
   *
   * @param name - 
   * @returns 
   * @Warning 
   */
  HMscPtr get_counterexample_hmsc(){
    return m_counterexample_hmsc;
  }
};


/////////////////////////////////////////////////////////


  /**
   * \brief 
   *
   * @param name - 
   * @returns 
   * @Warning 
   */
class CheckTimeTraceRaceListener: public PathFoundListener{
  public:
    
  void on_path_found(MscElementPList& path){
   
//code for testing:
//     std::cout << "new path:" << std::endl;
//     
//     MscElementPList::iterator path_it;
//     for(path_it=path.begin();path_it!=path.end();path_it++){
//       HMscNode* node = dynamic_cast<HMscNode*>(*path_it);
//       if(node){
// 	std::cout << node->get_attribute<int>("AllPathAlg",0) << std::endl;
//       }
//     }
//     
//     std::cout << std::endl;
//     

    //tightening path with causal closure
    HMscTighter tighter;
    (tighter).set_causal();
    std::pair<BMscIntervalSetMatrix,IntervalSetMatrix> pair = (tighter).tighten_BMscGraph_path(path);   

    
//code for testing:    
/*    Z120 z120;
    BMscPtr bmsc_copy_2;
    HMscFlatPathToBMscDuplicator duplicator2;
    bmsc_copy_2 = duplicator2.duplicate_path(path);
    z120.save_msc(std::cout,L"duplicated_path",bmsc_copy_2);*/
    
    
    //check time race
    // get matrix after tightening
    BMscIntervalSetMatrix causal_matrix = pair.first;
    IntervalSetMatrix matrix_result = pair.second;
    
    EventPVector events;
    events= causal_matrix.get_events(); //are there added events??
    EventPVector::iterator e_before;
    EventPVector::iterator e_after;

    EventTopologyHandler handler(causal_matrix.get_original_bmsc());
//    EventTopologyHandler handler = *causal_matrix.get_m_event_topology_handler(); //TODO why this is not possible? ask Ondra!!!    
    for (e_before=events.begin();e_before!=events.end();e_before++)
    { 
      Instance* instance1 = (*e_before)->get_instance();
      for (e_after=events.begin();e_after!=events.end();e_after++)
      {
	Instance* instance2 = (*e_after)->get_instance();
	if (instance1 && instance2  //we check for trace race only events which were originally present in HMSC
				    //the value of instance of these events should be set
	  && instance1 == instance2 //trace races are only on the same instance
	  && handler.visual_is_leq(*e_before,*e_after)
	  && //check whether intersection (-infty,0) and constraint corresponding to e_before and e_after is empty
	  !MscTimeIntervalSetD::set_intersection(MscTimeIntervalD(0,-D::infinity(),0,0),
	    matrix_result.operator()(causal_matrix.get_number(*e_before),
				     causal_matrix.get_number(*e_after))).is_empty())
        {
	    //time race found
	    //duplicate hmsc
	    Event* event_original1 = *e_before;
	    Event* event_original2 = *e_after;
	        
	    while(event_original1->get_original()!=NULL)
	    {
	      event_original1 = event_original1->get_original();
	    }
	    
	    while(event_original2->get_original()!=NULL)
	    {
	      event_original2 = event_original2->get_original();
	    }
	    
	    HMscPtr hmsc;
//            MscElementPListList results;
//            results.push_back(path);
	    HMscFlatPathDuplicator duplicator;
      event_original1->get_complete_message()->set_marked();
	    event_original1->set_marked();
	    event_original2->get_complete_message()->set_marked();
	    event_original2->set_marked();

	    hmsc = duplicator.duplicate_path(path);
	    
	    //marking elements
	    event_original1->get_complete_message()->set_marked(NONE);
	    event_original1->set_marked(NONE);
	    event_original2->get_complete_message()->set_marked(NONE);
	    event_original2->set_marked(NONE);
	    //copied from race checker
	    MscElementPList::const_iterator elem;
	    for(elem=path.begin();elem!=path.end();elem++)
	    {
	      ReferenceNode* r = dynamic_cast<ReferenceNode*>(*elem);
	      if(r && (r->get_bmsc().get()==event_original1->get_instance()->get_bmsc() ||
		r->get_bmsc().get()==event_original2->get_instance()->get_bmsc()))
	      {
		duplicator.get_copy(r)->set_marked();
	      }
	    }
	    
	    //throw exception with counterexample HMSC
// 	    HMscDuplicator dup;//TODO temporary, erase after use
// 	    throw TimeRaceInHMscException(dup.duplicate(hmsc));
	    throw TimeRaceInHMscException(hmsc);
        }
      }
    }


  }
  
  
  ~CheckTimeTraceRaceListener()
  {
    
  }
};



/**
 * \brief Checks time race for given BMsc or HMsc. 
 *
 * @Warning it is expected that BMsc/HMsc satisfy Deadlock Free, Livelock Free, Correct Time Constraint Syntax and Time Consistent if aplicable.
 *  
 */
class SCTIME_EXPORT TimeRaceChecker: public Checker, public BMscChecker, public HMscChecker
{
public:


  virtual std::wstring get_property_name() const
  {
    return L"Time Race";
  }

  virtual bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"time_trace_race/time_race.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const
  {
    Checker::PreconditionList precon_list;

      // checker for HMSC assumes msc to be deadlock free, livelock free, acyclic and FIFO
    HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);
    if(hmsc != NULL)
    {
      precon_list.push_back(PrerequisiteCheck(L"Deadlock Free", PrerequisiteCheck::PP_REQUIRED));
      precon_list.push_back(PrerequisiteCheck(L"Livelock Free", PrerequisiteCheck::PP_REQUIRED));
    }

    precon_list.push_back(PrerequisiteCheck(L"Correct Time Constraint Syntax", PrerequisiteCheck::PP_REQUIRED));
    precon_list.push_back(PrerequisiteCheck(L"Time Consistent", PrerequisiteCheck::PP_REQUIRED)); //TODO maybe causal time consistency would be better
    precon_list.push_back(PrerequisiteCheck(L"FIFO", PrerequisiteCheck::PP_REQUIRED));
    return precon_list;
  }

/**
* \brief Checks time race for given BMsc and ChannelMapper.
*
* @param bmsc - bmsc to be checked for time trace races
* @param mapper - channel mapper
* @return list of BMscs with marked events which cause time trace race
*/
  virtual std::list<BMscPtr> check(BMscPtr bmsc, ChannelMapperPtr mapper)
  {
    BMscIntervalSetMatrix causal_matrix(bmsc,true,mapper);
    causal_matrix.build_up();
    
    MscSolveTCSP solve;
    MscSolveTCSPReport report = solve.solveTCSP(causal_matrix);

    EventPVector events= causal_matrix.get_events();
    EventPVector::iterator e_before;
    EventPVector::iterator e_after;

    EventTopologyHandler handler(bmsc);
//    EventTopologyHandler handler = *causal_matrix.get_m_event_topology_handler(); //TODO why this is not possible? ask Ondra!!!
    


    for (e_before=events.begin();e_before!=events.end();e_before++)
    { 
      for (e_after=events.begin();e_after!=events.end();e_after++)
      {
	if (handler.visual_is_leq(*e_before,*e_after))
        {

          //check whether intersection (-infty,0) and constraint corresponding to e_before and e_after is empty
	  if(!MscTimeIntervalSetD::set_intersection(MscTimeIntervalD(0,-D::infinity(),0,0),
	    report.m_matrix_result.operator()(causal_matrix.get_number(*e_before),
					      causal_matrix.get_number(*e_after))).is_empty()
	    && ((*e_before)->get_instance() == (*e_after)->get_instance())
	    ) //races are only on the same instance
	  {
	    //time race found
	    // duplicate bmsc
	    BMscPtr bmsc_copy;
	    BMscDuplicator duplicator;
	    bmsc_copy = duplicator.duplicate_bmsc(bmsc);
	    Event* event_copy;
	    event_copy = duplicator.get_event_copy(*e_before);
// 	    std::cout << event_copy->get_receiver_label().c_str() << std::endl;
	    event_copy->set_marked();
            event_copy->get_message()->set_marked();
	    event_copy = duplicator.get_event_copy(*e_after);
	    event_copy->set_marked();
	    event_copy->get_message()->set_marked();
	    std::list<BMscPtr> my_list (1,bmsc_copy);
	    return my_list;
	  }
        }
      }
    }
    std::list<BMscPtr> my_list;
    return my_list;
  }

  virtual void cleanup_attributes() //TODO is something needed here?
  {

  }

/**
* \brief Checks time race for given HMsc and ChannelMapper.
* @param hmsc - hmsc to be checked for time trace races
* @param mapper - channel mapper
* @return list of HMscs with marked events which cause time trace race
*/
  //preconditions: deadlock-free, livelock-free hmsc
  virtual std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr mapper)
  {
    HMscPtr bmsc_graph;
    BMscGraphDuplicator duplicator;
	  
    bmsc_graph = duplicator.duplicate_hmsc(hmsc); //maybe duplicate(hmsc) would be better
    
    //find all end nodes for given bmsc_graph
    HMscNodePtrSet hmsc_nodes = hmsc.get()->get_nodes();
    HMscNodePtrSet::iterator nodes_it;
    EndNode* end_node;
    HMscNodePtrSet end_nodes;
    for(nodes_it=hmsc_nodes.begin();nodes_it!=hmsc_nodes.end();nodes_it++){
      end_node = dynamic_cast<EndNode*>((*nodes_it).get());
      if(end_node){
	end_nodes.insert(*nodes_it);
      }
    }

    
    AllPaths allpaths(hmsc,hmsc.get()->get_start(),end_nodes,3);
    CheckTimeTraceRaceListener race_listener;
    allpaths.add_path_found_listener(&race_listener);
    

    std::list<HMscPtr> my_list;
    
    try
    {
      allpaths.traverse();
    }
    catch(TimeRaceInHMscException& e)
    {
      HMscPtr error;
      error = e.get_counterexample_hmsc();
      my_list.push_back(error);
    }

    
    return my_list;
  }
};



#endif // _TIME_TRACE_RACE_H_


