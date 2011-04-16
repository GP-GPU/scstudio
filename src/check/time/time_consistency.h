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
 * $Id: time_consistency.h 1077 2011-04-06 16:22:35Z lkorenciak $
 */

#ifndef TIME_H_CON
#define TIME_H_CON

#include "time_pseudocode.h"
//#include "tightening.h"

#include <limits>
#include <set>
#include <list>
#include <vector>
#include <string>
#include <exception>


#include "check/pseudocode/msc_duplicators.h"
#include "hmsc_all_paths.h"
#include "hmsc_block_paths.h"
#include "traverse_erase.h"
#include "data/dfs_hmsc_traverser.h"

#include <iostream>

#include "data/Z120/z120.h"
#include <utility>


// #define _DEBUG
#ifdef _DEBUG
#include<iostream>
#define DEBUG(x) std::cerr << "DB: " << __FILE__ << ":" << __LINE__ << " " \
<< __FUNCTION__ << "() " << #x << " = " << (x) << std::endl
#define DEBUG_(x,y) std::cerr << "DB: " << __FILE__ << ":" << __LINE__ \
<< " " << __FUNCTION__ << "() " \
<< #x << " = " << (x) << " ; " \
<< #y << " = " << (y) << std::endl
#else
#define DEBUG(x)
#define DEBUG_(x,y)
#endif

typedef std::pair<Event*,Event*> EventPair;
typedef std::list<EventPair> EventPairsList;
typedef std::pair<MscTimeIntervalSetD,IntervalSetMatrix> ConstMatrixPair;
typedef std::pair<MscTimeIntervalSetD,BMscIntervalSetMatrix> ConstBMscMatrixPair;
typedef boost::intrusive_ptr<BMscIntervalMatrixConverter*> BMscIntervalMatrixConverterPtr;

struct MscSolveTCSPReport;
class MscSolveTCSP;



///////////////////////////////////////////////////////////////

class MscTimeInconsistencyException;


//! time inconsistency exception
class MscTimeInconsistencyException : public std::exception
{

public:
  virtual const char* what() const throw()
  {
    return "Time Inconsistency";
  }

};

//! Abstract class to the tightener
class SCTIME_EXPORT MscIntervalTightener
{
public:
  virtual ~MscIntervalTightener() {}

  //!returns NULL if inconsistent
  virtual const IntervalMatrix* tight(const IntervalMatrix&)
    throw(MscTimeInconsistencyException)=0;

  virtual std::pair<unsigned,unsigned> incon_position_get()=0;    
//  virtual std::pair<unsigned,unsigned> get_incon_position()=0;
};

//TODO erase
//! Abstract class to the Consistency checker
class SCTIME_EXPORT MscIntervalConsistencyCheck
{
public:
  virtual ~MscIntervalConsistencyCheck() {}

  virtual bool check_consistency(const IntervalMatrix&)=0;

  virtual std::pair<unsigned,unsigned> incon_position_get()=0;
};

//! Floyd Warshall: consistency checker and tightener, IntervalMatrix
class SCTIME_EXPORT FloydWarshall:
public MscIntervalTightener //,public MscIntervalConsistencyCheck
{
private:
  // inconsistent position
  std::pair<int,int> m_incon_position;

  void set_incon_position(int x, int y)
  {
    m_incon_position = std::make_pair(x,y);
  }

  void reset_incon_position()
  {
    set_incon_position(-1,-1);
  }

  bool incon_position_valid()
  {
    if(m_incon_position.first==-1||m_incon_position.second==-1)
      return false;

    return true;
  }

public:
  FloydWarshall()
  {
    reset_incon_position();
  }

  const IntervalMatrix* tight(const IntervalMatrix&)
    throw(MscTimeInconsistencyException);

  bool check_consistency(const IntervalMatrix&);

  std::pair<unsigned,unsigned> incon_position_get()
  {
    if(incon_position_valid())
      return std::make_pair((unsigned) m_incon_position.first,(unsigned) m_incon_position.second);

    throw std::runtime_error("Inconsistent position has not been set!");
  }

  ~FloydWarshall()
  {}
}; // end of FloydWarshall


class TightenBMsc;
class BMscTighter;

