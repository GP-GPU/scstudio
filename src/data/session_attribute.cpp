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
 * Copyright (c) 2009 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: session_attribute.cpp 602 2010-02-20 11:38:10Z kocianon $
 */
#include "data/session_attribute.h"

std::set<std::string> SA::RESERVED_ATTR;

void SA::reserve_attribute()
{
  std::pair<std::set<std::string>::iterator,bool> ret;
  ret = RESERVED_ATTR.insert(m_name);
  if(!ret.second)
    throw std::logic_error(std::string("Atribute with the same name already in use."));
}

void SA::cancel_attribute()
{
  RESERVED_ATTR.erase(m_name);
}

SA::~SA()
{
  cancel_attribute();
}

// $Id: session_attribute.cpp 602 2010-02-20 11:38:10Z kocianon $
