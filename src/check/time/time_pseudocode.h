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
 * $Id: time_pseudocode.h 1077 2011-04-06 16:22:35Z lkorenciak $
 */

#ifndef _TIME_PSEUDOCODE_H_
#define _TIME_PSEUDOCODE_H_

#define ERR_MATRIX_SMALLER "matrix: smaller than requested position."
#define ERR_MATRIX_VALID "matrix: not valid - boost size1()!=size2()"


#include "data/time.h"

#include "check/pseudocode/utils.h"
#include "check/time/export.h"
#include "check/pseudocode/msc_duplicators.h"

#include <set>
#include <stdexcept>

//Boost matix
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/graph/graph_concepts.hpp>

typedef MscTimeInterval<double> MscTimeIntervalD;
typedef MscTimeIntervalSet<double> MscTimeIntervalSetD;
typedef std::list<MscTimeIntervalD> IntervalList;
typedef std::vector<Event*> EventPVector;


typedef std::set<Event*> EventPSet;
typedef std::map<Event*,EventPSet> EventPMap;
typedef std::numeric_limits<double> D;
typedef boost::numeric::ublas::matrix<MscTimeIntervalD> BoostIntervalMatrix;
typedef boost::numeric::ublas::matrix<MscTimeIntervalSetD> BoostIntervalSetMatrix;
class HMscAllTimeRelations;

class IntervalMatrixFunc;
class BMscIntervalMatrixConverter;

class IntervalMatrix
{
  BoostIntervalMatrix boost_matrix;
public:
  IntervalMatrix()
  {
    boost_matrix = BoostIntervalMatrix();
  }

  IntervalMatrix(unsigned size)
  {
    boost_matrix = BoostIntervalMatrix(size,size);
  }

  const unsigned int size() const
  {
    if(boost_matrix.size1()!=boost_matrix.size2())
      throw std::runtime_error(ERR_MATRIX_VALID);
    return boost_matrix.size1();
  }

  //! adding to the matrix INTERVAL c to (x,y) and its inverse to (y,x)
  void fill(unsigned int x,unsigned int y,MscTimeIntervalD& c)
  {
    if (x>=boost_matrix.size1()||y>=boost_matrix.size1())
      throw std::runtime_error(ERR_MATRIX_SMALLER);
    boost_matrix.operator()(x,y)=c;
    boost_matrix.operator()(y,x)=MscTimeIntervalD::interval_inverse(c);
  }

  MscTimeIntervalD& operator() (const unsigned int x,const unsigned int y)
  {
    return boost_matrix.operator()(x,y);
  }

  const MscTimeIntervalD& operator() (const unsigned int x,const unsigned int y) const
  {
    return boost_matrix.operator()(x,y);
  }

  void resize(unsigned int size){
    boost_matrix.resize(size,size);
  }
  
  void print() const
  {
    std::cout<<boost_matrix<<std::endl;
  }
};

//TODO make template class for component matrix then define IntervalSetComponentMatrix, IntervalComponentMatrix
class IntervalComponentMatrix
{
//  int number_of_components;
  unsigned int size;
  std::vector<IntervalMatrix> components;
public:
  IntervalComponentMatrix()
  {
    size = 0;
  }
  
//   IntervalComponentMatrix(unsigned size)
//   {
//     this.size = size;
//     components.push_back(IntervalMatrix(size));
//   }

  const unsigned int get_size() const
  {
    return size;
  }

  const MscTimeIntervalD operator() (const unsigned int x,const unsigned int y) const
  {
    //searching component for x
    unsigned int index_x=x; //index y in the correct component
    unsigned int component_x =0;
    while(components[component_x].size()<=index_x)
    {
      index_x = index_x - components[component_x].size() + 1;
      component_x++;
      if(component_x==components.size()) throw std::runtime_error(ERR_MATRIX_SMALLER);
    }
    
    unsigned int index_y =y; //index x in the correct component
    unsigned int component_y =0;
    while(components[component_y].size()<=index_y)
    {
      index_y = index_y - components[component_y].size() + 1;
      component_y++;
      if(component_y==components.size()) throw std::runtime_error(ERR_MATRIX_SMALLER);
    }
    //both indices are in the same component
    if (component_x==component_y) return components[component_x].operator()(index_x,index_y);
    
    if(component_x<component_y)
    {
      MscTimeIntervalD interval(components[component_x].operator()(index_x,components[component_x].size()-1));
      // add intervals
      for(unsigned int i = component_x+1;i<component_y;i++)
      { 
	interval = interval.operator+(components[i].operator()(0,components[i].size()-1));
      }
      interval = interval.operator+(components[component_y].operator()(0,index_y));
      return interval;
    }
    else
    {
      MscTimeIntervalD interval(components[component_x].operator()(index_x,0));
      // add intervals
      for(unsigned int i=component_x-1;i>component_y;i--)
      {  
	interval = interval.operator+(components[i].operator()(components[i].size()-1,0));
      }
      interval = interval.operator+(components[component_y].operator()(components[component_y].size()-1,index_y));
      return interval;
    }
  }
  
