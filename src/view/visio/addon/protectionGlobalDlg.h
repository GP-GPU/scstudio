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
 * $Id: protectionGlobalDlg.h 860 2010-08-25 10:46:51Z mbezdeka $
 */

#pragma once

#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <atlmisc.h>

// include macro definition for registry folder
#include "dllmodule.h"

class CProtectionGlobalDlg: public CPropertyPageImpl<CProtectionGlobalDlg>, public CWinDataExchange<CProtectionGlobalDlg>
{
public:
  enum { IDD = IDD_PROTECTION_GLOBAL };

  CProtectionGlobalDlg(Visio::IVApplicationPtr vsoApp):m_vsoApp(vsoApp)
                        {   //set dialog properties
                          //m_psp.dwFlags |= PSP_HASHELP;
                        };
  ~CProtectionGlobalDlg();

protected:
  BEGIN_MSG_MAP(CProtectionGlobalDlg)
    MSG_WM_INITDIALOG(OnInitDialog)
    CHAIN_MSG_MAP(CPropertyPageImpl<CProtectionGlobalDlg>)
  END_MSG_MAP()


  BEGIN_DDX_MAP(CProtectionGlobalDlg)
    DDX_CHECK(IDC_DONT_ROTATE_CHECK, m_bRotateEnabled);
  END_DDX_MAP()

  int LoadRegistryData();
  int SaveRegistryData();

  //Member variables
  bool m_bRotateEnabled;

  Visio::IVApplicationPtr m_vsoApp;

public:
  //Message handlers
  BOOL OnInitDialog(HWND hWndFocus, LPARAM lParam);

  //Property page notification handlers (override method)
  int OnApply();
  void OnHelp();

  //Registry folder
  const wchar_t* GetRegistryFolder() { return SCSTUDIO_REGISTRY_ROOT _T("\\Protection"); }
};

// $Id: protectionGlobalDlg.h 860 2010-08-25 10:46:51Z mbezdeka $