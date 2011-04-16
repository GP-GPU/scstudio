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
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "enumerationDlg.h"
#include "enumerateUtils.h"

CEnumerationDlg::CEnumerationDlg(int startIndex, int enumType, BSTR addition)
{
	this->index = startIndex;
	this->numberingType = enumType;
	this->addition = addition;
}

LRESULT CEnumerationDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CenterWindow(GetParent());

  DoDataExchange();
  LoadRegistryData();
  

  return bHandled = FALSE;
}

LRESULT CEnumerationDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  if(wID == IDOK)
  {
    DoDataExchange(TRUE);
    if(SaveRegistryData() == 0)
		EndDialog(wID);
  }
  else if(wID == IDCANCEL)
	EndDialog(wID);

  return 0;
}

int CEnumerationDlg::LoadRegistryData()
{
  CEnumerateUtils::fillComboWithTypes(m_numbering);
  m_numbering.SetCurSel(this->numberingType);

  m_addition.SetWindowText(addition);
  m_addition.SetLimitText(4);

  std::wstring indexString = stringize() << index;
  m_index.SetWindowText(indexString.c_str());
  m_index.SetLimitText(4);

  return 0;
}

int CEnumerationDlg::SaveRegistryData()
{
  numberingType =	m_numbering.GetCurSel();
  
  addition = 0;
  m_addition.GetWindowText(addition);
  if(addition == 0)
	  addition = _T("");

  if(!m_index.GetWindowTextLength())
  {
	  MessageBox(_T("Starting index is empty!"), _T("Error"), MB_OK | MB_ICONERROR);
	  return 1;
  }

  BSTR temp = 0;
  m_index.GetWindowText(temp);
  std::wstringstream ss(std::wstringstream::in | std::stringstream::out);
  ss << _bstr_t(temp);
  ss >> index;

  if(index <= 0)
  {
	  MessageBox(_T("Starting index must be in range of 1-9999!"), _T("Error"), MB_OK | MB_ICONERROR);
	  return 1;
  }

  if(index > 3999 && numberingType == 1)
  {
	  MessageBox(_T("Index too big! Use another type of enumeration. Roman index can be from 0-3999."),
				 _T("Error"),MB_OK | MB_ICONERROR);
	  return 1;
  }

  return 0;
}
