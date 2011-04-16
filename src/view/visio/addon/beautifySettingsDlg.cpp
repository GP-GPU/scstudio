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
#include "beautifySettingsDlg.h"
#include "errors.h"
#include "document.h"
#include "pageutils.h"





LRESULT CBeautifySettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  CenterWindow(GetParent());

	LoadRegistryData();
  //Move values in variables into controls
  DoDataExchange(FALSE);

	UpdateControls();

  return bHandled = false;
}

LRESULT CBeautifySettingsDlg::OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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

int CBeautifySettingsDlg::LoadRegistryData()
{
	TRACE("CBeautifySettingDlg::LoadRegistryData() - Loading options from registry"); 

	Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, 0, visPageUnits)
	m_arrange								= GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("Arrange"), 1);
	m_incomplete_msg_value	= GetRegistry<float>(GetRegistryFolder(), NULL, _T("IncompleteMsgValue"), 20);
	m_use_const_length			= GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("ConstInstanceLength"), 1);
	m_instance_length_value = GetRegistry<float>(GetRegistryFolder(), NULL, _T("InstanceValue"), 40);
	m_use_first_length			= GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("FirstInstanceLength"), 0);
	m_use_parts_length			=	GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("PartsInstanceLength"), 0);	
	m_head_dist_value				= GetRegistry<float>(GetRegistryFolder(), NULL, _T("HeadDistValue"), 5);
	m_begin_cor_dist_value	= GetRegistry<float>(GetRegistryFolder(), NULL, _T("BeginCorDistValue"), 5);
	m_end_cor_dist_value		= GetRegistry<float>(GetRegistryFolder(), NULL, _T("EndCorDistValue"), 5);
	m_slope_value						= GetRegistry<float>(GetRegistryFolder(), NULL, _T("SlopeValue"), 0);
	m_successor_dist_value	= GetRegistry<float>(GetRegistryFolder(), NULL, _T("SuccDistValue"), 5);
	m_foot_dist_value				= GetRegistry<float>(GetRegistryFolder(), NULL, _T("FootDistValue"), 5);


  m_settingHead						= GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("SettingHead"), 1);
	m_use_const_head				=	GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("ConstHead"), 1);
	m_head_width_value      = GetRegistry<float>(GetRegistryFolder(), NULL, _T("HeadValue"), 10);

	m_settingCoregion				= GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("SettingCoregion"), 1);
	m_use_const_cor					=	GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("ConstCoregion"), 1);
	m_cor_width_value				= GetRegistry<float>(GetRegistryFolder(), NULL, _T("CoregionValue"), 10);

	m_settingSpace					= GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("SettingSpace"), 1);
	m_space_value						= GetRegistry<float>(GetRegistryFolder(), NULL, _T("SpaceValue"), 30);
	m_use_original_ord			=	GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("OriginalOrder"), 0);
	m_use_cross_ord					=	GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("CrossingOrder"), 0);
	m_use_going_back_ord		=	GetRegistry<DWORD>(GetRegistryFolder(), NULL, _T("GoingBackOrder"), 0);
#undef _u

	return 0;
}

