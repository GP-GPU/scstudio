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
 * $Id: optionsdlg.h 615 2010-02-22 22:47:55Z gotthardp $
 */

#pragma once

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>

class COptionsDlg
  : public ATL::CDialogImpl<COptionsDlg>, public CWinDataExchange<COptionsDlg>
{
public:
  enum { IDD = IDD_CHECK_OPTIONS };
  CListViewCtrl m_checklist;
  CComboBox m_channel_type;
  CComboBox m_ouputtype;

protected:
BEGIN_DDX_MAP(COptionsDlg)
  DDX_CONTROL_HANDLE(IDC_CHECKLIST, m_checklist)
  DDX_CONTROL_HANDLE(IDC_CHANNELTYPE, m_channel_type)
  DDX_CONTROL_HANDLE(IDC_OUTPUTLEVEL, m_ouputtype)
END_DDX_MAP()

BEGIN_MSG_MAP(COptionsDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
  COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
END_MSG_MAP()

// Handler prototypes:
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    CenterWindow(GetParent());

    DoDataExchange();
    LoadRegistryData();

    return bHandled = FALSE;
  }

  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
  {
    if(wID == IDOK)
      SaveRegistryData();

    EndDialog(wID);
    return 0;
  }

  int LoadRegistryData();
  int SaveRegistryData();
};

static const DWORD DEFAULT_CHECKER_PRIORITY = 0;
static const DWORD DEFAULT_CHANNEL = 0;
static const DWORD DEFAULT_OUTPUT_LEVEL = 2;

// $Id: optionsdlg.h 615 2010-02-22 22:47:55Z gotthardp $