  //adds new component as last component
  void push_back_component(IntervalMatrix& matrix)
  {
    components.push_back(matrix);
    if(size)
      size =size + matrix.size() -1;
    else
      size += matrix.size();
  }

  void print() const
  {
    for(unsigned int i=0;i<components.size();i++)
    {
      components[i].print();
    }
  }

};



class SCTIME_EXPORT IntervalSetMatrix
{
  BoostIntervalSetMatrix boost_matrix;

public:
  IntervalSetMatrix()
  {
    boost_matrix = BoostIntervalSetMatrix();
  }

  IntervalSetMatrix(unsigned size)
  {
    boost_matrix = BoostIntervalMatrix(size,size);
  }

  IntervalSetMatrix(const IntervalSetMatrix& matrix)
  {
    boost_matrix = BoostIntervalSetMatrix(matrix.boost_matrix);
  }

  //! adding to the matrix-set INTERVAL SET c to (x,y) and its inverse to (y,x)
  void fill(const unsigned int x, const unsigned int y,const MscTimeIntervalSetD& c)
  {
    if (x>=boost_matrix.size1()||y>=boost_matrix.size1())
      throw std::runtime_error(ERR_MATRIX_SMALLER);

    boost_matrix.operator()(x,y) = c;

    boost_matrix.operator()(y,x) = MscTimeIntervalSetD::interval_inverse(c);
  }
  
  //! adding to the matrix-set INTERVAL c to (x,y) and its inverse to (y,x)

  void fill(const unsigned x, const unsigned y, const MscTimeIntervalD& c)
  {
    if (x>=boost_matrix.size1()||y>=boost_matrix.size1())
      throw std::runtime_error(ERR_MATRIX_SMALLER);

    boost_matrix.operator()(x,y).insert(c);
    boost_matrix.operator()(y,x).insert(MscTimeIntervalD::interval_inverse(c));
  }

  void resize(unsigned int i)
  {
    boost_matrix.resize(i,i);
  }

  IntervalSetMatrix& operator=(const IntervalSetMatrix& matrix)
  {
    boost_matrix.operator=(matrix.boost_matrix);
    return *this;
  }

  IntervalSetMatrix& operator=(const IntervalMatrix& matrix)
  {
    resize(matrix.size());
      for(unsigned g = 0; g < matrix.size(); g++)
      {
        for(unsigned h = 0; h < matrix.size(); h++)
        {
          boost_matrix(g,h) = MscTimeIntervalSetD();
          boost_matrix(g,h).insert(matrix(g,h));
        }
      }
    return *this;
  }

  MscTimeIntervalSetD& operator() (unsigned int x, unsigned int y)
  {
    return boost_matrix.operator()(x,y);
  }

  const MscTimeIntervalSetD& operator() (unsigned int x, unsigned int y) const
  {
    return boost_matrix.operator()(x,y);
  }

  const unsigned int size() const
  {
    if(boost_matrix.size1()!=boost_matrix.size2())
      throw std::runtime_error(ERR_MATRIX_VALID);
    return boost_matrix.size1();
  }

  const BoostIntervalSetMatrix::iterator1 begin1()
  {
    return boost_matrix.begin1();
  }

  const BoostIntervalSetMatrix::iterator1 end1()
  {
    return boost_matrix.end1();
  }
  
  void print() const
  {
    std::cout<<boost_matrix<<std::endl;
  }
};

class IntervalSetComponentMatrix
{
  unsigned int m_size;
  std::vector<IntervalSetMatrix> m_components;
//  std::vector< std::pair<unsigned,unsigned>> index_to_component; //TODO we can use additional map to search indices and components in logarithmic time
public:
  IntervalSetComponentMatrix()
  {
    m_size = 0;
  }
  
  const unsigned int get_size() const
  {
    return m_size;
  }

  //!returns number of components
  const unsigned get_number_of_components() const
  {
    return m_components.size();
  }

