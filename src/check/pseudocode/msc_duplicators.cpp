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
 * $Id: msc_duplicators.cpp 1036 2011-02-09 11:05:45Z lkorenciak $
 */

#include "check/pseudocode/msc_duplicators.h"
#include "data/dfs_instance_events_traverser.h"
#include <map>

const std::string BMSC_DUPLICATOR_TRAVERSING_ATTR = "BDcolor";
const std::string BMSC_DUPLICATOR_COPY_ATTR = "BDcopy";
const std::string BMSC_GRAPH_DUPLICATOR_COPY_ATTR = "BGDcopy";
const std::string BMSC_GRAPH_DUPLICATOR_REF_ATTR = "BGDreferencing";
const std::string BMSC_GRAPH_DUPLICATOR_ENDLIST_ATTR = "BGDendlist";
const std::string DUPLICATOR_COPY_ATTR = "Dcopy";

Duplicator::Duplicator()
{
}

void Duplicator::cleanup_attributes()
{
  MscElementPList::const_iterator i;
  for(i=m_modified_elements.begin();i!=m_modified_elements.end();i++)
  {
    (*i)->remove_attribute<MscElement*>(DUPLICATOR_COPY_ATTR);
  }
  m_modified_elements.clear();
}

Duplicator::~Duplicator()
{
  cleanup_attributes();
}

MscElement*& Duplicator::get_copy(MscElement* e)
{
  bool just_set;
  MscElement*& copy = e->get_attribute<MscElement*>(DUPLICATOR_COPY_ATTR,NULL,just_set);
  if(just_set)
  {
    m_modified_elements.push_back(e);
  }
  return copy;
}

void Duplicator::set_copy(MscElement* original, MscElement* copy)
{
  MscElement*& c = get_copy(original);
  c = copy;
}

BMscDuplicator::BMscDuplicator()
{
}

BMscPtr BMscDuplicator::duplicate_bmsc(BMscPtr &bmsc)
{
  BMscPtr new_bmsc;
  InstancePtr new_empty;
  InstancePtrList::const_iterator instances;
  if(bmsc.get())
  {
    new_bmsc = new BMsc(bmsc.get());
    DFSAreaTraverser traverser(BMSC_DUPLICATOR_TRAVERSING_ATTR);
    EventsCreatorListener events_creator(this,&traverser,new_bmsc.get());
    traverser.add_white_event_found_listener(&events_creator);
    traverser.add_black_event_found_listener(&events_creator);
    traverser.add_gray_event_found_listener(&events_creator);
    traverser.traverse(bmsc);
    traverser.remove_all_listeners();
    MessagesCreatorListener messages_creator(this);
    TimeRelationCreatorListener time_creator(this);
    traverser.add_white_event_found_listener(&messages_creator);
    traverser.add_white_event_found_listener(&time_creator);
    traverser.traverse(bmsc);

    for(instances = bmsc->get_instances().begin();
        instances != bmsc->get_instances().end();
        instances++)
    {
      if(!instances->get()->has_events())
      {
        new_empty = new Instance(instances->get());
        new_bmsc.get()->add_instance(new_empty);
      }
    }


  }
  return new_bmsc;
}

Event* BMscDuplicator::get_event_copy(Event* e)
{
  MscElement* elem = get_copy(e);
  return dynamic_cast<Event*>(elem);
}

BMscDuplicator::~BMscDuplicator()
{
}

BMscPtr BMscDuplicator::duplicate(BMscPtr& bmsc)
{
  BMscDuplicator duplicator;
  return duplicator.duplicate_bmsc(bmsc);
}
///////////////////////////////////////////////

  //! chooses right type of HMscNode, makes and sets copy, return copy
  HMscNodePtr HMscDuplicator::process_node(HMscNode* n)
  {
    HMscNodePtr copy;
    ReferenceNode* reference_node;
    ConnectionNode* connection_node;
    EndNode* end_node;
    ConditionNode* condition_node;

    if((reference_node=dynamic_cast<ReferenceNode*>(n)))
      copy = create(reference_node);
    else if((connection_node= dynamic_cast<ConnectionNode*>(n)))
      copy = create(connection_node);
    else if((end_node= dynamic_cast<EndNode*>(n)))
      copy = create(end_node);
    else if((condition_node=dynamic_cast<ConditionNode*>(n)))
      copy = create(condition_node);
    else
      throw std::logic_error("Unknown type of HMscNode found. Add this type.");

    set_copy(n,copy.get());
    return copy;
  }

  HMscNode* HMscDuplicator::create(ReferenceNode* n)
  {
    ReferenceNode* copy = new ReferenceNode(n);
    // check inner msc
    inner_msc(n);
    return static_cast<HMscNode*>(copy);
  }


  HMscNode* HMscDuplicator::create(EndNode* n)
  {
    EndNode* copy = new EndNode(n);
    return static_cast<HMscNode*>(copy);
  }

  HMscNode* HMscDuplicator::create(ConditionNode* n)
  {
    ConditionNode* copy = new ConditionNode(n);
    return static_cast<HMscNode*>(copy);
  }

  HMscNode* HMscDuplicator::create(ConnectionNode* n)
  {
    ConnectionNode* copy = new ConnectionNode(n);
    return static_cast<HMscNode*>(copy);
  }
/////////////////
  HMscPtr HMscDuplicator::create(HMsc* hmsc)
  {
    HMscPtr copy = new HMsc(hmsc);
    set_copy(hmsc,copy.get());
    StartNode* start = create(hmsc->get_start().get());
    copy->set_start(start);
    return copy;
  }

  StartNode* HMscDuplicator::create(StartNode* n)
  {
    StartNode* copy = new StartNode(n);
    set_copy(n,copy);
    return copy;
  }