struct MscSolveTCSPReport {
  MscSolveTCSPReport(const IntervalSetMatrix& original)
  {
    m_number_of_consistent_choices =0;
    m_matrix_original = original;//TODO isn't this too slow? pointer should be faster
    m_matrix_result.resize(original.size());
    
    // null result matix
    for(unsigned i=0;i<original.size();i++)
    {
      for(unsigned j=0;j<original.size();j++)
      {
        m_matrix_result(i,j)=MscTimeIntervalSetD();
      }
    }
  }

  // matrix recieved to be tought
  IntervalSetMatrix m_matrix_original;
  // matrix with results from tightening
  IntervalSetMatrix m_matrix_result;
  // array of inconsistent matrices
  std::vector<IntervalMatrix> inconsis_mtxs;
  std::vector<std::pair<unsigned,unsigned> > inconsis_indxs;

  unsigned m_number_of_consistent_choices;//resp. number of csp matrices that were put together
};

struct MscSolveTCSPComponentMatrixReport {
  MscSolveTCSPComponentMatrixReport(const IntervalSetComponentMatrix& original)
  {
    m_number_of_consistent_choices =1;
    m_matrix_original = original;
  }

   // matrix recieved to be tought
  IntervalSetComponentMatrix m_matrix_original;
   // matrix with results from tightening
  IntervalSetComponentMatrix m_matrix_result;
  // array of inconsistent matrices
  // the number in pair states the component number for the interval matrix
  std::vector<std::pair<unsigned,IntervalMatrix> > inconsis_mtxs;
  std::vector<std::pair<unsigned,unsigned> > inconsis_indxs;
  unsigned m_number_of_consistent_choices;//resp. number of csp matrices that were put together
};

class SCTIME_EXPORT MscSolveTCSP
{
private:
  MscIntervalTightener* m_tightener;
  MscIntervalConsistencyCheck* m_con_checker;
  MscSolveTCSPReport* m_report;
  unsigned m_size;
  /**
   * \brief init simple matrix
   * puts to the matrix m interval (0,0)
   * if row == colomn and (-inf,inf) otherwise
   */
  void init_simple_matrix(IntervalMatrix& m);

  /**
   * \brief tries to insert all intervals on row,col position
   * Takes TimeIntervalSet on the m_matrix_to_original[row,col] position
   * and insert it to the  IntervalMatrix m[row,col] and checks
   * the matrix for consistency and if not so, interval is skipped
   */
  void insert_interval(IntervalMatrix m, unsigned int row, unsigned int col);


  //! increase row, col and order tightening
  void move_forward(const IntervalMatrix& m, unsigned int row, unsigned int col);

  //! tight to_tight and add result to report
  bool tight(const IntervalMatrix& to_tight);

protected:


public:
  MscSolveTCSP():
      m_tightener(new FloydWarshall())
  {
  }


  ~MscSolveTCSP()
  {
    delete m_tightener;
  }

  //! main function, first to call
  MscSolveTCSPReport solveTCSP(const IntervalSetMatrix matrix);

  //! solves matrix and returns report
  MscSolveTCSPComponentMatrixReport solveTCSP(const IntervalSetComponentMatrix matrix);

};


///////////////////////////////////////////////////////////////////

class TemporaryStrictEvent: public StrictEvent
{
  ReferenceNode* m_ref_node;
  bool m_is_top;
  
public:
  TemporaryStrictEvent(ReferenceNode* ref_node,bool is_top)
  {
    m_ref_node = ref_node;
    m_is_top = is_top;
  }
  
  ReferenceNode* get_ref_node()
  {
    return m_ref_node;
  }
  
  bool is_top()
  {
    return m_is_top;
  }
};


class TimeRelationRefNodeCycle: public WhiteNodeFoundListener,  public GrayNodeFoundListener, public NodeFinishedListener
{
   std::map<TimeRelationRefNode*,MscElementPListList> m_open_relations;
   std::map<TimeRelationRefNode*,MscElementPListList> m_cycle_relations;
  DFSHMscTraverser* m_hmsc_traverser;
  
public:
  TimeRelationRefNodeCycle(DFSHMscTraverser* hmsc_traverser)
  {
    m_hmsc_traverser = hmsc_traverser;
  }
    
  ~TimeRelationRefNodeCycle()
  {
  }
  
  //checks each of time relation events connected to node from parameter and if it is not in open_relations it adds it there, 
  //if it is open_relations it deletes the relation from there