  //! for given element on index x in component matrix returns pair (component index, index of element in this component) 
  std::pair<unsigned int, unsigned int> get_component_index(const unsigned int x) const
  {
    unsigned int index=x; //index x in the correct component
    unsigned int component =0;
    while(m_components[component].size()<=index)
    {
      index = index - m_components[component].size() + 1;
      component++;
      if(component==m_components.size()) throw std::runtime_error(ERR_MATRIX_SMALLER);
    }
    return std::make_pair<unsigned int, unsigned int>(component,index);
  }

//! if the indices are in the same component return offset of the component, else return -1
  int in_same_component(const unsigned x,const unsigned y,const unsigned component_x,const unsigned offset_x)
  {
    if(x>y)throw std::invalid_argument("Index x must be smaller or equal as index y.");
    if(m_components[component_x].size()>y-offset_x) return offset_x; //in same component
    if((x-offset_x+1 == m_components[component_x].size()) && (y-offset_x- m_components[component_x].size()< m_components[component_x+1].size()))
      return offset_x+m_components[component_x].size()-1;
    else
      return -1;
  }

  //! works only if the indices x and y are in the same component
  MscTimeIntervalSetD& operator() (const unsigned int x,const unsigned int y)
  {
    unsigned index_a;
    unsigned component_a;
    int offset_b;
    if(x<=y)
    {
      std::pair<unsigned int, unsigned int> pair = get_component_index(x);
      index_a=pair.second; //index x in the correct component
      component_a =pair.first;
      offset_b =in_same_component(x,y,component_a,x-index_a);
    }
    else
    {
      std::pair<unsigned int, unsigned int> pair = get_component_index(y);
      index_a=pair.second; //index x in the correct component
      component_a =pair.first;
      offset_b =in_same_component(y,x,component_a,y-index_a);
    }

    if (offset_b == -1) throw std::invalid_argument("Indices x and y are not in the same component.");
    
//  both indices are in the same component
    if (((x<=y)&&((unsigned)offset_b==x-index_a)) ||((x>y)&&((unsigned)offset_b==y-index_a)))
    {
      return m_components[component_a].operator()(x-offset_b,y-offset_b);
    }
    else
    {
      return m_components[component_a+1].operator()(x-offset_b,y-offset_b);
    }
  }

  //! returns element on the position x,y
  const MscTimeIntervalSetD get_element(const unsigned int x,const unsigned int y) const
  {
    std::pair<unsigned int, unsigned int> pair = get_component_index(x);
    unsigned int index_x=pair.second; //index x in the correct component
    unsigned int component_x =pair.first;
    
    pair = get_component_index(y);
    unsigned int index_y =pair.second; //index y in the correct component
    unsigned int component_y =pair.first;

    //both indices are in the same component
    if (component_x==component_y) return m_components[component_x].operator()(index_x,index_y);
    
    if(component_x<component_y)
    {
      MscTimeIntervalSetD interval(m_components[component_x].operator()(index_x,m_components[component_x].size()-1));
      // add intervals
      for(unsigned int i = component_x+1;i<component_y;i++)
      { 
	interval = interval.operator+(m_components[i].operator()(0,m_components[i].size()-1));
      }
      interval = interval.operator+(m_components[component_y].operator()(0,index_y));
      return interval;
    }
    else
    {
      MscTimeIntervalSetD interval(m_components[component_x].operator()(index_x,0));
      // add intervals
      for(unsigned int i=component_x-1;i>component_y;i--)
      {  
	interval = interval.operator+(m_components[i].operator()(m_components[i].size()-1,0));
      }
      interval = interval.operator+(m_components[component_y].operator()(m_components[component_y].size()-1,index_y));
      return interval;
    }
  }
  
  //!adds new component as last component
  void push_back_component(IntervalSetMatrix& matrix)
  {
    m_components.push_back(matrix);
    if(m_size)
      m_size =m_size + matrix.size() -1;
    else
      m_size += matrix.size();
  }

  void print() const
  {
    for(unsigned int i=0;i<m_components.size();i++)
    {
      m_components[i].print();
    }
  }
  
  //!returns the i-th component
  IntervalSetMatrix get_component(unsigned i) const
  {
    if(i>=m_components.size()) 
      throw std::invalid_argument("Index i is larger than number of components.");
    return m_components[i]; 
  }
  
  //!returns offset for indices in component on index index
  unsigned get_offset(unsigned index) const
  {
    unsigned offset=0;
    for(unsigned i=0;i<index;i++)
      offset += (m_components[i].size()-1);
    return offset;
  }
};


class SCTIME_EXPORT BMscIntervalSetMatrix: public IntervalSetMatrix
{
private:
  BMscPtr m_bmsc_original;
  unsigned m_size;
  bool m_builded;

  bool m_casually;
  ChannelMapperPtr m_mapper;

  std::map< unsigned,unsigned> m_filled_positions;  
//  IntervalSetMatrix m_matrix_original; // I think it is useless

