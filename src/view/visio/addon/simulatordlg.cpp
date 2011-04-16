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
 * $Id: simulatordlg.cpp 632 2010-02-28 16:38:28Z gotthardp $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "simulatordlg.h"

LRESULT CSimulatorDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CenterWindow(GetParent());

  LoadRegistryData();
  DoDataExchange();

  return bHandled = FALSE;
}

LRESULT CSimulatorDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  if(wID == IDOK)
  {
    DoDataExchange(TRUE);
    SaveRegistryData();
  }

  EndDialog(wID);
  return 0;
}

int CSimulatorDlg::LoadRegistryData()
{
  m_bin_width = GetRegistry<float>(SCSTUDIO_REGISTRY_ROOT _T("\\Simulator"), NULL,
    _T("BinWidth"), DEFAULT_SIM_BIN_WIDTH);
  m_max_message_delay = GetRegistry<float>(SCSTUDIO_REGISTRY_ROOT _T("\\Simulator"), NULL,
    _T("MaxMessageDelay"), DEFAULT_SIM_MESSAGE_DELAY);

  return 0;
}

int CSimulatorDlg::SaveRegistryData()
{
  SetRegistry<float>(HKEY_CURRENT_USER, SCSTUDIO_REGISTRY_ROOT _T("\\Simulator"),
    _T("BinWidth"), m_bin_width);
  SetRegistry<float>(HKEY_CURRENT_USER, SCSTUDIO_REGISTRY_ROOT _T("\\Simulator"),
    _T("MaxMessageDelay"), m_max_message_delay);

  return 0;
}

// $Id: simulatordlg.cpp 632 2010-02-28 16:38:28Z gotthardp $
