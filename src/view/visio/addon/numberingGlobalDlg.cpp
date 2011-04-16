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
 * $Id: numberingGlobalDlg.cpp 979 2010-09-20 14:50:33Z obouda $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "numberingGlobalDlg.h"
#include "enumerateUtils.h"
#include "document.h"

#include <htmlhelp.h>

BOOL CNumberingGlobalDlg::OnInitDialog(HWND hWndFocus, LPARAM lParam)
{
  //Loading registry data
  m_bAutoEnum = GetRegistry<bool>(GetRegistryFolder(),  NULL, _T("AutoEnum"), 0);			
  DoDataExchange(FALSE);	//update data from variables to fields

  CEnumerateUtils::fillComboWithTypes(m_numberingTypeCombo);
  m_addition.SetLimitText(4);

  //Number according to nearest msg OR nearest numbered msg
  if(GetRegistry<bool>(GetRegistryFolder(),  NULL, _T("AutoEnumType"), 0))
	  m_nearestNumberedMsg.SetCheck(BST_CHECKED);
  else
  {
	  m_nearestMsg.SetCheck(BST_CHECKED);
  	EnableNearestNumberedMsgControls(false);
  }

  //don't number OR use numbering style
  if(GetRegistry<bool>(GetRegistryFolder(),  NULL, _T("AutoEnumStyleEnabled"), 0))
	  m_useNumberingStyle.SetCheck(BST_CHECKED);
  else
	  m_dontNumber.SetCheck(BST_CHECKED);

  //Numbering type
  m_numberingTypeCombo.SetCurSel((m_iNumberingTypeCombo = GetRegistry<int>(GetRegistryFolder(),  NULL, _T("AutoEnumStyleNumType"), 0)));
  //Numbering index
  std::wstring strIndex = stringize() << (m_iIndex = GetRegistry<int>(GetRegistryFolder(),  NULL, _T("AutoEnumStyleIndex"), 1));
  m_index.SetWindowText(strIndex.c_str());
  
  //Addition string
  HKEY hKey;
  if(RegCreateKey(HKEY_CURRENT_USER, GetRegistryFolder(),&hKey) != ERROR_SUCCESS)
  {
	  MessageBox(_T("Failed load key for addition string!"));
	  return FALSE;
  }

  DWORD dwType = 0;
  wchar_t buffer[5] = {0};
  DWORD dwSize = sizeof(buffer);
  if(RegQueryValueEx(hKey,_T("AutoEnumStyleAddition"),0,&dwType, (LPBYTE)buffer, &dwSize) == ERROR_SUCCESS)
	  m_addition.SetWindowText((m_szAddition = _bstr_t(buffer)));
  else
	  m_addition.SetWindowText(m_szAddition = _T("."));
  
  RegCloseKey(hKey);

  //Decide whether enable controls
  EnableAllControls(m_bAutoEnum);

  return TRUE;
}

