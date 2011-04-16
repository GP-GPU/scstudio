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
 * $Id: globalSettingsDlg.h 860 2010-08-25 10:46:51Z mbezdeka $
 */

#pragma once

#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>

//dialogs for tabs
#include "numberingGlobalDlg.h"
#include "snapGlobalDlg.h"
#include "protectionGlobalDlg.h"

class CGlobalSettingsDlg : public CPropertySheetImpl<CGlobalSettingsDlg>
{
public:
  CGlobalSettingsDlg(Visio::IVApplicationPtr vsoApp, ATL::_U_STRINGorID title = (LPCTSTR) NULL,
    UINT uStartPage = 0, HWND hWndParent = NULL);

  ~CGlobalSettingsDlg();

  BEGIN_MSG_MAP(CGlobalSettingsDlg)
    MSG_WM_SHOWWINDOW(OnShowWindow)
    CHAIN_MSG_MAP(CPropertySheetImpl<CGlobalSettingsDlg>)
  END_MSG_MAP()

  //Message handlers
  void OnShowWindow(BOOL bShowing, int nReason);

protected:
  bool m_bCentered;

  //Pages of property sheet
  boost::shared_ptr<CNumberingGlobalDlg>  page1;
  boost::shared_ptr<CSnapGlobalDlg>       page2;
  boost::shared_ptr<CProtectionGlobalDlg> page3;

  Visio::IVApplicationPtr m_vsoApp;
};

// $Id: globalSettingsDlg.h 860 2010-08-25 10:46:51Z mbezdeka $