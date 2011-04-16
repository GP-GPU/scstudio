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
 * Copyright (c) 2008-2010 Ondrej Kocian <kocian.on@mail.muni.cz>
 *
 * $Id: time.h 1076 2011-04-05 15:35:43Z lkorenciak $
 */

#ifndef _TIME_H_
#define _TIME_H_

#include "data/fcmp.h"
#include "data/export.h"

#include<limits.h>
#include<algorithm>
#include<exception>
#include<list>
#include<cstdlib>
#include<iostream>
#include <sstream>
#include<cctype>
#include<cstring>
#include<string>
#include <limits>

#if defined(_MSC_VER)
// Visual C++ does not support functions declared using exception specification.
#pragma warning(disable: 4290)
#endif

/*
class MscIntervalCoupleUncomparable;
class MscIntervalCouple;
class MscTimeInterval;
class DecScaled;
*/
//class MscTimeSet;

// pow(x,n) optimized for int(x) and n>=0
// based on ipow.c by Mark Stephen (www.snippets.org)
inline long ipow(long x, unsigned int n)
{
  if (!n)
    return 1;  // 0**0 = 1
  if (x == 0)
    return 0;

  long t = 1;
  do
  {
    if (n & 1)
      t *= x;
    n /= 2;
    x *= x;
  } while (n);

  return t;
}

class MscIntervalCoupleUncomparable:public std::exception
{
public:

  virtual const char* what() const throw()
  {
    return "undecidable case";
  }
};

class MscIntervalStringConversionError:public std::exception
{
private:
  std::string msg;

public:

  MscIntervalStringConversionError(const std::string& m = ""):
  msg(std::string("String conversion: ") + m)
  {}

  virtual const char* what() const throw()
  {
    return msg.c_str();
  }

  ~MscIntervalStringConversionError() throw()
  {}

};

/**
 * \brief try to find char in string
 * Go throw the string set trying to find char
 * \return true if set contains char znak
 */
inline bool char_in_string(const char& znak,const std::string& set = "[](),")
{
  return set.find(znak)!=std::string::npos;
}

/**
 * \brief Get copy of string without white spaces and in lowercase
 * \param word string to be cleaned of white spaces
 * \return copy of string without white spaces
 */
inline std::string get_small_nospace(const std::string& word)
{
  std::string tmp;
  for(unsigned i=0;i<word.length();i++)
  {
    if(isspace(word[i]))
      continue;
    tmp+=tolower(word[i]);
  }
  return tmp;
}

/**
 * Decimal scaling structure, stands for:
 *  m_mant*10^m_exp;
 */
class DecScaled
{
private:
  long m_exp; // exponent
  long m_mant; // significand, coefficient or mantissa

  // keeps number aligned to the right
  void tidy()
  {
    while(m_mant%10==0 && m_mant!=0)
    {
      m_mant=m_mant/10;
      m_exp++;
    }
  }
public:
  DecScaled():m_exp(0),m_mant(0){}
  DecScaled(long mant):m_exp(0),m_mant(mant)
  {
    tidy();
  }

  DecScaled(long mant,long exp):m_exp(exp),m_mant(mant)
  {
    tidy();
  }
  DecScaled(const std::string &number);

  DecScaled& operator=(const DecScaled& right)
  {
    if(this==&right)
      return *this;
    m_mant = right.m_mant;
    m_exp = right.m_exp;
    return *this;
  }

  const DecScaled& operator+() const
  {
    return *this;
  }

  const DecScaled operator-() const
  {
    return DecScaled(-m_mant,m_exp);
  }

  const DecScaled operator+(const DecScaled& right) const
  {
    std::pair<DecScaled, DecScaled> norm = normalize(*this, right);
    return DecScaled(norm.first.m_mant+norm.second.m_mant, norm.first.m_exp);
  }

  const DecScaled operator-(const DecScaled& right) const
  {
    std::pair<DecScaled, DecScaled> norm = normalize(*this, right);
    return DecScaled(norm.first.m_mant-norm.second.m_mant, norm.first.m_exp);
  }

  const DecScaled operator*(const DecScaled& right) const
  {
    return DecScaled(m_mant*right.m_mant,m_exp+right.m_exp);
  }
/*
  const DecScaled operator/(const DecScaled& right) const
  {
    return DecScaled(m_mant/right.m_mant,m_ext-right.m_exp,blebleble_tojetezke);
  }
*/
  bool operator==(const DecScaled& right) const
  {
    return m_mant==right.m_mant && m_exp==right.m_exp;
  }

