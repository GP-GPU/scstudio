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
 * $Id: numberingGlobalDlg.h 842 2010-08-11 21:27:12Z mbezdeka $
 */

#pragma once

#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <string>
#include <sstream>
#include "data/msc.h"

#include <boost/shared_ptr.hpp>

class CNumberingGlobalDlg: public CPropertyPageImpl<CNumberingGlobalDlg>, public CWinDataExchange<CNumberingGlobalDlg>
{
public:
  enum { IDD = IDD_NUMBERING_GLOBAL };

  CNumberingGlobalDlg(Visio::IVApplicationPtr vsoApp):m_vsoApp(vsoApp)
                        {   //set dialog properties
                            m_psp.dwFlags |= PSP_HASHELP;
                        };

BEGIN_MSG_MAP(CNumberingGlobalDlg)
	MSG_WM_INITDIALOG(OnInitDialog)
	CHAIN_MSG_MAP(CPropertyPageImpl<CNumberingGlobalDlg>)
	COMMAND_HANDLER(IDC_ENUM_CHECK, BN_CLICKED, OnBnClickedEnumCheck)
	COMMAND_HANDLER(IDC_NEAREST_MSG,BN_CLICKED, OnBnClickedNearestMsg)
	COMMAND_HANDLER(IDC_NEAREST_NUMBERED_MSG,BN_CLICKED, OnBnClickedNearestNumberedMsg)
	COMMAND_HANDLER(IDC_DONT_NUMBER,BN_CLICKED, OnBnClickedDontNumber)
	COMMAND_HANDLER(IDC_USE_NUMBERING_STYLE,BN_CLICKED, OnBnClickedUseNumberingStyle)
END_MSG_MAP()


BEGIN_DDX_MAP(CNumberingGlobalDlg)
	DDX_CHECK(IDC_ENUM_CHECK,m_bAutoEnum);
	DDX_CONTROL_HANDLE(IDC_NEAREST_MSG, m_nearestMsg);
	DDX_CONTROL_HANDLE(IDC_NEAREST_NUMBERED_MSG, m_nearestNumberedMsg);
	DDX_CONTROL_HANDLE(IDC_DONT_NUMBER, m_dontNumber);
	DDX_CONTROL_HANDLE(IDC_USE_NUMBERING_STYLE, m_useNumberingStyle);
	DDX_CONTROL_HANDLE(IDC_COMBO_NUMBERING_GLOBAL, m_numberingTypeCombo);
	DDX_CONTROL_HANDLE(IDC_ADDITION_GLOBAL, m_addition);
	DDX_CONTROL_HANDLE(IDC_STARTING_INDEX_GLOBAL, m_index);
END_DDX_MAP()

protected:

  CButton	m_nearestMsg;
  CButton	m_nearestNumberedMsg;
  CButton	m_dontNumber;
  CButton	m_useNumberingStyle;
  CComboBox	m_numberingTypeCombo;
  CEdit		m_addition;
  CEdit		m_index;

  bool		m_bAutoEnum;
  int     m_iNumberingTypeCombo;
  int     m_iIndex;
  _bstr_t m_szAddition;

  Visio::IVApplicationPtr m_vsoApp;

  LRESULT OnBnClickedEnumCheck(WORD, WORD, HWND, BOOL&);
  LRESULT OnBnClickedNearestMsg(WORD, WORD, HWND, BOOL&);
  LRESULT OnBnClickedNearestNumberedMsg(WORD, WORD, HWND, BOOL&);
  LRESULT OnBnClickedDontNumber(WORD, WORD, HWND, BOOL&);
  LRESULT OnBnClickedUseNumberingStyle(WORD, WORD, HWND, BOOL&);
  
  void EnableAllControls(bool bEnable);
  void EnableNearestNumberedMsgControls(bool bEnable);
  void EnableNumberingStyleControls(bool bEnable);

public:
  //Message handlers
  BOOL OnInitDialog(HWND hWndFocus, LPARAM lParam);

  //Property page notification handlers (override method)
  int OnApply();
  void OnHelp();

  //Other functions  
  const wchar_t* GetRegistryFolder() { return SCSTUDIO_REGISTRY_ROOT _T("\\MessageNumbering"); }
};

// $Id: numberingGlobalDlg.h 842 2010-08-11 21:27:12Z mbezdeka $