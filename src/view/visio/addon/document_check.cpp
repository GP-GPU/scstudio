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
 * Copyright (c) 2007-2010 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: document_check.cpp 933 2010-09-15 21:58:58Z gotthardp $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "document.h"
#include "optionsdlg.h"
#include "extract.h"

#include "data/msc.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>

struct CheckExecutionStatus
{
  CheckExecutionStatus()
    : executed(false), satisfied(false)
  { }

  bool executed;
  bool satisfied;
};
typedef std::vector<CheckExecutionStatus> CheckExecutionStatusList;

CheckerPtrList::const_iterator CDocumentMonitor::find_checker(const std::wstring& property_name) const
{
  for(CheckerPtrList::const_iterator cpos = m_checkers.begin();
    cpos != m_checkers.end(); cpos++)
  {
    if(_wcsicmp((*cpos)->get_property_name().c_str(), property_name.c_str()) == 0)
      return cpos;
  }

  return m_checkers.end();
}

struct check_priority_t
{
  typedef boost::edge_property_tag kind;
};

struct max_priority_t
{
  typedef boost::vertex_property_tag kind;
};

template<typename MaxPriorityMap, typename CheckPriorityMap>
class bfs_priority_visitor : public boost::default_bfs_visitor
{
public:
  bfs_priority_visitor(MaxPriorityMap mmap, CheckPriorityMap cmap)
   : m_max_priority_map(mmap), m_check_priority_map(cmap)
  { }

  // check-->dependency edge
  template<typename Edge, typename DependencyGraph>
  void examine_edge(Edge e, const DependencyGraph& g) const
  {
    // priority of the dependency
    int priority = boost::get(m_check_priority_map, e);
    // priority of the check
    int src_priority = boost::get(m_max_priority_map, boost::source(e, g));
    // effective priority is the lowest priority of [check priority, dependency priority]
    // note: higher priority is represented by lower numbers
    if(src_priority > priority)
      priority = src_priority;

    // priority of a dependent check is the highest effective priority of all dependencies
    int trg_priority = boost::get(m_max_priority_map, boost::target(e, g));
    if(trg_priority > priority)
      boost::put(m_max_priority_map, boost::target(e, g), priority);
    return;
  }

private:
  MaxPriorityMap m_max_priority_map;
  CheckPriorityMap m_check_priority_map;
};

VAORC CDocumentMonitor::OnMenuVerify(Visio::IVApplicationPtr vsoApp)
{
  // clear the verification report
  m_reportView->Reset();

  CDrawingExtractor extractor(m_reportView);
  Visio::IVPagePtr vsoPage = vsoApp->GetActivePage();

  MscPtr msc = extractor.extract_msc(vsoPage);
  if(msc == NULL)
  {
    m_reportView->Print(RS_ERROR,
      stringize() << "Verification failed. Graphical errors in the drawing.");
    return VAORC_FAILURE;
  }

  // initialize a list of user defined priorities
  // items listed in the same order as m_checkers
  RequiredCheckersList priorities;
  priorities.insert(priorities.begin(), m_checkers.size(), PrerequisiteCheck::PP_DISREGARDED);

  // walk through all installed checkers
  for(CheckerPtrList::const_iterator cpos = m_checkers.begin();
    cpos != m_checkers.end(); cpos++)
  {
    size_t icheck = cpos - m_checkers.begin();

    priorities[icheck] = GetRegistry<DWORD>(SCSTUDIO_REGISTRY_ROOT _T("\\Checks"),
      (*cpos)->get_property_name().c_str(), _T("Priority"), DEFAULT_CHECKER_PRIORITY);
  }

  // execute the check
  int status = run_checks(msc, priorities);

  if(status > 0)
  {
    m_reportView->Print(RS_NOTICE,
      stringize() << "Drawing successfully verified.");
  }
  else if(status < 0)
  {
    m_reportView->Print(RS_ERROR,
      stringize() << "Verification failed. Properties violated.");
  }
  else // if(status == 0)
  {
    m_reportView->Print(RS_NOTICE,
      stringize() << "No verification algorithms applicable. No properties verified.");
  }

  return VAORC_SUCCESS;
}

