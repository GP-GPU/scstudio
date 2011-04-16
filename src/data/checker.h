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
 * $Id: checker.h 582 2010-02-10 20:46:58Z vacek $
 */

#ifndef _CHECKER_H
#define _CHECKER_H

#include <exception>
#include <map>

#include <boost/shared_ptr.hpp>

#include "data/msc.h"
#include "data/prerequisite_check.h"

class ChannelMapper;
class Checker;
class HMscChecker;
class BMscChecker;

typedef boost::shared_ptr<ChannelMapper> ChannelMapperPtr;
typedef boost::shared_ptr<Checker> CheckerPtr; 
typedef boost::shared_ptr<HMscChecker> HMscCheckerPtr;
typedef boost::shared_ptr<BMscChecker> BMscCheckerPtr;

/**
 * Basic abstract class for all checking algorithms.
 */
class SCMSC_EXPORT Checker
{
  
public:

  /**
   * Human readable name of the property being checked.
   */
  virtual std::wstring get_property_name() const = 0;

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const = 0;

  //! List of properties that must be satisfied before executing the check.
  typedef std::vector<PrerequisiteCheck> PreconditionList;
  //! Returns a list of preconditions for the check.
  virtual PreconditionList get_preconditions(MscPtr msc) const = 0;

  /**
   * Removes no more needed attributes.
   *
   * Descendat of this class should remove attributes of MscElements that are no 
   * more needed. This method should be called after finish of algorithm.
   */
  virtual void cleanup_attributes()=0;
  
  /**
   * Checks whether Checker supports given ChannelMapper.
   *
   * Deafult behaviour is false for all mappers, but it is neccessary to check 
   * out this behaviour in individual checkers.
   */
  virtual bool is_supported(ChannelMapperPtr chm)=0;
  
  virtual ~Checker()
  {
    
  }
};

//! module initialization function
typedef Checker** (*FInitCheckers)();

/**
* Basic abstract class for checking algorithms of HMsc.  
*/
class SCMSC_EXPORT HMscChecker
{

protected:

  /**
   * Hidden constructor.
   */
  HMscChecker()
  {
  }

public:
  virtual ~HMscChecker() {}
  
  /**
   * Checks HMsc against specific property.
   *
   * Returns a list of MscPathPtr with violating examples if there are any in hmsc 
   * otherwise the list is empty.
   * @param hmsc - HMsc to be checked 
   * @param mapper - ChannelMapper which is chosen as delivery semantic
   */ 
  virtual std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr mapper)=0;
};

/**
* Basic abstract class for checking algorithms of BMsc.
*/
class SCMSC_EXPORT BMscChecker
{
  
protected:
  
  /**
   * Singleton instance.
   */
  static BMscCheckerPtr m_checker;
  
public:
  virtual ~BMscChecker() {}

  /**
   * Checks BMsc against specific property.
   *
   * Returns a list of BMscPtr with violating examples if there are any in bmsc otherwise
   * the list is empty.
   * @param bmsc - BMsc to be checked
   * @param mapper - ChannelMapper which is chosen as delivery semantic
   */
  virtual std::list<BMscPtr> check(BMscPtr bmsc, ChannelMapperPtr mapper)=0;
};

/**
 * Abstract class whose purpose is to decide whether two Messages belongs to the 
 * same channel or not and assign to a message ID of channel which the message 
 * belongs to. This class shortly defines delivery semantic.
 */
class ChannelMapper
{
  
public:
  
  virtual ~ChannelMapper()
  {
    
  }
  
  /**
   * Returns true if e1's message belongs to the same channel as e2's message.
   */
  virtual bool same_channel(const Event* e1, const Event* e2) const=0;
  
  /**
   * Returns index of channel which event's message belongs into.
   *
   * @warning event must have set message
   */
  virtual size_t channel(const Event* event)=0;
  
  /**
   * Returns number of different channels mapped by this channel mapper
   */
  virtual size_t get_channel_count()=0;
  
  /**
   * Returns copy of this channel mapper without registered channels.
   *
   * Usefull if you need to have the same functionality of the original mapper,
   * but you don't want to work with all registered events by the original one.
   * This approach may increase efficiency in some cases.
   */
  virtual ChannelMapperPtr copy()=0;
};

/**
 * Basic implementation of ChannelMapper.
 */
template<typename MessagePart>
class GeneralMapper:public ChannelMapper
{
  
public:
  
  typedef std::map<MessagePart,size_t> ChannelMap;
    
protected:
  
  /**
   * Common instance of GeneralMapper.
   */
  static boost::shared_ptr<GeneralMapper<MessagePart> > m_instance;
  
  /**
   * Holds identificators of channels represented by MessagePart of MscMessage.
   */
  std::map<MessagePart,size_t> m_channels;

public:

  /**
   * Returns true if m1 belongs to the same channel as m2.
   */
  bool same_channel(const Event* e1, const Event* e2) const
  {
    return MessagePart::same_channel(e1,e2);
  }
  
