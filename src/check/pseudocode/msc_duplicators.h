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
 * $Id: msc_duplicators.h 1007 2010-12-09 15:39:20Z vacek $
 */

#ifndef _MSC_DUPLICATORS_H
#define _MSC_DUPLICATORS_H

#include "data/msc.h"
#include "data/dfs_area_traverser.h"
#include "data/dfs_bmsc_graph_traverser.h"
#include "data/dfs_hmsc_flat_traverser.h"
#include "check/pseudocode/export.h"
#include "check/time/time_pseudocode.h"

typedef std::list<Event*> EventPList;
typedef std::list<ReferenceNode*> ReferenceNodePList;
typedef std::list<HMsc*> HMscPList;
typedef std::list<ConnectionNode*> ConnectionNodePList;

class SCPSEUDOCODE_EXPORT Duplicator
{
protected:

  MscElementPList m_modified_elements;

  Duplicator();

public:

  virtual ~Duplicator();

  MscElement*& get_copy(MscElement* e);

  SuccessorNode* get_copy(SuccessorNode* s)
  {
    HMscNode* n = dynamic_cast<HMscNode*>(s);
    return dynamic_cast<SuccessorNode*>(get_copy(n));
  }

  PredecessorNode* get_copy(PredecessorNode* p)
  {
    HMscNode* n = dynamic_cast<HMscNode*>(p);
    return dynamic_cast<PredecessorNode*>(get_copy(n));
  }

  ReferenceNode* get_copy(ReferenceNode* p)
  {
    MscElement* n = static_cast<MscElement*>(p);
    return static_cast<ReferenceNode*>(get_copy(n));
  }

  HMsc* get_copy(HMsc* p)
  {
    MscElement* n = static_cast<MscElement*>(p);
    return static_cast<HMsc*>(get_copy(n));
  }

  BMsc* get_copy(BMsc* p)
  {
    MscElement* n = static_cast<MscElement*>(p);
    return static_cast<BMsc*>(get_copy(n));
  }

  Msc* get_copy(Msc* p)
  {
    MscElement* n = static_cast<MscElement*>(p);
    return static_cast<Msc*>(get_copy(n));
  }
  
  MscElementPList get_m_modified_elements(){
    return m_modified_elements;
  }

  void set_m_modified_elements(MscElementPList elements){
    m_modified_elements= elements;
  }

  void set_copy(MscElement* original, MscElement* copy);

  void cleanup_attributes();
};

class BMscDuplicator;
class HMscFlatDuplicator;
class HMscDuplicator;


class EventsCreatorListener:
  public WhiteEventFoundListener,public GrayEventFoundListener,
  public BlackEventFoundListener
{
protected:

  BMscDuplicator* m_duplicator;

  Instance* m_last_instance;

  Instance* m_last_new_instance;

  EventArea* m_last_area;

  EventArea* m_last_new_area;

  BMsc* m_bmsc;

  DFSAreaTraverser* m_traverser;

  void create_successor(Event* e);

  CoregionEvent* get_preceding_event();

public:

  EventsCreatorListener(BMscDuplicator* duplicator, DFSAreaTraverser* traverser, BMsc* bmsc);

  void on_white_event_found(Event* e);
  void on_gray_event_found(Event* e);
  void on_black_event_found(Event* e);

};

class MessagesCreatorListener:public WhiteEventFoundListener
{
protected:

  BMscDuplicator* m_duplicator;

public:

  MessagesCreatorListener(BMscDuplicator* duplicator);

  void on_white_event_found(Event* e);

};


class TimeRelationCreatorListener:public WhiteEventFoundListener
{
protected:

  BMscDuplicator* m_duplicator;

public:

  TimeRelationCreatorListener(BMscDuplicator* duplicator);

  void on_white_event_found(Event* e);

};

/**
 * \brief Duplicates BMsc
 *
 * Duplicated BMsc's elements have set attribute original to the original
 * BMsc's elements.
 */
class SCPSEUDOCODE_EXPORT BMscDuplicator:public Duplicator
{

public:

  BMscDuplicator();

  BMscPtr duplicate_bmsc(BMscPtr& bmsc);

  static BMscPtr duplicate(BMscPtr& bmsc);

  ~BMscDuplicator();

  Event* get_event_copy(Event* e);
};