///////////////////
  void HMscDuplicator::create_node_relation(HMscNode* n)
  {
    PredecessorNode* pre;
    pre = dynamic_cast<PredecessorNode*>(n);
    if(!pre)
      return;

    PredecessorNode* pre_copy;
    pre_copy = get_copy(pre);

    SuccessorNode* succ;

    NodeRelationPtrVector relations;
    NodeRelationPtrVector::iterator it;
    NodeRelationPtr rel_copy;

    relations = pre->get_successors();

    for(it=relations.begin();it!=relations.end();it++)
    {
      succ = (*it)->get_successor();
      rel_copy=pre_copy->add_successor(get_copy(succ));
      set_copy(it->get(),rel_copy.get());
    }
  }

  void HMscDuplicator::create_time_relation(HMscPtr hmsc)
  {

    // get all time relations
    HMscAllTimeRelations rels(hmsc);

    TimeRelationRefNodePtrSet::iterator it;
    for(it=rels.begin();it!=rels.end();it++)
    {
      TimeRelationRefNodePtr rel = *it;

      ReferenceNode* a = rel->get_ref_node_a();
      ReferenceNode* b = rel->get_ref_node_b();
      bool bottom_a = rel->is_bottom_node_a();
      bool bottom_b = rel->is_bottom_node_b();
      //check whether its not the same node (-> fix bottom_a)
      if(a==b)
        bottom_a=false;

      ReferenceNode* a_copy = get_copy(a);
      ReferenceNode* b_copy = get_copy(b);
      // built up new time relation
      TimeRelationRefNode* rel_copy;
      rel_copy = new TimeRelationRefNode(rel.get());

      rel_copy->glue_ref_node_a(bottom_a,a_copy);
      rel_copy->glue_ref_node_b(bottom_b,b_copy);
      set_copy(rel.get(),rel_copy);
    }
  }

  void HMscDuplicator::inner_msc(ReferenceNode* ref)
  {
    MscPtr msc = ref->get_msc();
    if(!msc.get())
      throw std::logic_error("Empty ReferenceNode - inner Msc is NULL pointer.");

    BMsc* bmsc;
    HMsc* hmsc;
    if((bmsc=dynamic_cast<BMsc*>(msc.get())))
    {
      inner_msc_found(bmsc);
    }
    else if((hmsc=dynamic_cast<HMsc*>(msc.get())))
    {
        inner_msc_found(hmsc);
    }
    else
      throw std::logic_error("Couldnt find right type of inner msc.");

    m_to_set_up.push(std::make_pair(ref,msc.get()));

  }

  void HMscDuplicator::inner_msc_found(HMscPtr hmsc)
  {
    if(m_hmscs.find(hmsc.get())==m_hmscs.end())
    {
      m_to_create_hm.push(hmsc.get());
      m_hmscs.insert(hmsc.get());
    }
  }

  void HMscDuplicator::inner_msc_found(BMscPtr bmsc)
  {
    if(m_bmscs.find(bmsc.get())==m_bmscs.end())
    {
      m_to_create_bm.push(bmsc.get());
      m_bmscs.insert(bmsc.get());
    }
  }


HMscPtr HMscDuplicator::duplicate(HMscPtr hmsc)
  {
    std::stack<BMscPtr> created_bmsc;
    std::stack<HMscPtr> created_hmsc;
     m_hmsc_root = hmsc;
     if(!hmsc.get()) // null hmsc
       throw std::logic_error("Null HMsc to duplicate.");

     m_to_create_hm.push(hmsc.get()); // insert to the stack to create
     m_hmscs.insert(m_hmsc_root.get()); // insert as allready found

     HMscPtr copy;
     BMscPtr b_copy;
     while(!m_to_create_hm.empty())
     {
      // std::cerr << "HMscDuplicator: creating hmsc" << std::endl;
        HMscPtr hmsc = m_to_create_hm.top();
        m_to_create_hm.pop();
        copy = create(hmsc.get());

        HMscNodePtrSet nodes = hmsc->get_nodes();
        HMscNodePtrSet::iterator it;
        for(it=nodes.begin();it!=nodes.end();it++)
        {
          HMscNodePtr node = process_node(it->get());
          copy->add_node(node);
        }

        // create start relations
        create_node_relation(static_cast<HMscNode*>(hmsc->get_start().get()));

        // create relations
        for(it=nodes.begin();it!=nodes.end();it++)
        {
          create_node_relation(it->get());
        }

        // create time  relations
        create_time_relation(hmsc);
        created_hmsc.push(copy);
     }

     while(!m_to_create_bm.empty())
     {
      // std::cerr << "HMscDuplicator: creating bmsc" << std::endl;
       BMscPtr bmsc = m_to_create_bm.top();
       m_to_create_bm.pop();
       b_copy = BMscDuplicator::duplicate(bmsc);
       created_bmsc.push(b_copy);
       set_copy(bmsc.get(),b_copy.get());
     }

     while(!m_to_set_up.empty())
     {
      // std::cerr << "HMscDuplicator: setting up" << std::endl;
       ReferenceNode* n = m_to_set_up.top().first;
       Msc* msc = m_to_set_up.top().second;
       m_to_set_up.pop();

       ReferenceNode* n_copy = get_copy(n);
       n_copy->set_msc(get_copy(msc));
     }
     HMsc* copy2 = get_copy(m_hmsc_root.get());
     return copy2;
  }


/////////////////////////////////////////////////////////////////////////////

EventsCreatorListener::EventsCreatorListener(BMscDuplicator* duplicator, DFSAreaTraverser* traverser, BMsc* bmsc):
  m_duplicator(duplicator),
  m_last_instance(NULL),
  m_last_new_instance(NULL),
  m_last_area(NULL),
  m_last_new_area(NULL),
  m_bmsc(bmsc),
  m_traverser(traverser)
{

}

