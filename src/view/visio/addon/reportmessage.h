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
 * $Id: reportmessage.h 583 2010-02-10 23:05:56Z gotthardp $
 */

#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>

#include <richedit.h>

#include "data/msc.h"
#include "data/reporter.h"

class CDocumentMonitor;

class ReportReference
{
public:
  ReportReference()
  {
    m_position.cpMin = 0;
    m_position.cpMax = 0;
  }

  virtual ~ReportReference() {}
  virtual void OnClick(CDocumentMonitor *monitor) const = 0;

  bool Coincide(const CHARRANGE& range)
  {
    return (range.cpMin < m_position.cpMax && range.cpMax > m_position.cpMin);
  }

  CHARRANGE m_position;
};

typedef boost::shared_ptr<ReportReference> ReportReferencePtr;
typedef std::vector<ReportReferencePtr> ReportReferencePtrList;

struct shapelist
{
  shapelist & operator << (Visio::IVShapePtr shape)
  {
    _bstr_t reference = shape->UniqueID[visGetOrMakeGUID];
    m_v.push_back(reference);
    return *this;
  }

  // note: must not return reference
  operator const std::vector<_bstr_t>() const
  {
    return m_v;
  }

private:
  std::vector<_bstr_t> m_v;
};

struct ReportMessage
{
  ReportMessage()
    : m_length(0)
  { }

  class Highlight
  {
  public:
    Highlight(TReportSeverity severity)
      : m_severity(severity)
    { }

    TReportSeverity m_severity;
  };

  ReportMessage& operator << (const Highlight &op)
  {
    // depending on severity, change the text style
    switch(op.m_severity)
    {
      case RS_REPORT:
      default:
        m_text << "\\b0\\cf0 ";
        break;
      case RS_NOTICE:
        m_text << "\\b ";
        break;
      case RS_WARNING:
        m_text << "\\cf1 ";
        break;
      case RS_ERROR:
        m_text << "\\cf1\\b ";
        break;
    }
    return *this;
  }

  ReportMessage& operator << (const std::string &text)
  {
    m_text << text;
    m_length += text.length();
    return *this;
  }

  ReportMessage& operator << (const std::wstring &text)
  {
    for(std::wstring::const_iterator pos = text.begin();
      pos != text.end(); pos++)
    {
      if(*pos < 128)
        m_text << (char)*pos;
      else
        m_text << "\\u" << int(*pos) << ";";
    }

    m_length += text.length();
    return *this;
  }

  void AppendLink(const std::string &text, const ReportReferencePtr& reference)
  {
    ReportReferencePtr ref = reference;
    ref->m_position.cpMin = m_length;

    m_text << text;
    m_length += text.length();

    ref->m_position.cpMax = m_length;
    m_references.push_back(ref);
  }

  // note: must not return reference
  std::string get_text() const
  {
    return m_text.str();
  }

  size_t get_length() const
  {
    return m_length;
  }

  const ReportReferencePtrList& get_references() const
  {
    return m_references;
  }

private:
  std::stringstream m_text; // RTF text
  size_t m_length; // length of plaintext

  ReportReferencePtrList m_references;
};

class ShapesReference : public ReportReference
{
public:
  ShapesReference(const std::vector<_bstr_t>& shapes)
    : m_shapes(shapes)
  { }

  virtual void OnClick(CDocumentMonitor *monitor) const;
  std::vector<_bstr_t> m_shapes; // shapes to select
};

class DrawingReference : public ReportReference
{
public:
  DrawingReference(const MscPtr& drawing)
    : m_drawing(drawing)
  { }

  virtual void OnClick(CDocumentMonitor *monitor) const;
  MscPtr m_drawing; // drawing to display
};

class HelpReference : public ReportReference
{
public:
  HelpReference(const std::wstring& help)
    : m_help(help)
  { }

  virtual void OnClick(CDocumentMonitor *monitor) const;
  std::wstring m_help; // help to display
};

class ReportLink
{
public:
  ReportLink(const std::string &text, const std::vector<_bstr_t>& value)
    : m_text(text), m_reference(new ShapesReference(value))
  { }

  ReportLink(const std::string &text, const MscPtr& value)
    : m_text(text), m_reference(new DrawingReference(value))
  { }

  ReportLink(const std::string &text, const std::wstring& value)
    : m_text(text), m_reference(new HelpReference(value))
  { }

  friend ReportMessage&
  operator<<(ReportMessage& report, const ReportLink& link)
  {
    report.AppendLink(link.m_text, link.m_reference);
    return report;
  }

private:
  std::string m_text;
  ReportReferencePtr m_reference;
};

// $Id: reportmessage.h 583 2010-02-10 23:05:56Z gotthardp $