  std::map<std::pair<unsigned,unsigned>, MscTimeIntervalSetD> m_position_to_interval;

  std::map<TimeRelation*, std::pair<unsigned,unsigned> > m_rel_to_position;
  std::map<EventPtr,unsigned> m_event_to_number; //! the number of event in the matrix //TODO check with Ondra whether correct

  EventPVector m_events;  //! Vector of all events: event=matrix[number]

  //! initialization of visual closure

  void build_up_matrix();
//  void build_up_matrix_causally();

  void cleanup_attributes()
  {
  }

public:
  /**
 * \brief Creates BMscIntervalSetMatrix for given BMsc.
 * The interval set in the place at resulting matrix corresponding to pair of events e and e' is intersection of following:
 *  [0] if e == e'
 *  [0,infty) or (-infty,0] if events e,e' are ordered via visual (when cuasal is false) or causal order (when causal is true)
 *  interval sets from time relations from the BMsc corresponding to events e,e'.
 *
 */

  BMscIntervalSetMatrix(BMscPtr bmsc, bool causally=false, ChannelMapperPtr mapper = boost::shared_ptr<ChannelMapper>()):
    m_bmsc_original(bmsc),m_builded(false),m_casually(causally),m_mapper(mapper)
  {
    EventTopologyHandler event_topology(m_bmsc_original);
    m_events = event_topology.get_topology();

    bool (EventTopologyHandler::*comp) (Event* a,Event * b) = NULL;
    if(causally)
    {
      event_topology.init_causal(m_mapper); //if doesn't work using race_pos22.mpr for testing may help
      comp = &EventTopologyHandler::causal_is_leq;
    }
    else
    {
      comp = &EventTopologyHandler::visual_is_leq;
    }

    unsigned i; // column and row in matrix

    // for all events in bmsc do:
    EventPVector::const_iterator e_v;
    EventPSet::const_iterator e_m;
    for (e_v=m_events.begin(),i=0;e_v!=m_events.end();e_v++,i++)
    {
      m_event_to_number[*e_v]=i; // assign number to the event
    }

    for (e_v=m_events.begin();e_v!=m_events.end();e_v++)
    {
      EventPSet set = EventFirstSuccessors::get(*e_v); // assigned to successors (0,inf) int
      for (e_m=set.begin();e_m!=set.end();e_m++)
      {
	if((event_topology.*comp)(*e_v,*e_m))
         operator()(*e_v,*e_m)=MscTimeIntervalD(0,D::infinity());
      }
    }

    for (e_v=m_events.begin();e_v!=m_events.end();e_v++)
    {

      TimeRelationEventPtrList::const_iterator it;
      TimeRelationEventPtrList relations = (*e_v)->get_time_relations();

      for (it=relations.begin();it!=relations.end();it++)
      {
        EventP a = (*it)->get_event_a();
        EventP b = (*it)->get_event_b();
        if (event_topology.visual_is_leq(a,b)) // a<=b
        {
	  // add time relation to the map
	  m_rel_to_position[it->get()]=std::make_pair(get_number(a),get_number(b));
	  if(m_position_to_interval.find(std::make_pair(get_number(a),get_number(b)))==m_position_to_interval.end())
	    operator()(a,b) = (*it)->get_interval_set();
          else
	    operator()(a,b) = MscTimeIntervalSetD::set_intersection((*it)->get_interval_set(),operator()(a,b));
        }

        if (event_topology.visual_is_leq(b,a)) // b<=a
        {
	  // add time relation to the map
	  m_rel_to_position[it->get()]=std::make_pair(get_number(b),get_number(a));
	  if(m_position_to_interval.find(std::make_pair(get_number(b),get_number(a)))==m_position_to_interval.end())
	    operator()(b,a) = (*it)->get_interval_set();
          else
	    operator()(b,a) = MscTimeIntervalSetD::set_intersection((*it)->get_interval_set(),operator()(b,a));
        }
      }
    }

  }

  using IntervalSetMatrix::operator();

  //! return position of the event in the matrix
  const unsigned get_number(EventP e)
  {
    return m_event_to_number[e];
  }

  //! if matrix was builded -> return reference from matrix, else return reference from map
  MscTimeIntervalSetD& operator()(EventP a,EventP b)
  {
      return operator()(get_number(a),get_number(b));
  }


  MscTimeIntervalSetD& operator()(const unsigned int a,const unsigned int b)
  {
    if(m_builded)
      return IntervalSetMatrix::operator()(a,b);
    else
      return m_position_to_interval[std::make_pair<unsigned,unsigned>(a,b)];
  }