/**
 * \brief HMscDuplicator creates exact copy of HMsc
 */

class SCPSEUDOCODE_EXPORT HMscDuplicator:
    public Duplicator
{
private:
  HMscPtr m_hmsc_root;

  std::set<HMsc*> m_hmscs; // set of created hmsc
  std::set<BMsc*> m_bmscs;  // set of created bmsc

  std::stack<BMsc*> m_to_create_bm;
  std::stack<HMsc*> m_to_create_hm;
  std::stack<std::pair<ReferenceNode*,Msc*> > m_to_set_up;

  void inner_msc(ReferenceNode*);
  void inner_msc_found(HMscPtr);
  void inner_msc_found(BMscPtr);

  HMscPtr create(HMsc* hmsc);
  StartNode* create(StartNode* n);

  HMscNode* create(ReferenceNode* n);
  HMscNode* create(EndNode* n);
  HMscNode* create(ConditionNode* n);
  HMscNode* create(ConnectionNode* n);

  HMscNodePtr process_node(HMscNode* n);
  void create_node_relation(HMscNode* n);
  void create_time_relation(HMscPtr);

public:
  HMscDuplicator()
  { }

  HMscPtr duplicate(HMscPtr hmsc);

};

class BMscGraphDuplicator;

/**
 * \brief Duplicates HMsc like it would be BMsc graph.
 *
 * Result of this duplicator is flattened version of HMsc (BMsc graph)
 * without unreachable HMscNodes and nodes which
 * man isn't able to get to EndNode from.
 *
 * All ReferenceNodes reference only BMsc, moreover each BMsc is
 * referenced only one time -- each BMsc is duplicated too. Kept elements
 * references to its original element via attribute m_original.
 *
 * Original ReferenceNodes which references HMsc are transformed into
 * ConnectionNodes referencing the ReferenceNode by m_original. StartNodes
 * which don't occure in HMsc to be duplicated are removed. EndNodes
 * of the same kind are transformed into ConnectionNodes referencing original
 * EndNodes.
 */
class SCPSEUDOCODE_EXPORT GraphCreatorListener:public WhiteNodeFoundListener,public GrayNodeFoundListener,
  public BlackNodeFoundListener,public NodeFinishedListener
{
protected:

  BMscGraphDuplicator* m_duplicator;

  DFSBMscGraphTraverser* m_traverser;

  HMscPList m_modified_hmscs;

  MscElementPList m_new_nodes;

  HMsc* m_hmsc;

  bool is_root_element(HMscNode* n);

  void process_new_node(HMscNode* old_node, HMscNodePtr& new_node);

  void process_nonwhite_node(HMscNode* n);

  PredecessorNode* get_predecessor();

  ReferenceNode*& get_referencing_node(HMsc* hmsc);

  ConnectionNodePList& get_end_list(ReferenceNode* reference);

  void add_to_end_list(ConnectionNode* new_end);

  void add_new_successor(HMscNode* new_successor);

  HMscNode* get_node_copy(HMscNode* n);

  NodeRelation* get_previous_relation();

public:

  GraphCreatorListener(BMscGraphDuplicator* duplicator, DFSBMscGraphTraverser* traverser, HMsc* hmsc);

  ~GraphCreatorListener();

  void on_white_node_found(HMscNode* n);
  void on_white_node_found(ReferenceNode* n);
  void on_white_node_found(StartNode* n);
  void on_white_node_found(EndNode* n);
  void on_white_node_found(ConditionNode* n);
  void on_white_node_found(ConnectionNode* n);

  void on_gray_node_found(HMscNode* n);

  void on_black_node_found(HMscNode* n);

  void on_node_finished(HMscNode* n);
  void on_node_finished(ReferenceNode* n);
  void on_node_finished(StartNode* n);
  void on_node_finished(EndNode* n);
  void on_node_finished(ConditionNode* n);
  void on_node_finished(ConnectionNode* n);
};

//////////////////////////////////////////

/**
 * \brief Creates time constraints on reference nodes which do not refer to HMsc
 */
class SCPSEUDOCODE_EXPORT NormalConstraintsCreatorListener:public WhiteNodeFoundListener
{
  private:
    std::map<TimeRelationRefNodePtr,ReferenceNode* > m_relations_to_close;
    BMscGraphDuplicator* m_duplicator;
  public:
    NormalConstraintsCreatorListener(BMscGraphDuplicator* duplicator);
    