  void on_white_node_found(HMscNode* node)
  {
    ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>(node);
    if(!ref_node) return;
    
    TimeRelationRefNodePtrSet set = ref_node->get_time_relations_top();
    TimeRelationRefNodePtrSet::iterator it;
    std::map<TimeRelationRefNode*,MscElementPListList>::iterator it2;
    for(it = set.begin();it!= set.end();it++)
    {
      it2 = m_open_relations.find(it->get());
      if(it2 == m_open_relations.end())
      {
	it2 = m_cycle_relations.find(it->get());
	if(it2 == m_cycle_relations.end())
	{
	  MscElementPListList list;
	  m_open_relations.insert(std::make_pair<TimeRelationRefNode*,MscElementPListList>( it->get(),list));
	}
	else
	{
	  it2->second = m_hmsc_traverser->get_reached_elements(); //este sa da odobrat a vlozit znova
	}
      }
      else
      {
	m_open_relations.erase(it2);
      }
    }
    
    set = ref_node->get_time_relations_bottom();    
    for(it = set.begin();it!= set.end();it++)
    {
      it2 = m_open_relations.find(it->get());
      if(it2 == m_open_relations.end())
      {
	it2 = m_cycle_relations.find(it->get());
	if(it2 == m_cycle_relations.end())
	{
	  MscElementPListList list;
	  m_open_relations.insert(std::make_pair<TimeRelationRefNode*,MscElementPListList>( it->get(),list));
	}
	else
	{
	  it2->second = m_hmsc_traverser->get_reached_elements(); //este sa da odobrat a vlozit znova
	}
      }
      else
      {
	m_open_relations.erase(it2);
      }
    }
  }
  
  void on_gray_node_found(HMscNode* node)
  {
    std::map<TimeRelationRefNode*,MscElementPListList>::iterator it_remove;
    for(std::map<TimeRelationRefNode*,MscElementPListList>::iterator it2=m_open_relations.begin();it2 != m_open_relations.end();)
    {
      m_cycle_relations.insert(std::make_pair<TimeRelationRefNode*,MscElementPListList>( it2->first,it2->second));
      it_remove = it2;
      it2++;
      m_open_relations.erase(it_remove);
    }
  }
  
  void on_node_finished(HMscNode* node)
  {
    ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>(node);
    if(!ref_node) return;
    TimeRelationRefNodePtrSet set = ref_node->get_time_relations_bottom();

    TimeRelationRefNodePtrSet::iterator it;
    std::map<TimeRelationRefNode*,MscElementPListList>::iterator it2;
    for(it = set.begin();it!= set.end();it++)
    {
      it2 = m_open_relations.find(it->get());
      if(it2 == m_open_relations.end())
      {
	it2 = m_cycle_relations.find(it->get());
	if(it2 == m_cycle_relations.end())
	{
	  m_open_relations.insert(std::make_pair<TimeRelationRefNode*,MscElementPListList>( it->get(),m_hmsc_traverser->get_reached_elements()));
	}
	else
	{
	  //do nothing
	}
      }
      else
      {
	m_open_relations.erase(it2);
      }
    }
  
    set = ref_node->get_time_relations_top();
    for(it = set.begin();it!= set.end();it++)
    {
      it2 = m_open_relations.find(it->get());
      if(it2 == m_open_relations.end())
      {
	it2 = m_cycle_relations.find(it->get());
	if(it2 == m_cycle_relations.end())
	{
	  m_open_relations.insert(std::make_pair<TimeRelationRefNode*,MscElementPListList>( it->get(),m_hmsc_traverser->get_reached_elements()));
	}
	else
	{
	  //do nothing
	}
      }
      else
      {
	m_open_relations.erase(it2);
      }
    }
  }

  
  std::map<TimeRelationRefNode*,MscElementPListList> get_cycle_relations()
  {
    return m_cycle_relations;
  }
  
};


class AllCombination: public std::vector<std::pair<EventP,EventP> >
{
private:
  std::list<std::vector<std::pair<EventP,EventP> > > lists;

  std::vector<std::vector<std::pair<EventP,EventP> > >* v_lists;
  std::vector<size_type>* iterators;
  bool ini;

  void set_value(size_t to)
  {
    for(size_t i= 0; i<= to;i++)
    {
      at(i)=((*v_lists)[i])[(*iterators)[i]];
    }

  }