void CBeautifySettingsDlg::UpdateControls()
{
	if(m_arrange)
	{
		m_constInstanceRdo.SetCheck(m_use_const_length ? BST_CHECKED : BST_UNCHECKED);
		m_firstInstanceRdo.SetCheck(m_use_first_length ? BST_CHECKED : BST_UNCHECKED);
		m_originalInstanceRdo.SetCheck((!m_use_const_length && !m_use_first_length && !m_use_parts_length) ? BST_CHECKED : BST_UNCHECKED);
		m_partsInstanceRdo.SetCheck(m_use_parts_length ? BST_CHECKED : BST_UNCHECKED);
	}
	m_constInstanceRdo.EnableWindow(m_arrange);
  m_firstInstanceRdo.EnableWindow(m_arrange);
	m_originalInstanceRdo.EnableWindow(m_arrange);
	m_partsInstanceRdo.EnableWindow(m_arrange);	
	m_incompleteMsgValue.EnableWindow(m_arrange);


	m_instanceLengthValue.EnableWindow(m_arrange && m_use_const_length);
	m_headDistValue.EnableWindow(m_arrange && m_use_parts_length);
	m_beginCorDistValue.EnableWindow(m_arrange && m_use_parts_length);
	m_endCorDistValue.EnableWindow(m_arrange && m_use_parts_length);
	m_slopeValue.EnableWindow(m_arrange && m_use_parts_length);
	m_successorDistValue.EnableWindow(m_arrange && m_use_parts_length);
	m_footDistValue.EnableWindow(m_arrange && m_use_parts_length); 

	m_constHeadRdo.SetCheck(m_use_const_head ? BST_CHECKED : BST_UNCHECKED);
	m_firstHeadRdo.SetCheck(!m_use_const_head ? BST_CHECKED : BST_UNCHECKED);
	m_constHeadRdo.EnableWindow(m_settingHead);
	m_firstHeadRdo.EnableWindow(m_settingHead);
	m_headWidthValue.EnableWindow(m_settingHead && m_use_const_head);

	m_constCoregionRdo.SetCheck(m_use_const_cor ? BST_CHECKED : BST_UNCHECKED);
	m_firstCoregionRdo.SetCheck(!m_use_const_cor ? BST_CHECKED : BST_UNCHECKED);
	m_constCoregionRdo.EnableWindow(m_settingCoregion);
	m_firstCoregionRdo.EnableWindow(m_settingCoregion);
	m_corWidthValue.EnableWindow(m_settingCoregion && m_use_const_cor);


	if(m_settingSpace)
	{
		m_originalOrderRdo.SetCheck(m_use_original_ord ? BST_CHECKED : BST_UNCHECKED);
		m_crossingOrderRdo.SetCheck(m_use_cross_ord ? BST_CHECKED : BST_UNCHECKED);
		m_goingBackOrderRdo.SetCheck(m_use_going_back_ord ? BST_CHECKED : BST_UNCHECKED);
		m_bothOrderRdo.SetCheck((!m_use_original_ord && !m_use_cross_ord && !m_use_going_back_ord) ? BST_CHECKED : BST_UNCHECKED);
	}
	m_originalOrderRdo.EnableWindow(m_settingSpace);
	m_crossingOrderRdo.EnableWindow(m_settingSpace);
	m_goingBackOrderRdo.EnableWindow(m_settingSpace);
	m_bothOrderRdo.EnableWindow(m_settingSpace);
	m_spaceValue.EnableWindow(m_settingSpace);
}

int CBeautifySettingsDlg::SaveRegistryData()
{
	TRACE("CBeautifySettingDlg::SaveRegistryData() - Saving options to registry"); 

	Visio::IVPagePtr page = m_vsoApp->ActivePage;
#define _u(x) CPageUtils::ConvertUnits(page, x, 0, visPageUnits)
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("Arrange"), (DWORD)m_arrange);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("ConstInstanceLength"),(DWORD)(m_constInstanceRdo.GetCheck() == BST_CHECKED));
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("FirstInstanceLength"),(DWORD)(m_firstInstanceRdo.GetCheck() == BST_CHECKED));
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("PartsInstanceLength"),(DWORD)(m_partsInstanceRdo.GetCheck() == BST_CHECKED));

  SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SettingHead"),(DWORD)m_settingHead);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("ConstHead"),(DWORD)(m_constHeadRdo.GetCheck() == BST_CHECKED));

	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SettingCoregion"),(DWORD)m_settingCoregion);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("ConstCoregion"),(DWORD)(m_constCoregionRdo.GetCheck() == BST_CHECKED));

	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SettingSpace"), (DWORD)m_settingSpace);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("OriginalOrder"),(DWORD)(m_originalOrderRdo.GetCheck() == BST_CHECKED));
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("CrossingOrder"),(DWORD)(m_crossingOrderRdo.GetCheck() == BST_CHECKED));
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("GoingBackOrder"),(DWORD)(m_goingBackOrderRdo.GetCheck() == BST_CHECKED));

	
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("HeadValue"), (float)m_head_width_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("CoregionValue"), (float)m_cor_width_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("InstanceValue"), (float)m_instance_length_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SpaceValue"), (float)m_space_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("HeadDistValue"), (float)m_head_dist_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("BeginCorDistValue"), (float)m_begin_cor_dist_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("EndCorDistValue"), (float)m_end_cor_dist_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SlopeValue"), (float)m_slope_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("SuccDistValue"), (float)m_successor_dist_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("FootDistValue"), (float)m_foot_dist_value);
	SetRegistry<>(HKEY_CURRENT_USER, GetRegistryFolder(), _T("IncompleteMsgValue"), (float)m_incomplete_msg_value);
