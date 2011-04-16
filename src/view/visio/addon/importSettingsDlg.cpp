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
 * Copyright (c) 2010 Zuzana Pekarcikova <z.pekarcikova@gmail.com>
 *
 * $Id: $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "importSettingsDlg.h"
#include "errors.h"
#include "document.h"
#include "pageutils.h"





LRESULT CImportSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CenterWindow(GetParent());

	LoadRegistryData();
  //Move values in variables into controls
  DoDataExchange(FALSE);

  return bHandled = false;
}

LRESULT CImportSettingsDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
  BOOL dataExchanged = FALSE;
  if(wID == IDOK)
  {
    dataExchanged = DoDataExchange(true);
    if (dataExchanged) 
      SaveRegistryData();
		else
			return PSNRET_INVALID;
  }

  EndDialog((wID == IDOK && dataExchanged));
  return 0;
}

int CImportSettingsDlg::LoadRegistryData()
{
	TRACE("CBeautifySettingDlg::LoadRegistryData() - Loading options from registry"); 

	Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, 0, visPageUnits)

	m_incomplete_msg_value_imp	= GetRegistry<float>(GetRegistryFolder(), NULL, _T("IncompleteMsgValueImport"), 20);
	m_head_dist_value_imp				= GetRegistry<float>(GetRegistryFolder(), NULL, _T("HeadDistValueImport"), 5);
	m_begin_cor_dist_value_imp	= GetRegistry<float>(GetRegistryFolder(), NULL, _T("BeginCorDistValueImport"), 5);
	m_end_cor_dist_value_imp		= GetRegistry<float>(GetRegistryFolder(), NULL, _T("EndCorDistValueImport"), 5);
	m_slope_value_imp						= GetRegistry<float>(GetRegistryFolder(), NULL, _T("SlopeValueImport"), 0);
	m_successor_dist_value_imp	= GetRegistry<float>(GetRegistryFolder(), NULL, _T("SuccDistValueImport"), 5);
	m_foot_dist_value_imp				= GetRegistry<float>(GetRegistryFolder(), NULL, _T("FootDistValueImport"), 5);
	m_head_width_value_imp      = GetRegistry<float>(GetRegistryFolder(), NULL, _T("HeadValueImport"), 10);
	m_cor_width_value_imp				= GetRegistry<float>(GetRegistryFolder(), NULL, _T("CoregionValueImport"), 10);
	m_space_value_imp						= GetRegistry<float>(GetRegistryFolder(), NULL, _T("SpaceValueImport"), 30);
#undef _u

	return 0;
}

void CImportSettingsDlg::UpdateControls()
{
}

int CImportSettingsDlg::SaveRegistryData()
{
	TRACE("CBeautifySettingDlg::SaveRegistryData() - Saving options to registry"); 

	Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, 0, visPageUnits)
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("HeadValueImport"), (float)m_head_width_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("CoregionValueImport"), (float)m_cor_width_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SpaceValueImport"), (float)m_space_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("HeadDistValueImport"), (float)m_head_dist_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("BeginCorDistValueImport"), (float)m_begin_cor_dist_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("EndCorDistValueImport"), (float)m_end_cor_dist_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SlopeValueImport"), (float)m_slope_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SuccDistValueImport"), (float)m_successor_dist_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("FootDistValueImport"), (float)m_foot_dist_value_imp);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("IncompleteMsgValueImport"), (float)m_incomplete_msg_value_imp);
#undef _u

	return 0;
}

void CImportSettingsDlg::OnDataValidateError(UINT nCtrlID, BOOL bSave, _XData& data)
{
  CString sMsg = "";
 
  switch (nCtrlID) {
  case IDC_HEAD_WIDTH_VALUE_IMP :
	case IDC_COR_WIDTH_VALUE_IMP : 
		sMsg.Format(_T("The size of width must be between %d mm and %d mm."),
			(int)data.floatData.nMin, (int)data.floatData.nMax);
		break;
	
	case IDC_HEAD_DIST_VALUE_IMP : 
		sMsg.Format(_T("The distance between the instance begin and the first element must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	case IDC_SLOPE_VALUE_IMP : 
		sMsg.Format(_T("The distance between messages ends must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	case IDC_FOOT_DIST_VALUE_IMP :
    sMsg.Format(_T("The distance between the last element and the instance end must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	case IDC_SPACE_VALUE_IMP :
		sMsg.Format(_T("The distance between the instance begin and the first element must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;

	case IDC_BEGIN_COR_DIST_VALUE_IMP :
	case IDC_END_COR_DIST_VALUE_IMP:
		sMsg.Format(_T("The distance between the coregion edges and events must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;

	case IDC_SUCCESSOR_DIST_VALUE_IMP:
		sMsg.Format(_T("The distance between two elements on instance must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;


	case IDC_INCOMPLETE_MSG_VALUE_IMP:
		sMsg.Format(_T("The length of incomplete message must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	
  default:
		sMsg = "Unknown error";
    TRACE("CImportSettingsDlg::OnDataValidateError() does not have a validation error message for a control");
  }

  MessageBox(sMsg, _T("Setting Import Error"), MB_ICONEXCLAMATION);
}




// $Id: 
