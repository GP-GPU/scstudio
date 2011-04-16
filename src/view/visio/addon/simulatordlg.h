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
 * $Id: simulatordlg.h 632 2010-02-28 16:38:28Z gotthardp $
 */

#pragma once

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>

class CSimulatorDlg
  : public ATL::CDialogImpl<CSimulatorDlg>, public CWinDataExchange<CSimulatorDlg>
{
public:
  enum { IDD = IDD_SIMULATOR_OPTIONS };
  float m_bin_width;
  float m_max_message_delay;

protected:
BEGIN_DDX_MAP(CSimulatorDlg)
  DDX_FLOAT(IDC_SIM_BIN_WIDTH, m_bin_width)
  DDX_FLOAT(IDC_SIM_MESSAGE_DELAY, m_max_message_delay)
END_DDX_MAP()

BEGIN_MSG_MAP(CSimulatorDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
  COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
END_MSG_MAP()

// Handler prototypes:
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

  int LoadRegistryData();
  int SaveRegistryData();
};

static const float DEFAULT_SIM_BIN_WIDTH = 1.0f; // [sec]
static const float DEFAULT_SIM_MESSAGE_DELAY = 10.0f; // [sec]

// $Id: simulatordlg.h 632 2010-02-28 16:38:28Z gotthardp $
