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
 * $Id: tightening.h 1068 2011-03-25 18:11:23Z lkorenciak $
 */

#ifndef _TIGHTENING_H_
#define _TIGHTENING_H_

#include "time_consistency.h"
#include "time_pseudocode.h"
#include "check/pseudocode/msc_duplicators.h"
#include "data/transformer.h"
#include "hmsc_all_paths.h"
#include "hmsc_block_paths.h"
#include "traverse_erase.h"

#include <vector>
#include <iostream>

#include "data/Z120/z120.h"
#include <utility>

/**
 * \brief 
 */


class  TightenBMsc
{

private:
  BMscPtr m_bmsc;
  BMscIntervalSetMatrix m_matrix;
  EventTopologyHandler m_event_top;
  AllReachableEventPVector m_events;

  MinimalEventPList m_mins;
  MaximalEventPList m_maxs;

  bool is_leq(Event* e,Event* f)
  {
    return m_event_top.visual_is_leq(e,f);
  }

  MscTimeIntervalSetD get_max_interval(BMscIntervalSetMatrix);

  //! tightens interval and matrix according to each other
  ConstBMscMatrixPair max_tightener(BMscIntervalSetMatrix,MscTimeIntervalSetD);

public:

  TightenBMsc(BMscPtr bmsc):m_bmsc(bmsc),m_matrix(bmsc),m_event_top(bmsc)
      ,m_events(bmsc),m_mins(bmsc),m_maxs(bmsc)
  {
  }

  //! tight BMsc due to interval
  ConstBMscMatrixPair tighten_msc(MscTimeIntervalSetD);

  static ConstBMscMatrixPair tight(BMscPtr bmsc,MscTimeIntervalSetD interval)
  {
    TightenBMsc tightening(bmsc);
    ConstBMscMatrixPair pair = tightening.tighten_msc(interval);
    //tightening.modificate(pair.second);
    return pair;
  }

}; // class TightenBMsc

typedef std::list<HMscNodePtr> HMscNodePtrList;

class SCTIME_EXPORT BMscTighter //:public Transformer // , public BMscTransformer
{

public:
  BMscTighter()
  {

  }

  ~BMscTighter()
  {

  }

  BMscPtr transform(BMscPtr bmsc);
};

class SCTIME_EXPORT HMscTighter: public PathFoundListener //:public Transformer // , public BMscTransformer
{

  private:
    bool m_causal;
public:
  HMscTighter():m_causal(false)
  {

  }

  ~HMscTighter()
  {
  }

