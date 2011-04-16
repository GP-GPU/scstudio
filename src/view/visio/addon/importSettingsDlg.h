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



class CImportSettingsDlg
  : public ATL::CDialogImpl<CImportSettingsDlg>, public CWinDataExchange<CImportSettingsDlg>
{
public:
  enum { IDD = IDD_IMPORT_SETTINGS };

	CImportSettingsDlg(Visio::IVApplicationPtr vsoApp):m_vsoApp(vsoApp) {}

	~CImportSettingsDlg(){}

	//Registry folder
  const wchar_t* GetRegistryFolder() { return SCSTUDIO_REGISTRY_ROOT _T("\\Beautify"); }

	float m_head_width_value_imp;
	float m_cor_width_value_imp;
	float m_space_value_imp;
	float m_head_dist_value_imp;
	float m_begin_cor_dist_value_imp;
	float m_end_cor_dist_value_imp;
	float m_slope_value_imp;
	float m_successor_dist_value_imp;
	float m_foot_dist_value_imp;
	float m_incomplete_msg_value_imp;

private:
  Visio::IVApplicationPtr m_vsoApp;

BEGIN_DDX_MAP(CImportSettingDlg)

	DDX_CONTROL_HANDLE(IDC_HEAD_WIDTH_VALUE_IMP, m_headWidthValueImp)
	DDX_CONTROL_HANDLE(IDC_COR_WIDTH_VALUE_IMP, m_corWidthValueImp)
	DDX_CONTROL_HANDLE(IDC_SPACE_VALUE_IMP, m_spaceValueImp)
	DDX_CONTROL_HANDLE(IDC_HEAD_DIST_VALUE_IMP, m_headDistValueImp)
	DDX_CONTROL_HANDLE(IDC_BEGIN_COR_DIST_VALUE_IMP, m_beginCorDistValueImp)
	DDX_CONTROL_HANDLE(IDC_END_COR_DIST_VALUE_IMP, m_endCorDistValueImp)
	DDX_CONTROL_HANDLE(IDC_SLOPE_VALUE_IMP, m_slopeValueImp)
	DDX_CONTROL_HANDLE(IDC_SUCCESSOR_DIST_VALUE_IMP, m_successorDistValueImp)
	DDX_CONTROL_HANDLE(IDC_FOOT_DIST_VALUE_IMP, m_footDistValueImp)
	DDX_CONTROL_HANDLE(IDC_INCOMPLETE_MSG_VALUE_IMP, m_incompleteMsgValueImp)


	DDX_FLOAT_RANGE(IDC_HEAD_WIDTH_VALUE_IMP, m_head_width_value_imp, 0, 500)
	DDX_FLOAT_RANGE(IDC_COR_WIDTH_VALUE_IMP, m_cor_width_value_imp, 1, 500)
	DDX_FLOAT_RANGE(IDC_SPACE_VALUE_IMP, m_space_value_imp, 1, 500)
	DDX_FLOAT_RANGE(IDC_HEAD_DIST_VALUE_IMP, m_head_dist_value_imp, 3, 500)
	DDX_FLOAT_RANGE(IDC_BEGIN_COR_DIST_VALUE_IMP, m_begin_cor_dist_value_imp, 1, 500)
	DDX_FLOAT_RANGE(IDC_END_COR_DIST_VALUE_IMP, m_end_cor_dist_value_imp, 1, 500)
	DDX_FLOAT_RANGE(IDC_SLOPE_VALUE_IMP, m_slope_value_imp, 0, 500)
	DDX_FLOAT_RANGE(IDC_SUCCESSOR_DIST_VALUE_IMP, m_successor_dist_value_imp, 1, 500)
	DDX_FLOAT_RANGE(IDC_FOOT_DIST_VALUE_IMP, m_foot_dist_value_imp, 3, 100000)
	DDX_FLOAT_RANGE(IDC_INCOMPLETE_MSG_VALUE_IMP, m_incomplete_msg_value_imp, 1, 500)
END_DDX_MAP()

BEGIN_MSG_MAP(CImportSettingDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

	COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
	COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
END_MSG_MAP()

	void OnDataValidateError (UINT nCtrlID, BOOL bSave, _XData& data);
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

  int LoadRegistryData();
  int SaveRegistryData();

  void UpdateControls();

	CEdit m_headWidthValueImp;
	CEdit m_corWidthValueImp;
	CEdit m_spaceValueImp;
	CEdit m_headDistValueImp;
	CEdit m_beginCorDistValueImp;
	CEdit m_endCorDistValueImp;
	CEdit m_slopeValueImp;
	CEdit m_successorDistValueImp;
	CEdit m_footDistValueImp; 
	CEdit m_incompleteMsgValueImp;
};


// $Id: 