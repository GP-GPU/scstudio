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
 * $Id: fifo_checker.h 582 2010-02-10 20:46:58Z vacek $
 */

#include <typeinfo>

#include <stack>
#include <string>
#include <vector>

#include "data/msc.h"
#include "data/checker.h"
#include "check/pseudocode/visual_closure_initiator.h"
#include "data/hmsc_reference_checker.h"
#include "check/order/export.h"

class FifoChecker;

typedef std::stack<Event*> EventPStack;
typedef boost::shared_ptr<FifoChecker> FifoCheckerPtr;

/** \brief Checks the FIFO property of a MSC.
 *
 * BMSC is FIFO if for all receive events c, d and their matching send events a, b it holds
 * that c < d => a < b, where < is the visual order and the two messages belong to the same
 * channel.
 * HMSC is FIFO if all referenced BMSCs satisfy FIFO.
 *
 */

class SCORDER_EXPORT FifoChecker
  : public Checker, public BMscChecker, public HMscReferenceChecker<FifoChecker>
{
  
protected:
  
  /**
   * Common instance
   */
  static FifoCheckerPtr m_instance;
  
  /**
   * Events with modified dynamic attributes
   */
  EventPStack m_modified_events;

  BMscPtr create_counter_example(BMscPtr& bmsc, Event* receive1, Event* receive2);
  
  /**
   * Checks whether e1 and e2 (from equivalent channel) are in consistent order.
   *
   * An e1 is supposed to be before e2. If this holds true is returned, false 
   * otherwise.
   */
  bool consistent_order(VisualClosureInitiator& initiator, Event* e1, Event* e2)
  {
    BoolVector& e1_closure = initiator.get_visual_closure(e1);
    size_t e2_index = initiator.get_topology_index(e2);
    return e1_closure[e2_index];
  }

public:

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"FIFO"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"fifo/fifo.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Name of channel id attribute
   */
  static const std::string channel_id_attribute;
  
  /**
   * Supports only SRChannelMapper and SRLChannelMapper
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    SRChannelMapperPtr p1 = boost::dynamic_pointer_cast<SRChannelMapper>(chm);
    if(p1 != NULL)
      return true;

    SRMChannelMapperPtr p2 = boost::dynamic_pointer_cast<SRMChannelMapper>(chm);
    if(p2 != NULL)
      return true;

    return false;
  }

  /**
   * Checks whether bmsc contains overtaking messages. 
   */
  std::list<BMscPtr> check(BMscPtr bmsc, ChannelMapperPtr mapper);
  
  void cleanup_attributes();
  
  /**
   * Returns common instance of FifoChecker
   */
  static FifoCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = FifoCheckerPtr(new FifoChecker());
    return m_instance;
  }
  
  /**
   * Setter of channel id attribute of message.
   */
  void set_channel_id(Event* e, size_t id)
  {
    e->set_attribute<size_t>(channel_id_attribute,id);
  }
  
  /**
   * Getter of channel id attribute of m.
   */
  size_t get_channel_id(Event* e)
  {
    return e->get_attribute<size_t>(channel_id_attribute,0);
  }
  
  /**
   * Determines whether event should be checked or not.
   *
   * Event should be checked if it is receive event and has matching event.
   */
  static bool should_be_checked(Event* e)
  {
    return e->is_receive() && e->is_matched();
  }
  
};

// $Id: fifo_checker.h 582 2010-02-10 20:46:58Z vacek $
