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
 * $Id: protectionGlobalDlg.cpp 860 2010-08-25 10:46:51Z mbezdeka $
 */

#include "stdafx.h"
#include "protectionGlobalDlg.h"
#include "errors.h"
#include "data/msc.h"

#include <htmlhelp.h>

CProtectionGlobalDlg::~CProtectionGlobalDlg()
{
  if(IsWindow())
    DestroyWindow();
}

BOOL CProtectionGlobalDlg::OnInitDialog(HWND hWndFocus, LPARAM lParam)
{
  LoadRegistryData();
  //Move values in variables into controls
  DoDataExchange(FALSE);

  return TRUE;
}

int CProtectionGlobalDlg::OnApply()
{
  //Move values in controls to variables
  if(!DoDataExchange(TRUE))
    return PSNRET_INVALID;
  
  SaveRegistryData();
 
  return PSNRET_NOERROR;
}

void CProtectionGlobalDlg::OnHelp()
{
}

int CProtectionGlobalDlg::LoadRegistryData()
{
  TRACE(_T("CProtectionGlobalDlg::LoadRegistryData() - Loading options from registry")); 
  m_bRotateEnabled = GetRegistry<bool>(GetRegistryFolder(),  NULL, _T("PreventInstanceRotation"), false);

  return 0;
}

int CProtectionGlobalDlg::SaveRegistryData()
{
  TRACE(_T("CProtectionGlobalDlg::LoadRegistryData() - Saving options to registry"));
  SetRegistry<bool>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("PreventInstanceRotation"), m_bRotateEnabled);

  return 0;
}
// $Id: protectionGlobalDlg.cpp 860 2010-08-25 10:46:51Z mbezdeka $