  BMscIntervalSetMatrix& operator=(const BMscIntervalSetMatrix& matrix)
  {
    if(m_bmsc_original!=matrix.m_bmsc_original)
      throw std::invalid_argument("BMscs dont match!");
    IntervalSetMatrix::operator=(matrix);
    return *this;
  }

  BMscIntervalSetMatrix& operator=(const IntervalSetMatrix& matrix)
  {
    IntervalSetMatrix::operator=(matrix);
    return *this;
  }

  BMscIntervalSetMatrix& operator=(const IntervalMatrix& matrix)
  {
    IntervalSetMatrix::operator=(matrix);
    return *this;
  }
  //! return all events in matrix
  const EventPVector get_events() const
  {
    return m_events;
  }

  const unsigned get_size() const
  {
    return m_events.size();
  }

  //! uses stored values of time relations in matrix(which could be modified) and returns modified bmsc
  BMscPtr get_modified_bmsc();

  //! Takes (modified) matrix and changes bmsc. If no matrix is presented, it fills the BMSC according to this matrix
  BMscPtr get_modified_bmsc(IntervalMatrix* matrix);  
  
  //! return original bmsc (assigned to the matrix)
  BMscPtr get_original_bmsc() const
  {
    return m_bmsc_original;
  }

  virtual ~BMscIntervalSetMatrix()
  {
  }

  /**
  *  puts IntervalSet in to the matrix to the position of events x,y, if it was not set before
  *  otherwise it puts there the intersection with previously set value
  */
  void fill_intersection(EventP x,EventP y,const MscTimeIntervalSetD& c)
  {
    unsigned x1 = get_number(x);
    unsigned x2 = get_number(y);
    std::map<unsigned,unsigned>::iterator it = m_filled_positions.find(x1);
    if(it == m_filled_positions.end() || it->second != x2)
    {
      m_filled_positions.insert(std::make_pair<unsigned,unsigned>(x1,x2));
      fill(x1,x2,c);
    }
    else
      fill(x1,x2,MscTimeIntervalSetD::set_intersection(c,operator()(x1,x2)));
  }

  //! puts IntervalSet in to the matrix to the position of events x,y
  void fill(EventP x,EventP y,const MscTimeIntervalSetD& c)
  {
    fill(get_number(x),get_number(y),c);
  }

  void fill(const unsigned x,const unsigned y,const MscTimeIntervalSetD& c)
  {
    operator()(x,y)=c;
    operator()(y,x)= MscTimeIntervalSetD::interval_inverse(c);
  }

  void build_up()
  {
    if(m_builded)
      throw std::runtime_error("Matrix was already builded");
    m_size = m_events.size();
    build_up_matrix();
  }

  //! goes through the map of time relations tied to the cells and checks whether the interval set isn't empty, returns list of those relations
  const std::list<TimeRelation*> get_rel_empty_set()
  {
    std::list<TimeRelation*> list;
    std::map<TimeRelation*, std::pair<unsigned,unsigned> >::iterator it;
    for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
    {
      if(operator()(it->second.first,it->second.second).is_empty())
	list.push_back(it->first);
    }
    return list;
  }

  const std::list<std::pair<unsigned,unsigned> > get_empty_set()
  {
    std::list<std::pair<unsigned,unsigned> > list;
    for(unsigned i=0;i<size();i++)
      for(unsigned j=0;j<size();j++)
      {
        if(operator()(i,j).is_empty())
          list.push_back(std::make_pair<unsigned,unsigned>(i,j));
      }
    return list;
  }

  //! ties time relation to the cell
  void tied_rel_to_cell(TimeRelationPtr relation, const unsigned i, const unsigned j)
  {
    m_rel_to_position[relation.get()]=std::make_pair<unsigned,unsigned>(i,j);
  }

  void tied_rel_to_cell(TimeRelationPtr relation, EventP i, EventP j)
  {
    tied_rel_to_cell(relation,get_number(i),get_number(j));
  }

  std::map<TimeRelation*, std::pair<unsigned,unsigned> > get_tied_time_relations()
  {
    return m_rel_to_position;
  }


  //! add event to the matrix (adding new row and column), returns column/row number in matrix
  const unsigned add_event(EventP event)
  {
    if(m_builded)
      throw std::runtime_error("Matrix was already builded!");
    m_event_to_number[event]=m_events.size();
    m_events.push_back(event);
    return m_event_to_number[event];
  }
}; // end of class BMscIntervalSetMatrix

class SCTIME_EXPORT BMscIntervalSetComponentMatrix: public IntervalSetComponentMatrix
{
private:
  BMscPtr m_bmsc_original;
  bool m_builded;

  bool m_casually; //variable for deciding whether visual or causal order should be used
  ChannelMapperPtr m_mapper;

