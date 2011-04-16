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
 * $Id: finddlg.h 1038 2011-02-09 22:26:51Z mbezdeka $
 */

#pragma once

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlddx.h>

class CFindDlg
  : public ATL::CDialogImpl<CFindDlg>, public CWinDataExchange<CFindDlg>
{
public:
  enum { IDD = IDD_FIND_FLOW };
  CTreeViewCtrl m_drawing1;
  CTreeViewCtrl m_drawing2;

  CFindDlg(Visio::IVApplicationPtr vsoApp);
  std::vector<Visio::IVPagePtr>  m_pages1;
  Visio::IVPagePtr               m_page2;

  //Diff
  CButton  m_diff;
  bool     m_diffEnabled;

protected:
BEGIN_DDX_MAP(CFindDlg)
  DDX_CONTROL_HANDLE(IDC_DRAWING1, m_drawing1)
  DDX_CONTROL_HANDLE(IDC_DRAWING2, m_drawing2)
  DDX_CONTROL_HANDLE(IDC_DIFF, m_diff)
END_DDX_MAP()

BEGIN_MSG_MAP(CFindDlg)
  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
  COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
  NOTIFY_HANDLER(IDC_DRAWING1, NM_CLICK, OnNMClickDrawing1)
  NOTIFY_HANDLER(IDC_DRAWING2, NM_CLICK, OnNMClickDrawing2)
END_MSG_MAP()

  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnCloseCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

  void InitializeTree(CTreeViewCtrl& tree, bool isPattern);
  std::vector<Visio::IVPagePtr> GetTreeSelection(CTreeViewCtrl& tree);

private:
  Visio::IVApplicationPtr m_vsoApp;

public:
  LRESULT OnNMClickDrawing1(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
  LRESULT OnNMClickDrawing2(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
  LRESULT OnTreeViewClicked(CTreeViewCtrl& tree);
  LRESULT OnTreeViewItemClicked(CTreeViewCtrl& tree, HTREEITEM item, UINT unFlags);
  LRESULT processCheckBoxes(CTreeViewCtrl& tree, HTREEITEM item, bool multiSelect, bool reverted = false);
  void setCheckBoxes(CTreeViewCtrl& tree, BOOL bCheck);
};

// $Id: finddlg.h 1038 2011-02-09 22:26:51Z mbezdeka $