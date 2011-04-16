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

#pragma once

#include <atldlgs.h>
#include <atlddx.h>
#include <atlctrls.h>
#include <atlcrack.h>
#include <atlmisc.h>

#include "data/msc.h"



class CBeautifySettingsDlg
  : public ATL::CDialogImpl<CBeautifySettingsDlg>, public CWinDataExchange<CBeautifySettingsDlg>
{
public:
  enum { IDD = IDD_BEAUTIFY_SETTINGS };

	CBeautifySettingsDlg(Visio::IVApplicationPtr vsoApp):m_vsoApp(vsoApp) {}

	~CBeautifySettingsDlg(){}

	//Registry folder
  const wchar_t* GetRegistryFolder() { return SCSTUDIO_REGISTRY_ROOT _T("\\Beautify"); }

	int m_arrange;
	int m_settingHead;
	int m_settingCoregion;
	int m_settingSpace;

	long m_use_const_length;  
	long m_use_first_length;	
	long m_use_parts_length;		
	long m_use_const_head;			
	long m_use_const_cor;		
	long m_use_original_ord;	
	long m_use_cross_ord;	
	long m_use_going_back_ord;

	float m_head_width_value;
	float m_cor_width_value;
	float m_instance_length_value;
	float m_space_value;
	float m_head_dist_value;
	float m_begin_cor_dist_value;
	float m_end_cor_dist_value;
	float m_slope_value;
	float m_successor_dist_value;
	float m_foot_dist_value;
	float m_incomplete_msg_value;

private:
  Visio::IVApplicationPtr m_vsoApp;

BEGIN_DDX_MAP(CImportSettingDlg)
	DDX_CHECK(IDC_ARRANGING, m_arrange)
	DDX_CONTROL_HANDLE(IDC_CONST_INSTANCE_RDO, m_constInstanceRdo)
	DDX_CONTROL_HANDLE(IDC_FIRST_INSTANCE_RDO, m_firstInstanceRdo)
	DDX_CONTROL_HANDLE(IDC_ORIGINAL_INSTANCE_RDO, m_originalInstanceRdo)
	DDX_CONTROL_HANDLE(IDC_PARTS_INSTANCE_RDO, m_partsInstanceRdo)

	DDX_CHECK(IDC_SETTING_HEAD, m_settingHead)
	DDX_CONTROL_HANDLE(IDC_CONST_HEAD_RDO, m_constHeadRdo)
	DDX_CONTROL_HANDLE(IDC_FIRST_HEAD_RDO, m_firstHeadRdo)

	DDX_CHECK(IDC_SETTING_COREGION, m_settingCoregion)
	DDX_CONTROL_HANDLE(IDC_CONST_COREGION_RDO, m_constCoregionRdo)
	DDX_CONTROL_HANDLE(IDC_FIRST_COREGION_RDO, m_firstCoregionRdo)

	DDX_CHECK(IDC_SETTING_SPACE, m_settingSpace)
	DDX_CONTROL_HANDLE(IDC_ORIGINAL_ORDER_RDO, m_originalOrderRdo)
	DDX_CONTROL_HANDLE(IDC_CROSSING_ORDER_RDO, m_crossingOrderRdo)
	DDX_CONTROL_HANDLE(IDC_GOING_BACK_ORDER_RDO, m_goingBackOrderRdo)
	DDX_CONTROL_HANDLE(IDC_BOTH_ORDER_RDO, m_bothOrderRdo)

	DDX_CONTROL_HANDLE(IDC_HEAD_WIDTH_VALUE, m_headWidthValue)
	DDX_CONTROL_HANDLE(IDC_COR_WIDTH_VALUE, m_corWidthValue)
	DDX_CONTROL_HANDLE(IDC_INSTANCE_LENGTH_VALUE, m_instanceLengthValue)
	DDX_CONTROL_HANDLE(IDC_SPACE_VALUE, m_spaceValue)
	DDX_CONTROL_HANDLE(IDC_HEAD_DIST_VALUE, m_headDistValue)
	DDX_CONTROL_HANDLE(IDC_BEGIN_COR_DIST_VALUE, m_beginCorDistValue)
	DDX_CONTROL_HANDLE(IDC_END_COR_DIST_VALUE, m_endCorDistValue)
	DDX_CONTROL_HANDLE(IDC_SLOPE_VALUE, m_slopeValue)
	DDX_CONTROL_HANDLE(IDC_SUCCESSOR_DIST_VALUE, m_successorDistValue)
	DDX_CONTROL_HANDLE(IDC_FOOT_DIST_VALUE, m_footDistValue)
	DDX_CONTROL_HANDLE(IDC_INCOMPLETE_MSG_VALUE, m_incompleteMsgValue)


	DDX_FLOAT_RANGE(IDC_HEAD_WIDTH_VALUE, m_head_width_value, 0, 500)
	DDX_FLOAT_RANGE(IDC_COR_WIDTH_VALUE, m_cor_width_value, 1, 500)
	DDX_FLOAT_RANGE(IDC_INSTANCE_LENGTH_VALUE, m_instance_length_value, 6, 500)
	DDX_FLOAT_RANGE(IDC_SPACE_VALUE, m_space_value, 1, 500)
	DDX_FLOAT_RANGE(IDC_HEAD_DIST_VALUE, m_head_dist_value, 3, 500)
	DDX_FLOAT_RANGE(IDC_BEGIN_COR_DIST_VALUE, m_begin_cor_dist_value, 1, 500)
	DDX_FLOAT_RANGE(IDC_END_COR_DIST_VALUE, m_end_cor_dist_value, 1, 500)
	DDX_FLOAT_RANGE(IDC_SLOPE_VALUE, m_slope_value, 0, 500)
	DDX_FLOAT_RANGE(IDC_SUCCESSOR_DIST_VALUE, m_successor_dist_value, 1, 500)
	DDX_FLOAT_RANGE(IDC_FOOT_DIST_VALUE, m_foot_dist_value, 3, 500)
	DDX_FLOAT_RANGE(IDC_INCOMPLETE_MSG_VALUE, m_incomplete_msg_value, 1, 500)
END_DDX_MAP()

BEGIN_MSG_MAP(CImportSettingDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

	COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
	COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

	
	COMMAND_ID_HANDLER(IDC_ARRANGING, OnArrange)
	COMMAND_ID_HANDLER(IDC_CONST_INSTANCE_RDO, OnConstInstanceRdo)
	COMMAND_ID_HANDLER(IDC_FIRST_INSTANCE_RDO, OnFirstInstanceRdo)
	COMMAND_ID_HANDLER(IDC_ORIGINAL_INSTANCE_RDO, OnOriginalInstanceRdo)
	COMMAND_ID_HANDLER(IDC_PARTS_INSTANCE_RDO, OnPartsInstanceRdo)

	COMMAND_ID_HANDLER(IDC_SETTING_HEAD, OnSettingHead)
	COMMAND_ID_HANDLER(IDC_CONST_HEAD_RDO, OnConstHeadRdo)
	COMMAND_ID_HANDLER(IDC_FIRST_HEAD_RDO, OnFirstHeadRdo)

	COMMAND_ID_HANDLER(IDC_SETTING_COREGION, OnSettingCoregion)
	COMMAND_ID_HANDLER(IDC_CONST_COREGION_RDO, OnConstCoregionRdo)
	COMMAND_ID_HANDLER(IDC_FIRST_COREGION_RDO, OnFirstCoregionRdo)

	COMMAND_ID_HANDLER(IDC_SETTING_SPACE, OnSettingSpace)
	COMMAND_ID_HANDLER(IDC_ORIGINAL_ORDER_RDO, OnOriginalOrderRdo)
	COMMAND_ID_HANDLER(IDC_CROSSING_ORDER_RDO, OnCrossingOrderRdo)
	COMMAND_ID_HANDLER(IDC_GOING_BACK_ORDER_RDO, OnBackOrderRdo)
	COMMAND_ID_HANDLER(IDC_BOTH_ORDER_RDO, OnBothOrderRdo)
END_MSG_MAP()

	void OnDataValidateError (UINT nCtrlID, BOOL bSave, _XData& data);
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnArrange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnConstInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFirstInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOriginalInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPartsInstanceRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	
	LRESULT OnSettingHead(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnConstHeadRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFirstHeadRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettingCoregion(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnConstCoregionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnFirstCoregionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSettingSpace(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOriginalOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCrossingOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBackOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnBothOrderRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);




  int LoadRegistryData();
  int SaveRegistryData();

  void UpdateControls();

	CButton m_constInstanceRdo;
	CButton m_firstInstanceRdo;
	CButton m_originalInstanceRdo;
	CButton m_partsInstanceRdo;
	CButton m_constHeadRdo;
	CButton m_firstHeadRdo;
	CButton m_constCoregionRdo;
	CButton m_firstCoregionRdo;
	CButton m_originalOrderRdo;
	CButton m_crossingOrderRdo;
	CButton m_goingBackOrderRdo;
	CButton m_bothOrderRdo;

	CEdit m_headWidthValue;
	CEdit m_corWidthValue;
	CEdit m_instanceLengthValue;
	CEdit m_spaceValue;
	CEdit m_headDistValue;
	CEdit m_beginCorDistValue;
	CEdit m_endCorDistValue;
	CEdit m_slopeValue;
	CEdit m_successorDistValue;
	CEdit m_footDistValue; 
	CEdit m_incompleteMsgValue;
	
};


// $Id: 