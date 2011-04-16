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
 * $Id: reportview.h 583 2010-02-10 23:05:56Z gotthardp $
 */

#pragma once
#include "reportmessage.h"
#include "resource.h"

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
#include <atlctrls.h>
#include <atlwin.h>

class CReportView;

class CRichList
  : public ATL::CWindowImpl<CRichList, CRichEditCtrl>
{
public:
  CRichList(CReportView *reporter);

protected:
BEGIN_MSG_MAP(CRichList)
  MESSAGE_HANDLER(WM_CREATE, OnCreate)
  MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
  MESSAGE_HANDLER(WM_HELP, OnHelp)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
  CReportView *m_reporter;
};

class CReportView : public ReportPrinter,
  public ATL::CWindowImpl<CReportView, ATL::CWindow, ATL::CNullTraits>
{
public:
  CReportView(CDocumentMonitor *monitor);

  void Reset();
  int Print(const ReportMessage& msg);

  int Print(TReportSeverity severity, const std::wstring& message,
    const std::wstring& help = std::wstring());
  int Print(TReportSeverity severity, const std::wstring& message,
    const std::vector<_bstr_t>& shapelist,
    const std::wstring& help = std::wstring());
  int Print(TReportSeverity severity, const std::wstring& message,
    const MscPtr& msc,
    const std::wstring& help = std::wstring());
  int Print(TReportSeverity severity, const std::wstring& message,
    const std::list<MscPtr>& msclist,
    const std::wstring& help = std::wstring());

  virtual int print(TReportSeverity severity, const std::wstring& message);

protected:
BEGIN_MSG_MAP(CReportView)
  MESSAGE_HANDLER(WM_CREATE, OnCreate)
  MESSAGE_HANDLER(WM_SIZE, OnSize)
  NOTIFY_CODE_HANDLER(EN_LINK, OnLink)
END_MSG_MAP()

// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  LRESULT OnLink(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnClick(const CHARRANGE& range);

  void OnFinalMessage(HWND hWnd);

private:
  CDocumentMonitor *m_documentMonitor;

  CRichList m_edit; // text of the report
  ReportReferencePtrList m_references; // hyperlinks
};

// $Id: reportview.h 583 2010-02-10 23:05:56Z gotthardp $
