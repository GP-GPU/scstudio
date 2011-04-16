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
 * $Id: messagesequencedlg.cpp 775 2010-05-13 18:38:46Z mbezdeka $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "messagesequencedlg.h"
#include "errors.h"
#include "document.h"
#include "pageutils.h"

CMessageSequenceDlg::CMessageSequenceDlg(Visio::IVApplicationPtr vsoApp, double mousePosY, MsgSeqDirection initDir)
{
  m_vsoApp = vsoApp;
  Visio::IVPagePtr page = vsoApp->ActivePage;

  // we got the mouse coordinates in the internal units
  m_mousePosY  = CPageUtils::ConvertUnits(page, mousePosY, 0, visPageUnits);

  m_pageHeight = CPageUtils::GetPageHeight(page);
  m_direction = initDir;

  m_pageUnits = CPageUtils::GetPageUnits(page);
}

LRESULT CMessageSequenceDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CenterWindow(GetParent());

  LoadRegistryData();
  if (m_mousePosY > 0 && m_mousePosY < m_pageHeight) {
    m_start_pos_y = m_mousePosY;
    m_old_start_pos_y = m_start_pos_y;
  }

  DoDataExchange(false);

  UpdateControls();

  // units
  LPCTSTR unitsStr = CPageUtils::VisioUnitToString(m_pageUnits);
  for (int i=0; i < ARRAYSIZE(m_unitLabels); i++)
  {
    m_unitLabels[i].SetWindowText(unitsStr);
  }

  // input limits
  m_startPosYEdit.SetLimitText(10);
  m_verticalSpaceEdit.SetLimitText(10);
  m_verticalSpaceLeftRightEdit.SetLimitText(10);
  m_leftMessageEdit.SetLimitText(255);
  m_rightMessageEdit.SetLimitText(255);

  // radio buttons
  switch (m_direction)
  {
  case MSDIR_LEFT:
    m_dirLeftRdo.SetCheck(BST_CHECKED);
    m_dirLeftRdo.SetFocus();
    break;
  case MSDIR_RIGHT:
  default:
    m_dirRightRdo.SetCheck(BST_CHECKED);
    m_dirRightRdo.SetFocus();
    break;
  case MSDIR_LEFT_RIGHT:
    m_dirLeftRightRdo.SetCheck(BST_CHECKED);
    m_dirLeftRightRdo.SetFocus();
    break;
  case MSDIR_RIGHT_LEFT:
    m_dirRightLeftRdo.SetCheck(BST_CHECKED);
    m_dirRightLeftRdo.SetFocus();
    break;
  }

  switch (m_coregionTreatment)
  {
  case MSCOR_ERROR:
    m_coregionErrorRdo.SetCheck(BST_CHECKED);
    m_coregionConnectLineRdo.SetCheck(BST_CHECKED);
    break;
  case MSCOR_CONTINUE:
    m_coregionContinueRdo.SetCheck(BST_CHECKED);
    m_coregionConnectLineRdo.SetCheck(BST_CHECKED);
    break;
  case MSCOR_CONNECT_LINE:
  default:
    m_coregionOrderRdo.SetCheck(BST_CHECKED);
    m_coregionConnectLineRdo.SetCheck(BST_CHECKED);
    break;
  case MSCOR_CONNECT_SIDE_SIDE:
    m_coregionOrderRdo.SetCheck(BST_CHECKED);
    m_coregionConnectSideSideRdo.SetCheck(BST_CHECKED);
    break;
  }

  return bHandled = false;
}

void CMessageSequenceDlg::OnDataValidateError(UINT nCtrlID, BOOL bSave, _XData& data)
{
  CString sMsg;
 
  switch (nCtrlID) {
  case IDC_MS_START_POS_Y:
    sMsg = "Starting Y-position must be on the drawing.";
    break;
  case IDC_MS_VERTICAL_SPACE:
  case IDC_MS_VERTICAL_SPACE_LEFT_RIGHT:
    sMsg = "Wrong vertical space, messages must fit to the drawing.";
    break;
  case IDC_MS_LEFT_MESSAGE:
  case IDC_MS_RIGHT_MESSAGE:
    sMsg = "The message caption is too long.";
    break;
  default:
    sMsg = "Unknown error";
    TRACE("CMessageSequenceDlg::OnDataValidateError() does not have a validation error message for a control");
  }
  
  MessageBox(sMsg, _T("Message Sequence Error"), MB_ICONEXCLAMATION);
}

LRESULT CMessageSequenceDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  BOOL dataExchanged = FALSE;
  if(wID == IDOK)
  {
    dataExchanged = DoDataExchange(true);
    if (!dataExchanged) {
      return 0; // there were errors, stay open
    }
    SaveRegistryData();
  }

  EndDialog((wID == IDOK && dataExchanged));
  return 0;
}

void CMessageSequenceDlg::UpdateControls()
{
  m_leftMessageEdit.EnableWindow(m_direction != MSDIR_RIGHT);
  m_rightMessageEdit.EnableWindow(m_direction != MSDIR_LEFT);

  bool coregionOrdering = (m_coregionTreatment == MSCOR_CONNECT_LINE || m_coregionTreatment == MSCOR_CONNECT_SIDE_SIDE);
  m_coregionConnectLineRdo.EnableWindow(coregionOrdering);
  m_coregionConnectSideSideRdo.EnableWindow(coregionOrdering);
}

