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
 * $Id: time_consistency.cpp 1076 2011-04-05 15:35:43Z lkorenciak $
 */

#include "time_consistency.h"

const IntervalMatrix* FloydWarshall::tight(const IntervalMatrix& matrix)
throw(MscTimeInconsistencyException)
{
  IntervalMatrix* copy = new IntervalMatrix(matrix);
  reset_incon_position();
  for(unsigned k=0; k < matrix.size(); k++)
  {
    for(unsigned i=0; i < matrix.size(); i++)
    {
      for(unsigned j=0; j < matrix.size(); j++)
      {
        copy->operator()(i,j) = MscTimeIntervalD::interval_intersection(copy->operator()(i,j),copy->operator()(i,k)+copy->operator()(k,j));
        if(copy->operator()(i,j).is_empty())
	{
          set_incon_position(i,j); // save inconsistent position
          DEBUG(copy->operator()(i,j));
//          throw MscTimeInconsistencyException();
	  return NULL;
        }
      }
    }
  }
  return copy;
}

void MscSolveTCSP::init_simple_matrix(IntervalMatrix& m)
{
  m.resize(m_size);

  for(unsigned i=0;i<m_size;i++)
  {
    for(unsigned j=0;j<m_size;j++)
    {
      if(i==j)
        m(i,j)=MscTimeIntervalD(0,0);
      else
        m(i,j)=MscTimeIntervalD(-D::infinity(),D::infinity());
    }
  }
}


void MscSolveTCSP::insert_interval(IntervalMatrix m, unsigned int row, unsigned int col)
{
  const IntervalList& list = m_report->m_matrix_original(row,col).get_set();

  if(list.size()==0)
  {
    throw std::runtime_error("Empty time interval set (cell) was found in matrix.");
  }

  IntervalList::const_iterator it;
  for(it=list.begin();it!=list.end();it++)
  {
    // insert interval and its inverse version
    m(row,col)=*it;
    if(row!=col)
      m(col,row)=MscTimeIntervalD::interval_inverse(*it);

    move_forward(m,row,col);
  }
}

void MscSolveTCSP::move_forward(const IntervalMatrix& m, unsigned int row, unsigned int col)
{
  DEBUG_(row,col);
  // increase
  col++;
  if(col==m_size)
    col=++row;

  // do whats on the menu
  if(row==m_size)
  {
    if(!tight(m))
    {
      m_report->inconsis_mtxs.push_back(m);
      m_report->inconsis_indxs.push_back(m_tightener->incon_position_get());
    }
  }
  else
    insert_interval(m,row,col);
}

//return false if inconsistent
bool MscSolveTCSP::tight(const IntervalMatrix& to_tight)
{
//  DEBUG(to_tight);
  //tight
  const IntervalMatrix* tmp = m_tightener->tight(to_tight);
  if(tmp == NULL) return false;
//  DEBUG(tmp);
  // remember both
  m_report->m_number_of_consistent_choices++;
  //add results the "Result interval set matrix"
  for(unsigned g=0;g<m_size;g++)
    for(unsigned h=0;h<m_size;h++)
      m_report->m_matrix_result(g,h).insert(tmp->operator()(g,h));
  delete tmp;
  return true;
}

MscSolveTCSPReport MscSolveTCSP::solveTCSP(const IntervalSetMatrix matrix)
{
  // initialization
  MscSolveTCSPReport report(matrix);
  m_report= &report;

  m_size = matrix.size();

  if(m_size>0)
  {
    IntervalMatrix m;
    init_simple_matrix(m);
    // start whole process

    insert_interval(m,0,0);
  }

  return report;
}

// run solveTCSP(const IntervalSetMatrix matrix) for every component in component matrix
//store the reports
//from the combine the reports to make final_report
//return final_report
MscSolveTCSPComponentMatrixReport MscSolveTCSP::solveTCSP(const IntervalSetComponentMatrix matrix)
{
  MscSolveTCSP msc_solve;
  MscSolveTCSPComponentMatrixReport final_report(matrix);

  for(unsigned i =0;i<matrix.get_number_of_components();i++)
  {
    MscSolveTCSPReport temporary_report = msc_solve.solveTCSP(matrix.get_component(i));
    final_report.m_matrix_result.push_back_component(temporary_report.m_matrix_result);//TODO won't be faster to use pointers
    for(unsigned j=0;j<temporary_report.inconsis_indxs.size();j++)
    {
      final_report.inconsis_mtxs.push_back(std::make_pair<unsigned,IntervalMatrix>(i,temporary_report.inconsis_mtxs[j]));
      final_report.inconsis_indxs.push_back(temporary_report.inconsis_indxs[j]);
    }
    final_report.m_number_of_consistent_choices *= temporary_report.m_number_of_consistent_choices;   
  }
  return final_report;
}

///////////////////////////////////////////////////////////////////


// $Id: time_consistency.cpp 1076 2011-04-05 15:35:43Z lkorenciak $
