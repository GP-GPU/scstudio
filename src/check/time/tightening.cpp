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
 * $Id: tightening.cpp 1071 2011-03-29 16:38:08Z lkorenciak $
 */

#include "check/time/tightening.h"

MscTimeIntervalSetD TightenBMsc::get_max_interval(BMscIntervalSetMatrix bmsc_matrix)
{
  MscTimeIntervalSetD max_intrset;
  MscTimeIntervalSetD evts_intrset;
  max_intrset.insert(MscTimeIntervalD(0,0));

  EventPairsList pairs;
  EventPList::iterator it_min;
  EventPList::iterator it_max;

    // for every pair (minimal,maximal) events find max_intrset
  for (it_min=m_mins.begin();it_min!=m_mins.end();it_min++)
  {
    for (it_max=m_maxs.begin();it_max!=m_maxs.end();it_max++)
    {
      evts_intrset=bmsc_matrix(*it_min,*it_max);
      max_intrset = MscTimeIntervalSetD::components_max(max_intrset,evts_intrset);
    }
  }
  return max_intrset;
}

ConstBMscMatrixPair TightenBMsc::tighten_msc(MscTimeIntervalSetD interval)
{

  BMscIntervalSetMatrix old_matrix(m_matrix);
  BMscIntervalSetMatrix current_matrix(m_matrix);

    // pair (MscTimeIntervalSetD,IntervalSetMatrix)
  ConstBMscMatrixPair pair(std::make_pair(interval,m_bmsc));
  do
  {
    old_matrix = current_matrix;
      // tight matrix
  //  current_matrix = MscSolveTCSP::solve(current_matrix);
      // tight both matrix and interval with respect to each other
    pair = max_tightener(current_matrix,interval);
      // assign tightened interval
    interval = pair.first;
  }
  while (!IntervalMatrixFunc::is_equal(old_matrix,current_matrix));
  m_matrix = current_matrix;
  return pair; // tighten interval, bmsc matrix
}

ConstBMscMatrixPair TightenBMsc::max_tightener(BMscIntervalSetMatrix t_matrix
    ,MscTimeIntervalSetD interval
                             )
{
  MscTimeIntervalSetD max_intrset = get_max_interval(t_matrix);
    // assign tightened interval
  interval = MscTimeIntervalSetD::set_intersection(interval,max_intrset);
  EventPVector::iterator it_v;

    // for all a \in events
    // for all b \in a.get_next do
  for (it_v = m_events.begin();it_v!=m_events.end();it_v++)
  {
    EventPSet succs = EventFirstSuccessors::get(*it_v);
    EventPSet::iterator it_succ;

    for (it_succ=succs.begin();it_succ!=succs.end();it_succ++)
    {
      EventPList::iterator it_min_events;
      EventPList::iterator it_max_events;
      MscTimeIntervalSetD path_union;

      for (it_min_events=m_mins.begin();it_min_events!=m_mins.end();it_min_events++)
      {
        for (it_max_events=m_maxs.begin();it_max_events!=m_maxs.end();it_max_events++)
        {
          if (is_leq(*it_min_events,*it_v)&&is_leq(*it_succ,*it_max_events))
            continue;
          path_union=MscTimeIntervalSetD::set_union(
              path_union,t_matrix(*it_min_events,*it_max_events));
        }
      }

      MscTimeIntervalSetD tight_interval;
      tight_interval = MscTimeIntervalSetD::set_union(interval,
          MscTimeIntervalSetD::zero_max_interval(path_union,interval));
      MscTimeIntervalSetD prefix_interval;
      prefix_interval.insert(MscTimeIntervalD(0,0));

      for (it_min_events=m_mins.begin();it_min_events!=m_mins.end();it_min_events++)
      {
        prefix_interval=MscTimeIntervalSetD::components_max(
            t_matrix(*it_min_events,*it_v),prefix_interval);
      }

      MscTimeIntervalSetD suffix_interval;
      suffix_interval.insert(MscTimeIntervalD(0,0));

      for (it_max_events=m_maxs.begin();
           it_max_events!=m_maxs.end();
           it_max_events++)
      {
        suffix_interval=MscTimeIntervalSetD::components_max(
            t_matrix(*it_succ,*it_max_events),suffix_interval);
      }

        //final tightening
      t_matrix(*it_v,*it_succ)=MscTimeIntervalSetD::set_intersection(
                t_matrix(*it_v,*it_succ)
          ,tight_interval-prefix_interval-suffix_interval);

    } // end for succs
  } // end for all events

  return std::make_pair(interval,t_matrix);
}

//gets BMscPtr, creates a matrix for this BMsc, tightens the constraints in the matrix and returns modified BMsc
BMscPtr BMscTighter::transform(BMscPtr bmsc)
{
// version with normal matrix
//   BMscIntervalSetMatrix matrix(bmsc);
//   matrix.build_up();
//   MscSolveTCSP solve;
//   MscSolveTCSPReport report = solve.solveTCSP(matrix);
//   matrix = report.m_matrix_result;
//   return matrix.get_modified_bmsc();

//version with component matrix
  BMscIntervalSetComponentMatrix matrix(bmsc);
  matrix.build_up();
  MscSolveTCSP solve;
  MscSolveTCSPComponentMatrixReport report = solve.solveTCSP(matrix);
//  matrix = report.m_matrix_result; //isn't this useless and complicated?
  return matrix.get_modified_bmsc(report.m_matrix_result);
}

// $Id: tightening.cpp 1071 2011-03-29 16:38:08Z lkorenciak $