  bool operator!=(const DecScaled& right) const
  {
    return m_mant!=right.m_mant || m_exp!=right.m_exp;
  }

  bool operator<(const DecScaled& right) const
  {
    std::pair<DecScaled, DecScaled> norm = normalize(*this, right);
    return norm.first.m_mant < norm.second.m_mant;
  }

  bool operator<=(const DecScaled& right) const
  {
    std::pair<DecScaled, DecScaled> norm = normalize(*this, right);
    return norm.first.m_mant <= norm.second.m_mant;
  }

  bool operator>(const DecScaled& right) const
  {
    std::pair<DecScaled, DecScaled> norm = normalize(*this, right);
    return norm.first.m_mant > norm.second.m_mant;
  }

  bool operator>=(const DecScaled& right) const
  {
    std::pair<DecScaled, DecScaled> norm = normalize(*this, right);
    return norm.first.m_mant >= norm.second.m_mant;
  }

  /**
   * Print the number in a scientific notation
   * http://en.wikipedia.org/wiki/Scientific_notation
   */
  friend std::ostream&
  operator<<(std::ostream& os, const DecScaled& value)
  {
    return os << value.m_mant << "E" << value.m_exp;
  }

protected:
  const std::pair<DecScaled, DecScaled>
  normalize(const DecScaled &left, const DecScaled &right) const
  {
    long division = abs(left.m_exp-right.m_exp);
    DecScaled a = left;
    DecScaled b = right;

    if(a.m_exp>b.m_exp)
    {
      a.m_mant*=ipow(10,division);
      a.m_exp-=division;
    }
    else
    {
      b.m_mant*=ipow(10,division);
      b.m_exp-=division;
    }

    return std::pair<DecScaled, DecScaled>(a, b);
  }
}; // end of DecScaled class


/**
 * @brief class representing couple value-number and bracket
 */
template<class T>
class SCMSC_EXPORT MscIntervalCouple
{
private:
  bool m_closed;
  T m_value;

public:
  MscIntervalCouple():
  m_closed(true),m_value(0)
  {
  }

  MscIntervalCouple(const std::string&,const bool&);

  MscIntervalCouple(const bool& b,const T& d);

  const bool& get_closed() const
  {
    return m_closed;
  }

  const T& get_value() const
  {
    return m_value;
  }

  void set_closed(const bool& closed)
  {
    m_closed = closed;
  }

  void set_value(const T& value)
  {
    m_value=value;
  }

  const MscIntervalCouple operator+(const MscIntervalCouple& right) const
  {
    return MscIntervalCouple(m_closed&&right.m_closed,m_value+right.m_value);
  }

  const MscIntervalCouple operator-(const MscIntervalCouple& right) const
  {
    return MscIntervalCouple(m_closed&&right.m_closed,m_value-right.m_value);
  }

  const MscIntervalCouple operator+(const T& value) const
  {
    return MscIntervalCouple(m_closed,m_value+value);
  }

  const MscIntervalCouple operator-(const T& value) const
  {
    return MscIntervalCouple(m_closed,m_value-value);
  }

  const MscIntervalCouple operator*(const T& value) const
  {
    return MscIntervalCouple(m_closed,m_value*value);
  }

  MscIntervalCouple& operator=(const MscIntervalCouple& right)
  {
    if(this==&right)
      return *this;
    m_value = right.m_value;
    m_closed = right.m_closed;
    return *this;
  }

  MscIntervalCouple& operator=(const T& value)
  {
    m_value = value;
    return *this;
  }

  MscIntervalCouple& operator=(const bool& b)
  {
    m_closed = b;
    return *this;
  }


  MscIntervalCouple& operator+=(const MscIntervalCouple& right)
  {
    m_value+=right.m_value;
    m_closed=right.m_closed && m_closed;
    return *this;
  }

  MscIntervalCouple& operator-=(const MscIntervalCouple& right)
  {
    m_value-=right.m_value;
    m_closed=right.m_closed && m_closed;
    return *this;
  }

  MscIntervalCouple& operator+=(const T& value)
  {
    m_value+=value;
    return *this;
  }

  MscIntervalCouple& operator-=(const T& value)
  {
    m_value-=value;
    return *this;
  }