  bool move_next(size_t i)
  {
    if(i>=v_lists->size())
    {
      return false;
    }

    (*iterators)[i]++;

    if((*iterators)[i]>=(*v_lists)[i].size())
    {
      (*iterators)[i]=0;
      return move_next(++i);
    }
    set_value(i);
    return true;
  }

public:
  AllCombination():v_lists(NULL),iterators(NULL),ini(false)
  {

  }

  ~AllCombination()
  {
    if(ini)
    {
      delete v_lists;
      delete iterators;
    }
  }

  void add_list(std::vector<std::pair<EventP,EventP> > floor)
  {
    lists.push_back(floor);
  }

  void init()
  {
    if(lists.size() == 0)
      return ;
    if(ini)
      throw std::runtime_error("Already allocated!");
    else
      ini = true;

    v_lists = new std::vector<std::vector<std::pair<EventP,EventP> > >(lists.begin(),lists.end());
    resize(lists.size());
    iterators = new std::vector<size_type>(lists.size(),0);
    set_value(lists.size()-1);
  }

  bool move_next()
  {
    return move_next(0);
  }

};




class SCTIME_EXPORT HMscConsistencyChecker: public PathFoundListener
{

  private:
    bool m_causal;
    std::list<Block>::iterator m_iter;
    std::list<HMscPtr> m_final_result;
public:
  HMscConsistencyChecker():m_causal(false)
  {

  }

  ~HMscConsistencyChecker()
  {
  }

  virtual std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr mapper)
  {
    //check whether the relations spanning across cycle contain infinity
    
    DFSHMscTraverser hmsc_traverser;    
    TimeRelationRefNodeCycle cycle_relations_finder(&hmsc_traverser);
    hmsc_traverser.add_white_node_found_listener(&cycle_relations_finder);
    hmsc_traverser.add_gray_node_found_listener(&cycle_relations_finder);
    hmsc_traverser.add_node_finished_listener(&cycle_relations_finder);
    hmsc_traverser.traverse(hmsc);
    
    
    std::map<TimeRelationRefNode*,MscElementPListList> cycle_relations = cycle_relations_finder.get_cycle_relations();
    std::map<TimeRelationRefNode*,MscElementPListList>::iterator it;
    for(it = cycle_relations.begin();it != cycle_relations.end();it++)
    {
      MscTimeIntervalD inter = (it->first)->get_interval_set().get_set().back(); //we assume that inteval sets are ordered (infty is last)
      if(!inter.is_upper_bound_infinity()) //check whether the interval set of time realtion contains infinity //the extension of time interval is needed
      {
	HMscPathDuplicator duplicator2;
	(it->second);
	MscElementPListList list3 = (it->second);
	HMscPtr ce_hmsc = duplicator2.duplicate_path(list3);
	duplicator2.get_copy(it->first)->set_marked();
	m_final_result.push_back(ce_hmsc);
      }
    }
  

    BMscGraphDuplicator graph_dup;
    HMscPtr bmsc_graph = graph_dup.duplicate_hmsc(hmsc);
   
    
    TraverseAndMarkBlocks block_marker;
    block_marker.travers_and_mark_blocks(bmsc_graph);

    int i=0;
    for (m_iter = block_marker.m_list_of_blocks.begin(); m_iter != block_marker.m_list_of_blocks.end(); m_iter++)  //iterate blocks
    {
      i++;
      HMscNodePtrSet m_last;
      m_last.insert(m_iter->get_end());
      AllPaths allpaths = AllPaths(bmsc_graph, m_iter->get_begin(), m_last, 1);
      allpaths.add_path_found_listener(this);
      allpaths.traverse();
      m_last.clear();
    }

    return m_final_result;
  }


