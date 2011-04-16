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
 * Copyright (c) 2008-2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: engmann.h 438 2009-10-25 15:32:55Z gotthardp $
 */

#ifndef _ENGMANN_H
#define _ENGMANN_H

#include "data/formatter.h"
#include "data/engmann/export.h"

class SCENGMANN_EXPORT Engmann : public Formatter, public ImportFormatter
{
public:
  //! file extension used to distinguish this format
  // note: DLL in Windows cannot return pointers to static data
  virtual std::string get_extension() const
  { return "cfi"; }
  //! human readable description of this format
  virtual std::string get_description() const
  { return "Engmann Message Chart"; }

  //! import MSC document
  virtual std::vector<MscPtr> load_msc(const std::string &filename);
  //! Returns a list of transformation for this format.
  virtual TransformationList get_transformations(MscPtr msc) const;

protected:
};

#endif /* _ENGMANN_H */

// $Id: engmann.h 438 2009-10-25 15:32:55Z gotthardp $
