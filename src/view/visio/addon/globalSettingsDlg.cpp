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
 * $Id: globalSettingsDlg.cpp 860 2010-08-25 10:46:51Z mbezdeka $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "GlobalSettingsDlg.h"

CGlobalSettingsDlg::CGlobalSettingsDlg(Visio::IVApplicationPtr vsoApp, ATL::_U_STRINGorID title,UINT uStartPage, HWND hWndParent):
CPropertySheetImpl<CGlobalSettingsDlg>(title,uStartPage,hWndParent)
{
  m_bCentered = false;

  //Disable APPLY button and CONTEXT HELP button
  m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_NOCONTEXTHELP;
 
  //Property pages contructors
  page1 = boost::shared_ptr<CNumberingGlobalDlg>(new CNumberingGlobalDlg(vsoApp));
  page2 = boost::shared_ptr<CSnapGlobalDlg>(new CSnapGlobalDlg(vsoApp));
  page3 = boost::shared_ptr<CProtectionGlobalDlg>(new CProtectionGlobalDlg(vsoApp));

  //Adding pages
  page1->SetTitle(_T("Numbering")); AddPage(*page1);
  page2->SetTitle(_T("Snap && Glue")); AddPage(*page2);
  page3->SetTitle(_T("Protection")); AddPage(*page3);
}

CGlobalSettingsDlg::~CGlobalSettingsDlg()
{
}

void CGlobalSettingsDlg::OnShowWindow(BOOL bShowing, int nReason)
{
  if(bShowing && !m_bCentered)
  {
    m_bCentered = true;
    CenterWindow(m_psh.hwndParent);
  }
}

// $Id: globalSettingsDlg.cpp 860 2010-08-25 10:46:51Z mbezdeka $