  bool operator==(const T& right) const
  { return m_closed && m_value == right; }

  bool operator!=(const T& right) const
  { return !m_closed || m_value != right; }

  bool operator<(const T& right) const
  { return m_value < right; }

  bool operator<=(const T& right) const
  { return (m_closed && m_value <= right) || m_value < right; }

  bool operator>(const T& right) const
  { return m_value > right; }

  bool operator>=(const T& right) const
  { return (m_closed && m_value >= right) || m_value > right; }

  /**
   * @warning: does not bother with left and right couple
   */
  bool operator==(const MscIntervalCouple& right) const throw()
  {
    return m_value==right.m_value && m_closed==right.m_closed;
  }

  /**
   * @warning: does not bother with left and right couple
   */
  bool operator!=(const MscIntervalCouple& right) const throw()
  {
    return !(m_value==right.m_value && m_closed==right.m_closed);
  }

  /**
   * @warning bother about left and right
   * @throw MscIntervalCoupleUncomparable in case it cant be decided
   */
  bool operator<(const MscIntervalCouple& right) const throw(MscIntervalCoupleUncomparable)
  {
    if(m_value!=right.m_value)
      return m_value<right.m_value;

    if(*this==right && m_closed)
      return false;
    else
      throw MscIntervalCoupleUncomparable();

  }

  /**
   * @warning bother about left and right
   * @throw MscIntervalCoupleUncomparable in case it cant be decided
   */
  bool operator>(const MscIntervalCouple& right) const throw(MscIntervalCoupleUncomparable)
  {
    if(m_value!=right.m_value)
      return m_value>right.m_value;

    if(*this==right && m_closed)
      return false;
    else
      throw MscIntervalCoupleUncomparable();
  }
};

template<>
inline bool MscIntervalCouple<double>::operator==(const double& right) const
{ return m_closed && fcmp(m_value, right) == 0; }

template<>
inline bool MscIntervalCouple<double>::operator!=(const double& right) const
{ return !m_closed || fcmp(m_value, right) != 0; }

template<>
inline bool MscIntervalCouple<double>::operator<(const double& right) const
{ return fcmp(m_value, right) < 0; }

template<>
inline bool MscIntervalCouple<double>::operator<=(const double& right) const
{ return (m_closed && fcmp(m_value, right) <= 0) || fcmp(m_value, right) < 0; }

template<>
inline bool MscIntervalCouple<double>::operator>(const double& right) const
{ return fcmp(m_value, right) > 0; }

template<>
inline bool MscIntervalCouple<double>::operator>=(const double& right) const
{ return (m_closed && fcmp(m_value, right) >= 0) || fcmp(m_value, right) > 0; }

// end of MscIntervalCouple class

template<class P>
std::ostream&
operator<<(std::ostream& os, const MscIntervalCouple<P>& value)
{
  if(value.get_value() == std::numeric_limits<P>::infinity())
    os << "inf";
  else if(value.get_value() == -std::numeric_limits<P>::infinity())
    os << "-inf";
  else
    os << value.get_value();

  return os;
}

/**
 * @brief class representing Interval
 */
template<class T>
class MscTimeInterval
{
private:
  MscIntervalCouple<T> m_begin;
  MscIntervalCouple<T> m_end;

public:
  MscTimeInterval()
  {}

  MscTimeInterval(T value):m_begin(true,value),m_end(true,value)
  {
  }

  MscTimeInterval(T left_value, T right_value)
  :m_begin(true,left_value),m_end(true,right_value)
  {

  }

  MscTimeInterval(const bool& left_closed,const T& left_value,
    const T& right_value,const bool& right_closed)
  :m_begin(left_closed,left_value),m_end(right_closed,right_value)
  {
  }
  MscTimeInterval(const MscIntervalCouple<T>& begin,
    const MscIntervalCouple<T>& end)
  :m_begin(begin),m_end(end)
  {
  }

  MscTimeInterval(const std::string& interval)
    throw(MscIntervalStringConversionError)
  {
    assign(interval);
  }