  HMscPtr transform(HMscPtr h)
  {
    HMscDuplicator duplicator;
    HMscPtr result = duplicator.duplicate(h);

    BMscGraphDuplicator graph_dup;
    HMscPtr bmsc_graph = graph_dup.duplicate_hmsc(result);

    EraseAllConstraints erase;
    erase.erase_all_constraints(result);

    TraverseAndMarkBlocks block_marker;
    block_marker.travers_and_mark_blocks(bmsc_graph);


    AllPathsAllBlocks block_to_path(block_marker.m_list_of_blocks,bmsc_graph);
    block_to_path.set_listener(this);
    block_to_path.all_paths_all_blocks();

    return result;
  }


/**
* \brief Tightens BMscGraph path and returns BMscIntervalSetMatrix with original constraints and IntervalSetMatrix with tightened constraints.
*/
  std::pair<BMscIntervalSetMatrix,IntervalSetMatrix> tighten_BMscGraph_path(std::list<MscElement*>& path)
  {
     HMscFlatPathToBMscDuplicator duplicator;
     BMscPtr bmsc = duplicator.duplicate_path(path);



     SRChannelMapperPtr srm = SRChannelMapper::instance();
     BMscIntervalSetMatrix b_matrix(bmsc,m_causal,srm); // TODO: channelmapper

     std::map<BMsc*,int> bmsc_to_count;
     std::map<const TimeRelationRefNodePtrSet*,EventP> rel_to_event;
     std::set<TimeRelationRefNode*> open_rel;

     Event* a;
     AllCombination combination;
     for(std::list<MscElement*>::iterator it=path.begin();it!=path.end();it++)
     {
       ReferenceNode* ref = dynamic_cast<ReferenceNode*>(*it);
       if(ref==NULL)
         continue;
       BMscPtr in = ref->get_bmsc();
       bmsc_to_count[in.get()]++; // @Warning initial integer number = 0?
       TimeRelationRefNodePtrSet* top = &(ref->get_time_relations_top());
       TimeRelationRefNodePtrSet* bottom = &(ref->get_time_relations_bottom());


        if(top->size()>0)
        {
          a = new StrictEvent();
          b_matrix.add_event(a);
          rel_to_event[top] = a;

          MinimalEventPList min(in);
          std::vector<std::pair<EventP,EventP> > floor;
          for(MinimalEventPList::iterator m=min.begin();m!=min.end();m++)
          {
            Event* copy = dynamic_cast<Event*>(duplicator.get_copy_with_occurence(*m,bmsc_to_count[in.get()]));
            b_matrix.fill(a,copy,MscTimeIntervalD(0,D::infinity()));
            floor.push_back(std::make_pair<EventP,EventP>(a,copy));
          }
          if(!min.empty()) combination.add_list(floor);

        }
        // open/close relations
        for(TimeRelationRefNodePtrSet::const_iterator it=top->begin();it!=top->end();it++)
        {
          if(open_rel.find(it->get()) != open_rel.end())
	  {//time relation is open -> close it
	    const TimeRelationRefNodePtrSet* other;
	    //find the correct time relation (for getting previously created
	    // node to which the processed time relation (it*) should be connected)
	    if(ref==(*it)->get_ref_node_a()&&(*it)->is_top_node_a())
	    {
	    if((*it)->is_top_node_b())
	        other = &((*it)->get_ref_node_b()->get_time_relations_top());
	      else
	        other = &((*it)->get_ref_node_b()->get_time_relations_bottom());
	    }
	    else
	    {
	      if((*it)->is_top_node_a())
	        other = &((*it)->get_ref_node_a()->get_time_relations_top());
	      else
	        other = &((*it)->get_ref_node_a()->get_time_relations_bottom());
	    }
	    //fill the time relation to the matrix (node a was found later)
	    b_matrix.fill_intersection(rel_to_event[other],a,(*it)->get_interval_set());
	    b_matrix.tied_rel_to_cell(*it,rel_to_event[other],a);
	  }
	  else //time relation has not been opened yet -> open it
	    open_rel.insert((*it).get());

        }

        if(bottom->size()>0)
        {
          a = new StrictEvent();
          b_matrix.add_event(a);
          rel_to_event[bottom] = a;
          MaximalEventPList max(in);
          std::vector<std::pair<EventP,EventP> > floor;
          for(MaximalEventPList::iterator m=max.begin();m!=max.end();m++)
          {
	    Event* copy = dynamic_cast<Event*>(duplicator.get_copy_with_occurence(*m,bmsc_to_count[in.get()]));
            b_matrix.fill(copy,a,MscTimeIntervalD(0,D::infinity()));
            floor.push_back(std::make_pair<EventP,EventP>(copy,a));
          }
          if(!max.empty()) combination.add_list(floor);
        }

        // open/close relations
        for(TimeRelationRefNodePtrSet::iterator it=bottom->begin();it!=bottom->end();it++)
        {
          if(open_rel.find(it->get()) != open_rel.end())
	  { //time relation is open -> close it
	    const TimeRelationRefNodePtrSet* other;
	    //find the correct time relation (for getting previously created
	    // node to which the processed time relation (it*) should be connected)
	    if(ref==(*it)->get_ref_node_a()&&(*it)->is_bottom_node_a())
	    {
	      if((*it)->is_top_node_b())
	        other = &((*it)->get_ref_node_b()->get_time_relations_top());
	      else
	        other = &((*it)->get_ref_node_b()->get_time_relations_bottom());
	    }
	    else
	    {
	      if((*it)->is_top_node_a())
	        other = &((*it)->get_ref_node_a()->get_time_relations_top());
	      else
	        other = &((*it)->get_ref_node_a()->get_time_relations_bottom());
	    }
    	    //fill the time relation to the matrix (node a was found later)
	    b_matrix.fill_intersection(rel_to_event[other],a,(*it)->get_interval_set());
	    b_matrix.tied_rel_to_cell(*it,rel_to_event[other],a);
	  }
	  else //time relation has not been opened yet -> open it
	    open_rel.insert((*it).get());
        }
     }
     b_matrix.build_up();

     //path without events is trivially tight
     if(b_matrix.size()==0) return std::make_pair<BMscIntervalSetMatrix,IntervalSetMatrix>(b_matrix,b_matrix);
     
     IntervalSetMatrix result;
     result.resize(b_matrix.size());

     combination.init();
     if(combination.size()==0){
       //tighten the matrix
       MscSolveTCSP solve;
       MscSolveTCSPReport report = solve.solveTCSP(b_matrix);
       if(report.m_number_of_consistent_choices==0){ // inconsistent
	 throw std::runtime_error("Inconsistent path, but should be consistent due to preconditions.");
       }
       return std::make_pair<BMscIntervalSetMatrix,IntervalSetMatrix>(b_matrix,report.m_matrix_result);
     }


     do
     {
       // push there [0,0] interval
       for(unsigned i=0;i<combination.size();i++)
       {
         b_matrix.fill(combination[i].first,combination[i].second,MscTimeIntervalD(0,0));
       }
       MscSolveTCSP solve;
       if(b_matrix.get_rel_empty_set().size()!=0) // inconsistent
         throw  std::runtime_error("inconsistent matrix - found empty time interval in matrix.");
       MscSolveTCSPReport report = solve.solveTCSP(b_matrix);
       if(report.m_number_of_consistent_choices!=0) // consistent => fill the values to result matrix
       {
	 IntervalSetMatrix new_result = report.m_matrix_result;

	 for(unsigned i=0;i<new_result.size();i++)
	   for(unsigned j=0;j<new_result.size();j++)
	     result(i,j) = MscTimeIntervalSetD::set_union(result(i,j),new_result(i,j));
       }

       //return back previous values
       for(unsigned i=0;i<combination.size();i++)
       {
         b_matrix.fill(combination[i].first,combination[i].second,MscTimeIntervalD(0,D::infinity()));
       }


     } while(combination.move_next());


    return std::make_pair<BMscIntervalSetMatrix,IntervalSetMatrix>(b_matrix,result);
  }

/**
* \brief Tightens path and fills result to the original HMsc using the interval set disjunction. This allows us to store in the original HMsc partial result of whole tightening.
*/
  virtual void on_path_found(std::list<MscElement*>& path)
   {
     std::pair<BMscIntervalSetMatrix,IntervalSetMatrix> pair = tighten_BMscGraph_path(path);
     BMscIntervalSetMatrix b_matrix = pair.first;
     IntervalSetMatrix result = pair.second;

     std::map<TimeRelation*, std::pair<unsigned,unsigned> > relations = b_matrix.get_tied_time_relations();


     for(std::map<TimeRelation*, std::pair<unsigned,unsigned> >::iterator it=relations.begin();it!=relations.end();it++)
     {
       TimeRelation* copy = it->first;
       if(copy->get_original()==NULL || copy->get_original()->get_original()==NULL)
         throw std::runtime_error("Too close to original time relation, something went wrong");

       while(copy->get_original()->get_original()!=NULL)
       {
         copy = copy->get_original();
       }
       copy->get_interval_set() = MscTimeIntervalSetD::set_union(copy->get_interval_set(),result(it->second.first,it->second.second));

     }
  }

