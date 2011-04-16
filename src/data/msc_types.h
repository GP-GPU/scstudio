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
 * Copyright (c) 2008 Jindra Babica <babica@mail.muni.cz>
 *
 * $Id: msc_types.h 656 2010-03-03 21:36:00Z gotthardp $
 */

#ifndef _MSC_TYPES_H
#define _MSC_TYPES_H

#ifndef NOMINMAX
#define NOMINMAX // disable Windows min and max macros
#endif

#include "data/fcmp.h"
#include "data/export.h"

#include <algorithm>
#include <list>
#include <map>
#include <string>
#if defined(_MSC_VER)
#include <comutil.h> // _bstr_t
#endif

template <class T>
struct nocase_comparator: public std::binary_function<std::basic_string<T>, std::basic_string<T>, bool>
{
  struct nocase_compare: public std::binary_function<T, T, bool>
  {
    bool operator() (const T& c1, const T& c2) const
    { return tolower(c1) < tolower(c2); };
  };

  bool operator() (const std::basic_string<T>& s1, const std::basic_string<T>& s2) const
  {
    return std::lexicographical_compare(
      s1.begin(), s1.end(), s2.begin(), s2.end(), nocase_compare());
  }
};

char SCMSC_EXPORT strip_diacritics(wchar_t ch);

std::wstring SCMSC_EXPORT TOWSTRING(const std::string& s);

//! Helper class to construct a message for Reporter::print()
/*!
 * reporter->print(stringize() << "value: " << number);
 */
template<typename C>
struct basic_stringize
{
  template<typename T>
  basic_stringize<C> & operator << (const T& t)
  {
    m_s << t;
    return *this;
  }

  // note: must not return reference
  operator const std::basic_string<C>() const
  {
    return m_s.str();
  }

#if defined(_MSC_VER)
  template<>
  basic_stringize<C> & operator << (const _bstr_t& t)
  {
    // choose conversion
    // _bstr_t has both char* and wchar_t* operators
    m_s << (const C*)t;
    return *this;
  }

  // note: must not return reference
  operator const _bstr_t() const
  {
    return m_s.str().c_str();
  }
#endif

private:
  std::basic_stringstream<C> m_s;
};

typedef basic_stringize<wchar_t> stringize;

typedef double Coordinate;
typedef double Size;

/**
 * Denotes form of instance's axis as described by ITU-T
 */
typedef enum
{
  LINE,
  COLUMN
} InstanceAxisForm;

//! 2D coordinates in millimeters, having [0,0] upper left
class MscPoint
{
private:
  Coordinate m_x;
  Coordinate m_y;
  
public:
  
  MscPoint(Coordinate x=0,Coordinate y=0)
  {
    m_x = x;
    m_y = y;
  }
  
  Coordinate get_x() const
  {
    return m_x;
  }
  
  void set_x(Coordinate x)
  {
    m_x = x;
  }
  
  Coordinate get_y() const
  {
    return m_y;
  }
  
  void set_y(Coordinate y)
  {
    m_y = y;
  }

  MscPoint operator + (const MscPoint& p2) const
  {
    return MscPoint(m_x+p2.m_x, m_y+p2.m_y);
  }

  MscPoint operator - (const MscPoint& p2) const
  {
    return MscPoint(m_x-p2.m_x, m_y-p2.m_y);
  }

  bool operator < (const MscPoint& p2) const
  {
    int resy = fcmp(m_y, p2.m_y);
    return resy < 0 || (resy == 0 && fcmp(m_x, p2.m_x) < 0);
  }
};

class PolyLine
{
  std::list<MscPoint> m_points;

public:
  
  const std::list<MscPoint>& get_points() const
  {
    return m_points;
  }
  
};

template <class Ptr>
class PtrIDMap
{
private:
  typedef std::map<Ptr, long> TPtrIDMapper;
  TPtrIDMapper m_mapper;

public:
  // this function assigns a unique identifier to Ptr objects
  long get_id(const Ptr& message)
  {
    typename std::map<Ptr, long>::iterator pos = m_mapper.find(message);
    if(pos != m_mapper.end())
      return pos->second;

    long result = m_mapper.size();
    m_mapper[message] = result;

    return result;
  }
};

template <class Ptr>
void push_back_if_unique(std::list<Ptr>& list, const Ptr& item)
{
  if(item != NULL &&
    std::find(list.begin(), list.end(), item) == list.end())
  {
    list.push_back(item);
  }
}

#endif /* _MSC_TYPES_H */

// $Id: msc_types.h 656 2010-03-03 21:36:00Z gotthardp $