  std::map<EventP,EventP> m_filled_positions;  

  std::map<std::pair<EventP,EventP>, MscTimeIntervalSetD> m_position_to_interval;
  
  std::map<TimeRelation*, std::pair<EventP,EventP> > m_rel_to_position;
  std::map<EventPtr,unsigned> m_event_to_number; //! the number of event in the matrix

  std::map<EventP,EventP> m_event_to_bmsc_event;
  std::map<EventP,EventP> m_bmsc_event_to_event;
  
  EventPVector m_events;  //! Vector of all events in matrix

  void build_up_matrix();

  void cleanup_attributes()
  {
  }

public:
  /**
 * \brief Creates BMscIntervalSetComponentMatrix for given BMsc.
 * The interval set in the place at resulting component matrix corresponding to pair of events e and e' is intersection of following:
 * [0] if e == e'
 * [0,infty) or (-infty,0] if events e,e' are ordered via visual (when causally is false) or causal order (when causally is true)
 * interval sets from time relations in the bmsc corresponding to events e,e'.
 *
 */
  BMscIntervalSetComponentMatrix(BMscPtr bmsc, bool causally=false, ChannelMapperPtr mapper = boost::shared_ptr<ChannelMapper>()):
    m_bmsc_original(bmsc),m_builded(false),m_casually(causally),m_mapper(mapper)
  {
  }

  using IntervalSetComponentMatrix::operator();

  //! return position of the event in the matrix
  const unsigned get_number(EventP e)
  {
    if(!m_builded) throw std::runtime_error("Component matrix is not builded yet.");
    return m_event_to_number[e];
  }

  //! return reference to interval set for events a,b (works only if the events x and y are in the same component or the matrix is not builded yet)
  MscTimeIntervalSetD& operator()(EventP a,EventP b)
  {
    if(m_builded)
      return IntervalSetComponentMatrix::operator()(get_number(a),get_number(b));
    else
      return m_position_to_interval[std::make_pair<EventP,EventP>(a,b)];
  }

  //! return reference to interval set on position a,b (works only if the events x and y are in the same component or the matrix is not builded yet)
  MscTimeIntervalSetD& operator()(const unsigned int a,const unsigned int b)
  {
    if(m_builded)
      return IntervalSetComponentMatrix::operator()(a,b);
    else
      throw std::runtime_error("Component matrix is not builded yet.");
  }
  
  //! returns correct interval set on positions x and y, works only if the component matrix is builded
  const MscTimeIntervalSetD get_element(const unsigned int x,const unsigned int y) const
  {
    if(m_builded)
      return IntervalSetComponentMatrix::get_element(x,y);
    else
      throw std::runtime_error("Component matrix is not builded yet.");
  }

  //! returns correct interval set for events a and b, works only if the component matrix is builded
  const MscTimeIntervalSetD get_element(EventP a,EventP b)
  {
    if(m_builded)
      return IntervalSetComponentMatrix::get_element(get_number(a),get_number(b));
    else
      throw std::runtime_error("Component matrix is not builded yet.");
  }



//   BMscIntervalSetMatrix& operator=(const BMscIntervalSetMatrix& matrix)
//   {
//     if(m_bmsc_original!=matrix.m_bmsc_original)
//       throw std::invalid_argument("BMscs do not match!");
//     IntervalSetMatrix::operator=(matrix);
//     return *this;
//   }
//
//   BMscIntervalSetMatrix& operator=(const IntervalSetMatrix& matrix)
//   {
//     IntervalSetMatrix::operator=(matrix);
//     return *this;
//   }
// 
//   BMscIntervalSetMatrix& operator=(const IntervalMatrix& matrix)
//   {
//     IntervalSetMatrix::operator=(matrix);
//     return *this;
//   }

  //! return all events in matrix
  const EventPVector get_events() const
  {
    if(!m_builded)
      throw std::runtime_error("Component matrix was not builded yet.");    
    return m_events;
  }

  //! Takes matrix and component number and changes bmsc. If no matrix is presented, it fills the BMSC according to this (component) matrix
  BMscPtr get_modified_bmsc(unsigned component,IntervalMatrix matrix);
 
  //! uses stored values of time relations in matrix(which could be modified) and returns modified bmsc
  BMscPtr get_modified_bmsc();

  //! uses stored values of time relations in matrix and returns modified bmsc
  BMscPtr get_modified_bmsc(IntervalSetComponentMatrix matrix); 
  
  //! return original bmsc (assigned to the matrix)
  BMscPtr get_original_bmsc() const
  {
    return m_bmsc_original;
  }