int CNumberingGlobalDlg::OnApply()
{
  BOOL bResult = DoDataExchange(true);

  if(!m_index.GetWindowTextLength())
  {
    MessageBox(_T("Starting index is empty!"), _T("Error"), MB_OK | MB_ICONERROR);
    return 1;
  }

  BSTR temp = 0;
  int index;
  m_index.GetWindowText(temp);
  std::wstringstream ss(std::wstringstream::in | std::stringstream::out);
  ss << _bstr_t(temp); 
  ss >> index;

  if(index <= 0 || index > 9999)
  {
    MessageBox(_T("Starting index must be in range of 1-9999!"), _T("Error"), MB_OK | MB_ICONERROR);
	  return PSNRET_INVALID;
  }
  if(index > 3999 && m_numberingTypeCombo.GetCurSel() == 1)
  {
    MessageBox(_T("Index too big! Use another type of enumeration. Roman index can be from 0-3999."),
	           _T("Error"),MB_OK | MB_ICONERROR);
	  return PSNRET_INVALID;
  }

  if(bResult)
  {
	  SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("AutoEnum"), m_bAutoEnum);
	  SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("AutoEnumType"),m_nearestNumberedMsg.GetCheck() == BST_CHECKED);
	  SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("AutoEnumStyleEnabled"),m_useNumberingStyle.GetCheck() == BST_CHECKED);
	  SetRegistry<int>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("AutoEnumStyleNumType"),m_numberingTypeCombo.GetCurSel());
	  SetRegistry<int>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("AutoEnumStyleIndex"),index);
  	
	  //Store addition string
	  m_addition.GetWindowTextW(temp);
	  if(!temp) temp = _T("");
	  HKEY hKey;
	  if(RegCreateKey(HKEY_CURRENT_USER, GetRegistryFolder(), &hKey) != ERROR_SUCCESS)
      MessageBox(_T("Failed to create key for addition string!"));
	  else if(RegSetValueEx(hKey,_T("AutoEnumStyleAddition"),0, REG_SZ, (CONST LPBYTE)temp, 2*sizeof(BSTR)) != ERROR_SUCCESS)
		  MessageBox(_T("Failed to set registry key for addition string!"));
	  RegCloseKey(hKey);

    //If numbering style changed, erase autoNumberingGroupID -> force creating new autoNumbering style
    if((_tcsicmp(temp,m_szAddition) != 0) || (m_iIndex != index) || m_iNumberingTypeCombo != m_numberingTypeCombo.GetCurSel())
      SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("AutoEnumStyleIndexChanged"), true);

	  return PSNRET_NOERROR;
  }
	
  return PSNRET_INVALID;
}

void CNumberingGlobalDlg::OnHelp()
{
  std::basic_string<TCHAR> path = GetVisioModulePath();
  path += _T("\\help\\frontend\\message_numbering.html");
  ShellExecuteW(NULL, L"open", path.c_str() , NULL, NULL, SW_SHOWNORMAL);
}

void CNumberingGlobalDlg::EnableAllControls(bool bEnable)
{
  m_nearestMsg.EnableWindow(bEnable);
  m_nearestNumberedMsg.EnableWindow(bEnable);
  if(!bEnable || m_nearestNumberedMsg.GetCheck() == BST_CHECKED)
	EnableNearestNumberedMsgControls(bEnable);
}

void CNumberingGlobalDlg::EnableNearestNumberedMsgControls(bool bEnable)
{
  m_dontNumber.EnableWindow(bEnable);
  m_useNumberingStyle.EnableWindow(bEnable);
  if(!bEnable || m_dontNumber.GetCheck() != BST_CHECKED)
    EnableNumberingStyleControls(bEnable);
  else
    EnableNumberingStyleControls(!bEnable);
}	

void CNumberingGlobalDlg::EnableNumberingStyleControls(bool bEnable)
{
  m_numberingTypeCombo.EnableWindow(bEnable);
  m_addition.EnableWindow(bEnable);
  m_index.EnableWindow(bEnable);
}

LRESULT CNumberingGlobalDlg::OnBnClickedEnumCheck(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DoDataExchange(TRUE);
	EnableAllControls(m_bAutoEnum);

	return 0;
}

LRESULT CNumberingGlobalDlg::OnBnClickedNearestMsg(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  EnableNearestNumberedMsgControls(false);
  return 0;
}

LRESULT CNumberingGlobalDlg::OnBnClickedNearestNumberedMsg(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  EnableNearestNumberedMsgControls(true);
  return 0;
}

LRESULT CNumberingGlobalDlg::OnBnClickedDontNumber(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  EnableNumberingStyleControls(false);
  return 0;
}

LRESULT CNumberingGlobalDlg::OnBnClickedUseNumberingStyle(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  EnableNumberingStyleControls(true);
  return 0;
}

// $Id: numberingGlobalDlg.cpp 979 2010-09-20 14:50:33Z obouda $