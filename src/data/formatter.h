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
 * $Id: formatter.h 438 2009-10-25 15:32:55Z gotthardp $
 */

#ifndef _FORMATTER_H
#define _FORMATTER_H

#include <boost/shared_ptr.hpp>

#include "data/msc.h"
#include "data/reporter.h"
#include "data/prerequisite_check.h"

#if defined(_MSC_VER)
// FIXME: to be removed once the Formatter has some implementation in a .cpp file
#pragma warning(disable: 4275)
#endif

class Formatter : public Reporter
{
public:
  virtual ~Formatter() {}

  //! file extension used to distinguish this format
  virtual std::string get_extension() const = 0;
  //! human readable description of the format
  virtual std::string get_description() const = 0;
};

typedef boost::shared_ptr<Formatter> FormatterPtr;

class ImportFormatter
{
public:
  virtual ~ImportFormatter() {}

  //! import MSC document
  virtual std::vector<MscPtr> load_msc(const std::string &filename) = 0;

  //! List of transformations that must be performed after the import.
  typedef std::vector<std::wstring> TransformationList;
  //! Returns a list of transformation for this format.
  virtual TransformationList get_transformations(MscPtr msc) const = 0;
};

typedef boost::shared_ptr<ImportFormatter> ImportFormatterPtr;

class ExportFormatter
{
public:
  virtual ~ExportFormatter() {}

  //! List of properties that must be satisfied before executing the export.
  typedef std::vector<PrerequisiteCheck> PreconditionList;
  //! Returns a list of preconditions for this format.
  virtual PreconditionList get_preconditions(MscPtr msc) const = 0;

  //! export MSC document
  virtual int save_msc(std::ostream& stream, const std::wstring &name,
    const MscPtr& selected_msc, const std::vector<MscPtr>& msc = std::vector<MscPtr>()) = 0;
};

typedef boost::shared_ptr<ExportFormatter> ExportFormatterPtr;

//! module initialization function
typedef Formatter** (*FInitFormatters)();

#endif /* _FORMATTER_H */

// $Id: formatter.h 438 2009-10-25 15:32:55Z gotthardp $