  void assign(const std::string& interval)
    throw(MscIntervalStringConversionError)
  {
    // remove spaces and get lowercase of string
    std::string tmp = get_small_nospace(interval);

    bool b_closed = true;
    bool e_closed = true;

    if(tmp.empty())
      throw MscIntervalStringConversionError("no time interval");

    //looking for starting brackets
    if(!char_in_string(tmp[0],std::string("([")))
    {
      // ok, start bracket is not there, what about end one?
      if(!char_in_string(tmp[tmp.length()-1],")]"))
      {
        MscIntervalCouple<T> begin(tmp,b_closed);
        MscIntervalCouple<T> end(tmp,e_closed);

        m_begin = begin;
        m_end = end;

        if(!is_valid())
          throw MscIntervalStringConversionError(this->to_string()+std::string(" is not valid interval."));

        return;
      }
      else
        throw MscIntervalStringConversionError(std::string("Could find starting bracket of ")+tmp);
    }
    // detecting ending bracket
    if(!char_in_string(tmp[tmp.length()-1],")]"))
        throw MscIntervalStringConversionError(std::string("Missing ending bracket in ")+tmp);

    // setting up closed
    if(tmp[0]=='(')
      b_closed = false;
    if(tmp[tmp.length()-1]==')')
      e_closed = false;

    // erasing both brackets from string
    tmp.erase(tmp.begin());
    tmp.erase(tmp.end()-1);
    // check for allowed characters

    std::string b(tmp);
    std::string e(tmp);

    // detecting comma
    size_t comma_pos= tmp.find(',');
    if(comma_pos!=std::string::npos)
    {
      e = std::string(tmp.substr(comma_pos+1));
      tmp.erase(comma_pos); // erase all from comma including comma
      b = std::string(tmp);
    }

    // Create MscIntervalCouples
    MscIntervalCouple<T> begin(b,b_closed);
    MscIntervalCouple<T> end(e,e_closed);

    // set them up
    m_begin = begin;
    m_end = end;

    // check validity
    if(!is_valid())
      throw MscIntervalStringConversionError(this->to_string()+std::string(" is not valid interval."));
  }

  const MscIntervalCouple<T>& get_begin() const
  {
    return m_begin;
  }

  T get_begin_value() const
  {
    return m_begin.get_value();
  }

  const bool& get_begin_closed() const
  {
    return m_begin.get_closed();
  }


  const MscIntervalCouple<T>& get_end() const
  {
    return m_end;
  }

  T get_end_value() const
  {
    return m_end.get_value();
  }

  const bool& get_end_closed() const
  {
    return m_end.get_closed();
  }

 void set(T left_value,T right_value)
  {
    m_begin=left_value;
    m_end=right_value;
  }

  void set(bool& left,T& b,T& e,bool& right)
  {
    m_begin=b;
    m_begin.set_closed(left);
    m_end=e;
    m_end.set_closed(right);
  }

  void set_begin(const MscIntervalCouple<T> begin)
  {
    m_begin=begin;
  }

  void set_begin_closed(const bool& b)
  {
    m_begin.set_closed(b);
  }

  void set_end(const MscIntervalCouple<T> end)
  {
    m_end=end;
  }

  void set_end_closed(const bool& b)
  {
    m_end.set_closed(b);
  }

  MscTimeInterval& operator=(const T& value)
  {
    m_begin=m_end=true;
    m_begin=m_end=value;
    return *this;
  }

  MscTimeInterval& operator=(const MscTimeInterval& right)
  {
   if(this==&right)
     return *this;

    m_begin=right.m_begin;
    m_end=right.m_end;

    return *this;
  }

  const MscTimeInterval operator+(const MscTimeInterval& right) const
  {
    return MscTimeInterval(m_begin+right.m_begin,m_end+right.m_end);
  }

  const MscTimeInterval operator+(const T& value) const
  {
    return MscTimeInterval(m_begin+value,m_end+value);
  }

  /**
   * @warning: inteval minus
   * (a,b) - (c,d) = (a-d, b-c)
   */
  const MscTimeInterval operator-(const MscTimeInterval& right) const
  {
    return MscTimeInterval(m_begin-right.m_end,m_end-right.m_begin);
  }

  const MscTimeInterval operator-(const T& value) const
  {
    return MscTimeInterval(m_begin-value,m_end-value);
  }

  const MscTimeInterval operator*(const T& value) const
  {
    return MscTimeInterval(m_begin*value,m_end*value);
  }

  bool operator==(const MscTimeInterval& right) const
  {
    if(this==&right)
      return true;
    return (m_begin==right.m_begin)&&(m_end==right.m_end);
  }

  bool operator!=(const MscTimeInterval& right) const
  {
    return !((m_begin==right.m_begin)&&(m_end==right.m_end));
  }