int CDocumentMonitor::run_checks(MscPtr& msc, const RequiredCheckersList& priorities)
{
  CheckExecutionStatusList status;
  // initialize status for each checker
  status.insert(status.begin(), m_checkers.size(), CheckExecutionStatus());

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
    // vertex properties
    boost::property<boost::vertex_color_t, boost::default_color_type,
    boost::property<max_priority_t, int> >,
    // edge properties
    boost::property<check_priority_t, int> > DependencyGraph;

  typedef boost::graph_traits<DependencyGraph>::vertex_descriptor Vertex;
  typedef boost::graph_traits<DependencyGraph>::edge_descriptor Edge;

  size_t main_index = m_checkers.size();
  // vertex 0..N-1 correspond to individual checkers
  // vertex N corresponds to the user required check
  DependencyGraph G(main_index+1);

  typedef boost::property_map<DependencyGraph, max_priority_t>::type MaxPriorityMap;
  typedef boost::property_map<DependencyGraph, check_priority_t>::type CheckPriorityMap;
  // vertex property describing priority of the check
  MaxPriorityMap max_priority_map = boost::get(max_priority_t(), G);
  // edge property describing priority of the dependency
  CheckPriorityMap check_priority_map = boost::get(check_priority_t(), G);

  // user checks are always required
  boost::put(max_priority_map, main_index, PrerequisiteCheck::PP_REQUIRED);
  // walk through the list of required checkers
  for(RequiredCheckersList::const_iterator cpos = priorities.begin();
    cpos != priorities.end(); cpos++)
  {
    size_t icheck = cpos - priorities.begin();
    // initialize the property
    boost::put(max_priority_map, icheck, PrerequisiteCheck::PP_DISREGARDED);

    if(*cpos < PrerequisiteCheck::PP_DISREGARDED)
    {
      // add dependency for each user required check
      std::pair<Edge,bool> res = boost::add_edge(main_index, icheck, G);
      boost::put(check_priority_map, res.first, *cpos);
    }

    Checker::PreconditionList preconditions = m_checkers[icheck]->get_preconditions(msc);
    // check the preconditions
    for(Checker::PreconditionList::const_iterator ppos = preconditions.begin();
      ppos != preconditions.end(); ppos++)
    {
      CheckerPtrList::const_iterator checker = find_checker(ppos->property_name);
      if(checker != m_checkers.end())
      {
        size_t idep = checker - m_checkers.begin();
        // add dependency to the graph
        // if edge (u,v) appears in the graph, then v comes before u in the ordering
        std::pair<Edge,bool> res = boost::add_edge(icheck, idep, G);
        boost::put(check_priority_map, res.first, ppos->priority);
      }
    }
  }

  bfs_priority_visitor<MaxPriorityMap, CheckPriorityMap>
    vis(max_priority_map, check_priority_map);
  // calculate priorities of dependent checks
  boost::breadth_first_search(G, boost::vertex(main_index, G), boost::visitor(vis));

  typedef std::vector<Vertex> Container;
  Container check_order;

  try
  {
    // calculate execution order
    // note: output iterator in reverse topological order
    boost::topological_sort(G, std::back_inserter(check_order));
  }
  catch(boost::not_a_dag)
  {
    m_reportView->Print(RS_ERROR, stringize() << "internal error: broken dependencies");
    return VAORC_FAILURE;
  }

  DWORD channel_type = GetRegistry<DWORD>(SCSTUDIO_REGISTRY_ROOT, NULL, _T("ChannelType"), DEFAULT_CHANNEL);
  switch(channel_type)
  {
    case 0:
    default:
      m_reportView->Print(RS_REPORT, _T("Use channel for each Sender-Receiver pair."));
      break;
    case 1:
      m_reportView->Print(RS_REPORT, _T("Use channel for each Sender-Receiver-Message pair."));
      break;
  }

  enum OutputLevel
  {
    OL_ERROR,
    OL_WARNING,
    OL_NOTIFY
  };
  OutputLevel output_level = (OutputLevel)GetRegistry<DWORD>(SCSTUDIO_REGISTRY_ROOT, NULL,
    _T("OutputLevel"), DEFAULT_OUTPUT_LEVEL);

  int executed_count = 0;
  int error_count = 0;

  // execute the checkers
  for(Container::const_iterator cpos = check_order.begin();
    cpos != check_order.end(); cpos++)
  {
    if(*cpos == main_index)
      continue;
    // do not execute disregarded checks
    if(boost::get(max_priority_map, *cpos) >= PrerequisiteCheck::PP_DISREGARDED)
      continue;

    CheckerPtr msc_checker = m_checkers[*cpos];
    bool all_preconditions = true;

    Checker::PreconditionList preconditions = msc_checker->get_preconditions(msc);
    // verify the preconditions
    for(Checker::PreconditionList::const_iterator ppos = preconditions.begin();
      ppos != preconditions.end(); ppos++)
    {
      CheckerPtrList::const_iterator checker = find_checker(ppos->property_name);
      if(checker != m_checkers.end())
      {
        size_t idep = checker - m_checkers.begin();

        if((!status[idep].executed || !status[idep].satisfied)
          && ppos->priority == PrerequisiteCheck::PP_REQUIRED)
        {
          all_preconditions = false;
        }
      }
      else
      {
        if(ppos->priority == PrerequisiteCheck::PP_REQUIRED)
          all_preconditions = false;

        m_reportView->Print(RS_WARNING, stringize() << "internal error: "
          << "skipping " << msc_checker->get_property_name()
          << " due to unknown dependency: " << ppos->property_name);
      }
    }

    ChannelMapperPtr channel_mapper;
    switch(channel_type)
    {
      case 0:
      default:
        channel_mapper = SRChannelMapperPtr(new SRChannelMapper());
        break;
      case 1:
        channel_mapper = SRMChannelMapperPtr(new SRMChannelMapper());
        break;
    }

    std::list<MscPtr> result;
    bool result_set = false;

    BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);
    BMscCheckerPtr bmsc_checker = boost::dynamic_pointer_cast<BMscChecker>(msc_checker);
    if(bmsc_checker != NULL && bmsc != NULL)
    {
      if(!all_preconditions)
      {
        m_reportView->Print(RS_WARNING, stringize() << msc->get_label() << ": "
          << msc_checker->get_property_name()
          << " skipped. Required prerequisites not satisfied.", msc_checker->get_help_filename());

        if(boost::get(max_priority_map, *cpos) == PrerequisiteCheck::PP_REQUIRED)
          error_count++;
        continue;
      }

      try
      {
        std::list<BMscPtr> bresult = bmsc_checker->check(bmsc, channel_mapper);

        for(std::list<BMscPtr>::const_iterator bpos = bresult.begin();
          bpos != bresult.end(); bpos++)
        {
          result.push_back(boost::dynamic_pointer_cast<Msc>(*bpos));
        }
        result_set = true;
      }
      catch(std::exception &exc)
      {
        m_reportView->Print(RS_ERROR, stringize() << msc->get_label() << ": "
          << msc_checker->get_property_name()
          << " skipped. Internal error: " << exc.what());

        if(boost::get(max_priority_map, *cpos) == PrerequisiteCheck::PP_REQUIRED)
          error_count++;
        continue;
      }
    }

    HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);
    HMscCheckerPtr hmsc_checker = boost::dynamic_pointer_cast<HMscChecker>(msc_checker);
    if(hmsc_checker != NULL && hmsc != NULL)
    {
      if(!all_preconditions)
      {
        m_reportView->Print(RS_WARNING, stringize() << msc->get_label() << ": "
          << msc_checker->get_property_name()
          << " skipped. Required prerequisites not satisfied.", msc_checker->get_help_filename());

        if(boost::get(max_priority_map, *cpos) == PrerequisiteCheck::PP_REQUIRED)
          error_count++;
        continue;
      }

      try
      {
        std::list<HMscPtr> hresult = hmsc_checker->check(hmsc, channel_mapper);

        for(std::list<HMscPtr>::const_iterator hpos = hresult.begin();
          hpos != hresult.end(); hpos++)
        {
          result.push_back(boost::dynamic_pointer_cast<Msc>(*hpos));
        }
        result_set = true;
      }
      catch(std::exception &exc)
      {
        m_reportView->Print(RS_ERROR, stringize() << msc->get_label() << ": "
          << msc_checker->get_property_name()
          << " skipped. Internal error: " << exc.what());

        if(boost::get(max_priority_map, *cpos) == PrerequisiteCheck::PP_REQUIRED)
          error_count++;
        continue;
      }
    }

    if(!result_set)
    {
      // checker is not compatible with the drawing
      status[*cpos].executed = false;
    }
    else if(!result.empty())
    {
      // check failed
      status[*cpos].executed = true;
      status[*cpos].satisfied = false;
      executed_count++;

      // report failure of
      //  - user required tests
      //  - required dependencies of required tests
      if(boost::get(max_priority_map, *cpos) == PrerequisiteCheck::PP_REQUIRED)
      {
        m_reportView->Print(RS_ERROR, stringize() << msc->get_label() << ": "
          << msc_checker->get_property_name()
          << " violated.", result, msc_checker->get_help_filename());
        error_count++;
      }

      // if enabled, report failure all executed tests
      if(boost::get(max_priority_map, *cpos) == PrerequisiteCheck::PP_RECOMMENDED
        && output_level >= OL_WARNING)
      {
        m_reportView->Print(RS_WARNING, stringize() << msc->get_label() << ": "
          << msc_checker->get_property_name()
          << " violated.", result, msc_checker->get_help_filename());
      }
    }
    else
    {
      // check succeeded
      status[*cpos].executed = true;
      status[*cpos].satisfied = true;
      executed_count++;

      // if enabled, report success of user required tests
      if(boost::get(max_priority_map, *cpos) == PrerequisiteCheck::PP_REQUIRED
        && output_level >= OL_NOTIFY)
      {
        m_reportView->Print(RS_NOTICE, stringize() << msc->get_label() << ": "
          << msc_checker->get_property_name()
          << " satisfied.", msc_checker->get_help_filename());
      }
    }

    msc_checker->cleanup_attributes();
  }

  if(error_count > 0)
    return -1; // failed
  else
    if(executed_count > 0)
      return 1; // correct
    else
      return 0; // nothing executed
}

// $Id: document_check.cpp 933 2010-09-15 21:58:58Z gotthardp $
