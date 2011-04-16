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
 * Copyright (c) 2008 Jindra Babica <babica@mail.muni.cz>
 *
 * $Id: race_checker.h 1007 2010-12-09 15:39:20Z vacek $
 */

#ifndef _HMSC_RACE_CHECKER_H
#define _HMSC_RACE_CHECKER_H

#include <exception>
#include <stack>

#include "data/msc.h"
#include "data/dfs_bmsc_graph_traverser.h"
#include "check/pseudocode/causal_closure_initiator.h"
#include "check/pseudocode/utils.h"
#include "data/node_finder.h"
#include "check/race/footprint.h"
#include "check/race/export.h"
#include "check/pseudocode/msc_duplicators.h"

class BMscRaceCheckingListener;
class RaceInBMscException;
class RaceInHMscException;

typedef std::stack<ReferenceNode*> ReferenceNodePStack;
typedef std::list<BMsc*> BMscPList;

/**
 * Stands for mechanism for computation of MinP
 *
 * MinP is used in checking Footprint and BMsc to be race free (Algortihm 1 in 
 * the article).
 */
class MinimalEventsInitiator:public WhiteRefNodeFoundListener
{
protected:
  
  VisualClosureInitiator* m_vis_initiator;
  CausalClosureInitiator* m_caus_initiator;
  InstanceIdMarker* m_instance_marker;
  std::string m_events_attribute;
  BMscPList m_modified_bmscs;
  
public:
  
  MinimalEventsInitiator(
    VisualClosureInitiator* vis_initiator=NULL,
    CausalClosureInitiator* caus_initiator=NULL,
    InstanceIdMarker* instance_marker=NULL,
    const std::string& events_attribute = "minimal_events")
  {
    m_vis_initiator = vis_initiator;
    m_caus_initiator = caus_initiator;
    m_instance_marker = instance_marker;
    m_events_attribute = events_attribute;
  }
  
  virtual ~MinimalEventsInitiator()
  {
    cleanup_attributes();
  }
  
  void on_white_node_found(ReferenceNode* node);
  
  ExtremeEvents& get_events(BMsc* b);
  
  virtual void cleanup_attributes();
  
};

/**
 * Stands for mechanism for computation of MaxP
 *
 * MaxP is used to compute new Footprint f2 of the old Footprint f1 concatenated with
 * some BMsc b. Events in MaxP of b replace Events of f1 on the same instances and
 * becomes members of new Footprint f2.
 *
 * When computing new Footprint it is neccessary to find relation between Events
 * in f1 and Events in MaxP of b in case there is some Event from f1 which belongs
 * to new f2 too. This listener prepares structure for this purpose too.
 *
 * Details are described in Lemma 15 of the article.
 */
class MaximalEventsInitiator:public MinimalEventsInitiator
{
  
protected:
  
  std::string m_events_less_attribute;

  
public:
  
  MaximalEventsInitiator(
    VisualClosureInitiator* vis_initiator=NULL,
    CausalClosureInitiator* caus_initiator=NULL,
    InstanceIdMarker* instance_marker=NULL,
    const std::string& events_greater_attribute = "maximal_events_greater",
    const std::string& events_less_attribute = "maximal_events_less"):
  MinimalEventsInitiator(
    vis_initiator,caus_initiator,instance_marker,events_greater_attribute)
  {
    m_events_less_attribute = events_less_attribute;
  }
  
  ExtremeEvents& get_events_greater(BMsc* b)
  {
    return get_events(b);
  }
  
  ExtremeEvents& get_events_less(BMsc* b);
  
  void on_white_node_found(ReferenceNode* node);
  
  void cleanup_attributes();
  
};

class FootprintTraverser:public WhiteRefNodeFoundListener,public NodeFinder, public GrayRefNodeFoundListener
{
private:
  
  std::set<FootprintPtr,FootprintPtrComparator> todo;
  std::set<FootprintPtr,FootprintPtrComparator> done;
  std::list<RaceInHMscException> m_counterexamples;

  ChannelMapper* m_mapper;
  MinimalEventsInitiator* m_min;
  MaximalEventsInitiator* m_max;

  FootprintPtr m_footprint;
  
  FootprintPtr extract_todo();

  //! The node with the "first node" attribute set. It is necessary for cleanup.
  HMscNode *m_first_node;
  
  /**
   * To understand this method see Algorithm 1 in the article
   */
  void check_race(BMsc* b, FootprintPtr& f);

public:
  
  FootprintTraverser(
    HMsc* hmsc,
    ChannelMapper* mapper,
    size_t instances_count,
    MinimalEventsInitiator* min,
    MaximalEventsInitiator* max);
    
  void on_white_node_found(ReferenceNode* node);

  void on_gray_node_found(ReferenceNode *n)
  {
    if(n->is_attribute_set("rc_fn"))
    {
      n->remove_attribute<bool>("rc_fn");
      on_white_node_found(n);
    }
  }
  
  void traverse();

  const std::list<RaceInHMscException>& get_counterexamples()
  {
    return m_counterexamples;
  }

  void traverse(HMscNode* node);

  void cleanup()
  {
    m_first_node->remove_attribute<bool>("rc_fn");
  }
  //void traverse(PredecessorNode* node);

};