  virtual ~BMscIntervalSetComponentMatrix()
  {
  }

  /**
  *  puts IntervalSet in to the component matrix to the position of events x,y, if it was not set before
  *  otherwise it puts there the intersection with previously set value
  *  works only if the events x and y are in the same component or the matrix is not builded yet
  */
  void fill_intersection(EventP x,EventP y,const MscTimeIntervalSetD& c)
  {
    std::map<EventP,EventP>::iterator it = m_filled_positions.find(x);
    if(it == m_filled_positions.end() || it->second != y)
    {
      m_filled_positions.insert(std::make_pair<EventP,EventP>(x,y));
      fill(x,y,c);
    }
    else
      fill(x,y,MscTimeIntervalSetD::set_intersection(c,operator()(x,y)));
  }

  //! puts interval set into the matrix to the position of events x,y and also it's inverse to inverse position (works only if the events x and y are in the same component or the matrix is not builded yet)
  void fill(EventP x,EventP y,const MscTimeIntervalSetD& c)
  {
    operator()(x,y)=c;
    if(m_builded) //small optimisation- we do not need to store inverse in map m_position_to_interval
      operator()(y,x)= MscTimeIntervalSetD::interval_inverse(c);
  }

//   void fill(const unsigned x,const unsigned y,const MscTimeIntervalSetD& c)
//   {
//     operator()(x,y)=c;
//     operator()(y,x)= MscTimeIntervalSetD::interval_inverse(c);
//   }

  //! builds up the component matrix according to bmsc and added interval sets
  void build_up()
  {
    if(m_builded)
      throw std::runtime_error("Component matrix was already builded");
    build_up_matrix();
  }

  //! goes through the map of time relations tied to the cells and checks whether the interval set isn't empty, returns list of those relations
  const std::list<TimeRelation*> get_rel_empty_set()
  {

    std::list<TimeRelation*> list;
    std::map<TimeRelation*, std::pair<EventP,EventP> >::iterator it;
    for(it=m_rel_to_position.begin();it!=m_rel_to_position.end();it++)
    {
      if(operator()(it->second.first,it->second.second).is_empty())
	list.push_back(it->first);
    }
    return list;
  }

  //! ties time relation to the cell
  void tie_rel_to_cell(TimeRelationPtr relation, EventP i, EventP j)
  {
    m_rel_to_position[relation.get()]=std::make_pair<EventP,EventP>(i,j);
  }

  std::map<TimeRelation*, std::pair<EventP,EventP> > get_tied_time_relations()
  {
    return m_rel_to_position;
  }

//   const std::list<std::pair<unsigned,unsigned> > get_empty_set()
//   {
//     std::list<std::pair<unsigned,unsigned> > list;
//     for(unsigned i=0;i<get_size();i++)
//       for(unsigned j=0;j<get_size();j++)
//       {
//         if(operator()(i,j).is_empty())
//           list.push_back(std::make_pair<unsigned,unsigned>(i,j));
//       }
//     return list;
//   }

//  //! ties time relation to the cell
//   void tie_rel_to_cell(TimeRelationPtr relation, const unsigned i, const unsigned j)
//   {
//     m_rel_to_position[relation.get()]=std::make_pair<unsigned,unsigned>(i,j);
//   }
// 

  /**
  * \brief Add event to the component matrix- adds added_event to same component as original_event
  *
  * adds maximaly one event to the event originally in BMSC (other events than the last one will not be added)
  * no event is added if original_event is not in m_bmsc_original
  */
  const void add_event_to_component(EventP original_event,EventP added_event)
  {
    if(m_builded)
      throw std::runtime_error("Matrix was already builded!");
    m_bmsc_event_to_event[original_event] = added_event;
    m_event_to_bmsc_event[added_event] = original_event;
  }
}; // end of class BMscIntervalSetComponentMatrix


// pick up all time relation in HMsc
class HMscAllTimeRelations:public std::set<TimeRelationRefNodePtr>, public WhiteRefNodeFoundListener
{
private:

  void pick_up_time_relations(TimeRelationRefNodePtrSet set)
  {
    TimeRelationRefNodePtrSet::iterator it;

    for (it=set.begin();it!=set.end();it++)
      insert(*it);
  }

protected:

public:

  HMscAllTimeRelations(HMscPtr hmsc)
  {
    DFSHMscFlatTraverser hmsc_traverser("all_time_relations");
    hmsc_traverser.add_white_node_found_listener(this);
    hmsc_traverser.traverse(hmsc);
  }


  virtual void on_white_node_found(ReferenceNode* node)
  {
    TimeRelationRefNodePtrSet time_relations;

    time_relations=node->get_time_relations_top();
    pick_up_time_relations(time_relations);
    time_relations=node->get_time_relations_bottom();
    pick_up_time_relations(time_relations);
  }

};