  void set_causal()
  {
    m_causal = true;
  }
};



class SCTIME_EXPORT Tighter:public Transformer
{

public:
//distinguishes between BMscPtr and HMSCPtr and calls the appropriate metohd transform
  MscPtr transform(MscPtr msc)
  {
    BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);

    if (bmsc.get())
    {
      BMscTighter tighter;
      MscPtr msc = tighter.transform(bmsc).get();
      return msc;
    }

    HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);

    if(hmsc.get())
    {
      HMscTighter tighter;
      MscPtr msc = tighter.transform(hmsc).get();
      return msc;
    }



    return NULL;
  }

  //! Human readable name of the transformation.
  virtual std::wstring get_name() const
  {
    return L"Tighten Time";
  }


  //! Returns a list of preconditions for this transformation.
  PreconditionList get_preconditions(MscPtr msc) const
  {
    Checker::PreconditionList precon_list;
    precon_list.push_back(PrerequisiteCheck(L"Correct Time Constraint Syntax", PrerequisiteCheck::PP_REQUIRED));
    precon_list.push_back(PrerequisiteCheck(L"Time Consistent", PrerequisiteCheck::PP_REQUIRED));
    return precon_list;
  }

};

/*
class TightenSetOfPaths
{
private:
//  std::set<BMscPtr> m_bmsc_on_path_set;
//  std::set<BMscPtr> m_bmsc_all_set;
  std::set<BMscPtr> m_bmsc_copy1;
  std::set<BMscPtr> m_bmsc_copy2;

  std::list<BMscPtr> m_msc_list;
  std::list<TimeRelationRefNodePtr> m_constr;
  IntervalSetMatrix m_t;
  BMscPtr m_msc;
  MscTimeIntervalSetD i_prime_prime;
public:
//  HMscAllPaths

  MscTimeIntervalSetD tighten(
    HMscAllPaths paths,
    MscTimeIntervalSetD i,
    HMscPtr hmsc
  )
  {


    //initialization of both copies
    BMscDuplicator bmsc_duplicator_1;
    std::list<HMscPath>::iterator it;

    for (it=paths.first.begin();it!=paths.first.end();it++)
    {
      std::list<HMscNodePtr>::iterator node;

      for (node=(*it).first.begin();node!=(*it).first.end();node++)
      {
        ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>((*node).get());
        BMscPtr bmsc_ptr = ref_node->get_bmsc();
        m_bmsc_copy1.insert(bmsc_duplicator_1.duplicate_bmsc(bmsc_ptr)); //in case that on paths are reference nodes only
      }
    }

    BMscDuplicator bmsc_duplicator_2;

    HMscNodePtrSet all_nodes = hmsc->get_nodes();
    HMscNodePtrSet::iterator hmsc_node;

    for (hmsc_node=all_nodes.begin();hmsc_node!=all_nodes.end();hmsc_node++)
    {
      ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>((*hmsc_node).get());

      if (!ref_node)
        continue;

      BMscPtr bmsc = ref_node->get_bmsc();

      if (!bmsc)
        continue;

      m_bmsc_copy2.insert(bmsc_duplicator_2.duplicate_bmsc(bmsc));
    }



    //  std::list<HMscPath>::iterator it;

    for (it=paths.first.begin();it!=paths.first.end();it++)
    {

      if (!(*it).second)
      {
        m_msc_list.clear();
        m_constr.clear();
        HMscNodePtrList::iterator n_path;
        HMscNodePtrList::iterator n_path_previous;
        n_path = it->first.begin();
        ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>((*n_path).get());
        BMscPtr from_node = ref_node->get_bmsc();

        if (from_node)
          m_msc = from_node;

        n_path_previous = n_path;

        n_path++;

        for (;n_path!=it->first.end();n_path++)
        {
          n_path_previous++;
          ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>((*n_path).get());

          if (!ref_node)
            continue;

          TimeRelationRefNodePtrSet constr_set = ref_node->get_time_relations_top();

          TimeRelationRefNodePtrSet::iterator it_c;

          bool has_constraint = false;

          for (it_c=constr_set.begin();it_c!=constr_set.end();it_c++)
          {
            if ((*it_c)->get_ref_node_b() == (*n_path_previous).get())  //Warning: we assume that nodes in time relation ref node are ordered
            {
              has_constraint = true;
              break;
            }

          }

          if (has_constraint)
          {
            m_constr.push_back(*it_c);
            m_msc_list.push_back(m_msc);

            ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>((*n_path).get());
            BMscPtr from_node = ref_node->get_bmsc();

            if (from_node)
              m_msc = from_node;

          }

          else
          {
            BMscPtr msc_tmp;
            ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>((*n_path).get());
            BMscPtr from_node = ref_node->get_bmsc();

            if (from_node)
              msc_tmp = from_node;

//            m_msc = concatenate(m_msc, msc_tmp);  //TODO:concatenation
          }

        }

        ConstMatrixPair pair;

        if (m_msc_list.empty())
        {
          TightenBMsc tighten_msc(m_msc);
          pair = tighten_msc.tighten_msc(i);
          m_t = pair.second;
          i_prime_prime = MscTimeIntervalSetD::set_intersection(i_prime_prime,pair.first);
        }

        else
        {
          m_msc_list.push_back(m_msc);
          MscTimeIntervalSetD i_prime_prime_prime;
//   TightenMsgPath tighten_msg_path(m_msc_list);             //TODO: MSG PATH
          i_prime_prime_prime = tighten_msg_path(m_msc_list,m_constr,i);

          i_prime_prime = MscTimeIntervalSetD::set_intersection(i_prime_prime,pair.first);
        }
      }


      HMscNodePtrList::iterator path_it;

      for (path_it = it->first.begin(); path_it!= it->first.end(); path_it++)
      {
        //TODO:write get_all_constraints
        std::set<TimeRelationEventPtr> constraints;
// constraints = get_all_constraints(*path_it);
        std::set<TimeRelationEventPtr>::iterator tr_it;

        for (tr_it = constraints.begin(); tr_it != constraints.end(); tr_it++)
        {
          TimeRelationEvent* relation_copy = dynamic_cast<TimeRelationEvent*>(bmsc_duplicator_1.get_copy((*tr_it).get()));
          MscTimeIntervalSetD interval_set = MscTimeIntervalSetD::set_union(relation_copy->get_interval_set(), (*tr_it)->get_interval_set());
          relation_copy->set_interval_set(interval_set);

        }
      }

      //TODO:upravit nakopirovat povodne obmedzenia naspat
//      m_bmsc_all_set = m_bmsc_copy2;

      for (path_it = it->first.begin(); path_it!= it->first.end(); path_it++)
      {
        //TODO:write get_all_constraints
        std::set<TimeRelationEventPtr> constraints;
// constraints = get_all_constraints(*path_it);
        std::set<TimeRelationEventPtr>::iterator tr_it;

        for (tr_it = constraints.begin(); tr_it != constraints.end(); tr_it++)
        {

          //TODO:upravit
          TimeRelationEvent* relation_copy = dynamic_cast<TimeRelationEvent*>(bmsc_duplicator_2.get_copy((*tr_it).get()));
          (*tr_it)->set_interval_set(relation_copy->get_interval_set());

        }
      }
    }


    std::set<BMscPtr>::iterator b_it2;

    for (b_it2 = m_bmsc_copy2.begin(); b_it2!= m_bmsc_copy2.end(); b_it2++)
    {
      //TODO:write get_all_constraints
      std::set<TimeRelationEventPtr> constraints;
//   constraints = get_all_constraints(*b_it);
      std::set<TimeRelationEventPtr>::iterator tr_it2;

      for (tr_it2 = constraints.begin(); tr_it2 != constraints.end(); tr_it2++)
      {
        TimeRelationEvent* relation_copy = dynamic_cast<TimeRelationEvent*>((*tr_it2)->get_original());

        //TODO:upravit
        relation_copy->set_interval_set((*tr_it2)->get_interval_set());

      }

    }

    return i_prime_prime;

  }

  MscTimeIntervalSetD tighten_msg_path(
    std::list<BMscPtr> list_bmsc,
    std::list<TimeRelationRefNodePtr> constr,
    MscTimeIntervalSetD interval
  )
  {
    MscTimeIntervalSetD tmp_interval(interval);
    std::vector<MscTimeIntervalSetD> durations;

    for (size_t i=0;i<list_bmsc.size();i++)
    {
      MscTimeIntervalSetD tmp;

      tmp.insert(MscTimeIntervalD(0,D::infinity()));
      durations.push_back(tmp);
    }

    MscTimeIntervalSetD s_interval;

    do
    {
      s_interval = tmp_interval;
      std::list<BMscPtr>::iterator bmsc;

      unsigned int i=0;

      for (bmsc=list_bmsc.begin();bmsc!=list_bmsc.end();bmsc++,i++)
      {
        MscTimeIntervalSetD tmp_i=tmp_interval;

        for (size_t j=0;j<list_bmsc.size();j++)
        {
          if (i==j)
            continue;

          tmp_i = tmp_i - durations[j];
        }

        std::list<TimeRelationRefNodePtr>::iterator c_it;

        for (c_it=constr.begin();c_it!=constr.end();c_it++)
          tmp_i = tmp_i - (*c_it)->get_interval_set();

        durations[i]=TightenBMsc::tight(*bmsc,MscTimeIntervalSetD::set_intersection(tmp_i,durations[i]));
      }

      MscTimeIntervalSetD interval_set;

      std::list<TimeRelationRefNodePtr>::iterator c_it2;

      for (c_it2=constr.begin();c_it2!=constr.end();c_it2++)
      {
        interval_set = (*c_it2)->get_interval_set();
        MscTimeIntervalSetD tmp_i=tmp_interval;

        for (size_t j=0;j<list_bmsc.size();j++)
        {
          tmp_i = tmp_i - durations[j];
        }

        interval_set = (*c_it2)->get_interval_set();

        std::list<TimeRelationRefNodePtr>::iterator c_it;

        for (c_it=constr.begin();c_it!=constr.end();c_it++)
        {
          interval_set = (*c_it2)->get_interval_set();

          if (*c_it==*c_it2)
            continue;

          tmp_i = tmp_i - (*c_it)->get_interval_set();
        }

        interval_set = (*c_it2)->get_interval_set();

        (*c_it2)->set_interval_set(MscTimeIntervalSetD::set_intersection((*c_it2)->get_interval_set(),tmp_i));
        interval_set = (*c_it2)->get_interval_set();
      }

      MscTimeIntervalSetD tmp;

      tmp.insert(MscTimeIntervalD(0,0));

      for (size_t j=0;j<list_bmsc.size();j++)
      {
        tmp = tmp + durations[j];
      }

      std::list<TimeRelationRefNodePtr>::iterator c_it;

      for (c_it=constr.begin();c_it!=constr.end();c_it++)
      {
        MscTimeIntervalSetD uu = (*c_it)->get_interval_set();
        tmp = uu + tmp;
      }

      tmp_interval = MscTimeIntervalSetD::set_intersection(tmp_interval,tmp);

    }

    while (s_interval!=tmp_interval);

    return tmp_interval;
  }

};*/

#endif //_TIGHTENING_H_

// $Id: tightening.h 1068 2011-03-25 18:11:23Z lkorenciak $
