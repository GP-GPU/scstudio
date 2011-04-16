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
 * $Id: reportview.cpp 643 2010-03-01 22:37:17Z gotthardp $
 */

#include "stdafx.h"
#include "document.h"
#include "reportview.h"
#include "errors.h"

CRichList::CRichList(CReportView *reporter)
{
  m_reporter = reporter;
}

LRESULT CRichList::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
  if(lRet == -1)
    return lRet; // error

  long mask = GetEventMask();
  // enable the EN_LINK notification
  SetEventMask(mask | ENM_LINK);

  return lRet;
}

LRESULT CRichList::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  POINT pos = { LOWORD(lParam), HIWORD(lParam) };
  // convert pixel position [x,y] to a character position
  int ch = CharFromPos(pos);
  int line = LineFromChar(ch);

  TRACE("CRichList::OnLButtonDblClk() called at line " << line);
  CHARRANGE range;
  range.cpMin = LineIndex(line);
  range.cpMax = range.cpMin+LineLength(line)-1;

  return m_reporter->OnClick(range);
}

LRESULT CRichList::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  LONG ch1, ch2;
  // get caret position
  GetSel(ch1, ch2);
  int line = LineFromChar((ch1+ch2)/2);

  TRACE("CRichList::OnHelp() called at line " << line);
  CHARRANGE range;
  range.cpMin = LineIndex(line);
  range.cpMax = range.cpMin+LineLength(line)-1;

  return m_reporter->OnClick(range);
}

CReportView::CReportView(CDocumentMonitor *monitor)
  : m_edit(this)
{
  m_documentMonitor = monitor;
}

void CReportView::Reset()
{
  // window is active
  if(::IsWindow(m_edit.m_hWnd))
  {
    // reset the view
    m_edit.SetTextEx(NULL);
  }

  // reset the internal data
  m_references.clear();
}

static DWORD CALLBACK
EditStreamCallBack(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  const char **ppstr = (const char**)dwCookie;

  if(strlen(*ppstr) < (size_t)cb)
  {
    *pcb = (LONG)strlen(*ppstr);
    memcpy(pbBuff, *ppstr, *pcb);
  }
  else
  {
    *pcb = cb;
    memcpy(pbBuff, *ppstr, *pcb);
    *ppstr = *ppstr + cb;
  }
  return 0;
}

void SetSelection(CRichList& edit, int startChar, int endChar)
{
  edit.SetSel(startChar, endChar);
  CHARFORMAT2 cf;
  cf.cbSize = sizeof(cf);
  cf.dwMask = CFM_LINK;
  cf.dwEffects = CFE_LINK;
  edit.SetSelectionCharFormat(cf);
}

int CReportView::Print(const ReportMessage& msg)
{
  // display the report view
  m_documentMonitor->ShowReportView();

  long pos = m_edit.GetWindowTextLength();
  // put caret after the last character
  m_edit.SetSel(pos, pos);

  std::string pstr =
    "{\\rtf1\\ansi"
    "{\\colortbl;\\red255\\green0\\blue0;}"
    "{\\fonttbl\\f0\\fswiss Helvetica;}\\f0\\fs16";
  pstr += msg.get_text();
  pstr +=
    "\\line}";

  const char *cstr = pstr.c_str();

  EDITSTREAM es = {(DWORD_PTR)&cstr, 0, EditStreamCallBack};
  // append RTF to the list
  long res = m_edit.StreamIn(SF_RTF | SFF_SELECTION, es);

  // position of the first character
  int cpos = m_edit.GetTextLengthEx(GTL_NUMCHARS)-msg.get_length()-1;

  for(ReportReferencePtrList::const_iterator rpos = msg.get_references().begin();
    rpos != msg.get_references().end(); rpos++)
  {
    ReportReferencePtr ref = *rpos;
    ref->m_position.cpMin += cpos;
    ref->m_position.cpMax += cpos;

    SetSelection(m_edit, ref->m_position.cpMin, ref->m_position.cpMax);
    m_references.push_back(ref);
  }

  m_edit.LineScroll(1);
  return 0;
}