#undef _u

	return 0;
}

void CBeautifySettingsDlg::OnDataValidateError(UINT nCtrlID, BOOL bSave, _XData& data)
{
  CString sMsg = "";
 
  switch (nCtrlID) {
  case IDC_HEAD_WIDTH_VALUE :
	case IDC_COR_WIDTH_VALUE : 
		sMsg.Format(_T("The size of width must be between %d mm and %d mm."),
			(int)data.floatData.nMin, (int)data.floatData.nMax);
		break;
	
	case IDC_HEAD_DIST_VALUE : 
		sMsg.Format(_T("The distance between the instance begin and the first element must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	case IDC_SLOPE_VALUE : 
		sMsg.Format(_T("The distance between messages ends must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	case IDC_FOOT_DIST_VALUE :
    sMsg.Format(_T("The distance between the last element and the instance end must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	case IDC_SPACE_VALUE :
		sMsg.Format(_T("The distance between the instance begin and the first element must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;

	case IDC_BEGIN_COR_DIST_VALUE :
	case IDC_END_COR_DIST_VALUE:
		sMsg.Format(_T("The distance between the coregion edges and events must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;

	case IDC_SUCCESSOR_DIST_VALUE:
		sMsg.Format(_T("The distance between two elements on instance must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;


	case IDC_INSTANCE_LENGTH_VALUE :
	case IDC_INCOMPLETE_MSG_VALUE:
		sMsg.Format(_T("The length must be between %d mm and %d mm."),
                (int)data.floatData.nMin, (int)data.floatData.nMax );
    break;
	
  default:
		sMsg = "Unknown error";
    TRACE("CBeautifySettingsDlg::OnDataValidateError() does not have a validation error message for a control");
  }

  MessageBox(sMsg, _T("Setting Beautify Error"), MB_ICONEXCLAMATION);
}

LRESULT CBeautifySettingsDlg::OnArrange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_arrange = !m_arrange;
	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnConstInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_length = 1;
	m_use_parts_length = 0;
	m_use_first_length = 0;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnFirstInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_length = 0;
	m_use_parts_length = 0;
	m_use_first_length = 1;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnOriginalInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_length = 0;
	m_use_parts_length = 0;
	m_use_first_length = 0;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnPartsInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_length = 0;
	m_use_parts_length = 1;
	m_use_first_length = 0;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnSettingHead(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_settingHead = !m_settingHead;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnConstHeadRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_head = 1;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnFirstHeadRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_head = 0;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnSettingCoregion(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_settingCoregion = !m_settingCoregion;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnConstCoregionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_cor = 1;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnFirstCoregionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_const_cor = 0;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnSettingSpace(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_settingSpace = !m_settingSpace;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnOriginalOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_original_ord = 1;
	m_use_cross_ord = 0;
	m_use_going_back_ord = 0;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnCrossingOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_original_ord = 0;
	m_use_cross_ord = 1;
	m_use_going_back_ord = 0;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnBackOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_original_ord = 0;
	m_use_cross_ord = 0;
	m_use_going_back_ord = 1;

	UpdateControls();
	return 0;
}

LRESULT CBeautifySettingsDlg::OnBothOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_use_original_ord = 0;
	m_use_cross_ord = 0;
	m_use_going_back_ord = 0;

	UpdateControls();
	return 0;
}




// $Id: 