LRESULT CMessageSequenceDlg::OnDirectionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  if (m_dirLeftRdo.GetCheck() == BST_CHECKED)
    m_direction = MSDIR_LEFT;
  else if (m_dirRightLeftRdo.GetCheck() == BST_CHECKED)
    m_direction = MSDIR_RIGHT_LEFT;
  else if (m_dirLeftRightRdo.GetCheck() == BST_CHECKED)
    m_direction = MSDIR_LEFT_RIGHT;
  else
    m_direction = MSDIR_RIGHT;

  UpdateControls();
  return 0;
}

LRESULT CMessageSequenceDlg::OnCoregionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  if (m_coregionErrorRdo.GetCheck() == BST_CHECKED)
    m_coregionTreatment = MSCOR_ERROR;
  else if (m_coregionContinueRdo.GetCheck() == BST_CHECKED)
    m_coregionTreatment = MSCOR_CONTINUE;
  else
    if (m_coregionConnectSideSideRdo.GetCheck() == BST_CHECKED)
      m_coregionTreatment = MSCOR_CONNECT_SIDE_SIDE;
    else
      m_coregionTreatment = MSCOR_CONNECT_LINE;

  UpdateControls();
  return 0;
}

int CMessageSequenceDlg::LoadRegistryData()
{
  TRACE("CMessageSequenceDlg::LoadRegistryData() loading options from registry");

  Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, 0, visPageUnits)
  m_start_pos_y = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("StartPosY"), DEFAULT_MS_START_POS_Y));
  m_old_start_pos_y = m_start_pos_y;
  m_vertical_space = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("VerticalSpace"), DEFAULT_MS_VERTICAL_SPACE));
  m_vertical_space_left_right = _u(GetRegistry<double>(GetRegistryFolder(), NULL, _T("VerticalSpaceLeftRight"), DEFAULT_MS_VERTICAL_SPACE_LEFT_RIGHT));
#undef _u
  m_coregionTreatment = (MsgSeqCoregionTreatment)GetRegistry<int>(GetRegistryFolder(), NULL, _T("CoregionTreatment"), DEFAULT_MS_COREGION_TREATMENT);

  // FIXME: load message captions from the registry
  wcsncpy(m_left_msg_captions, DEFAULT_MS_LEFT_MESSAGE_CAPTION, 255);
  m_left_msg_captions[255] = '\0';
  wcsncpy(m_right_msg_captions, DEFAULT_MS_RIGHT_MESSAGE_CAPTION, 255);
  m_right_msg_captions[255] = '\0';
  /*
  TCHAR data[256];
  DWORD dataLen = 256*sizeof(TCHAR);
  const TCHAR* keyName = SCSTUDIO_REGISTRY_ROOT _T("\\MessageSequence") _T("LeftMessageCaption");
  DWORD ret = RegQueryValueEx(HKEY_CURRENT_USER, keyName, NULL, NULL, (LPBYTE) data, &dataLen);
  if (ret == ERROR_SUCCESS)
    wcsncpy(m_left_msg_captions, data, 255);
  else
    wcsncpy(m_left_msg_captions, DEFAULT_MS_LEFT_MESSAGE_CAPTION, 255);
  m_left_msg_captions[255] = '\0';

  dataLen = 256*sizeof(TCHAR);
  keyName = SCSTUDIO_REGISTRY_ROOT _T("\\MessageSequence") _T("RightMessageCaption");
  ret = RegQueryValueEx(HKEY_CURRENT_USER, keyName, NULL, NULL, (LPBYTE) data, &dataLen);
  if (ret == ERROR_SUCCESS)
    wcsncpy(m_right_msg_captions, data, 255);
  else
    wcsncpy(m_right_msg_captions, DEFAULT_MS_RIGHT_MESSAGE_CAPTION, 255);
  m_right_msg_captions[255] = '\0';
  */
  /*
  CString left = GetRegistry<>(GetRegistryFolder(), NULL, _T("LeftMessageCaption"), DEFAULT_MS_LEFT_MESSAGE_CAPTION);
  CString right = GetRegistry<>(GetRegistryFolder(), NULL, _T("RightMessageCaption"), DEFAULT_MS_RIGHT_MESSAGE_CAPTION);
  wcsncpy(m_left_msg_captions, left, 255);
  m_left_msg_captions[255] = '\0';
  wcsncpy(m_right_msg_captions, right, 255);
  m_right_msg_captions[255] = '\0';
  */

  return 0;
}

int CMessageSequenceDlg::SaveRegistryData()
{
  TRACE("CMessageSequenceDlg::SaveRegistryData() saving options to registry");

  Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, visPageUnits, 0)
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("StartPosY"), _u(m_start_pos_y));
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("VerticalSpace"), _u(m_vertical_space));
  SetRegistry<double>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("VerticalSpaceLeftRight"), _u(m_vertical_space_left_right));
#undef _u
  SetRegistry<int>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("CoregionTreatment"), m_coregionTreatment);

  // FIXME: store message captions to the registry
  /*
  CString left = m_left_msg_captions;
  CString right = m_right_msg_captions;
  SetRegistry<CString>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("LeftMessageCaption"), left);
  SetRegistry<CString>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("RightMessageCaption"), right);
  */

  return 0;
}

// $Id: messagesequencedlg.cpp 775 2010-05-13 18:38:46Z mbezdeka $
