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
 * $Id: transformer.h 439 2009-10-25 20:48:28Z gotthardp $
 */

#ifndef _TRANSFORMER_H
#define _TRANSFORMER_H

#include <boost/shared_ptr.hpp>

#include "data/msc.h"
#include "data/configurator.h"
#include "data/reporter.h"
#include "data/prerequisite_check.h"

#if defined(_MSC_VER)
// FIXME: to be removed once the Transformer has some implementation in a .cpp file
#pragma warning(disable: 4275)
#endif

class Transformer : public ConfigReader, public Reporter
{
public:
  virtual ~Transformer() {}

  //! Human readable name of the transformation.
  virtual std::wstring get_name() const = 0;

  //! List of properties that must be satisfied before executing the transformation.
  typedef std::vector<PrerequisiteCheck> PreconditionList;
  //! Returns a list of preconditions for this transformation.
  virtual PreconditionList get_preconditions(MscPtr msc) const = 0;

  //! Transform a MSC drawing.
  virtual MscPtr transform(MscPtr msc) = 0;
};

typedef boost::shared_ptr<Transformer> TransformerPtr;

//! module initialization function
typedef Transformer** (*FInitTransformers)();

#endif /* _TRANSFORMER_H */

// $Id: transformer.h 439 2009-10-25 20:48:28Z gotthardp $
