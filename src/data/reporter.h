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
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: reporter.h 558 2010-01-28 21:35:53Z gotthardp $
 */

#ifndef _REPORTER_H
#define _REPORTER_H

#include <sstream>

enum TReportSeverity
{
  RS_REPORT = 0, //! progress reports
  RS_NOTICE, //! operation succeeded
  RS_WARNING, //! problem detected, but operation continues
  RS_ERROR, //! error detected, operation failed
  __RS_LAST
};

class ReportPrinter
{
public:
  virtual ~ReportPrinter() {}

  virtual int print(TReportSeverity severity, const std::wstring& message) = 0;
};

class StreamReportPrinter : public ReportPrinter
{
public:
  StreamReportPrinter(std::wostream& stream)
    : m_stream(stream) {}

  virtual int print(TReportSeverity severity, const std::wstring& message)
  {
    m_stream << message << std::endl;
    return 0;
  }

private:
  std::wostream& m_stream;
};

class Reporter
{
public:
  Reporter() : m_printer(NULL) {}
  Reporter(ReportPrinter* printer) : m_printer(printer) {}
  virtual ~Reporter() {}

  void set_printer(ReportPrinter* printer)
  { m_printer = printer; }

  int print_report(TReportSeverity severity, const std::wstring& message)
  { return m_printer ? m_printer->print(severity, message) : 1; }

private:
  ReportPrinter* m_printer;
};

#endif /* _REPORTER_H */

// $Id: reporter.h 558 2010-01-28 21:35:53Z gotthardp $