class BMscAllTimeRelationPtrSet:public std::set<TimeRelationEventPtr>
{
private:
public:
  BMscAllTimeRelationPtrSet(BMscPtr bmsc)
  {
    AllReachableEventPVector all_events(bmsc);
    std::vector<EventP>::iterator e;
    for(e=all_events.begin();e!=all_events.end();e++)
    {
      TimeRelationEventPtrList time_relations;
      std::list<TimeRelationEventPtr>::iterator rel;
      time_relations=(*e)->get_time_relations();
      for(rel=time_relations.begin();rel!=time_relations.end();rel++)
      {
        insert(*rel);
      }
    }
  }
};

class IntervalMatrixFunc
{
public:
  //! check whether two matrices are equal
  static
  bool is_equal(
    IntervalSetMatrix lhs,
    IntervalSetMatrix rhs)
  {

    if (&lhs == &rhs)
      return true;

    if (lhs.size() != rhs.size())
      return false;

    BoostIntervalSetMatrix::iterator1 l(lhs.begin1());

    BoostIntervalSetMatrix::iterator1 r(rhs.begin1());

    while (l != lhs.end1())
    {
      if (*l != *r)
        return false;

      ++l;
      ++r;
    }

    return true;
  }

  static
  void print_out(IntervalSetMatrix matrix)
  {
    std::cout << "matrix: " << matrix.size() << std::endl;

    for (unsigned i=0;i<matrix.size();i++)
    {
      for (unsigned j=0;j<matrix.size();j++)
      {
        std::cout << " {" << matrix(i,j) << "}";
      }

      std::cout << std::endl;
    }

  }
}; //end of IntervalMatrix


typedef std::pair<std::list<HMscNodePtr>,bool> HMscPath;
typedef std::pair<std::list<HMscPath>,bool> HMscAllPaths;

class ConstraintAllPaths
{
private:
  HMscNodePtrSet m_nodes_set;
  bool m_contains_cycle_globally;
  HMscPtr m_hmsc;
  TimeRelationRefNodePtr m_relation;
  std::list<HMscPath> m_paths;
  std::string m_number;
public:
  ConstraintAllPaths(
    HMscPtr hmsc,
    TimeRelationRefNodePtr relation,
    const std::string& number="ConstraintAllPaths"
  )
      :m_hmsc(hmsc),m_relation(relation),m_number(number)
  {

  }

  ~ConstraintAllPaths()
  {
    cleanup_attributes();
  }

  void set_number(HMscNodePtr e,int number)
  {
    return e->set_attribute<int>(m_number,number);
  }

  const int get_number(HMscNodePtr e) const
  {
    return e->get_attribute<int>(m_number,0);
  }

  void cleanup_attributes()
  {
    HMscNodePtrSet::iterator node;

    for (node=m_nodes_set.begin();node!=m_nodes_set.end();node++)
      (*node)->remove_attribute<int>(m_number);
  }


  static HMscAllPaths get(
    HMscPtr hmsc,
    TimeRelationRefNodePtr relation
  )
  {
    ConstraintAllPaths allpaths(hmsc,relation);
    return allpaths.get_set_of_paths();
  }

  HMscAllPaths get_set_of_paths();

  void all_paths(HMscNodePtr, std::list<HMscNodePtr>, bool);

};

class TimeRelationsFunc
{
  private:

  public:
    /*! finds all time relations between event "a" and "b"
     * @Warning dont mind about direction of timerelation
     * @Complexity: linear to the number of relations on event that hols
     * less time relations (implementation of std::list.size() constant..)
     * go through the list of time relations of event a and finds time relatins
     * to "b" or from "b" to "a"
     */
    static const TimeRelationEventPtrList get(const EventP a,const EventP b)
    {
      TimeRelationEventPtrList found;
      TimeRelationEventPtrList relations;

      a->get_time_relations().size() < b->get_time_relations().size() ? relations = a->get_time_relations() : relations = b->get_time_relations();
      TimeRelationEventPtrList::const_iterator it;

      for(it=relations.begin();it!=relations.end();it++)
      {
        TimeRelationEventPtr rel = *it;
        if(rel->get_event_a()==a && rel->get_event_b()==b)
          found.push_back(rel);
        else if(rel->get_event_a()==b && rel->get_event_b()==a)
          found.push_back(rel);
      }

      return found;
    }
};
#endif // _TIME_PSEUDOCODE_H_

// $Id: time_pseudocode.h 1077 2011-04-06 16:22:35Z lkorenciak $