void EventsCreatorListener::on_white_event_found(Event* e)
{
  if(m_last_instance!=e->get_instance())
  {
    InstancePtr new_instance = new Instance(e->get_instance());
    m_bmsc->add_instance(new_instance);
    m_last_instance = e->get_instance();
    m_last_new_instance = new_instance.get();
  }
  if(m_last_area!=e->get_general_area())
  {
    StrictOrderArea* strict = dynamic_cast<StrictOrderArea*>(e->get_general_area());
    EventAreaPtr area;
    if(strict)
    {
      area = new StrictOrderArea(strict);
    }
    else
    {
      area = new CoregionArea(dynamic_cast<CoregionArea*>(e->get_general_area()));
    }
    m_last_new_area = area.get();
    m_last_new_instance->add_area(area);
    m_last_area = e->get_general_area();
  }
  CoregionEvent* coreg_event = dynamic_cast<CoregionEvent*>(e);
  if(coreg_event)
  {
    CoregionEventPtr new_e = new CoregionEvent(e);
    m_duplicator->set_copy(e,new_e.get());
    dynamic_cast<CoregionArea*>(m_last_new_area)->add_event(new_e);
  }
  else
  {
    StrictEventPtr new_e = new StrictEvent(e);
    m_duplicator->set_copy(e,new_e.get());
    dynamic_cast<StrictOrderArea*>(m_last_new_area)->add_event(new_e);
  }
  create_successor(e);
}

CoregionEvent* EventsCreatorListener::get_preceding_event()
{
  const MscElementPList& elements = m_traverser->get_reached_elements();
  if(elements.size()>1)
  {
    //in this case currently traversed event isn't alone in elements
    MscElementPList::const_iterator i = elements.end();
    i--; i--;
    if(dynamic_cast<CoregionEventRelation*>(*i))
    {
      i--;
      return dynamic_cast<CoregionEvent*>(*i);
    }
  }
  return NULL;
}

void EventsCreatorListener::on_gray_event_found(Event* e)
{
  create_successor(e);
}

void EventsCreatorListener::on_black_event_found(Event* e)
{
  create_successor(e);
}