/**
* \brief Tightens BMscGraph path and returns BMscIntervalSetMatrix with original constraints and IntervalSetMatrix with tightened constraints.
*/
  virtual void on_path_found(std::list<MscElement*>& path)
  {

     std::list<HMscPtr> counter_examples;
     HMscFlatPathToBMscDuplicator duplicator;
     BMscPtr bmsc = duplicator.duplicate_path(path);

     SRChannelMapperPtr srm = SRChannelMapper::instance();
     BMscIntervalSetMatrix b_matrix(bmsc,false,srm); // TODO: channelmapper

    // check inconsistents time relations/ intervals - matrix has already empty interval set on cell that is tied to the time relation
    std::list<TimeRelation*> empty = b_matrix.get_rel_empty_set();
    if(empty.size()!=0) // ups, inconsistency was found, mark relations and push back BMsc
    {
      //duplicate path
      MscElementPListList path_list = m_iter->get_path_to_block();

      if(path_list.size() > 1){
	throw std::runtime_error("Found hierarchical path. Expected flat path.");
      }
      
      path_list.back().pop_back(); //the path is too long    
      //concatenate paths and then duplicate
      path.insert(path.begin(),path_list.back().begin(),path_list.back().end());
      path_list.clear();
      path_list.push_back(path);

      HMscPathDuplicator duplicator;
      HMscPtr example = duplicator.duplicate_path(path_list);
      
      std::list<TimeRelation*>::iterator it;
      for(it=empty.begin();it!=empty.end();it++)
      {
	TimeRelation* copy = *it;
	while(copy->get_original()->get_original()!=NULL)
	{
	  copy = copy->get_original();
	}
        duplicator.get_copy(copy)->set_marked();
      }

      m_final_result.push_back(example);
      return ;
    }     
     
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
          a = new TemporaryStrictEvent(ref,true);
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
          a = new TemporaryStrictEvent(ref,false);
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


     IntervalSetMatrix result;
     result.resize(b_matrix.size());

     combination.init();
     if(combination.size()==0){
       //tighten the matrix
       MscSolveTCSP solve;
       MscSolveTCSPReport report = solve.solveTCSP(b_matrix);
       if(report.m_number_of_consistent_choices==0)  // inconsistent
	{
	  std::list<HMscPtr> c_example_list= make_counter_example(b_matrix,&report, path);
	  m_final_result.insert(m_final_result.end(),c_example_list.begin(),c_example_list.end());
	}
       return ;
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
       if(report.m_number_of_consistent_choices!=0) // inconsistent => fill the values to result matrix
       {
	 IntervalSetMatrix new_result = report.m_matrix_result;

	 for(unsigned i=0;i<new_result.size();i++)
	   for(unsigned j=0;j<new_result.size();j++)
	     result(i,j) = MscTimeIntervalSetD::set_union(result(i,j),new_result(i,j));
       }
       else
       {
	 std::list<HMscPtr> c_example_list = make_counter_example(b_matrix,&report, path);
	 counter_examples.insert(counter_examples.end(),c_example_list.begin(),c_example_list.end());
       }
       

       //return back previous values
       for(unsigned i=0;i<combination.size();i++)
       {
         b_matrix.fill(combination[i].first,combination[i].second,MscTimeIntervalD(0,D::infinity()));
       }

     } while(combination.move_next());

     if(result(0,0).get_set().empty())
       m_final_result.insert(m_final_result.end(),counter_examples.begin(),counter_examples.end());
     
    return ;
  }

  std::list<HMscPtr> make_counter_example(BMscIntervalSetMatrix b_matrix, MscSolveTCSPReport* report,std::list<MscElement*> path)
  {
    HMscPtr example;
    HMscPtr final_example;
    std::list<HMscPtr> result;
    MscElementPListList path_list = m_iter->get_path_to_block();

    if(path_list.size() > 1){
      throw std::runtime_error("Found hierarchical path. Expected flat path.");
    }
    
    path_list.back().pop_back(); //the path is too long    
    //concatenate paths and then duplicate
    path.insert(path.begin(),path_list.back().begin(),path_list.back().end());
    path_list.clear();
    path_list.push_back(path);
    //set the last time constraint marked
    MscTimeIntervalSetD last_interval;
    TimeRelationEventPtrList rel_list;
    TimeRelationEventPtr new_rel;
    EventPVector events = b_matrix.get_events();
    
    //duplicate path
    HMscPathDuplicator duplicator;
    example = duplicator.duplicate_path(path_list);
    
    //for each inconsistency create counter-example
    for(unsigned int i = 0; i < report->inconsis_mtxs.size(); i++)
    {
      // mark the inconsistent time relation
      last_interval = report->inconsis_mtxs[i](report->inconsis_indxs[i].first, report->inconsis_indxs[i].second);

      //duplicate hmsc for one counter-example
      HMscDuplicator duplicator2;
      final_example = duplicator2.duplicate(example);

      unsigned int index_a = report->inconsis_indxs[i].first;
      Event* event_a = events[index_a];
      
      unsigned int index_b = report->inconsis_indxs[i].second;
      EventP event_b = events[index_b]; 

      TemporaryStrictEvent* t_s_event_a = dynamic_cast<TemporaryStrictEvent*>(event_a);
      TemporaryStrictEvent* t_s_event_b = dynamic_cast<TemporaryStrictEvent*>(event_b);      
      
      std::map<TimeRelation*, std::pair<unsigned,unsigned> > tied_relations = b_matrix.get_tied_time_relations();

      bool got_counter_example = false;
      if(t_s_event_a && t_s_event_b)
	for(std::map<TimeRelation*, std::pair<unsigned,unsigned> >::iterator it=tied_relations.begin();it!=tied_relations.end();it++)
	{
	  //check whether relation in it->first is between pair of inconsistent events
	  if((it->second.first == index_a && it->second.second == index_b) 
	    || (it->second.first == index_b && it->second.second == index_a))
	  {
	    got_counter_example = true;
	    //marking the time relation
	    duplicator2.get_copy(duplicator.get_copy(it->first))->set_marked();
	    result.push_back(final_example);
	    break;
	  }
	}
      if(got_counter_example) continue; //we already created counter_example for inconsistency i
    

      //get the right original of event_a
      if(event_a->get_original()) //if we are taking care of event which did not previosly occur in the path, it does not have original
	while(event_a->get_original()->get_original()!=NULL)
	{
	  event_a = event_a->get_original();
	}



      //get the right original of event_b
      if(event_b->get_original())//if we are taking care of event which did not previosly occur in the path, it does not have original
	while(event_b->get_original()->get_original()!=NULL)
	{
	  event_b = event_b->get_original();
	}
      
      if(!t_s_event_a && !t_s_event_b && event_a->get_instance()->get_bmsc() == event_b->get_instance()->get_bmsc())
      {// we know that inconsistency is in one bmsc between event_a and event_b
	rel_list = TimeRelationsFunc::get(event_a,event_b);
	DEBUG(rel_list.size());
	switch(rel_list.size())
	{
	  case 0: //there is no time relation between event_a and event_b
	  {       //create new one
	    new_rel = new TimeRelationEvent(last_interval);
	    new_rel->glue_events(dynamic_cast<Event*>(duplicator2.get_copy(event_a)),
				 dynamic_cast<Event*>(duplicator2.get_copy(event_b)));
	    new_rel->set_marked();
	    break;
	  }
	  case 1: //the time relation between event_a and event_b exists-> mak it
	  {
	    duplicator2.get_copy(rel_list.begin()->get())->set_marked();
	    break;
	  }
	  default:
	  {
	    for(TimeRelationEventPtrList::iterator it = rel_list.begin();it!=rel_list.end();it++)
	    {
	      duplicator2.get_copy(it->get())->set_marked();
	    }
	  }
	}
      }
      else //the inconsitent events are not in same bmsc
      {
	if(!t_s_event_a) //event_a exists in some bmsc -> mark it
	{
	  duplicator2.get_copy(event_a)->set_marked();
	}
	
	if(!t_s_event_b) //event_a exists in some bmsc -> mark it
	{
	  duplicator2.get_copy(event_b)->set_marked();
	}
      }

      //marking of the hmsc nodes where are events causing inconsistency
      MscElementPList::const_iterator elem;
      ReferenceNode* ref;
      TimeRelationRefNodePtrSet set;
      TimeRelationRefNodePtrSet::iterator it;
      if(t_s_event_a)
      {
	//marking the reference node
	ref = t_s_event_a->get_ref_node();
	duplicator2.get_copy(duplicator.get_copy(ref))->set_marked();
	//marking of all time realtions from the correct boundary of reference node
	if(t_s_event_a->is_top())
	  set = ref->get_time_relations_top();
	else
	  set = ref->get_time_relations_bottom();
	for(it = set.begin();it != set.end();it++)
	{
	  MscElement* relation = duplicator.get_copy((*it).get());
	  if(relation)  duplicator2.get_copy(relation)->set_marked();
	}
      }
      else
      {
	for(elem=path.begin();elem!=path.end();elem++)
	{
	  //searching for correct reference node
	  ReferenceNode* r = dynamic_cast<ReferenceNode*>(*elem);
	  if(r && (r->get_bmsc().get() == event_a->get_instance()->get_bmsc()))
	  {
	    duplicator2.get_copy(duplicator.get_copy(r))->set_marked();
	  }
	}

      }

      if(t_s_event_b)
      {
	//marking the reference node
	ref = t_s_event_b->get_ref_node();
	duplicator2.get_copy(duplicator.get_copy(ref))->set_marked();
	//marking of all time realtions from the correct boundary of reference node
	if(t_s_event_b->is_top())
	  set = ref->get_time_relations_top();
	else
	  set = ref->get_time_relations_bottom();
	for(it = set.begin();it != set.end();it++)
	{
	  MscElement* relation = duplicator.get_copy((*it).get());
	  if(relation)  duplicator2.get_copy(relation)->set_marked();
	}
      }
      else
      {
	//searching for correct reference node
	for(elem=path.begin();elem!=path.end();elem++)
	{
	  ReferenceNode* r = dynamic_cast<ReferenceNode*>(*elem);
	  if(r && (r->get_bmsc().get() == event_b->get_instance()->get_bmsc()))
	  {
	    duplicator2.get_copy(duplicator.get_copy(r))->set_marked();
	  }
	}	
      }
      result.push_back(final_example);
     }
    return result;
  }

  void set_causal()
  {
    m_causal = true;
  }
};

