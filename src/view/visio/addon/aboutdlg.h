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
 * $Id: aboutdlg.h 546 2010-01-10 14:47:31Z gotthardp $
 */

#pragma once

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
#include <atldlgs.h>
#include <atlctrls.h>

class CAboutDlg : public ATL::CDialogImpl<CAboutDlg>
{
public:
  enum { IDD = IDD_ABOUTBOX };

  std::wstring m_version;

protected:
BEGIN_MSG_MAP(CAboutDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
END_MSG_MAP()

// Handler prototypes:
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    CenterWindow(GetParent());
    ::SetWindowText(GetDlgItem(IDC_ABOUT_VERSION), m_version.c_str());
    return TRUE;
  }

  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
  {
    EndDialog(wID);
    return 0;
  }
};

// $Id: aboutdlg.h 546 2010-01-10 14:47:31Z gotthardp $
