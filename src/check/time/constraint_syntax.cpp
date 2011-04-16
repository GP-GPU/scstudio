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
 * $Id: constraint_syntax.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "constraint_syntax.h"
#include "data/dfs_refnode_hmsc_traverser.h"
#include "check/pseudocode/utils.h"

 void ConstraintsChecker::on_new_inner_bmsc_found(BMscPtr bmsc, ReferenceNode* refNode)
  {
    std::wstring name = (bmsc.get())->get_label();
    BMscConstraintCheck bmsc_checker;
    if(bmsc_checker.check_copy(bmsc))
    {
      m_changed=true;
      wrong_msc.push(bmsc.get());
    }
  }

  void ConstraintsChecker::on_new_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* refNode)
  {

    HMscConstraintCheck hmsc_checker("HConstraintsChecker a", "HConstraintsChecker b");
    if(hmsc_checker.check_copy(hmsc))
    {
      m_changed=true;
      wrong_msc.push(hmsc.get());
    }
  }

std::list<HMscPtr> ConstraintsChecker::check(HMscPtr hmsc, ChannelMapperPtr mapper)
  {
    std::list<HMscPtr> result;
    // empty hmsc handler
    if(!hmsc.get())
      return result;
    m_changed = false;
    
    DFSHMscFlatTraverser traverser("ConstraintsChecker");
    DFSHMscsTraverse tra(&traverser);
    m_tra = &tra;
    traverser.add_inner_bmsc_listener(this);
    traverser.add_inner_hmsc_listener(this);

    m_original = hmsc;
    HMscDuplicator duplicator;
    m_copy = duplicator.duplicate(m_original);

    inner_hmsc_found(m_copy,NULL);
    tra.traverse(m_copy);
    if(!m_changed)
	  {
      return result;
	  }

    mark();
    m_tra=NULL;
		if(m_copy != NULL)
	    result.push_back(m_copy);
    return result;
  }

  std::list<BMscPtr> ConstraintsChecker::check(BMscPtr bmsc, ChannelMapperPtr mapper)
  {
	std::list<BMscPtr> result;
    BMscPtr ret;
    BMscConstraintCheck checker;
    ret = checker.check(bmsc);
		if(ret != NULL)
	    result.push_back(ret);
    return result;
  }

  void ConstraintsChecker::mark()
  {
    while(!wrong_msc.empty())
    {
      Msc* msc = wrong_msc.top();
      wrong_msc.pop();
      mark(msc);
    }
  }
  void ConstraintsChecker::mark(ReferenceNode* n)
  {
    if(n->get_marked() == MARKED)
      return;
    n->set_marked(MARKED);
    std::list<Msc*> parents;
    std::list<Msc*>::iterator it;
    parents = m_tra->get_parents(n);

    for(it=parents.begin();it!=parents.end();it++)
    {
      mark(*it);
    }
  }
  void ConstraintsChecker::mark(Msc* msc)
  {
    std::list<ReferenceNode*> parents;
    std::list<ReferenceNode*>::iterator it;
    
    parents =m_tra->get_parents(msc);
    for(it=parents.begin();it!=parents.end();it++)
    {
      mark(*it);
    }
  }

////////////////////////////////////////////

HMscConstraintCheck::HMscConstraintCheck(
  const std::string& local
  ,const std::string& first_find
  ):
  m_local_active_constraints(local),m_first_find_with(first_find)
{}

