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
 * $Id: snapGlobalDlg.h 861 2010-08-26 00:30:31Z mbezdeka $
 */

#pragma once

#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <atlmisc.h>

// include macro definition for registry folder
#include "dllmodule.h"

class CSnapGlobalDlg: public CPropertyPageImpl<CSnapGlobalDlg>, public CWinDataExchange<CSnapGlobalDlg>
{
public:
  enum { IDD = IDD_SNAP_GLOBAL };

  CSnapGlobalDlg(Visio::IVApplicationPtr vsoApp):m_vsoApp(vsoApp)
                        {   //set dialog properties
                          m_psp.dwFlags |= PSP_HASHELP;
                        };
  ~CSnapGlobalDlg();

protected:
  BEGIN_MSG_MAP(CSnapGlobalDlg)
    MSG_WM_INITDIALOG(OnInitDialog)
    COMMAND_HANDLER(IDC_EDGE_THREATMENT_ENABLED, BN_CLICKED, OnBnClickedEdgeEnabled)
    CHAIN_MSG_MAP(CPropertyPageImpl<CSnapGlobalDlg>)
  END_MSG_MAP()


  BEGIN_DDX_MAP(CSnapGlobalDlg)
    DDX_CHECK(IDC_BUTTONS_ENABLED, m_bButtonsEnabled);
    DDX_CHECK(IDC_KEYS_ENABLED, m_bKeysEnabled);
    DDX_CHECK(IDC_SNAP_ENABLED, m_bSnapEnabled);
    DDX_CHECK(IDC_EDGE_THREATMENT_ENABLED, m_bEdgeTreatmentEnabled);
    DDX_RADIO(IDC_STRAIGHTEN, m_iRadioSelection);
    DDX_RADIO(IDC_EDGE_PRESERVE_TYPE, m_iEdgeRadioSelection);
  END_DDX_MAP()

  int LoadRegistryData();
  int SaveRegistryData();

  //Member variables
  bool m_bKeysEnabled;
  bool m_bSnapEnabled;
  bool m_bButtonsEnabled;
  bool m_bOnlyOnLine;
  bool m_bEdgeTreatmentEnabled;
  int  m_iRadioSelection;
  int  m_iEdgeRadioSelection;

  Visio::IVApplicationPtr m_vsoApp;

  void EnableControls(bool bEnable = true);
  LRESULT OnBnClickedEdgeEnabled(WORD, WORD, HWND, BOOL&);

public:
  //Message handlers
  BOOL OnInitDialog(HWND hWndFocus, LPARAM lParam);

  //Property page notification handlers (override method)
  int OnApply();
  void OnHelp();

  //Registry folder
  const wchar_t* GetRegistryFolder() { return SCSTUDIO_REGISTRY_ROOT _T("\\MessageSnapping"); }
};

// $Id: snapGlobalDlg.h 861 2010-08-26 00:30:31Z mbezdeka $