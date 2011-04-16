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
 * $Id: session_attribute.h 920 2010-09-13 19:38:03Z gotthardp $
 */

#ifndef _SESSION_ATTRIBUTE_H_
#define _SESSION_ATTRIBUTE_H_

#include <set>
#include <string>
#include <exception>
#include <stack>
#include "data/msc.h"
#include "data/export.h"

/**
 * Works with MscElements attrubutes: setter and getter.
 * What was set through SesstionAttribute is automaticly
 * removed when the object is destroyed.
 */

class SCMSC_EXPORT SA
{
protected:
  //! static set that keeps attributes in use
  static std::set<std::string> RESERVED_ATTR;
  //! name of attribute
  const std::string m_name;

  //! insert attribute in to the set, throw exception, if there is already one with the same name
  void reserve_attribute();

  //! remove attribute from the set of used attribute
  void cancel_attribute();

  SA(std::string name):m_name(name)
  {
    reserve_attribute();
  }

  virtual ~SA();

};

template<class Type>
class SessionAttribute: public SA
{

private:
  std::stack<MscElementPtr> m_elements; //! stack of MscElements with set attribute
  const Type m_def; //! default value to set if attribute isnt set

public:
  /**
   * @param name is name for new atrribute
   * @param def is default value for attribute
   */
  SessionAttribute(const std::string name, const Type& def):SA(name),m_def(def)
  {
  }

  ~SessionAttribute()
  {
    if(!m_elements.empty())
    {
      std::cerr << "SessionAttribute Warning: removing attribute "
        << m_name << " in Destructor";
      clean_up();
    }
  }

  const std::string get_name()
  {
    return m_name;
  }

  /**
   * Gets atrribute m_name from the MscElement e, if the attribute
   * m_name isnt set, default value def is used to set attribute
   * 
   * @param e is MscElement to get attribute from
   * @param def is value to set if the attribute isnt set
   */
  Type& get(MscElement* e, const Type& def)
  {
    bool created(false);
    Type& ret = e->get_attribute<Type>(m_name,def,created);
    if(created)
    {
      m_elements.push(e);
    }
    return ret;
  }

  /**
   * Gets atrribute m_name from the MscElement e, if the attribute
   * m_name isnt set, default value m_def is used to set attribute
   *
   * @param e is MscElement to get attribute from
   */
  Type& get(MscElement* e)
  {
    bool created(false);
    Type& ret = e->get_attribute<Type>(m_name,m_def,created);
    if(created)
    {
      m_elements.push(e);
    }
    return ret;
  }

  /**
   * Set attribute m_name with value attribute to the MscElement e
   *@param e MscElement
   *@param attribute to set to MscElement
   *@Warning MscElements could are inserted to the stack EVERY TIME
   * attribute is set.
   */
  void set(MscElement* e,const Type& attribute)
  {
    e->set_attribute<Type>(m_name,attribute);
    m_elements.push(e); //! WARNING
  }

  /**
   * Cleaning funtion, removes set attributes from the MscElements
   */
  void clean_up()
  {
    while(!m_elements.empty())
    {
      MscElementPtr e;
      e = m_elements.top();
      m_elements.pop();
      e->remove_attribute<Type>(m_name);
    }
  }
};

#endif // _SESSION_ATTRIBUTE_H_

// $Id: session_attribute.h 920 2010-09-13 19:38:03Z gotthardp $