void EventsCreatorListener::create_successor(Event* e)
{
  CoregionEvent* coreg_new = dynamic_cast<CoregionEvent*>(m_duplicator->get_event_copy(e));
  if(coreg_new)
  {
    CoregionEvent* preceding = get_preceding_event();
    if(preceding)
    {
      CoregionEvent* preceding_new = dynamic_cast<CoregionEvent*>(m_duplicator->get_event_copy(preceding));
      preceding_new->add_successor(coreg_new);
     // std::cerr << "creating coreg rel " << preceding_new << " -> " << coreg_new << std::endl;	
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

MessagesCreatorListener::MessagesCreatorListener(BMscDuplicator* duplicator):
m_duplicator(duplicator)
{

}

void MessagesCreatorListener::on_white_event_found(Event* e)
{
  Event* event_copy = m_duplicator->get_event_copy(e);
  if(e->is_matched())
  {
    if(e->is_send())
    {
      Event* matching_copy = m_duplicator->get_event_copy(e->get_matching_event());
      MscMessagePtr complete = new CompleteMessage(event_copy,matching_copy,e->get_complete_message().get());
      event_copy->set_message(complete);
      matching_copy->set_message(complete);
      m_duplicator->set_copy(e->get_message().get(),complete.get());
    }
  }
  else
  {
    MscMessagePtr incomplete = new IncompleteMessage(e->get_incomplete_message().get());
    event_copy->set_message(incomplete);
    m_duplicator->set_copy(e->get_message().get(),incomplete.get());
  }
}

////////////////////////////////////////////////////////////////////////

TimeRelationCreatorListener::TimeRelationCreatorListener(BMscDuplicator* duplicator):
m_duplicator(duplicator)
{

}

void TimeRelationCreatorListener::on_white_event_found(Event* e)
{
  TimeRelationEventPtrList relations = e->get_time_relations();
  TimeRelationEventPtrList::iterator it;
  for(it=relations.begin();it!=relations.end();it++)
  {
    if((*it)->get_event_a()!=e)
      continue;

    Event* a_copy = m_duplicator->get_event_copy((*it)->get_event_a());
    Event* b_copy = m_duplicator->get_event_copy((*it)->get_event_b());
    TimeRelationEventPtr relation = new TimeRelationEvent((*it).get());
    relation->glue_events(a_copy, b_copy);
    m_duplicator->set_copy((*it).get(),relation.get());
    if((it->get())->get_marked() == MARKED)
      relation->set_marked();

  }

}

////////////////////////////////////////////////////////////////////////

GraphCreatorListener::GraphCreatorListener(BMscGraphDuplicator* duplicator, DFSBMscGraphTraverser* traverser,HMsc* hmsc):
m_duplicator(duplicator),m_traverser(traverser),m_hmsc(hmsc)
{
}

void GraphCreatorListener::on_white_node_found(HMscNode *n)
{
  ReferenceNode* reference_node = dynamic_cast<ReferenceNode*>(n);
  if(reference_node)
    return on_white_node_found(reference_node);

  StartNode* start_node = dynamic_cast<StartNode*>(n);
  if(start_node)
    return on_white_node_found(start_node);

  EndNode* end_node = dynamic_cast<EndNode*>(n);
  if(end_node)
    return on_white_node_found(end_node);

  ConditionNode* condition_node = dynamic_cast<ConditionNode*>(n);
  if(condition_node)
    return on_white_node_found(condition_node);

  ConnectionNode* connection_node = dynamic_cast<ConnectionNode*>(n);
  if(connection_node)
    return on_white_node_found(connection_node);
}

void GraphCreatorListener::on_white_node_found(ReferenceNode* n)
{
  BMscPtr bmsc = n->get_bmsc();
  HMscNodePtr new_node;
  if(bmsc.get())
  {
    BMscPtr new_bmsc = BMscDuplicator::duplicate(bmsc);
    ReferenceNode* reference = new ReferenceNode(n);
    reference->set_msc(new_bmsc);
    new_node = reference;
  }
  else
  {
    ConnectionNode* connection = new ConnectionNode(n);
    new_node = connection;
    ReferenceNode*& attribute = get_referencing_node(n->get_hmsc().get());
    attribute = n;
  }
  process_new_node(n,new_node);
}

void GraphCreatorListener::on_white_node_found(StartNode* n)
{
  if(is_root_element(n))
  {
    StartNode *new_node = new StartNode(n);
    m_hmsc->set_start(new_node);
    m_new_nodes.push_back(new_node);
    m_duplicator->set_copy(n,new_node);
  }
}

void GraphCreatorListener::on_white_node_found(EndNode* n)
{
  HMscNodePtr new_node;
  if(is_root_element(n))
  {
    EndNode* end = new EndNode(n);
    new_node = end;
  }
  else
  {
    ConnectionNode* connection = new ConnectionNode(n);
    new_node = connection;
    add_to_end_list(connection);
  }
  process_new_node(n,new_node);
}

ConnectionNodePList& GraphCreatorListener::get_end_list(ReferenceNode* reference)
{
  static ConnectionNodePList empty;
  //attribute is removed when reference is finished in traversing
  return reference->get_attribute<ConnectionNodePList>(BMSC_GRAPH_DUPLICATOR_ENDLIST_ATTR,empty);
}

void GraphCreatorListener::add_to_end_list(ConnectionNode* new_end)
{
  HMsc* original_hmsc = new_end->get_original()->get_owner();
  ReferenceNode* original_reference = get_referencing_node(original_hmsc);
  ConnectionNodePList& end_list = get_end_list(original_reference);
  end_list.push_back(new_end);
}

void GraphCreatorListener::on_white_node_found(ConditionNode* n)
{
//  PredecessorNode* predecessor = get_predecessor();
  HMscNodePtr new_node = new ConditionNode(n);
  process_new_node(n,new_node);
}

void GraphCreatorListener::on_white_node_found(ConnectionNode* n)
{
//  PredecessorNode* predecessor = get_predecessor();
  HMscNodePtr new_node = new ConnectionNode(n);
  process_new_node(n,new_node);
}

void GraphCreatorListener::process_new_node(HMscNode* old_node, HMscNodePtr& new_node)
{
  m_hmsc->add_node(new_node);
  add_new_successor(new_node.get());
  m_new_nodes.push_back(new_node.get());
  m_duplicator->set_copy(old_node,new_node.get());
}

bool GraphCreatorListener::is_root_element(HMscNode* n)
{
  return n->get_owner()==m_hmsc->get_original();
}

PredecessorNode* GraphCreatorListener::get_predecessor()
{
  return dynamic_cast<PredecessorNode*>(m_new_nodes.back());
}

HMscNode* GraphCreatorListener::get_node_copy(HMscNode* n)
{
  return dynamic_cast<HMscNode*>(m_duplicator->get_copy(n));
}

void GraphCreatorListener::process_nonwhite_node(HMscNode* n)
{
  HMscNode* copy = get_node_copy(n);
  add_new_successor(copy);
}

void GraphCreatorListener::on_gray_node_found(HMscNode* n)
{
  process_nonwhite_node(n);
}

void GraphCreatorListener::on_black_node_found(HMscNode* n)
{
  process_nonwhite_node(n);
}

void GraphCreatorListener::on_node_finished(HMscNode* n)
{
  StartNode* start_node = dynamic_cast<StartNode*>(n);
  if(start_node)
    return on_node_finished(start_node);

  EndNode* end_node = dynamic_cast<EndNode*>(n);
  if(end_node)
    return on_node_finished(end_node);

  ReferenceNode* reference_node = dynamic_cast<ReferenceNode*>(n);
  if(reference_node)
    return on_node_finished(reference_node);

  ConditionNode* condition_node = dynamic_cast<ConditionNode*>(n);
  if(condition_node)
    return on_node_finished(condition_node);

  ConnectionNode* connection_node = dynamic_cast<ConnectionNode*>(n);
  if(connection_node)
    return on_node_finished(connection_node);
}

void GraphCreatorListener::on_node_finished(ReferenceNode* n)
{
  n->remove_attribute<ConnectionNodePList>(BMSC_GRAPH_DUPLICATOR_ENDLIST_ATTR);
  m_new_nodes.pop_back();
}

void GraphCreatorListener::on_node_finished(StartNode* n)
{
  if(is_root_element(n))
  {
    m_new_nodes.pop_back();
  }
}

void GraphCreatorListener::on_node_finished(EndNode* n)
{
  m_new_nodes.pop_back();
}

void GraphCreatorListener::on_node_finished(ConditionNode* n)
{
  m_new_nodes.pop_back();
}

void GraphCreatorListener::on_node_finished(ConnectionNode* n)
{
  m_new_nodes.pop_back();
}

ReferenceNode*& GraphCreatorListener::get_referencing_node(HMsc* hmsc)
{
  bool just_set;
  ReferenceNode*& reference = hmsc->get_attribute<ReferenceNode*>(BMSC_GRAPH_DUPLICATOR_REF_ATTR,NULL,just_set);
  if(just_set)
  {
    m_modified_hmscs.push_back(hmsc);
  }
  return reference;
}

void GraphCreatorListener::add_new_successor(HMscNode* new_successor)
{
  SuccessorNode* new_succ = dynamic_cast<SuccessorNode*>(new_successor);
  PredecessorNode* predecessor = get_predecessor();
  ConnectionNode* connection = dynamic_cast<ConnectionNode*>(predecessor);
  if(connection)
  {
    ReferenceNode* original_reference = dynamic_cast<ReferenceNode*>(connection->get_original());
    if(original_reference &&
      new_successor->get_original()->get_owner()==original_reference->get_owner())
    {
      //connection was transformed from ReferenceNode and probably contains end_list,
      //elements of the end_list must become predecessors of new_successor which
      //must be from same HMsc as original_reference
      ConnectionNodePList& end_list = get_end_list(original_reference);
      ConnectionNodePList::const_iterator i;
      for(i=end_list.begin();i!=end_list.end();i++)
      {
        (*i)->add_successor(new_succ);
      }
      return;
    }
  }
  predecessor->add_successor(new_succ)->set_original(get_previous_relation());
}

NodeRelation* GraphCreatorListener::get_previous_relation()
{
  //it is supposed that this method is called when node was found by traverser
  //therefore the second one before end must be relation
  return dynamic_cast<NodeRelation*>(*(--(--(m_traverser->get_reached_elements().back().end()))));
}

GraphCreatorListener::~GraphCreatorListener()
{
  while(!m_modified_hmscs.empty())
  {
    HMsc* h = m_modified_hmscs.back();
    h->remove_attribute<ReferenceNode*>(BMSC_GRAPH_DUPLICATOR_REF_ATTR);
    m_modified_hmscs.pop_back();
  }
}

////////////////////////////////////////////////////////////

NormalConstraintsCreatorListener::NormalConstraintsCreatorListener(BMscGraphDuplicator* duplicator):
  m_duplicator(duplicator)
{  

}

void NormalConstraintsCreatorListener::on_white_node_found(HMscNode* n)
{
  ReferenceNode* ref_node_original;
  ReferenceNode* ref_node_b = dynamic_cast<ReferenceNode*>(n);;
  ReferenceNode** ref_node_a;
  TimeRelationRefNodePtrSet relations;
  TimeRelationRefNodePtrSet::iterator rel_it;
  
  
  if(!ref_node_b){
    return;
  }
  
  ref_node_original = dynamic_cast<ReferenceNode*>(n->get_original());

  relations = ref_node_original->get_time_relations_top();
  
  for(rel_it=relations.begin();rel_it!=relations.end();rel_it++){
    
    //tries to access the map value for *rel_it
    //if the value does not exist, map creates new entry
    ref_node_a = &m_relations_to_close[*rel_it];
    if(*ref_node_a)
    {
      //time relation was already discovered
      //duplicate time relation
      //the relaton should be between ref_node_a an ref_node_b 
      TimeRelationRefNode* rel_copy = new TimeRelationRefNode((*rel_it).get());
      
      //checking wheter relation is connected to top or bottom of ref_node_a
      bool bottom_a = (*rel_it).get()->is_bottom_node_a();
      bool bottom_b = (*rel_it).get()->is_bottom_node_b();

      //we know that one of bottom_a bottom_b is false
      //(relation is connected to top of this node)
      bottom_a = (bottom_a || bottom_b);

      rel_copy->glue_ref_node_a(bottom_a,*ref_node_a);
      rel_copy->glue_ref_node_b(false,ref_node_b);
      m_duplicator->set_copy((*rel_it).get(),rel_copy);
      //erase relation from relations_to_close
      m_relations_to_close.erase(*rel_it);
    }else{
      //time relation was just discovered
      //set the ref node to the approprate place in relations_to_close
      (*ref_node_a) = ref_node_b;
    }
  }

  relations = ref_node_original->get_time_relations_bottom();

  for(rel_it=relations.begin();rel_it!=relations.end();rel_it++){
    
    //tries to access the map value for key *rel_it
    //if the value does not exist, map creates new entry    
    ref_node_a = &m_relations_to_close[*rel_it];
   
    if(*ref_node_a)
    {
      //time relation was already discovered
      //duplicate time relation
      //the relaton should be between ref_node_a an ref_node_b 
      TimeRelationRefNode* rel_copy = new TimeRelationRefNode((*rel_it).get());
      //checking wheter relation is connected to top or bottom of ref_node_a
      bool top_a = (*rel_it).get()->is_top_node_a();
      bool top_b = (*rel_it).get()->is_top_node_b();

      //we know that one of top_a top_b is false
      //(relation is connected to bottom of this node)      
      top_a = (top_a || top_b);

      rel_copy->glue_ref_node_a(!top_a,*ref_node_a);
      rel_copy->glue_ref_node_b(true,ref_node_b);
      m_duplicator->set_copy((*rel_it).get(),rel_copy);
      //erase relation from relations_to_close
      m_relations_to_close.erase(*rel_it);
    }else{
      //time relation was just discovered
      //set the ref node to the approprate place in relations_to_close
      (*ref_node_a) = ref_node_b;
    }
  }
}
  
NormalConstraintsCreatorListener::~NormalConstraintsCreatorListener()
{
}

//////////////////////////////////////////////////////////////

HighLevelConstraintsCreatorListener::HighLevelConstraintsCreatorListener(BMscGraphDuplicator* duplicator):
  m_duplicator(duplicator)
{  
}

void HighLevelConstraintsCreatorListener::on_white_node_found(HMscNode* n)
{
  ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>(n) ;
  if(ref_node && ref_node->get_hmsc().get() && 
    !(ref_node->get_time_relations_top().empty() && ref_node->get_time_relations_bottom().empty()))
  {
      std::cout<<"throwing exception"<<std::endl;
      throw std::runtime_error("High-level constraint- cannot create alternative");
  }
    
}

HighLevelConstraintsCreatorListener::~HighLevelConstraintsCreatorListener()
{
}

////////////////////////////////////////////////////////////

HMscPtr BMscGraphDuplicator::duplicate_hmsc(HMscPtr& hmsc)
{
  HMscPtr new_hmsc;
  if(hmsc.get())
  {
    new_hmsc = new HMsc(hmsc.get());
    DFSBMscGraphTraverser traverser;
    GraphCreatorListener listener(this,&traverser,new_hmsc.get());
    traverser.add_black_node_found_listener(&listener);
    traverser.add_gray_node_found_listener(&listener);
    traverser.add_node_finished_listener(&listener);
    traverser.add_white_node_found_listener(&listener);
    traverser.traverse(hmsc);

    
    traverser.remove_all_listeners();
    traverser.cleanup_traversing_attributes();
    HighLevelConstraintsCreatorListener listener2(this);
    traverser.add_white_node_found_listener(&listener2);
//    try{
      traverser.traverse(hmsc.get());
//    }catch(std::runtime_error){
//	traverser.get_reached_elements();
//	throw std::runtime_error("");
//    }
   
    NormalConstraintsCreatorListener listener3(this);
    traverser.remove_all_listeners();
    traverser.cleanup_traversing_attributes();
    traverser.add_white_node_found_listener(&listener3);
    traverser.traverse(new_hmsc);
    
  }
  return new_hmsc;
}

HMscPtr BMscGraphDuplicator::duplicate(HMscPtr& hmsc)
{
  BMscGraphDuplicator duplicator;
  return duplicator.duplicate_hmsc(hmsc);
}

BMscGraphDuplicator::~BMscGraphDuplicator()
{
}

//////////////////////////////////////////////////////////////////

HMscPtr HMscPathDuplicator::duplicate_path(const MscElementPListList& path)
{
  HMscPtr root;
  if(path.size()>0)
  {
    MscElementPListList::const_iterator h;
    PredecessorNode* new_pred(NULL);
    for(h=path.begin();h!=path.end();h++)
    {
      HMscPtr new_hmsc;
      NodeRelation* old_rel(NULL);
      MscElementPList::const_iterator e;
      for(e=(*h).begin();e!=(*h).end();e++)
      {
        if(e==(*h).begin())
        {
          StartNode* start = dynamic_cast<StartNode*>(*e);
          new_hmsc = new HMsc(start->get_owner());
          if(!root.get())
          {
            root = new_hmsc;
          }
          else
          {
            //hmsc is surely referenced from ReferenceNode
            ReferenceNode* ref = dynamic_cast<ReferenceNode*>(new_pred);
            ref->set_msc(new_hmsc);
          }
          StartNode* new_start = new StartNode(start);
          new_hmsc->set_start(new_start);
          new_pred = new_start;
          set_copy(start,new_start);
          set_copy(start->get_owner(),new_hmsc.get());
        }
        else
        {
          NodeRelation* rel = dynamic_cast<NodeRelation*>(*e);
          if(rel)
          {
            old_rel = rel;
          }
          else
          {
            ReferenceNode* ref_node;
            ConditionNode* cond_node;
            ConnectionNode* conn_node;
            EndNode* end_node;

            NodeRelationPtr new_rel;

            if((ref_node = dynamic_cast<ReferenceNode*>(*e)))
            {
              ReferenceNode* new_ref = new ReferenceNode(ref_node);
              new_ref->set_msc(ref_node->get_msc());
              new_hmsc->add_node(new_ref);
              new_rel = new_ref->add_predecessor(new_pred);
              new_pred = new_ref;
              set_copy(ref_node,new_ref);
              // copy time relations
	      process_time_relations(ref_node);
            }
            else if((cond_node = dynamic_cast<ConditionNode*>(*e)))
            {
              ConditionNode* new_cond = new ConditionNode(cond_node);
              new_hmsc->add_node(new_cond);
              new_rel = new_cond->add_predecessor(new_pred);
              new_pred = new_cond;
              set_copy(cond_node,new_cond);
            }
            else if((conn_node = dynamic_cast<ConnectionNode*>(*e)))
            {
              ConnectionNode* new_conn = new ConnectionNode(conn_node);
              new_hmsc->add_node(new_conn);
              new_rel = new_conn->add_predecessor(new_pred);
              new_pred = new_conn;
              set_copy(conn_node,new_conn);
            }
            else if((end_node = dynamic_cast<EndNode*>(*e)))
            {
              EndNode* new_end = new EndNode(end_node);
              new_rel = new_end->add_predecessor(new_pred);
              new_hmsc->add_node(new_end);
              set_copy(end_node,new_end);
            }
            else
              throw std::runtime_error("Unknown node type");

            new_rel->set_original(old_rel);
            new_rel->set_line(old_rel->get_line());
            set_copy(old_rel,new_rel.get());
          }
        }
      }
    }
  }
  return root;
}


void HMscPathDuplicator::process_time_relations(
   ReferenceNode* ref_node
  ,TimeRelationRefNodePtrSet time_relations
  ,bool bottom
  )
{

  TimeRelationRefNodePtrSet::iterator it;
  TimeRelationRefNodePtr relation_copy;

  for(it=time_relations.begin();it!=time_relations.end();it++)
  {
    ReferenceNode* a = (ReferenceNode*) get_copy((*it)->get_ref_node_a());
    ReferenceNode* b = (ReferenceNode*) get_copy((*it)->get_ref_node_b());
    
    if(!a || !b) return ; // do nothing until both reference nodes are duplicated

    relation_copy = new TimeRelationRefNode((*it).get());
    set_copy(it->get(),relation_copy.get());
      
    // if its the same, false, true ??
    if(a==b)
    {
      if(bottom)
	relation_copy->glue_ref_nodes(false,a,true,a);
      return ;
    }
    relation_copy->glue_ref_node_a((*it)->is_bottom_node_a(), a);
    relation_copy->glue_ref_node_b((*it)->is_bottom_node_b(), b);
  }
}

void HMscPathDuplicator::process_time_relations(ReferenceNode* ref_node)
{

  TimeRelationRefNodePtrSet time_relations;
  time_relations = ref_node->get_time_relations_top();
  process_time_relations(ref_node,time_relations,false);

  time_relations = ref_node->get_time_relations_bottom();
  process_time_relations(ref_node,time_relations,true);

}

HMscPathDuplicator::~HMscPathDuplicator()
{
}

//////////////////////////////////////////////////////////////////

const Coordinate FLAT_PATH_INITIAL_X = 20;
const Coordinate FLAT_PATH_INITIAL_Y = 10;
const Coordinate FLAT_PATH_VERTICAL_STEP = 20;
const Coordinate FLAT_PATH_HORIZONTAL_STEP = 20;

HMscPtr HMscFlatPathDuplicator::duplicate_path(const MscElementPList& path, bool persist_original)
{
  HMscPtr root;
  Coordinate current_depth(0);
  Coordinate current_index(0);
  bool new_elem;
  if(path.size()>0)
  {
    PredecessorNode* new_pred(NULL);
    NodeRelation* old_rel(NULL);
    MscElementPList::const_iterator e;
    for(e=path.begin();e!=path.end();e++)
    {
     // std::cerr<<"doing another element in path: "<<*e<<" and its copy is: "<<this->get_copy(*e) <<std::endl;
      (*e)->get_attribute<bool>("path_new_elem", false, new_elem);

      if(e==path.begin())
      {
        StartNode* start = dynamic_cast<StartNode*>(*e);
        root = new HMsc(start->get_owner());
        StartNode* new_start = new StartNode(start);
        root->set_start(new_start);
        new_pred = new_start;
        set_copy(start,new_start,persist_original);
        set_copy(start->get_owner(),root.get(),persist_original);
      }
      else
      {
        NodeRelation* rel = dynamic_cast<NodeRelation*>(*e);
        if(rel)
        {
          old_rel = rel;
        }
        else
        {
          ReferenceNode* ref_node;
          ConditionNode* cond_node;
          ConnectionNode* conn_node;
          EndNode* end_node;

          NodeRelationPtr new_rel;

          if((ref_node = dynamic_cast<ReferenceNode*>(*e)))
          {
            ReferenceNode* new_ref;
            if(new_elem)
            {
              new_ref = new ReferenceNode(ref_node);
              BMscDuplicator duplicator2;
              if(ref_node->get_bmsc())
              {
                BMscPtr temp = ref_node->get_bmsc();
                new_ref->set_msc(duplicator2.duplicate_bmsc(temp));
              }
              else
                new_ref->set_msc(ref_node->get_msc());
              duplicator2.cleanup_attributes();

              root->add_node(new_ref);
            }
            else
              new_ref = dynamic_cast<ReferenceNode*>((*e)->get_attribute<MscElement*>(DUPLICATOR_COPY_ATTR,NULL));

            new_rel = new_ref->add_predecessor(new_pred);
            new_pred = new_ref;
            set_copy(ref_node,new_ref,persist_original);
          }
          else if((cond_node = dynamic_cast<ConditionNode*>(*e)))
          {
            ConditionNode* new_cond;
            if(new_elem)
            {
              new_cond = new ConditionNode(cond_node);
              root->add_node(new_cond);
            }
            else
              new_cond = dynamic_cast<ConditionNode*>((*e)->get_attribute<MscElement*>(DUPLICATOR_COPY_ATTR,NULL));


            new_rel = new_cond->add_predecessor(new_pred);
            new_pred = new_cond;
            set_copy(cond_node,new_cond,persist_original);
          }
          else if((conn_node = dynamic_cast<ConnectionNode*>(*e)))
          {
            ConnectionNode* new_conn;
            if(new_elem)
            {
              new_conn = new ConnectionNode(conn_node);
              root->add_node(new_conn);
            }
            else
              new_conn = dynamic_cast<ConnectionNode*>((*e)->get_attribute<MscElement*>(DUPLICATOR_COPY_ATTR,NULL));

            new_rel = new_conn->add_predecessor(new_pred);
            new_pred = new_conn;
            set_copy(conn_node,new_conn,persist_original);
          }
          else if((end_node = dynamic_cast<EndNode*>(*e)))
          {
            EndNode* new_end = new EndNode(end_node);
            new_rel = new_end->add_predecessor(new_pred);
            root->add_node(new_end);
            set_copy(end_node,new_end,persist_original);
          }
          else
            throw std::runtime_error("Unknown node type");

          new_rel->set_original(old_rel);
          new_rel->set_line(old_rel->get_line());
          set_copy(old_rel,new_rel.get(),persist_original);
        }
      }
      //process position of HMscNode
      HMscNode* n = dynamic_cast<HMscNode*>(*e);
      if(n && new_elem)
      {
        dynamic_cast<HMscNode*>(get_copy(*e))->set_position(
          MscPoint(
            FLAT_PATH_INITIAL_X+current_depth*FLAT_PATH_HORIZONTAL_STEP,
            FLAT_PATH_INITIAL_Y+current_index*FLAT_PATH_VERTICAL_STEP
          )
        );
        if(e!=path.begin() && (dynamic_cast<StartNode*>(*e) ||
          (dynamic_cast<ConnectionNode*>(*e) && dynamic_cast<ReferenceNode*>((*e)->get_general_original()))))
        {
          current_depth++;
        }
        if(dynamic_cast<EndNode*>(*e) || dynamic_cast<EndNode*>((*e)->get_general_original()))
        {
          current_depth--;
          dynamic_cast<HMscNode*>(get_copy(*e))->set_position(
            MscPoint(
              FLAT_PATH_INITIAL_X+current_depth*FLAT_PATH_HORIZONTAL_STEP,
              FLAT_PATH_INITIAL_Y+current_index*FLAT_PATH_VERTICAL_STEP
            )
          );
        }
        current_index++;
      }
    }
    for(e=path.begin();e!=path.end();e++)
      (*e)->remove_attribute<bool>("path_new_elem");
    
    for(e=path.begin();e!=path.end();e++)
    {
      ReferenceNode* ref_node = dynamic_cast<ReferenceNode*>(*e);
      if(ref_node)
	process_time_relations(ref_node,persist_original);
    }
  }
  return root;
}


void HMscFlatPathDuplicator::process_time_relations(
   ReferenceNode* ref_node
  ,TimeRelationRefNodePtrSet time_relations
  ,bool bottom
  ,bool persist_original
  )
{

  TimeRelationRefNodePtrSet::iterator it;
  TimeRelationRefNodePtrSet::iterator it2;
  TimeRelationRefNodePtr relation_copy;

  for(it=time_relations.begin();it!=time_relations.end();it++)
  {
    it2 = m_open_relations.find(*it);
    if(it2 == m_open_relations.end())
    {
      m_open_relations.insert(*it);
      return;
    }
    ReferenceNode* a = (ReferenceNode*) get_copy((*it)->get_ref_node_a());
    ReferenceNode* b = (ReferenceNode*) get_copy((*it)->get_ref_node_b());
    
    relation_copy = new TimeRelationRefNode((*it).get());
    set_copy(it->get(),relation_copy.get(),persist_original);
      //std::cerr<<"gluing time relation between: "<<a<<" | "<<b<< "value: "<<it->get()->get_interval_set()<<std::endl;
     // std::cerr<<"original nodes              : "<<(*it)->get_ref_node_a()<<" | "<<(*it)->get_ref_node_b()<< "value: "<<it->get()->get_interval_set()<<std::endl;
    // if its the same, false, true ??
    if(a==b)
    {
      if(bottom)
	relation_copy->glue_ref_nodes(false,a,true,a);
      return ;
    }
    relation_copy->glue_ref_node_a((*it)->is_bottom_node_a(), a);
    relation_copy->glue_ref_node_b((*it)->is_bottom_node_b(), b);
  }
}

void HMscFlatPathDuplicator::process_time_relations(ReferenceNode* ref_node, bool persist_original)
{
  //std::cerr<<"new reference node: "<<ref_node<<std::endl;
  TimeRelationRefNodePtrSet time_relations;
  time_relations = ref_node->get_time_relations_top();
  process_time_relations(ref_node,time_relations,false,persist_original);

  time_relations = ref_node->get_time_relations_bottom();
  process_time_relations(ref_node,time_relations,true,persist_original);

}

void HMscFlatPathDuplicator::set_copy(MscElement* original, MscElement* copy, bool persist_original)
{
  if(persist_original && original->get_general_original())
  {
    copy->set_general_original(original->get_general_original());
  }
  Duplicator::set_copy(original,copy);
}

HMscFlatPathDuplicator::~HMscFlatPathDuplicator()
{
}


BMscPtr HMscFlatPathToBMscDuplicator::duplicate_path(const MscElementPList& path)
{
  BMscPtr new_bmsc_ptr= new BMsc();
  BMscPtr* stored_copy;
  BMscPtr original_bmsc;
  std::map<BMscPtr,BMscPtr> used_bmscs;
  BMscDuplicator duplicator2;
  std::list<BMscPtr> created_bmscs;
  
  InstancePtrList instances;
  InstancePtrList::iterator inst_it;
  InstancePtrList new_instances;
  InstancePtrList::iterator new_inst_it;
  Instance* new_instance;
  EventAreaPtr start_area;
  EventArea* current_area;
  MscElementPList remaining_elements;
  MscElementPList duplicator_modified_elements;
  
  m_modified_elements.clear();

  MscElementPList::const_iterator e;
  MscElementPList::const_iterator f;
  ReferenceNode* ref_node;

  if(path.size() == 0)
  {
    return new_bmsc_ptr;
  }

  for(e=path.begin();e!=path.end();e++)
  {
    ref_node = dynamic_cast<ReferenceNode*>(*e);
    if(ref_node)
    {
      original_bmsc = ref_node->get_bmsc();
      if(original_bmsc.get()==0) {
	return new_bmsc_ptr;
      }
    }
  }

  for(e=path.begin();e!=path.end();e++)
  {
    ref_node = dynamic_cast<ReferenceNode*>(*e);
    if(ref_node)
    {
      original_bmsc = ref_node->get_bmsc();
      
      //check whether the original_bmsc already occured
      stored_copy = &(used_bmscs[original_bmsc]);
      if((*stored_copy).get())
      {
	//the original_bmsc was already duplicated
	//make new copy and use stored_copy
	(*stored_copy) = duplicator2.duplicate_bmsc(*stored_copy);
      }else{
	// the original_bmsc has not occured yet
	//make new copies of original_bmsc and store the second one
	(*stored_copy) = duplicator2.duplicate_bmsc(original_bmsc);
      }
      created_bmscs.push_back(*stored_copy);
      
    }else if(!(dynamic_cast<NodeRelation*>(*e)
	    || dynamic_cast<ConnectionNode*>(*e)
	    || dynamic_cast<ConditionNode*>(*e)
	    || dynamic_cast<StartNode*>(*e)
	    || dynamic_cast<EndNode*>(*e)))
    {  
      throw std::runtime_error("Unknown type");
    }
  }


  std::list<BMscPtr>::iterator bmscs_it;
  for(bmscs_it=created_bmscs.begin();bmscs_it!=created_bmscs.end();bmscs_it++)
  {

    instances = (*bmscs_it).get()->get_instances();
    for(inst_it = instances.begin();inst_it != instances.end();inst_it++)
    {  
      new_instance = NULL;
      new_instances = new_bmsc_ptr.get()->get_instances();
      
      //checking whether the instance with the same instance label as (*inst_it) is already present in new_bmsc
      for(new_inst_it=new_instances.begin();new_inst_it != new_instances.end();new_inst_it++){
	if((*inst_it).get()->get_label().compare((*new_inst_it).get()->get_label())==0)
	  new_instance = (*new_inst_it).get();
      }
      
      if(new_instance){
	//instance with the same instance label is present in new_bmsc
	//concatenate the processes
	if(!((*inst_it).get()->is_empty())){
	  start_area = (*inst_it).get()->get_first();
	  current_area = start_area.get();      //TODO check whether I changed everything necessary
	  current_area->set_instance(new_instance);
	  while(current_area->get_next().get()){
	    current_area = current_area->get_next().get();
	    current_area->set_instance(new_instance);
	  }
	  if(new_instance->is_empty()){
	    new_instance->set_first(start_area);
	  }else{
	    new_instance->get_last().get()->set_next(start_area); //concatenation
	  }
	  new_instance->set_last(current_area);
	}
      }else{
	//there is no instance with the same label present in new_bmsc
	//add the instance to the new_bmsc
	new_bmsc_ptr.get()->add_instance(*inst_it);
	(*inst_it).get()->set_original(NULL); //there can be more originals of this instance
      }
    }
  }


  //move m_modified_elements from old duplicator to this duplicator, except copy_bmsc
  remaining_elements.clear();
  duplicator_modified_elements = duplicator2.get_m_modified_elements();
  for(f=duplicator_modified_elements.begin();f!=duplicator_modified_elements.end();f++)
  {
    if(dynamic_cast<Instance*>(*f) || dynamic_cast<BMsc*>(*f)){
      remaining_elements.push_back(*f);
    }else{
      m_modified_elements.push_back(*f);
    }
  }
  duplicator2.set_m_modified_elements(remaining_elements);
  duplicator2.cleanup_attributes();

  
  return new_bmsc_ptr;
}

MscElement* HMscFlatPathToBMscDuplicator::get_copy_with_occurence(MscElement* element, int occurence){  //TODO get_copy(,) did not work for some reason ???
   MscElement* my_element = element;
   int my_occurence = occurence;
   while(my_occurence){
     if(my_element == NULL)
       return NULL;
     my_element = this->get_copy(my_element);
     my_occurence--;
   }
   return my_element;
}


HMscFlatPathToBMscDuplicator::~HMscFlatPathToBMscDuplicator()
{
}

// $Id: msc_duplicators.cpp 1036 2011-02-09 11:05:45Z lkorenciak $
