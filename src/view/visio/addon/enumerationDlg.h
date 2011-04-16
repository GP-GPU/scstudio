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

#pragma once

#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <string>
#include <sstream>
#include "data/msc.h"

class CEnumerationDlg
  : public ATL::CDialogImpl<CEnumerationDlg>, public CWinDataExchange<CEnumerationDlg>
{
public:
  CEnumerationDlg(int startIndex, int enumType, BSTR addition);
  enum { IDD = IDD_ENUMERATION_OPTIONS };
  enum NumberingType
  {
	  NT_NUMBERS = 0,
	  NT_GREEK,
	  NT_LETTER_SMALL,
	  NT_LETTER_BIG
  };

  CComboBox m_numbering;
  CEdit		m_addition;
  CEdit		m_index;

protected:
BEGIN_DDX_MAP(CEnumerationDlg)
	DDX_CONTROL_HANDLE(IDC_COMBO_NUMBERING, m_numbering); 
	DDX_CONTROL_HANDLE(IDC_ADDITION, m_addition);
	DDX_CONTROL_HANDLE(IDC_STARTING_INDEX, m_index);
END_DDX_MAP()

BEGIN_MSG_MAP(CEnumerationDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
  COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
END_MSG_MAP()

// Handler prototypes:

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

  int LoadRegistryData();
  int SaveRegistryData();

  int			numberingType;
  BSTR			addition;
  int			index;

public:
	  int			getNumberingType()	{ return numberingType; }
	  BSTR			getAddition()		{ return addition; }
	  int			getStartingIndex()	{ return index; }
};
