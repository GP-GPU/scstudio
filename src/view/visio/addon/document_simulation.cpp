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
 * $Id: document_simulation.cpp 632 2010-02-28 16:38:28Z gotthardp $
 */

#include "stdafx.h"
#include "addon.h"
#include "dllmodule.h"
#include "document.h"
#include "extract.h"

#include "data/msc.h"

static const int EXCEL_MAX_ROWS = 65536;
static const int EXCEL_MAX_COLUMNS = 256;

VAORC CDocumentMonitor::OnMenuSimulationStart(Visio::IVApplicationPtr vsoApp)
{
  // clear the verification report
  m_reportView->Reset();

  // step 1: extract the drawing to be simulated
  CDrawingExtractor extractor(m_reportView);
  Visio::IVPagePtr vsoPage = vsoApp->GetActivePage();

  MscPtr msc = extractor.extract_msc(vsoPage);
  if(msc == NULL)
  {
    m_reportView->Print(RS_ERROR,
      stringize() << "Simulation failed. Graphical errors in the drawing.");
    return VAORC_FAILURE;
  }

#if(EXCEL_FOUND)
  // step 2: initialize the connection to excel
  if(FAILED(m_excelApp.GetActiveObject("Excel.Application")))
  {
    m_reportView->Print(RS_ERROR,
      stringize() << "Simulation failed. Open Microsoft Excel to receive simulation results.");
    return VAORC_FAILURE;
  }

  // we may be connected to a hidden application
  m_excelApp->Visible = true;

  Excel::_WorkbookPtr workbook = m_excelApp->ActiveWorkbook;
  // if no workbook is active, create a new workbook
  if(workbook == NULL)
    workbook = m_excelApp->Workbooks->Add();

  m_outputSheet = workbook->ActiveSheet;
  // if no sheet is active, create a new sheet
  if(m_outputSheet == NULL)
    m_outputSheet = workbook->Sheets->Add();

  m_annotatedColumns.clear();
  // extract column names and search for the first empty column
  // note: first column lists values
  int next_column;
  for(next_column = 2; next_column < EXCEL_MAX_COLUMNS; next_column++)
  {
    _bstr_t name = m_outputSheet->Cells->Item[1][next_column];
    if(name.length() == 0)
      break;

    m_annotatedColumns[(const char *)name] = next_column;
  }

  m_annotatedRows = 0;
#endif // EXCEL_FOUND

  // step 3: start the simulation
  m_simulator->start(msc);

  // step 4: reconfigure menu
  m_simulationStartMenuItem->Visible = false;
  m_simulationStopMenuItem->Visible = true;
  m_vsoDocument->CustomMenus->UpdateUI();
  m_simulationStartToolbarItem->Visible = false;
  m_simulationStopToolbarItem->Visible = true;
  m_vsoDocument->CustomToolbars->UpdateUI();

  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuSimulationStop(Visio::IVApplicationPtr vsoApp)
{
  // step 1: stop the simulation
  m_simulator->stop();

  // step 2: disconnect the sheet
#if(EXCEL_FOUND)
  m_outputSheet = NULL;
  m_excelApp = NULL;
#endif

  // step 3: reconfigure menu
  m_simulationStartMenuItem->Visible = true;
  m_simulationStopMenuItem->Visible = false;
  m_vsoDocument->CustomMenus->UpdateUI();
  m_simulationStartToolbarItem->Visible = true;
  m_simulationStopToolbarItem->Visible = false;
  m_vsoDocument->CustomToolbars->UpdateUI();

  return VAORC_SUCCESS;
}

void CDocumentMonitor::OnSimulationResult()
{
#if(EXCEL_FOUND)
  for(MeasurementMap::DensityMap::const_iterator mpos = m_marker_measurements.density.begin();
    mpos != m_marker_measurements.density.end(); mpos++)
  {
    int column;

    // check the column
    AnnotatedColumns::const_iterator icol = m_annotatedColumns.find(mpos->first);
    if(icol == m_annotatedColumns.end())
    {
      column = m_annotatedColumns.size()+2;
      m_annotatedColumns[mpos->first] = column;

      m_outputSheet->Cells->Item[1][column] = mpos->first.c_str();
    }
    else
      column = icol->second;

    // check the row
    if(m_annotatedRows < mpos->second.size())
    {
      for(size_t i = m_annotatedRows; i < mpos->second.size(); i++)
        m_outputSheet->Cells->Item[i+2][1] = (double)i*m_marker_measurements.bin_width;

      m_annotatedRows = mpos->second.size();
    }

    for(size_t i = 0; i < mpos->second.size(); i++)
      m_outputSheet->Cells->Item[i+2][column] = mpos->second[i];
  }
#else
  for(MeasurementMap::DensityMap::const_iterator mpos = m_marker_measurements.density.begin();
    mpos != m_marker_measurements.density.end(); mpos++)
  {
    stringize report;
    report << mpos->first.c_str() << " =";

    for(size_t i = 0; i < mpos->second.size(); i++)
      stringize() << " " << mpos->second[i];

    m_reportView->Print(RS_REPORT, report);
  }
#endif // EXCEL_FOUND

  // loop forever until terminated by the user
  m_simulator->next();
}

void CDocumentMonitor::OnSimulationError()
{
  OnMenuSimulationStop(m_vsoApp);

  m_reportView->Print(RS_ERROR,
    stringize() << "Simulation failed. " << m_marker_error.c_str());
}

// $Id: document_simulation.cpp 632 2010-02-28 16:38:28Z gotthardp $