//////////////////////////////////////////////////////////////////////////////////

class SCTIME_EXPORT ConsistencyChecker: public Checker, public BMscChecker, public HMscChecker
{
public:


  virtual std::wstring get_property_name() const
  {
    return L"Time Consistent";
  }

  virtual bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"time_consistency/time_consistency.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const
  {
    Checker::PreconditionList precon_list;
    precon_list.push_back(PrerequisiteCheck(L"Correct Time Constraint Syntax", PrerequisiteCheck::PP_REQUIRED));
    return precon_list;
  }

  virtual std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr mapper)
  {
    HMscConsistencyChecker hmsc_checker;
    std::list<HMscPtr> resultsXXX = hmsc_checker.check(hmsc,mapper);
    return  resultsXXX;
  }

  virtual std::list<BMscPtr> check(BMscPtr bmsc, ChannelMapperPtr mapper)
  {
//version with normal matrix
//     std::list<BMscPtr> res_list;
//     BMscIntervalSetMatrix matrix(bmsc);
// 
// 
//     // check inconsistents time relations/ intervals - matrix has already empty interval set on cell that is tied to the time relation
//     std::list<TimeRelation*> empty = matrix.get_rel_empty_set();
//     if(empty.size()!=0) // ups, inconsistency was found, mark relations and push back BMsc
//     {
//       std::list<TimeRelation*>::iterator it;
//       for(it=empty.begin();it!=empty.end();it++)
//       {
//         (*it)->set_marked();
//       }
// 
//       BMscPtr error = BMscDuplicator::duplicate(bmsc);
//       res_list.push_back(error);
//       return res_list;
//     }
//     // lets take next step, build it up
//     matrix.build_up();
// 
//     // check for empty interval sets => inconsistency
// 
// 
//     MscSolveTCSP solve;
//     MscSolveTCSPReport report = solve.solveTCSP(matrix);
// 
//     // there was at least one matrix found -> consistent
//     if(report.m_number_of_consistent_choices!=0)
//       return res_list;
// 
//     // ok, incosistency was found
//     if(report.inconsis_mtxs.size()==0)
//       throw std::runtime_error("No inconsistent matrix was found even there is some inconsistency!");
//     // pick the inconsistent matrix, (create and) mark inconsistent interval
//     
//     MscTimeIntervalSetD last_interval;
//     TimeRelationEventPtrList rel_list;
//     TimeRelationEventPtr new_rel;
//     EventPVector events = matrix.get_events();
//     
//     for(unsigned int i = 0; i < report.inconsis_mtxs.size(); i++)
//     {
//       
// 
//       BMscPtr copy_bmsc = matrix.get_modified_bmsc(&report.inconsis_mtxs[i]);
//       BMscDuplicator duplicator;
//       copy_bmsc = duplicator.duplicate_bmsc(copy_bmsc);
// 
//       // mark the inconsistent time relation
//       last_interval = report.inconsis_mtxs[i](report.inconsis_indxs[i].first, report.inconsis_indxs[i].second);
// 
//       EventP event_a = duplicator.get_event_copy(events[report.inconsis_indxs[i].first]);
//       EventP event_b = duplicator.get_event_copy(events[report.inconsis_indxs[i].second]);
// 
//       rel_list = TimeRelationsFunc::get(event_a,event_b);
//       DEBUG(rel_list.size());
//       switch(rel_list.size())
//       {
//         case 0:
//         {
//           new_rel = new TimeRelationEvent(last_interval);
//           new_rel->glue_events(event_a,event_b);
// 	  new_rel->set_marked();
//           break;
//         }
//         case 1:
//           rel_list.begin()->get()->set_marked();
//           break;
//         default:
// 	{
// 	  for(TimeRelationEventPtrList::iterator it = rel_list.begin();it!=rel_list.end();it++)
// 	  {
// 	    it->get()->set_marked();
// 	  }
// 	}
//       }
// 
//       res_list.push_back(copy_bmsc);
// 
//     }
//     
//     
// 
//     return res_list;

//version with component matrix
    std::list<BMscPtr> res_list;
    BMscIntervalSetComponentMatrix matrix(bmsc);


    //build it up
    matrix.build_up();
    // check inconsistents time relations/ intervals - matrix has already empty interval set on cell that is tied to the time relation
    std::list<TimeRelation*> empty = matrix.get_rel_empty_set();
    if(empty.size()!=0) // ups, inconsistency was found, mark relations and push back BMsc
    {
      std::list<TimeRelation*>::iterator it;
      for(it=empty.begin();it!=empty.end();it++)
      {
        (*it)->set_marked();
      }

      BMscPtr error = BMscDuplicator::duplicate(bmsc);
      res_list.push_back(error);
      return res_list;
    }    
    // check for empty interval sets => inconsistency
    MscSolveTCSP solve;
    MscSolveTCSPComponentMatrixReport report = solve.solveTCSP(matrix);
    
    // there was at least one matrix found -> consistent
    if(report.m_number_of_consistent_choices!=0)
      return res_list;

    // ok, incosistency was found
    if(report.inconsis_mtxs.size()==0)
      throw std::runtime_error("No inconsistent matrix was found even there is some inconsistency!");
    // pick the inconsistent matrix, (create and) mark inconsistent interval
    MscTimeIntervalSetD last_interval;
    TimeRelationEventPtrList rel_list;
    TimeRelationEventPtr new_rel;
    EventPVector events = matrix.get_events();
    for(unsigned int i = 0; i < report.inconsis_mtxs.size(); i++)
    {
      BMscPtr copy_bmsc = matrix.get_modified_bmsc(report.inconsis_mtxs[i].first,report.inconsis_mtxs[i].second);
      BMscDuplicator duplicator;
      copy_bmsc = duplicator.duplicate_bmsc(copy_bmsc);

      // mark the inconsistent time relation
      last_interval = report.inconsis_mtxs[i].second(report.inconsis_indxs[i].first, report.inconsis_indxs[i].second);

      unsigned offset = matrix.get_offset(report.inconsis_mtxs[i].first);
      EventP event_a = duplicator.get_event_copy(events[report.inconsis_indxs[i].first+offset]);
      EventP event_b = duplicator.get_event_copy(events[report.inconsis_indxs[i].second+offset]);

      rel_list = TimeRelationsFunc::get(event_a,event_b);
      DEBUG(rel_list.size());
      switch(rel_list.size())
      {
        case 0:
        {
          new_rel = new TimeRelationEvent(last_interval);
          new_rel->glue_events(event_a,event_b);
	  new_rel->set_marked();
          break;
        }
        case 1:
          rel_list.begin()->get()->set_marked();
          break;
        default:
	{
	  for(TimeRelationEventPtrList::iterator it = rel_list.begin();it!=rel_list.end();it++)
	  {
	    it->get()->set_marked();
	  }
	}
      }

      res_list.push_back(copy_bmsc);

    }
    return res_list;
  }

  virtual void cleanup_attributes()
  {

  }
};

#endif

// $Id: time_consistency.h 1077 2011-04-06 16:22:35Z lkorenciak $
