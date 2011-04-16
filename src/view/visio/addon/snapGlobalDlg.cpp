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
 * Copyright (c) 2010 Martin Bezdeka <mbezdeka@seznam.cz>
 *
 * $Id: snapGlobalDlg.cpp 979 2010-09-20 14:50:33Z obouda $
 */

#include "stdafx.h"
#include "snapGlobalDlg.h"
#include "errors.h"
#include "data/msc.h"

#include <htmlhelp.h>

CSnapGlobalDlg::~CSnapGlobalDlg()
{
  if(IsWindow())
    DestroyWindow();
}

BOOL CSnapGlobalDlg::OnInitDialog(HWND hWndFocus, LPARAM lParam)
{
  LoadRegistryData();
  //Move values in variables into controls
  DoDataExchange(FALSE);

  //Enable/disable controls on the dialog
  EnableControls(m_bEdgeTreatmentEnabled);

  return TRUE;
}

int CSnapGlobalDlg::OnApply()
{
  //Move values in controls to variables
  if(!DoDataExchange(TRUE))
    return PSNRET_INVALID;
  
  SaveRegistryData();
 
  return PSNRET_NOERROR;
}

void CSnapGlobalDlg::OnHelp()
{
  std::basic_string<TCHAR> path = GetVisioModulePath();
  path += _T("\\help\\frontend\\settings.html");
  ShellExecute(NULL, L"open", path.c_str() , NULL, NULL, SW_SHOWNORMAL);
}

int CSnapGlobalDlg::LoadRegistryData()
{
  TRACE(_T("CSnapGlobalDlg::LoadRegistryData() - Loading options from registry")); 
  //Set buttons check box to checked (for whole time)
  m_bButtonsEnabled = true;
  m_bKeysEnabled = GetRegistry<bool>(GetRegistryFolder(),  NULL, _T("KeysEnabled"), false);
  m_bSnapEnabled = GetRegistry<bool>(GetRegistryFolder(),  NULL, _T("SnapEnabled"), false);
  m_bEdgeTreatmentEnabled = GetRegistry<bool>(GetRegistryFolder(),  NULL, _T("EdgeTreatmentEnabled"), false);
  m_iRadioSelection = GetRegistry<int>(GetRegistryFolder(),  NULL, _T("SnapType"), 0);
  m_iEdgeRadioSelection = GetRegistry<int>(GetRegistryFolder(),  NULL, _T("EdgeTreatmentType"), 0);

  return 0;
}

int CSnapGlobalDlg::SaveRegistryData()
{
  TRACE(_T("CSnapGlobalDlg::LoadRegistryData() - Saving options to registry"));
  SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SnapEnabled"), m_bSnapEnabled);
  SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("KeysEnabled"), m_bKeysEnabled);
  SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("EdgeTreatmentEnabled"), m_bEdgeTreatmentEnabled);
  SetRegistry<int>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SnapType"), m_iRadioSelection);
  SetRegistry<int>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("EdgeTreatmentType"), m_iEdgeRadioSelection);

  return 0;
}

void CSnapGlobalDlg::EnableControls(bool bEnable)
{
  GetDlgItem(IDC_EDGE_PRESERVE_TYPE).EnableWindow(bEnable);
  GetDlgItem(IDC_EDGE_CHANGE_TYPE).EnableWindow(bEnable);
}

LRESULT CSnapGlobalDlg::OnBnClickedEdgeEnabled(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DoDataExchange(TRUE);
  EnableControls(m_bEdgeTreatmentEnabled);
	return 0;
}

// $Id: snapGlobalDlg.cpp 979 2010-09-20 14:50:33Z obouda $