void HMscConstraintCheck::add_local_active_constraints(ReferenceNode* n)
  {
    n->set_attribute<TimeRelationRefNodePtrSet>(m_local_active_constraints,m_active_constraints);
    to_be_clean.push(n);
  }

 TimeRelationRefNodePtrSet HMscConstraintCheck::get_local_active_constraints(ReferenceNode* n)
  {
    TimeRelationRefNodePtrSet empty;
    if(!(n->is_attribute_set(m_local_active_constraints)))
      return empty;
    else
      return n->get_attribute<TimeRelationRefNodePtrSet>(m_local_active_constraints,empty);
  }

    ///
  void HMscConstraintCheck::set_first_find_with(TimeRelationRefNode* n, ReferenceNodePtr ref)
  {
    n->set_attribute<ReferenceNodePtr>(m_first_find_with,ref);
    to_be_clean.push(n);
  }

  ///
  ReferenceNodePtr HMscConstraintCheck::get_first_find_with(TimeRelationRefNode* n)
  {
    ReferenceNodePtr ref;
    if(!(n->is_attribute_set(m_first_find_with)))
      return ref.get();
    else
      return n->get_attribute<ReferenceNodePtr>(m_first_find_with,ref);
  }


void HMscConstraintCheck::mark_time_relations(const TimeRelationRefNodePtrSet& relations)
  {
    TimeRelationRefNodePtrSet::const_iterator it;
    for(it=relations.begin();it!=relations.end();it++)
      mark_time_relation(it->get());
  }

  void HMscConstraintCheck::mark_time_relation(TimeRelationRefNode* relation)
  {
    relation->set_marked(MARKED);
  }

/** updating global set of active constraints (m_active_constraints)
  *  constraints with the equal begin and end reference node are ignored
  * @Warning WHITENODE function
  */

void HMscConstraintCheck::check_active_set(
  ReferenceNode* n,
  TimeRelationRefNodePtrSet constraint_set
)
{
  TimeRelationRefNodePtrSet::iterator it;

  for(it=constraint_set.begin(); it!=constraint_set.end(); it++)
  {
    TimeRelationRefNodePtr constraint = *it;

    // ignore constraints with the equal begin and end reference node
    if(constraint->get_ref_node_a()==constraint->get_ref_node_b())
      continue;
    TimeRelationRefNodePtrSet::iterator found_constraint;

    found_constraint=m_active_constraints.find(constraint);

    if(found_constraint==m_active_constraints.end())
    {
      set_first_find_with(constraint.get(),n);
      m_active_constraints.insert(constraint);
    }
    else
    {
//       if(get_first_find_with(constraint.get()).get()==n)
//       {
//         if(constraint->get_ref_node_a()==constraint->get_ref_node_b())
//           continue; //time relation on one refnode
//         else
//           throw constraint.get(); // begin of refnode was refound?
//       // end of timerelation was found, remove it from the global set
//       }
//       else
      m_active_constraints.erase(found_constraint);
    }
  }
}

void HMscConstraintCheck::on_white_node_found(ReferenceNode* n)
{

  add_local_active_constraints(n);
  //TOP
    TimeRelationRefNodePtrSet constraint_set;
    constraint_set = n->get_time_relations_top();

    check_active_set(n,constraint_set);

    //BOTTOM
    constraint_set = n->get_time_relations_bottom();
    //  add_active_constraints_bottom(n);

    check_active_set(n,constraint_set);

  if(m_active_constraints.size()!=0 && IsEndingNode::check(n))
  {
    m_broken_rel.insert(m_active_constraints.begin(),m_active_constraints.end());
  }
}

void HMscConstraintCheck::on_gray_node_found(ReferenceNode* n)
{
  TimeRelationRefNodePtrSet constraints_set;

  constraints_set = get_local_active_constraints(n);
  if(constraints_set!=m_active_constraints)
  {
    std::set<TimeRelationRefNodePtr> error(constraints_set);
    std::set<TimeRelationRefNodePtr>::iterator it;
    std::set<TimeRelationRefNodePtr>::iterator inside;
    for(it=m_active_constraints.begin();it!=m_active_constraints.end();it++)
    {
      std::pair<std::set<TimeRelationRefNodePtr>::iterator,bool> is_there;
      is_there = error.insert(*it);
      if(!is_there.second)
      {
        error.erase(is_there.first);
      }
    }
    m_broken_rel.insert(error.begin(),error.end());
  }
}

