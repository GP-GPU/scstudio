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
 * Copyright (c) 2010 Ondrej Bouda <ondrej.bouda@wheee.cz>
 *
 * $Id: instancesfielddlg.cpp 1040 2011-02-15 17:34:51Z mbezdeka $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "instancesfielddlg.h"
#include "errors.h"
#include "document.h"
#include "pageutils.h"

CInstancesFieldDlg::CInstancesFieldDlg(Visio::IVApplicationPtr vsoApp, double mousePosX, double mousePosY)
{
  m_vsoApp = vsoApp;
  Visio::IVPagePtr page = vsoApp->ActivePage;

  // we got the mouse coordinates in the internal units
  m_mousePosX  = CPageUtils::ConvertUnits(page, mousePosX, 0, visPageUnits);
  m_mousePosY  = CPageUtils::ConvertUnits(page, mousePosY, 0, visPageUnits);

  m_pageWidth  = CPageUtils::GetPageWidth(page);
  m_pageHeight = CPageUtils::GetPageHeight(page);

  m_pageUnits = CPageUtils::GetPageUnits(page);
}

void CInstancesFieldDlg::OnDataValidateError(UINT nCtrlID, BOOL bSave, _XData& data)
{
  CString sMsg = "";
 
  switch (nCtrlID) {
  case IDC_IF_INSTANCES_CNT:
    sMsg.Format(_T("The number of instances must be between %d and %d."),
                data.intData.nMin, data.intData.nMax );
    break;
  default:
    TRACE("CInstancesFieldDlg::OnDataValidateError() does not have a validation error message for a control");
  }

  MessageBox(sMsg, _T("Add Instances Error"), MB_ICONEXCLAMATION);
}

LRESULT CInstancesFieldDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CenterWindow(GetParent());

  LoadRegistryData();
  if (m_mousePosX > 0 && m_mousePosY > 0) {
    m_start_pos_x = m_mousePosX;
    m_start_pos_y = m_mousePosY;
  }
  DoDataExchange(false);

  UpdateSpacingControls();

  // units
  LPCTSTR unitsStr = CPageUtils::VisioUnitToString(m_pageUnits);
  for (int i=0; i < ARRAYSIZE(m_unitLabels); i++)
  {
    m_unitLabels[i].SetWindowText(unitsStr);
  }

  m_instancesCntEdit.SetSel(0, -1);

  return bHandled = false;
}

LRESULT CInstancesFieldDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  BOOL dataExchanged = FALSE;
  if(wID == IDOK)
  {
    dataExchanged = DoDataExchange(true);
    if (dataExchanged) {
      SaveRegistryData();
    }
  }

  EndDialog((wID == IDOK && dataExchanged));
  return 0;
}

void CInstancesFieldDlg::UpdateSpacingControls()
{
  m_totalWidthRdo.SetCheck(m_use_const_spacing ? BST_UNCHECKED : BST_CHECKED);
  m_spacingRdo.SetCheck(m_use_const_spacing ? BST_CHECKED : BST_UNCHECKED);
  m_totalWidthEdit.EnableWindow(!m_use_const_spacing);
  m_spacingEdit.EnableWindow(m_use_const_spacing);
}

LRESULT CInstancesFieldDlg::OnTotalWidthRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  m_use_const_spacing = false;
  UpdateSpacingControls();
  return 0;
}

LRESULT CInstancesFieldDlg::OnSpacingRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  m_use_const_spacing = true;
  UpdateSpacingControls();
  return 0;
}

LRESULT CInstancesFieldDlg::OnDefaultCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  m_start_pos_x = 0.0f;
  m_start_pos_y = m_pageHeight - CPageUtils::ConvertUnits(m_vsoApp->ActivePage, IF_BORDER, 0, visPageUnits);
  m_total_width = (m_pageWidth > 0 ?
    m_pageWidth :
    CPageUtils::ConvertUnits(m_vsoApp->ActivePage, DEFAULT_IF_TOTAL_WIDTH, 0, visPageUnits)
    );
  m_instance_length = (m_pageHeight > 0 ?
    m_pageHeight - 2 * CPageUtils::ConvertUnits(m_vsoApp->ActivePage, IF_BORDER, 0, visPageUnits) :
    CPageUtils::ConvertUnits(m_vsoApp->ActivePage, DEFAULT_IF_INSTANCE_LENGTH, 0, visPageUnits)
    );

  DoDataExchange(false);

  m_use_const_spacing = false;
  UpdateSpacingControls();

  m_instancesCntEdit.SetSel(0, -1);
  return 0;
}

int CInstancesFieldDlg::LoadRegistryData()
{
  TRACE("CInstancesFieldDlg::LoadRegistryData() loading options from registry");

  Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, 0, visPageUnits)
  m_instances_cnt     =    GetRegistry<int>  (GetRegistryFolder(),  NULL, _T("InstancesCnt"), DEFAULT_IF_INSTANCES_CNT);
  m_instance_length   = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("InstanceLength"), DEFAULT_IF_INSTANCE_LENGTH));
  m_instance_width    = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("InstanceWidth"), DEFAULT_IF_INSTANCE_WIDTH));
  m_start_pos_x       = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("StartPosX"), DEFAULT_IF_START_POS_X));
  m_start_pos_y       = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("StartPosY"), DEFAULT_IF_START_POS_Y));
  m_use_const_spacing =    GetRegistry<bool> (GetRegistryFolder(),  NULL, _T("SpacingSwitch"),  DEFAULT_IF_SPACING_SWITCH);
  m_total_width       = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("TotalWidth"), DEFAULT_IF_TOTAL_WIDTH));
  m_spacing           = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("Spacing"), DEFAULT_IF_SPACING));
#undef _u

  return 0;
}

int CInstancesFieldDlg::SaveRegistryData()
{
  TRACE("CInstancesFieldDlg::SaveRegistryData() saving options to registry");

  Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, visPageUnits, 0)
  SetRegistry<int>   (HKEY_CURRENT_USER, GetRegistryFolder(), _T("InstancesCnt"),      m_instances_cnt);
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("InstanceLength"), _u(m_instance_length));
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("InstanceWidth"),  _u(m_instance_width));
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("StartPosX"),      _u(m_start_pos_x));
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("StartPosY"),      _u(m_start_pos_y));
  SetRegistry<bool>  (HKEY_CURRENT_USER, GetRegistryFolder(), _T("SpacingSwitch"),     m_use_const_spacing);
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("TotalWidth"),     _u(m_total_width));
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("Spacing"),        _u(m_spacing));
#undef _u

  return 0;
}

// $Id: instancesfielddlg.cpp 1040 2011-02-15 17:34:51Z mbezdeka $
