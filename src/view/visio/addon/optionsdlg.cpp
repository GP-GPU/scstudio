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
 * $Id: optionsdlg.cpp 632 2010-02-28 16:38:28Z gotthardp $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "optionsdlg.h"

#include "data/prerequisite_check.h"

int COptionsDlg::LoadRegistryData()
{
  // (1) configuration options
  // load the channel mapper
  m_channel_type.InsertString(0, _T("Sender-Receiver pair"));
  m_channel_type.InsertString(1, _T("Sender-Receiver-Message pair"));
  DWORD channel_type = GetRegistry<DWORD>(SCSTUDIO_REGISTRY_ROOT, NULL, _T("ChannelType"), DEFAULT_CHANNEL);
  m_channel_type.SetCurSel(channel_type);

  // load the output level
  m_ouputtype.InsertString(0, _T("Errors only"));
  m_ouputtype.InsertString(1, _T("Errors and warnings"));
  m_ouputtype.InsertString(2, _T("All results"));
  DWORD output_level = GetRegistry<DWORD>(SCSTUDIO_REGISTRY_ROOT, NULL, _T("OutputLevel"), DEFAULT_OUTPUT_LEVEL);
  m_ouputtype.SetCurSel(output_level);

  // (2) load checker priorities
  m_checklist.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
  m_checklist.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 175);

  HKEY hSubKey;
  if(RegOpenKeyEx(HKEY_CURRENT_USER, SCSTUDIO_REGISTRY_ROOT _T("\\Checks"),
    0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
  {
    return 1;
  }

  TCHAR achClass[MAX_PATH];  // buffer for class name 
  DWORD cchClassName = MAX_PATH; // size of class string 
  DWORD cSubKeys;            // number of subkeys 
  DWORD cbMaxSubKey;         // longest subkey size 
  DWORD cchMaxClass;         // longest class string 
  DWORD cValues;             // number of values for key 
  DWORD cchMaxValue;         // longest value name 
  DWORD cbMaxValueData;      // longest value data 
  DWORD cbSecurityDescriptor; // size of security descriptor 
  FILETIME ftLastWriteTime;  // last write time 
 
  // Get the class name and the value count. 
  RegQueryInfoKey(hSubKey,   // key handle 
    achClass,                // buffer for class name 
    &cchClassName,           // size of class string 
    NULL,                    // reserved 
    &cSubKeys,               // number of subkeys 
    &cbMaxSubKey,            // longest subkey size 
    &cchMaxClass,            // longest class string 
    &cValues,                // number of values for this key 
    &cchMaxValue,            // longest value name 
    &cbMaxValueData,         // longest value data 
    &cbSecurityDescriptor,   // security descriptor 
    &ftLastWriteTime);       // last write time 
 
  // enumerate the child keys, until RegEnumKeyEx fails
  for(int i = cSubKeys; i >= 0; i--) 
  { 
    TCHAR achKey[MAX_PATH]; 
    DWORD keyLength = MAX_PATH;

    LONG retCode = RegEnumKeyEx(hSubKey, 
      i, 
      achKey, 
      &keyLength, 
      NULL, 
      NULL, 
      NULL, 
      &ftLastWriteTime); 
    if(retCode == ERROR_SUCCESS) 
    {
      int item = m_checklist.InsertItem(0, achKey, 0);

      DWORD user_priority = GetRegistry<DWORD>(SCSTUDIO_REGISTRY_ROOT _T("\\Checks"),
        achKey, _T("Priority"), DEFAULT_CHECKER_PRIORITY);
      if(user_priority <= PrerequisiteCheck::PP_REQUIRED)
        m_checklist.SetCheckState(item, TRUE);
    } 
  }

  return 0;
}

int COptionsDlg::SaveRegistryData()
{
  // (1) store the configuration options
  DWORD channel_type = m_channel_type.GetCurSel();
  SetRegistry<DWORD>(HKEY_CURRENT_USER, SCSTUDIO_REGISTRY_ROOT, _T("ChannelType"), channel_type);
  DWORD output_level = m_ouputtype.GetCurSel();
  SetRegistry<DWORD>(HKEY_CURRENT_USER, SCSTUDIO_REGISTRY_ROOT, _T("OutputLevel"), output_level);

  // (2) store checker priorities
  for(int item = 0; item < m_checklist.GetItemCount(); item++)
  {
    TCHAR item_text[MAX_PATH];
    m_checklist.GetItemText(item, 0, item_text, MAX_PATH);

    PrerequisiteCheck::Priority user_priority =
      m_checklist.GetCheckState(item) ? PrerequisiteCheck::PP_REQUIRED : PrerequisiteCheck::PP_DISREGARDED;

    TCHAR subkey[MAX_PATH];
    _tcscpy(subkey, SCSTUDIO_REGISTRY_ROOT _T("\\Checks\\"));
    _tcscat(subkey, item_text);

    SetRegistry<DWORD>(HKEY_CURRENT_USER, subkey, _T("Priority"), user_priority);
  }

  return 0;
}

// $Id: optionsdlg.cpp 632 2010-02-28 16:38:28Z gotthardp $