void HMscConstraintCheck::on_black_node_found(ReferenceNode* n)
{
  TimeRelationRefNodePtrSet constraints_set;

  constraints_set = get_local_active_constraints(n);
  if(constraints_set!=m_active_constraints)
  {
    std::set<TimeRelationRefNodePtr> error(constraints_set);
    std::set<TimeRelationRefNodePtr>::iterator it;
    std::set<TimeRelationRefNodePtr>::iterator inside;
    for(it=m_active_constraints.begin();it!=m_active_constraints.end();it++)
    {
      std::pair<std::set<TimeRelationRefNodePtr>::iterator,bool> is_there;
      is_there = error.insert(*it);
      if(!is_there.second)
      {
        error.erase(is_there.first);
      }
    }
    m_broken_rel.insert(error.begin(),error.end());
  }
}

void HMscConstraintCheck::on_node_finished(ReferenceNode* n)
{
  // fix global set
  m_active_constraints = get_local_active_constraints(n);
}

void HMscConstraintCheck::pick_up_broken_rel()
{
  DFSRefNodeFlatHMscTraverser traverser("ref node tra");
  traverser.add_white_node_found_listener(this);
  traverser.add_gray_node_found_listener(this);
  traverser.add_black_node_found_listener(this);
  traverser.add_node_finished_listener(this);

  traverser.traverse(m_hmsc);
}
/*
HMscPtr HMscConstraintCheck::check(HMscPtr hmsc)
{
  m_hmsc = hmsc;
  HMscPtr p;
  pick_up_broken_rel();
  if(m_broken_rel.empty())
    return NULL;

  p = m_duplicator.duplicate(hmsc);
  std::set<TimeRelationRefNodePtr>::iterator it;
  std::set<TimeRelationRefNodePtr> copies;
  for(it=m_broken_rel.begin();it!=m_broken_rel.end();it++)
  {
    TimeRelationRefNodePtr copy;
    copy = static_cast<TimeRelationRefNode*>( m_duplicator.get_copy(it->get()));
    copies.insert(copy);
  }
  mark_time_relations(copies);

  return p;
}*/

bool HMscConstraintCheck::check_copy(HMscPtr hmsc)
{
  m_hmsc = hmsc;
  pick_up_broken_rel();

  if(m_broken_rel.empty())
    return false;

  mark_time_relations(m_broken_rel);

  return true;
}

void HMscConstraintCheck::cleanup_attributes()
  {
    while(!to_be_clean.empty())
    {
      MscElement* n = to_be_clean.top();
      n->remove_attribute<TimeRelationRefNodePtrSet>(m_local_active_constraints);
      n->remove_attribute<ReferenceNodePtr>(m_first_find_with);

      to_be_clean.pop();
    }
  }

///////////////////////////////////////////////////////////

void BMscConstraintCheck::pick_up_broken_rel()
{
  EventTopologyHandler events_top(m_bmsc);
    BMscAllTimeRelationPtrSet time_relations(m_bmsc);
    std::stack<TimeRelationEventPtr> broken_rel;
    std::set<TimeRelationEventPtr>::iterator rel;
    for(rel=time_relations.begin();rel!=time_relations.end();rel++)
    {
      Event* a = (*rel)->get_event_a();
      Event* b = (*rel)->get_event_b();
      if(!events_top.visual_is_leq(a,b)&&!events_top.visual_is_leq(b,a))
        m_broken_rel.push(*rel);
    }

}

BMscPtr BMscConstraintCheck::check(BMscPtr bmsc)
  {
    m_bmsc = bmsc;
    pick_up_broken_rel();
    if(m_broken_rel.empty())
      return NULL;
    else
    {
      BMscPtr bmsc_copy;
      BMscDuplicator duplicator;
      bmsc_copy = duplicator.duplicate_bmsc(bmsc);

      while(!m_broken_rel.empty())
      {
        duplicator.get_copy((m_broken_rel.top()).get())->set_marked(MARKED);
        m_broken_rel.pop();
      }
      return bmsc_copy;
    }
  }

// $Id: constraint_syntax.cpp 1029 2011-02-02 22:17:59Z madzin $
