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
 * $Id: beautify.h 1019 2011-01-04 07:15:39Z xpekarc $
 */

#include "data/msc.h"
#include "data/transformer.h"
#include "export.h"

class SCBEAUTIFY_EXPORT Beautify : public Transformer
{
public:
  Beautify();
  virtual ~Beautify()	{}

  //! Human readable name of the transformation.
  virtual std::wstring get_name() const
  { return L"Beautify"; }

  //! Returns a list of preconditions for this transformation.
  virtual PreconditionList get_preconditions(MscPtr msc) const;

  //! Transform a MSC drawing.
  virtual MscPtr transform(MscPtr msc);

	virtual void set_is_imported(bool is_imported)
	{
		m_is_imported = is_imported;
	}

	bool get_is_imported(void)
	{
		return m_is_imported;
	}
	bool m_is_imported;

protected:
  // note: insertion to m_processing must not invalidate iterators
  std::list<MscPtr> m_processing;
	

  int transform_bmsc(BMscPtr bmsc);
  int transform_hmsc(HMscPtr hmsc);
};

// $Id: beautify.h 1019 2011-01-04 07:15:39Z xpekarc $