  /**
   * Returns index of channel which event's message belongs into.
   */
  size_t channel(const Event* event)
  {
    MessagePart part(event);
    typename ChannelMap::iterator i = m_channels.find(part);
    if(i==m_channels.end())
    {
      size_t id = m_channels.size();
      m_channels[part] = id;
      return id;
    }
    return i->second;
  }
  
  /**
   * Returns common instance of ChannelMapper
   */
  static boost::shared_ptr<GeneralMapper<MessagePart> > instance()
  {
    if(m_instance.get()==NULL)
    {
      boost::shared_ptr<GeneralMapper<MessagePart> > p(new GeneralMapper<MessagePart>());
      m_instance = p;
    }
    return m_instance;
  }
  
  /**
   * Returns number of channel registered by this channel mapper
   */
  size_t get_channel_count()
  {
    return m_channels.size();
  }
  
  /**
   * Returns copy of this channel mapper without registered channels.
   *
   * Usefull if you need to have the same functionality of the original mapper,
   * but you don't want to work with all registered events by the original one.
   */
  ChannelMapperPtr copy()
  {
    return ChannelMapperPtr(new GeneralMapper<MessagePart>());
  }
  
};

/**
 * Used in GeneralMapper template to decide whether two messages belongs to the
 * same channel (delivery semantic).
 *
 * For decision send and receive instances' labels are used. 
 *
 * To see delivery semantic of this class see operator<() and same_channel() 
 * methods of this class.
 */
class SRMessagePart 
{
  
  std::wstring m_sender;
  
  std::wstring m_receiver;
  
public:
  
  SRMessagePart(const Event* e)
  {
    m_sender = e->get_sender_label();
    m_receiver = e->get_receiver_label();
  }

  /**
   * Used in std::map as comparision function
   */
  bool operator<(const SRMessagePart& mp) const
  {
    int s = m_sender.compare(mp.m_sender);
    int r = m_receiver.compare(mp.m_receiver);
    return s<0 || s+r<0;
  }

  /**
   * Channels of message m1 and m2 are same if and only if m1 has got same 
   * sender and receiver as m2.
   */
  static bool same_channel(const Event* e1, const Event* e2)
  {
    if((e1->is_send() && e2->is_send()) || (e1->is_receive() && e2->is_receive()))
    {
      return e1->get_sender_label()==e2->get_sender_label() &&
        e1->get_receiver_label()==e2->get_receiver_label();
    } 
    return false;
  }
};

/**
 * Used in GeneralMapper template to decide whether two messages belongs to the
 * same channel (delivery semantic).
 *
 * For decision send and receive instances' labels and message label are used. 
 *
 * To see delivery semantic of this class see operator<() and same_channel() 
 * methods of this class.
 */
class SRMMessagePart {

  std::wstring m_sender;

  std::wstring m_receiver;

  std::wstring m_label;

public:

  SRMMessagePart(const Event* e)
  {
    m_sender = e->get_sender_label();
    m_receiver = e->get_receiver_label();
    m_label = e->get_message()->get_label();
  }

  bool operator<(const SRMMessagePart& mp) const
  {
    int s = m_sender.compare(mp.m_sender);
    int r = m_receiver.compare(mp.m_receiver);
    int l = m_label.compare(mp.m_label);
    return s<0 || (s==0 && (r<0 || (r==0 && l<0)));
  }

  /**
   * Channels of message of e1 and e2 are same if and only if e1 has got same 
   * sender, receiver and label as e2.
   */
  static bool same_channel(const Event* e1,const Event* e2)
  {
    return e1->get_sender_label()==e2->get_sender_label() &&
            e1->get_receiver_label()==e2->get_receiver_label() &&
            e1->get_message()->get_label()==e2->get_message()->get_label();
  }
};

typedef GeneralMapper<SRMessagePart> SRChannelMapper;
typedef GeneralMapper<SRMMessagePart> SRMChannelMapper;

typedef boost::shared_ptr<SRChannelMapper> SRChannelMapperPtr;
typedef boost::shared_ptr<SRMChannelMapper> SRMChannelMapperPtr;

template <class T> 
boost::shared_ptr<GeneralMapper<T> > GeneralMapper<T>::m_instance;

enum NodeColor
{
  WHITE,
  GRAY,
  BLACK
};

/**
 * Trohwn when acyclic relation is required and cycle was found.
 */
class CycleDetectedException:public std::exception
{
  virtual const char* what() const throw()
  {
    return "Acyclic graph expected but there was found cycle";
  }
};

/**
 * Thrown when unsupported channel mapper acquired.
 */
class UnsupportedMapperException:public std::exception
{
  virtual const char* what() const throw()
  {
    return "Unsupported channel mapper";
  }
};

#endif /* _CHECKER_H */

// $Id: checker.h 582 2010-02-10 20:46:58Z vacek $