  bool includes(const T& value) const
  {
    // open and closed intervals are handled by the comparison operators
    // see MscIntervalCouple<>::operator <=
    return m_begin <= value && m_end >= value;
  }

  /**
   * \brief tests if the interval is valid
   * Tests beginning and ending value
   *
   * * beginning value > ending value:
   *     invalid interval
   *
   * *  beginning value < ending value:
   *     valid interval
   *
   * *  MscIntervalException (begin == end, brackets difference)
   *      cases:
   *        (4,4], [4,4)
   *      interval is not valid in both cases.
   *
   * *  beginning value == ending value, brackets are same
   *      cases:
   *        (4,4) - not valid
   *        [4,4] - valid interval
   * \return true if the interval is valid
   */

  bool is_valid() const
  {
    try
    {
      if(m_begin>m_end)
        return false;
    }
    catch(MscIntervalCoupleUncomparable)
    {
      return false;
    }
    if(m_begin.get_value()==std::numeric_limits<T>::infinity())
        return false;

    if(m_end.get_value()==-std::numeric_limits<T>::infinity())
      return false;

    if(m_begin.get_value()==-std::numeric_limits<T>::infinity() && m_begin.get_closed())
      return false;

    if(m_end.get_value()==std::numeric_limits<T>::infinity() && m_end.get_closed())
      return false;

    if(m_begin==m_end && !m_begin.get_closed())
      return false;

    return true;
  }

  /**
   * \brief tests the interval for emptiness
   * Tests beginning and ending value
   *
   * * beginning value < ending value:
   *     interval not empty
   *
   * * beginning value == ending value and bothe ends are closed
   *     interval not empty
   *
   * * this method assumes that there are correct brackets if infty is used
   *   it is sufficient to use this method after intersection of two valid intervals
   *
   * \return true if the interval is empty
   */
  bool is_empty() const
  {
//     if(m_begin.get_value()<m_end.get_value()|| 
//       (m_begin.get_value() == m_end.get_value() && m_begin.get_closed() && m_end.get_closed()))
//         return false;
//     return true;
    return !(m_begin.get_value() < m_end.get_value() || 
       (m_begin.get_value() == m_end.get_value() && m_begin.get_closed() && m_end.get_closed()));
  }

  void set_empty()
  {
    set(1,0);
  }

  void set_infinity()
  {
    m_begin = -std::numeric_limits<T>::infinity();
    m_begin.set_closed(false);
    m_end = std::numeric_limits<T>::infinity();
    m_end.set_closed(false);
  }

