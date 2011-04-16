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
 * $Id: constraint_syntax.h 1029 2011-02-02 22:17:59Z madzin $
 */

#include <string>
#include <stack>
#include <set>
#include <exception>

#include "check/time/export.h"
#include "data/dfs_hmsc_flat_traverser.h"
#include "data/checker.h"
#include "data/msc.h"
#include "check/pseudocode/msc_duplicators.h"
#include "check/pseudocode/utils.h"
#include "check/time/time_pseudocode.h"

class HMscConstraintCheck;
class BMscConstraintCheck;

typedef boost::shared_ptr<HMscConstraintCheck> HMscConstraintCheckPtr;

/*
class ConstrainException:public std::exception
{

public:
  virtual const char* what() const throw()
  {
    return "Inconsistent constrain path found";
  }
};
*/

class BMscConstraintCheck;
class HMscConstraintCheck;

class SCTIME_EXPORT ConstraintsChecker:
  public Checker
  , public HMscChecker
  , public BMscChecker
  , public InnerBMscFoundListener
  , public InnerHMscFoundListener
{
private:
  HMscPtr m_copy;
  HMscPtr m_original;
  DFSHMscsTraverse* m_tra;

  bool m_changed;
  std::stack<Msc*> wrong_msc;

  // marking functions
  void mark();
  void mark(ReferenceNode* n);
  void mark(Msc* msc);

public:
  ConstraintsChecker(): m_changed(false)
  {}

  std::list<BMscPtr> check(BMscPtr, ChannelMapperPtr);
  std::list<HMscPtr> check(HMscPtr, ChannelMapperPtr);


  void on_new_inner_bmsc_found(BMscPtr bmsc, ReferenceNode* refNode);
  void on_new_inner_hmsc_found(HMscPtr hmsc, ReferenceNode* refNode);

  std::wstring get_property_name() const
  {
    return L"Correct Time Constraint Syntax";
  }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"time_syntax/time_syntax.html"; }

Checker::PreconditionList get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList precon_list;
  // no preconditions
  precon_list.push_back(PrerequisiteCheck(L"Acyclic", PrerequisiteCheck::PP_REQUIRED));
  return precon_list;
}

  void cleanup_attributes(){}

  bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }

};


class HMscConstraintCheck:
    public WhiteRefNodeFoundListener
  , public GrayRefNodeFoundListener
  , public BlackRefNodeFoundListener
  , public RefNodeFinishedListener
{
private:
  std::string m_local_active_constraints; ///< attr of local set of constraints
  std::string m_first_find_with; /// attr of TimeRelation, RefNode first find with
  std::stack<MscElement*> to_be_clean; ///< stack of items with atribute
  std::set<TimeRelationRefNodePtr> m_active_constraints; ///< global set of active constrains
  HMscDuplicator m_duplicator;
  std::set<TimeRelationRefNodePtr> m_broken_rel;
  HMscPtr m_hmsc;

/////// BEGIN attribute handler
  /// add global set of open time constraints to the ReferenceNode
  void add_local_active_constraints(ReferenceNode* n);
  /// get set of local active constraints from ReferenceNode
  TimeRelationRefNodePtrSet get_local_active_constraints(ReferenceNode* n);
  void set_first_find_with(TimeRelationRefNode* n, ReferenceNodePtr ref);
  ReferenceNodePtr get_first_find_with(TimeRelationRefNode* n);
//////// END attribute handler

  ///
  void check_active_set(ReferenceNode* n, TimeRelationRefNodePtrSet set);
  void pick_up_broken_rel();
protected:

  //! Marking functions to highlight (set_marked(true)) in visio
  void mark_time_relations(const TimeRelationRefNodePtrSet&);
  void mark_time_relation(TimeRelationRefNode*);

public:

  HMscConstraintCheck(
    const std::string& local="constraints checker, local constrains"
    ,const std::string& first_find="constraints checker, first find with"
  );

  ~HMscConstraintCheck()
  {
    cleanup_attributes();
  }

  /// Checks whether hmsc satisfy deadlock free property.
//  HMscPtr check(HMscPtr hmsc);
  bool check_copy(HMscPtr hmsc);

  void on_white_node_found(ReferenceNode*);
  void on_gray_node_found(ReferenceNode*);
  void on_black_node_found(ReferenceNode*);
  void on_node_finished(ReferenceNode*);

  void cleanup_attributes();
};

class BMscConstraintCheck
{
private:
  std::stack<TimeRelationEventPtr> m_broken_rel;
  BMscPtr m_bmsc;
  void pick_up_broken_rel();
public:
// Checker interface

  BMscPtr check(BMscPtr bmsc);
  //bool - incorrect relation has been found
  bool check_copy(BMscPtr bmsc)
  {
    m_bmsc = bmsc;
    pick_up_broken_rel();
    if(m_broken_rel.empty())
      return false;

    while(!m_broken_rel.empty())
    {
      ((m_broken_rel.top()).get())->set_marked(MARKED);
      m_broken_rel.pop();
    }

    return true;
  }

  ~BMscConstraintCheck()
  {
  }
};

// $Id: constraint_syntax.h 1029 2011-02-02 22:17:59Z madzin $
