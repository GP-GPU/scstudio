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
 * Copyright (c) 2010 Ondrej Bouda <ondrej.bouda@wheee.cz>
 *
 * $Id: instancesfielddlg.h 1040 2011-02-15 17:34:51Z mbezdeka $
 */

#pragma once

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlwinx.h>
#include <atlmisc.h>

class CInstancesFieldDlg
  : public ATL::CDialogImpl<CInstancesFieldDlg>, public CWinDataExchange<CInstancesFieldDlg>
{
public:
  enum { IDD = IDD_INSTANCES_FIELD_OPTIONS };
  int m_instances_cnt;
  double m_instance_length;
  double m_instance_width; // NOTE: not in the form, just for the user to be able to change it via the Registry
  double m_start_pos_x;
  double m_start_pos_y;
  bool m_use_const_spacing;
  double m_total_width;
  double m_spacing;

  CInstancesFieldDlg(Visio::IVApplicationPtr vsoApp, double mousePosX=0.0, double mousePosY=0.0);

  const wchar_t* GetRegistryFolder() { return SCSTUDIO_REGISTRY_ROOT _T("\\InstancesField"); }

private:
  Visio::IVApplicationPtr m_vsoApp;
  short m_pageUnits;

  double m_mousePosX;
  double m_mousePosY;

  double m_pageWidth;
  double m_pageHeight;

protected:
BEGIN_DDX_MAP(CInstancesFieldDlg)
  DDX_INT_RANGE(IDC_IF_INSTANCES_CNT, m_instances_cnt, 1, 100)
  DDX_FLOAT(IDC_IF_INSTANCE_LENGTH, m_instance_length)
  DDX_FLOAT(IDC_IF_START_POS_X, m_start_pos_x)
  DDX_FLOAT(IDC_IF_START_POS_Y, m_start_pos_y)
  DDX_FLOAT(IDC_IF_TOTAL_WIDTH, m_total_width)
  DDX_FLOAT(IDC_IF_SPACING, m_spacing)

  DDX_CONTROL_HANDLE(IDC_IF_INSTANCES_CNT, m_instancesCntEdit)
  DDX_CONTROL_HANDLE(IDC_OPTIONS_GROUP_BOX, m_optionsGroupBox)
  DDX_CONTROL_HANDLE(IDC_TOTAL_WIDTH_RDO, m_totalWidthRdo)
  DDX_CONTROL_HANDLE(IDC_SPACING_RDO, m_spacingRdo)
  DDX_CONTROL_HANDLE(IDC_DEFAULT_BTN, m_defaultValuesBtn)
  DDX_CONTROL_HANDLE(IDC_IF_TOTAL_WIDTH, m_totalWidthEdit)
  DDX_CONTROL_HANDLE(IDC_IF_SPACING, m_spacingEdit)
  DDX_CONTROL_HANDLE(IDC_IF_UNITS0, m_unitLabels[0])
  DDX_CONTROL_HANDLE(IDC_IF_UNITS1, m_unitLabels[1])
  DDX_CONTROL_HANDLE(IDC_IF_UNITS2, m_unitLabels[2])
  DDX_CONTROL_HANDLE(IDC_IF_UNITS3, m_unitLabels[3])
  DDX_CONTROL_HANDLE(IDC_IF_UNITS4, m_unitLabels[4])
END_DDX_MAP()

BEGIN_MSG_MAP(CInstancesFieldDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
  COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
  // FIXME: would like to make the label switch the options as well, but the following does not work:
  //COMMAND_ID_HANDLER(IDC_OPTIONS_SWITCH_LBL, OnOptionsSwitch)
  COMMAND_ID_HANDLER(IDC_TOTAL_WIDTH_RDO, OnTotalWidthRdo)
  COMMAND_ID_HANDLER(IDC_SPACING_RDO, OnSpacingRdo)
  COMMAND_ID_HANDLER(IDC_DEFAULT_BTN, OnDefaultCmd)
END_MSG_MAP()

// Handler prototypes:
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)

  void OnDataValidateError (UINT nCtrlID, BOOL bSave, _XData& data);

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnTotalWidthRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnSpacingRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnDefaultCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

  int LoadRegistryData();
  int SaveRegistryData();

  void UpdateSpacingControls();

  CStatic m_optionsGroupBox;
  CButton m_totalWidthRdo;
  CButton m_spacingRdo;
  CButton m_defaultValuesBtn;

  CEdit m_instancesCntEdit;
  CEdit m_totalWidthEdit;
  CEdit m_spacingEdit;

  CStatic m_unitLabels[5];
};

static const int DEFAULT_IF_INSTANCES_CNT = 2;
static const double DEFAULT_IF_INSTANCE_LENGTH = 1.5748031496062992; // 40 mm
static const double DEFAULT_IF_INSTANCE_WIDTH = 0.3937007874015748; //10 mm
static const double DEFAULT_IF_START_POS_X = 0.0;
static const double DEFAULT_IF_START_POS_Y = 0.0;
static const int DEFAULT_IF_SPACING_SWITCH = true;
static const double DEFAULT_IF_TOTAL_WIDTH = 7.8740157480314960; // 200 mm
static const double DEFAULT_IF_SPACING = 1.1811023622047244094; // 30 mm
static const double IF_BORDER = 0.0787401574803149; // 2 mm border for the default command

// $Id: instancesfielddlg.h 1040 2011-02-15 17:34:51Z mbezdeka $