  bool is_infinity() const
  {
    return m_begin.get_value() == -std::numeric_limits<T>::infinity() &&
      m_end.get_value() == std::numeric_limits<T>::infinity();
  }

/**
 * \brief checks whether the upper bound of time interval is infinity
 * @returns true if the upper bound of time interval is infinity, false otherwise
 */
  bool is_upper_bound_infinity()
  {
    return m_end.get_value() == std::numeric_limits<T>::infinity();
  }

static MscTimeInterval interval_inverse(const MscTimeInterval& inter)
{
  return MscTimeInterval(inter.get_end_closed(),-inter.get_end_value(), \
    -inter.get_begin_value(), inter.get_begin_closed());
}


/**
 * \brief intersection of two intervals
 * @returns intersection of two intervals
 * @warning returned interval has to be valid, is_valid() may return false, CHECK FOR VALIDITY
 */
static MscTimeInterval interval_intersection(
  const MscTimeInterval& left, const MscTimeInterval& right)
{
  MscIntervalCouple<T> begin;
  try
  {
    begin = std::max(left.m_begin,right.m_begin);
  }
  catch(MscIntervalCoupleUncomparable){
    begin = left.m_begin.get_value();
    begin = false;
  }
  MscIntervalCouple<T> end;
  try
  {
    end = std::min(left.m_end,right.m_end);
  }
  catch(MscIntervalCoupleUncomparable){
    end = left.m_end.get_value();
    end = false;
  }

  return MscTimeInterval(begin,end);
}

/**
 * \brief union of two interval
 * @returns intersection of two intervals if they have intersection, else return empty interval;
 * @warning CHECK if the returned interval is not empty!!!
 */

static MscTimeInterval interval_union(
  const MscTimeInterval& left, const MscTimeInterval& right)
{
  // check if they have intersection if not return empty interval
  if(!interval_intersection(left,right).is_valid())
  {
    MscTimeInterval a;
    a.set_empty();
    return a;
  }
  MscIntervalCouple<T> begin;
  try
  {
    begin = std::min(left.m_begin,right.m_begin);
  }
  catch(MscIntervalCoupleUncomparable){
    begin = left.m_begin.get_value();
    begin = true;
  }
  MscIntervalCouple<T> end;
  try
  {
    end = std::max(left.m_end,right.m_end);
  }
  catch(MscIntervalCoupleUncomparable)
  {
    end = left.m_end.get_value();
    end = true;
  }

  return MscTimeInterval(begin,end);
}
/**
 *@brief return max(begins) and max(ends) Interval
 * Takes two itervals (a,b) (c,d) and returns Inerval(max(a,c),max(b,d))
 */
static MscTimeInterval components_max(
  const MscTimeInterval& left, const MscTimeInterval& right)
{
  MscIntervalCouple<T> begin;
  MscIntervalCouple<T> end;
  try
  {
    begin = std::max(left.m_begin,right.m_begin);
  }
  catch(MscIntervalCoupleUncomparable)
  {
    begin = left.m_begin.get_value();
    begin = false;
  }

  try
  {
    end = std::max(left.m_end,right.m_end);
  }
  catch(MscIntervalCoupleUncomparable)
  {
    end = left.m_end.get_value();
    end = true;
  }

  return MscTimeInterval(begin,end);
}

/**
 * @brief prints interval to the string
 * @return interval string
 */
std::string to_string()
{
  std::ostringstream stream;
  stream << *this;
  return stream.str();
}

friend std::ostream&
operator<<(std::ostream& os, const MscTimeInterval<T>& interval)
{
  if(interval.get_begin_value() != interval.get_end_value())
  {
    if (interval.get_begin_closed())
      os << "[";
    else
      os << "(";

    os
      << interval.get_begin()
      << ","
      << interval.get_end();

    if (interval.get_end_closed())
      os << "]";
    else
      os << ")";
  }
  else
  {
    os
      << "["
      << interval.get_begin()
      << "]";
  }

  return os;
}

}; //end of MscTimeInterval class


/**
 * \brief List of intervals
 *  Keeping sorted list of intervals, dont have intersection
 */
template<class T>
class SCMSC_EXPORT MscTimeIntervalSet
{
private:
  typedef std::list< MscTimeInterval<T> > IntervalList;
  IntervalList m_set;

public:
  MscTimeIntervalSet(){}

  MscTimeIntervalSet(const std::string& s_interval)
  {
    assign(s_interval);
  }

  void assign(const std::string& s_interval)
  {
    int inside = 0;
    std::string tmp;

    for(size_t i=0;i<s_interval.length();i++)
    {
      if(s_interval[i]=='[' || s_interval[i]=='(')
        inside++;
      else if(s_interval[i]==']' || s_interval[i]==')')
        inside--;

      if(inside == 0 && s_interval[i]=='+')
      {
        MscTimeInterval<T> interval(tmp);
        insert(interval);
        tmp="";
      }
      else
        tmp+=s_interval[i];
    }

    MscTimeInterval<T> interval(tmp);
    insert(interval);
  }

  MscTimeIntervalSet(const MscTimeInterval<T>& interval)
  {
      m_set.push_back(interval);
  }

  MscTimeIntervalSet(const MscTimeIntervalSet& set):m_set(set.m_set)
  {
  }