class AddInstance: public WhiteRefNodeFoundListener
{
public:
  void on_white_node_found(ReferenceNode* n)
  {
    BMscPtr bmsc = n->get_bmsc();
    if(!bmsc.get())
      return;
    InstancePtr i(new Instance(L"uqgors"));
    bmsc->add_instance(i);
    StrictOrderAreaPtr a(new StrictOrderArea());
    i->add_area(a);
    EventPtr e = a->add_event();
    IncompleteMessagePtr m(new IncompleteMessage(LOST));
    m->glue_event(e);
  }
};

class PostProcess: public WhiteRefNodeFoundListener
{
private:
  unsigned m_unique_id;
public:
   PostProcess(void)
    :m_unique_id(1)
  {}
  void on_white_node_found(ReferenceNode* n)
  {
    std::wstringstream ss;

    BMscPtr bmsc = n->get_bmsc();
    if(!bmsc.get())
      return;
    bmsc->remove_instances(L"uqgors");
    ss << "(" <<m_unique_id++ << ")" << bmsc->get_label();
    bmsc->set_label(ss.str());
  }
  void reset(void)
  {
    m_unique_id = 1;
  }
};

class SCRACE_EXPORT RaceChecker: public Checker, public BMscChecker, public HMscChecker
{

protected:
  
  VisualClosureInitiator m_visual_initiator;
  
  CausalClosureInitiator m_causal_initiator;
  
  InstanceIdMarker m_instance_marker;
  
  MinimalEventsInitiator m_min_events_initiator;
  
  MaximalEventsInitiator m_max_events_initiator;

  BMscGraphDuplicator m_graph_duplicator;
  
  /**
   * Checks HMsc to have only race free BMscs
   */
  std::list<HMscPtr> check_bmscs(HMscPtr hmsc, ChannelMapperPtr mapper);
  
  /**
   * It was found that e1<e2 but not e1<<e2
   */
  BMscPtr create_counter_example(Event* e1, Event* e2);
  
  /**
   *
   */
  HMscPtr create_counter_example(RaceInHMscException e);
  
  /**
   * Precomputes neccessary things for race checking
   */
  void prepare_hmsc(HMscPtr hmsc,ChannelMapperPtr mapper);
  
  /**
   * Coorrespond to Algorithm 1 of referenced article
   */
  void check_concatenation(const Footprint& f, BMscPtr bmsc, ChannelMapperPtr mapper);
  
  /**
   * Creates counter example in case there is any race violating BMsc
   */
  std::list<HMscPtr> create_counter_example(const MscElementPListList& path, std::list<BMscPtr> examples);
  
  std::list<HMscPtr> check_hmsc(HMscPtr hmsc, ChannelMapperPtr mapper);
  
public:
  
  RaceChecker();

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Race Free"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"race/race.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  std::list<BMscPtr> check_bmsc(BMscPtr bmsc, ChannelMapperPtr mapper);
  
  std::list<BMscPtr> check(BMscPtr bmsc, ChannelMapperPtr mapper);
  
  std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr mapper);
  
  void cleanup_attributes();

  bool is_supported(ChannelMapperPtr chm)
  {
    // FIXME: this is a dummy function only; Jindra needs to verify
    return true;
  }

  bool check_events(Event* e1, Event* e2);
};

class RaceInBMscException:public std::exception
{
private:
  
  /**
   * Counter example containing race
   */
  BMscPtr m_example;
  
public:
  
  RaceInBMscException(BMscPtr example)
  {
    m_example = example;
  }
  
  ~RaceInBMscException() throw ()
  {
    
  }
  
  virtual const char* what() const throw()
  {
    return "There was found race in BMsc";
  }
  
  BMscPtr get_example()
  {
    return m_example;
  }
};

/**
 * Used to check all BMscs in HMsc to satisfy race free property.
 */
class BMscRaceCheckingListener:public WhiteRefNodeFoundListener
{
  
  RaceChecker* m_checker;
  
  ChannelMapperPtr m_mapper;
  
public:
  
  BMscRaceCheckingListener(RaceChecker* checker, ChannelMapperPtr mapper);
  
  void on_white_node_found(ReferenceNode* node);
  
};

class BMscResultsCatcher:public WhiteRefNodeFoundListener
{
  
public:
 
  void on_white_node_found(ReferenceNode* node)
  {
    if(node->is_attribute_set("rc_bmsc"))
    {
      std::list<BMscPtr> result, dummy;
      result = node->get_attribute("rc_bmsc", dummy);
      node->remove_attribute<std::list<BMscPtr> >("rc_bmsc");
      throw result;
    }
  }
  
};

class RaceInHMscException:public std::exception
{
private:
  
  FootprintPtr m_footprint;
  Event* m_first;
  Event* m_second;
  MscElementPList m_path_to_second;
  
public:
  
  RaceInHMscException(FootprintPtr& footprint,Event* first, Event* second, 
    const MscElementPList& path_to_second);
  
  ~RaceInHMscException() throw ()
  {
    
  }

  FootprintPtr& get_footprint();
  Event* get_first();
  Event* get_second();
  MscElementPList& get_path_to_second();
};

#endif /* _HMSC_RACE_CHECKER_H */

// $Id: race_checker.h 1007 2010-12-09 15:39:20Z vacek $
