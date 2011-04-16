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
 * $Id: messagesequencedlg.h 772 2010-05-13 00:13:42Z obouda $
 */

#pragma once

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlwinx.h>
#include <atlmisc.h>
#include "enums.h"

class CMessageSequenceDlg
  : public ATL::CDialogImpl<CMessageSequenceDlg>, public CWinDataExchange<CMessageSequenceDlg>
{
public:
  enum { IDD = IDD_MESSAGE_SEQUENCE_OPTIONS };

  MsgSeqDirection m_direction;
  MsgSeqCoregionTreatment m_coregionTreatment;

  double m_start_pos_y;
  double m_vertical_space;
  double m_vertical_space_left_right;
  wchar_t m_left_msg_captions[256];
  wchar_t m_right_msg_captions[256];

  CMessageSequenceDlg(Visio::IVApplicationPtr vsoApp, double mousePosY=0.0, MsgSeqDirection initDir=MSDIR_RIGHT);

  const wchar_t* GetRegistryFolder() { return SCSTUDIO_REGISTRY_ROOT _T("\\MessageSequence"); }

private:
  Visio::IVApplicationPtr m_vsoApp;

  double m_mousePosY;
  double m_pageHeight;
  double m_old_start_pos_y;
  short m_pageUnits;

protected:
BEGIN_DDX_MAP(CMessageSequenceDlg)
  DDX_FLOAT_RANGE(IDC_MS_START_POS_Y, m_start_pos_y, 0, m_pageHeight)
  DDX_FLOAT_RANGE(IDC_MS_VERTICAL_SPACE, m_vertical_space, 0, m_pageHeight)
  DDX_FLOAT_RANGE(IDC_MS_VERTICAL_SPACE_LEFT_RIGHT, m_vertical_space_left_right, 0, m_pageHeight)
  DDX_TEXT(IDC_MS_LEFT_MESSAGE, m_left_msg_captions)
  DDX_TEXT(IDC_MS_RIGHT_MESSAGE, m_right_msg_captions)

  DDX_CONTROL_HANDLE(IDC_MS_DIR_LEFT, m_dirLeftRdo)
  DDX_CONTROL_HANDLE(IDC_MS_DIR_RIGHT, m_dirRightRdo)
  DDX_CONTROL_HANDLE(IDC_MS_DIR_LEFT_RIGHT, m_dirLeftRightRdo)
  DDX_CONTROL_HANDLE(IDC_MS_DIR_RIGHT_LEFT, m_dirRightLeftRdo)
  DDX_CONTROL_HANDLE(IDC_MS_LEFT_MESSAGE, m_leftMessageEdit)
  DDX_CONTROL_HANDLE(IDC_MS_RIGHT_MESSAGE, m_rightMessageEdit)
  DDX_CONTROL_HANDLE(IDC_MS_START_POS_Y, m_startPosYEdit)
  DDX_CONTROL_HANDLE(IDC_MS_VERTICAL_SPACE, m_verticalSpaceEdit)
  DDX_CONTROL_HANDLE(IDC_MS_VERTICAL_SPACE_LEFT_RIGHT, m_verticalSpaceLeftRightEdit)
  DDX_CONTROL_HANDLE(IDC_MS_COREGION_ERROR, m_coregionErrorRdo)
  DDX_CONTROL_HANDLE(IDC_MS_COREGION_CONTINUE, m_coregionContinueRdo)
  DDX_CONTROL_HANDLE(IDC_MS_COREGION_ORDER, m_coregionOrderRdo)
  DDX_CONTROL_HANDLE(IDC_MS_CONNECT_WITH_LINE, m_coregionConnectLineRdo)
  DDX_CONTROL_HANDLE(IDC_MS_CONNECT_WITH_SIDE_SIDE, m_coregionConnectSideSideRdo)
  DDX_CONTROL_HANDLE(IDC_MS_UNITS0, m_unitLabels[0])
  DDX_CONTROL_HANDLE(IDC_MS_UNITS1, m_unitLabels[1])
  DDX_CONTROL_HANDLE(IDC_MS_UNITS2, m_unitLabels[2])
END_DDX_MAP()

BEGIN_MSG_MAP(CMessageSequenceDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
  COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
  COMMAND_ID_HANDLER(IDC_MS_DIR_LEFT, OnDirectionRdo)
  COMMAND_ID_HANDLER(IDC_MS_DIR_RIGHT, OnDirectionRdo)
  COMMAND_ID_HANDLER(IDC_MS_DIR_LEFT_RIGHT, OnDirectionRdo)
  COMMAND_ID_HANDLER(IDC_MS_DIR_RIGHT_LEFT, OnDirectionRdo)
  COMMAND_ID_HANDLER(IDC_MS_COREGION_ERROR, OnCoregionRdo)
  COMMAND_ID_HANDLER(IDC_MS_COREGION_CONTINUE, OnCoregionRdo)
  COMMAND_ID_HANDLER(IDC_MS_COREGION_ORDER, OnCoregionRdo)
  COMMAND_ID_HANDLER(IDC_MS_CONNECT_WITH_LINE, OnCoregionRdo)
  COMMAND_ID_HANDLER(IDC_MS_CONNECT_WITH_SIDE_SIDE, OnCoregionRdo)
END_MSG_MAP()

// Handler prototypes:
//	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//	LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
//	LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)

  void OnDataValidateError (UINT nCtrlID, BOOL bSave, _XData& data);

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnDirectionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnCoregionRdo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

  int LoadRegistryData();
  int SaveRegistryData();

  void UpdateControls();

  CButton m_dirLeftRdo;
  CButton m_dirRightRdo;
  CButton m_dirLeftRightRdo;
  CButton m_dirRightLeftRdo;
  CButton m_coregionErrorRdo;
  CButton m_coregionContinueRdo;
  CButton m_coregionOrderRdo;
  CButton m_coregionConnectLineRdo;
  CButton m_coregionConnectSideSideRdo;

  CEdit m_leftMessageEdit;
  CEdit m_rightMessageEdit;
  CEdit m_startPosYEdit;
  CEdit m_verticalSpaceEdit;
  CEdit m_verticalSpaceLeftRightEdit;

  CStatic m_unitLabels[3];
};

static const double DEFAULT_MS_START_POS_Y = 0.0;
static const double DEFAULT_MS_VERTICAL_SPACE = 0.1968503937007874; // 5 mm
static const double DEFAULT_MS_VERTICAL_SPACE_LEFT_RIGHT = 0.1968503937007874; // 5 mm
static const wchar_t* DEFAULT_MS_LEFT_MESSAGE_CAPTION = _T("");
static const wchar_t* DEFAULT_MS_RIGHT_MESSAGE_CAPTION = _T("");
static const MsgSeqCoregionTreatment DEFAULT_MS_COREGION_TREATMENT = MSCOR_CONNECT_LINE;

// $Id: messagesequencedlg.h 772 2010-05-13 00:13:42Z obouda $