  /**
   *\brief inserts interval to the list on the right possition
   * find the right place in the list for m_begin and eats all
   * the interval on the way to the right place of m_end
   */
  MscTimeInterval<T> insert(const MscTimeInterval<T>& interval)
  {
    MscTimeInterval<T> tmp(interval);

    //in case of empty list
    if(m_set.empty())
    {
      m_set.push_back(interval);
      return m_set.front();
    }
    // Dealing with the right place for m_begin
    typename IntervalList::iterator it;
    for(it=m_set.begin();it!=m_set.end();it++)
    {
      //  interval.m_begin x it->m_begin
      try
      {
        if(interval.get_begin()<it->get_begin()
           || interval.get_begin()==it->get_begin())
        {
          break;
        }
      }
      catch(MscIntervalCoupleUncomparable)
      {
        // it closed false
        // tmp closed true
        if(tmp.get_begin_closed())
          break;
        // it closed true
        // tmp closed false
        if(it->get_begin_closed())
        {
          tmp.set_begin_closed(true);
          break;
        }
        // it closed false
        // tmp closed false
        break;
      }
      // interval.m_begin x it->m_end
      try
      {
        if(tmp.get_begin()<it->get_end()
          || tmp.get_begin()==it->get_end())
        {
          tmp.set_begin(it->get_begin());
          break;
        }
      }
      catch(MscIntervalCoupleUncomparable)
      {
        if(!tmp.get_begin_closed()&&!it->get_end_closed())
        {
          it++;
          break;
        }
        tmp.set_begin(it->get_begin());
        break;
      }
    } // end of for

    // Dealing with the end of interval
    typename IntervalList::iterator it2;
    it2=it; // keeping position iterator, just in case

    for(;it2!=m_set.end();m_set.erase(it2++))
    {
      // interval.m_end x it2->m_begin
      try
      {
        if(tmp.get_end()<it2->get_begin())
        {
          m_set.insert(it2,tmp);
          return tmp;
        }
        if(interval.get_end()==it2->get_begin())
        {
          tmp.set_end(it2->get_end());
          m_set.insert(it2,tmp);
          m_set.erase(it2);
          return tmp;
        }
      }
      catch(MscIntervalCoupleUncomparable)
      {
       if(!tmp.get_end_closed()&&!it2->get_begin_closed())
        {
          m_set.insert(it2,tmp);
          return tmp;
        }

        tmp.set_end(it2->get_end());
        m_set.insert(it2,tmp);
        m_set.erase(it2);
        return tmp;
      }

      // interval.m_end x it2->m_end
      try
      {
        if(interval.get_end()<it2->get_end())
        {
          tmp.set_end(it2->get_end());
          m_set.insert(it2,tmp);
          m_set.erase(it2);
          return tmp;
        }
        if(interval.get_end()==it2->get_end())
        {
          m_set.insert(it2,tmp);
          m_set.erase(it2);
          return tmp;
        }
      }
      catch(MscIntervalCoupleUncomparable)
      {
        if(!tmp.get_end_closed()&&!it2->get_end_closed())
        {
          m_set.insert(it2,tmp);
          m_set.erase(it2);
          return tmp;
        }

        tmp.set_end_closed(true);
        m_set.insert(it2,tmp);
        m_set.erase(it2);
        return tmp;
      }
    }

     // it2==m_set.end()
      m_set.push_back(tmp);
      return tmp;
  }


  const MscTimeIntervalSet operator+(const MscTimeIntervalSet& set)
  {
    MscTimeIntervalSet<T> new_set;

    for(typename IntervalList::const_iterator it=m_set.begin();
      it != m_set.end(); it++)
    {
      for(typename IntervalList::const_iterator it2 = set.m_set.begin();
        it2 != set.m_set.end(); it2++)
      {
        new_set.insert(*it+*it2);
      }
    }

    return new_set;
  }

  MscTimeIntervalSet operator-(const MscTimeIntervalSet& set)
  {
    MscTimeIntervalSet<T> new_set;

    for(typename IntervalList::const_iterator it = m_set.begin();
      it != m_set.end(); it++)
    {
      for(typename IntervalList::const_iterator it2 = set.m_set.begin();
        it2 != set.m_set.end(); it2++)
      {
        new_set.insert(*it-*it2);
      }
    }

    return new_set;
  }

  const MscTimeIntervalSet& operator=(const MscTimeIntervalSet& set)
  {
    if(this==&set)
      return *this;
    m_set=set.m_set;
    return *this;
  }

  const bool operator==(const MscTimeIntervalSet& set)
  {
    if(this==&set)
      return true;
    return set.m_set==this->m_set;
  }

  const bool operator!=(const MscTimeIntervalSet& set)
  {
    if(this==&set)
      return false;
    return set.m_set!=this->m_set;
  }

  bool includes(const T& value) const
  {
    for(typename IntervalList::const_iterator it = m_set.begin();
      it != m_set.end(); it++)
    {
      // if included in at least one of the intervals in the union
      if(it->includes(value))
        return true;
    }

    return false;
  }

  void clear(){
    m_set.clear();
  }

  const IntervalList& get_set() const
  {
    return m_set;
  }

  bool is_empty() const
  {
    return m_set.empty();
  }