    ~NormalConstraintsCreatorListener();  
  public:
    void on_white_node_found(HMscNode* n);  
};

/**
 * \brief throws exception if reference node which refers to HMsc was found
 */
//\brief Creates time constraints on reference nodes which refer to HMsc
class SCPSEUDOCODE_EXPORT HighLevelConstraintsCreatorListener:public WhiteNodeFoundListener
{
  private:
    BMscGraphDuplicator* m_duplicator;
  public:
    HighLevelConstraintsCreatorListener(BMscGraphDuplicator* duplicator);
    
    ~HighLevelConstraintsCreatorListener();
    
    void on_white_node_found(HMscNode* n);
};


//////////////////////////////////////////

/**
 * \brief Duplicates HMsc like it would be BMsc graph.
 *
 * Result of this duplicator is flattened version of HMsc (BMsc graph)
 * without unreachable HMscNodes and nodes which
 * man isn't able to get to EndNode from.
 *
 * All ReferenceNodes reference only BMsc, moreover each BMsc is
 * referenced only one time -- each BMsc is duplicated too. Kept elements
 * references to its original element via attribute m_original.
 *
 * Original ReferenceNodes which references HMsc are transformed into
 * ConnectionNodes referencing the ReferenceNode by m_original. StartNodes
 * which don't occur in HMsc to be duplicated are removed. EndNodes
 * of the same kind are transformed into ConnectionNodes referencing original
 * EndNodes.
 */
class SCPSEUDOCODE_EXPORT BMscGraphDuplicator:public Duplicator
{

public:

  HMscPtr duplicate_hmsc(HMscPtr& hmsc);

  static HMscPtr duplicate(HMscPtr& hmsc);

  ~BMscGraphDuplicator();

};

/**
 * \brief Duplicates path in HMsc.
 *
 * The path is supposed to be generated by traversers -- similar structure.
 * The duplicator does not duplicate the inner BMscs. 
 */
class SCPSEUDOCODE_EXPORT HMscPathDuplicator:public Duplicator
{
private:
  void process_time_relations(ReferenceNode* ref_node);
  void process_time_relations(ReferenceNode*,TimeRelationRefNodePtrSet,bool);
public:

  ~HMscPathDuplicator();

  HMscPtr duplicate_path(const MscElementPListList& path);
};

/**
 * \brief Duplicates path in BMsc graph - not hierarchical path
 *
 * The duplicator duplicates the inner BMscs. 
 */
class SCPSEUDOCODE_EXPORT HMscFlatPathDuplicator:public Duplicator
{
  TimeRelationRefNodePtrSet m_open_relations;
  void process_time_relations(ReferenceNode*,bool);
  void process_time_relations(ReferenceNode*,TimeRelationRefNodePtrSet,bool,bool);
  void set_copy(MscElement* original, MscElement* copy, bool persist_original);

public:

  ~HMscFlatPathDuplicator();

  HMscPtr duplicate_path(const MscElementPList& path, bool persist_original=true);
};


/**
 * \brief Duplicates path in BMsc graph - not hierarchical path and creates BMsc.
 * The "path" is any path from any node in BMsc graph to any node of BMsc graph. 
 * The values of get_copy are not exactly defined/set for BMscPtr and InstancePtr.
 * If one bmsc occurs in path more than once the elements from (n+1)-th occurence 
 * are duplicated from n-th (already created) occurence of the bmsc.
 * That is why one has to traverse to right copy through references. For example, 
 * if I want to get copy of an element from bmsc from its third occurance in the path,
 * I have to call three times method get_copy()
 * i.e. duplicator.get_copy(duplicator.get_copy(duplicator.get_copy(element)));
 */
class SCPSEUDOCODE_EXPORT HMscFlatPathToBMscDuplicator:public Duplicator 
{


public:

  ~HMscFlatPathToBMscDuplicator();

  BMscPtr duplicate_path(const MscElementPList& path);
  MscElement* get_copy_with_occurence(MscElement* element, int occurence);
};


#endif /* _MSC_DUPLICATORS_H */

// $Id: msc_duplicators.h 1007 2010-12-09 15:39:20Z vacek $