int CReportView::Print(TReportSeverity severity, const std::wstring& message,
  const std::wstring& help)
{
  ReportMessage msg;
  msg << ReportMessage::Highlight(severity) << message << ReportMessage::Highlight(RS_REPORT);

  if(!help.empty())
    msg << " [" << ReportLink("?", help) << "]";

  Print(msg);
  return 0;
}

int CReportView::Print(TReportSeverity severity, const std::wstring& message,
  const std::vector<_bstr_t>& shapelist, const std::wstring& help)
{
  ReportMessage msg;
  msg << ReportMessage::Highlight(severity) << message << ReportMessage::Highlight(RS_REPORT);

  if(!shapelist.empty())
    msg << " [" << ReportLink("show", shapelist) << "]";

  if(!help.empty())
    msg << " [" << ReportLink("?", help) << "]";

  Print(msg);
  return 0;
}

int CReportView::Print(TReportSeverity severity, const std::wstring& message,
  const MscPtr& msc, const std::wstring& help)
{
  ReportMessage msg;
  msg << ReportMessage::Highlight(severity) << message << ReportMessage::Highlight(RS_REPORT);

  if(msc)
    msg << " [" << ReportLink("show", msc) << "]";

  if(!help.empty())
    msg << " [" << ReportLink("?", help) << "]";

  Print(msg);
  return 0;
}

int CReportView::Print(TReportSeverity severity, const std::wstring& message,
  const std::list<MscPtr>& msclist, const std::wstring& help)
{
  ReportMessage msg;
  msg << ReportMessage::Highlight(severity) << message << ReportMessage::Highlight(RS_REPORT);

  if(msclist.size() > 1)
  {
    int i = 0;
    msg << " show";
    for(std::list<MscPtr>::const_iterator mpos = msclist.begin();
      mpos != msclist.end(); mpos++)
    {
      msg << " [" << ReportLink(basic_stringize<char>() << ++i, *mpos) << "]";
    }
  }
  else if(msclist.size() == 1)
    msg << " [" << ReportLink("show", msclist.front()) << "]";

  if(!help.empty())
    msg << " [" << ReportLink("?", help) << "]";

  Print(msg);
  return 0;
}

int CReportView::print(TReportSeverity severity, const std::wstring& message)
{
  Print(ReportMessage() << ReportMessage::Highlight(severity) << message);
  return 0;
}

LRESULT CReportView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);
  if(lRet == -1)
    return lRet; // error

  HWND hwndTV = m_edit.Create(
    m_hWnd,
    0, NULL,
    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
      ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
    WS_EX_STATICEDGE);

  return lRet;
}

LRESULT CReportView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  int nWidth = LOWORD(lParam);
  int nHeight = HIWORD(lParam);

  m_edit.MoveWindow(0, 0, nWidth, nHeight, 1);
  return 0;
}

LRESULT CReportView::OnLink(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
  ENLINK *link = (ENLINK *)pnmh;
  if (link->msg == WM_LBUTTONDOWN)
  {
    int line = m_edit.LineFromChar(link->chrg.cpMin);
    TRACE("CReportView::OnLink() called on line " << line);

    OnClick(link->chrg);
  }

  return 0;
}

LRESULT CReportView::OnClick(const CHARRANGE& range)
{
  for(ReportReferencePtrList::const_iterator rpos = m_references.begin();
    rpos != m_references.end(); rpos++)
  {
    if((*rpos)->Coincide(range))
    {
      (*rpos)->OnClick(m_documentMonitor);
      return 0;
    }
  }

  return 0;
}

void CReportView::OnFinalMessage(HWND hWnd)
{
  TRACE("CReportView::OnFinalMessage() called");

  m_references.clear();
  m_documentMonitor->OnHideReportView();
}

// $Id: reportview.cpp 643 2010-03-01 22:37:17Z gotthardp $