  void set_infinity()
  {
    MscTimeInterval<T> interval;
    interval.set_infinity();

    m_set.clear();
    m_set.push_back(interval);
  }

  bool is_infinity() const
  {
    typename IntervalList::const_iterator it;
    // if at least one of the intervals in the union is infinite
    for(it=m_set.begin();it!=m_set.end();it++)
    {
      if(it->is_infinity())
        return true;
    }

    return false;
  }

  static MscTimeIntervalSet<T> set_union(
    const MscTimeIntervalSet& left, const MscTimeIntervalSet& right)
  {
    MscTimeIntervalSet<T> new_set;
    new_set.m_set = left.m_set;
    for(typename IntervalList::const_iterator it = right.m_set.begin();
      it != right.m_set.end(); it++)
    {
      new_set.insert(*it);
    }
    return new_set;
  }

  static MscTimeIntervalSet<T> set_intersection(
    const MscTimeIntervalSet& left, const MscTimeIntervalSet& right)
  {
    MscTimeIntervalSet<T> new_set;
    for(typename IntervalList::const_iterator it = left.m_set.begin();
      it != left.m_set.end(); it++)
    {
      for(typename IntervalList::const_iterator it2 = right.m_set.begin();
        it2 != right.m_set.end(); it2++)
      {
        MscTimeInterval<T> tmp;
        tmp = MscTimeInterval<T>::interval_intersection(*it,*it2);
        if(tmp.is_valid())
          new_set.insert(tmp);
	//else
	//  std::cerr<< "not valid " << tmp << std::endl;
      }
    }
    return new_set;
  }

  static MscTimeIntervalSet<T> components_max(
    const MscTimeIntervalSet& left, const MscTimeIntervalSet& right)
  {
    MscTimeIntervalSet<T> new_set;
    for(typename IntervalList::const_iterator it = left.m_set.begin();
      it != left.m_set.end(); it++)
    {
      for(typename IntervalList::const_iterator it2 = right.m_set.begin();
        it2 != right.m_set.end(); it2++)
      {
        MscTimeInterval<T> tmp;
        tmp = MscTimeInterval<T>::components_max(*it,*it2);
        if(tmp.is_valid())
          new_set.insert(tmp);
      }
    }
    return new_set;
  }

  static MscTimeIntervalSet interval_inverse(const MscTimeIntervalSet& inter)
  {
    MscTimeIntervalSet<T> new_set;
     for(typename IntervalList::const_iterator it = inter.m_set.begin();
      it != inter.m_set.end(); it++)
    {
      new_set.insert(MscTimeInterval<T>::interval_inverse(*it));
    }

    return new_set;
  }

  /**
   *\brief makes intersection of arguments
   * if intersection is empty set returns empty set
   * else returns interval [0,maxValue(intersection) with
   * appropriate right boundary ("]" or ")")
   *
   */
  static MscTimeIntervalSet<T> zero_max_interval(
    const MscTimeIntervalSet& left, const MscTimeIntervalSet& right)
  {
    MscTimeIntervalSet<T> inter = set_intersection(left,right);
    if(inter.is_empty())
      return inter;

    MscTimeIntervalSet<T> ret;
    MscTimeInterval<T> back = inter.m_set.back();
    MscTimeInterval<T> new_int(true,0,back.get_end_value(),back.get_end_closed());

    ret.insert(new_int);
    return ret;
  }

/**
 * @brief prints interval set to the string
 * @return interval set string
 */
std::string to_string()
{
  std::ostringstream stream;
  stream << *this;
  return stream.str();
}

/**
 * @brief output stream of interval set
 * Intervals are connected by "+" character
 * interval+interval+interval+interval+...
 * (3,4)+(5,6)+[10,13]
 * @return stream with printed interval set
 */
friend std::ostream&
operator<<(std::ostream& os, const MscTimeIntervalSet<T>& interval)
{
  if(interval.m_set.empty())
    return os << "";
  std::list <MscTimeInterval<T> > set;
  set=interval.m_set;
  typename IntervalList::iterator it = set.begin();
  // print the first one in the set
  os << *it;
  // set iterator to the second element
  it++;
  for(;it!=set.end();it++)
    os << "+" << *it; // and every other one gets "+" in front of it
  return os;
}

};


#endif // _TIME_H_

// $Id: time.h 1076 2011-04-05 15:35:43Z lkorenciak $
