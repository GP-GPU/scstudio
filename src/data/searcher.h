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
 * $Id: searcher.h 1038 2011-02-09 22:26:51Z mbezdeka $
 */

#ifndef _SEARCHER_H
#define _SEARCHER_H

#include <boost/shared_ptr.hpp>

#include "data/msc.h"
#include "data/reporter.h"
#include "data/prerequisite_check.h"

#if defined(_MSC_VER)
// FIXME: to be removed once the Searcher has some implementation in a .cpp file
#pragma warning(disable: 4275)
#endif

class Searcher : public Reporter
{
public:
  virtual ~Searcher() {}

  //! List of properties that must be satisfied before executing the search.
  typedef std::vector<PrerequisiteCheck> PreconditionList;
  //! Returns a list of preconditions for this search.
  virtual PreconditionList get_preconditions(MscPtr msc) const = 0;

  //! Find the first occurence of needle in haystack.
  virtual MscPtr find(MscPtr haystack, MscPtr needle) = 0;
  virtual MscPtr find(MscPtr msc, std::vector<MscPtr>& bmscs) = 0;

  //! Make diff between specification and flow
  virtual MscPtr diff(MscPtr specification, std::vector<MscPtr>& flows) = 0;
};

typedef boost::shared_ptr<Searcher> SearcherPtr;

//! module initialization function
typedef Searcher* (*FInitMembership)();

#endif /* _SEARCHER_H */

// $Id: searcher.h 1038 2011-02-09 22:26:51Z mbezdeka $
