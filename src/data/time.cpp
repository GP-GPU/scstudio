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
 * Copyright (c) 2008-2009 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: time.cpp 967 2010-09-17 21:20:37Z gotthardp $
 */

#include "data/time.h"
#include <limits>

DecScaled::DecScaled(const std::string &number)
 : m_exp(0), m_mant(0)
{
  const char* pos = number.c_str();
  register char ch;

  long neg = 1;
  while(isspace(ch = *pos++))
    /* skip */;
  if(ch == '-')
    neg = -1;
  else if(ch == '+')
    /* skip */;
  else
    --pos;

  while((ch = *pos++), isdigit(ch))
    m_mant = m_mant*10 + (ch-'0');
  if(ch == '.')
  {
    while((ch = *pos++), isdigit(ch))
    {
      m_mant = m_mant*10 + (ch-'0');
      m_exp--;
    }
  }

  if(neg<0)
    m_mant = -m_mant;
  tidy();
}

template <>
MscIntervalCouple<double>::MscIntervalCouple(const std::string& number, const bool& closed):
  m_closed(closed),
  m_value(0)
{
  std::string lowcase_value;
  lowcase_value.reserve(number.length());
  // transform new_value to lowercase for case insensitive comparison
  std::transform(number.begin(), number.end(),
    std::back_inserter(lowcase_value), ::tolower);

  if(lowcase_value == "inf" || lowcase_value == "+inf")
    m_value = std::numeric_limits<double>::infinity();
  else if(lowcase_value == "-inf")
    m_value = -std::numeric_limits<double>::infinity();
  else
  {
    char* endptr;
    m_value = strtod(number.c_str(), &endptr);

    if(endptr != NULL && *endptr != 0)
      throw MscIntervalStringConversionError(number + std::string(" is not a valid number."));
  }
}

template <>
MscIntervalCouple<DecScaled>::MscIntervalCouple(const std::string &number, \
	const bool& closed):
   m_closed(closed)
  ,m_value(0)
{
  if(number[0]=='i')
  {

    DecScaled tmp(std::numeric_limits<long>::max());
    m_value = tmp;
  }
  else
  {
    DecScaled tmp(number);
    m_value = tmp;
  }
}

template <>
MscIntervalCouple<double>::MscIntervalCouple(const bool& b,const double& d):
     m_closed(b)
     ,m_value(d)
{
  // -inf,inf has to have _close value false to be valid --> REWRITTING!
  if(m_value==std::numeric_limits<double>::infinity() || m_value==-std::numeric_limits<double>::infinity())
    m_closed=false;
}

template <>
MscIntervalCouple<DecScaled>::MscIntervalCouple(const bool& b,const DecScaled& d):
     m_closed(b)
     ,m_value(d)
{
}
/*

template class SCMSC_EXPORT MscTimeInterval<double>;
template class SCMSC_EXPORT MscTimeInterval<DecScaled>;
*/
template class SCMSC_EXPORT MscIntervalCouple<double>;
template class SCMSC_EXPORT MscTimeIntervalSet<double>;
template class SCMSC_EXPORT MscTimeIntervalSet<DecScaled>;


// template class SCMSC_EXPORT MscIntervalCouple<DecScaled>;

// $Id: time.cpp 967 2010-09-17 21:20:37Z gotthardp $
