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
 * $Id: acyclic_checker.h 794 2010-05-21 06:46:05Z vacek $
 */

#include "data/checker.h"
#include "data/msc.h"
#include "data/dfs_events_traverser.h"
#include "data/hmsc_reference_checker.h"
#include "check/order/export.h"
#include "data/elementary_cycles_traverser.h"

class AcyclicChecker;

class FindCycles: public ElementaryCycleListener
{
private:
  MscElementPListList m_cycles;
public:
  void on_elementary_cycle_found(const MscElementPList &cycle)
  {
    m_cycles.push_back(cycle);
  }
  const MscElementPListList& get_cycles(void)
  {
    return m_cycles;
  }
};
typedef boost::shared_ptr<AcyclicChecker> AcyclicCheckerPtr;

class AssignHMscListener: public WhiteEventFoundListener
{
private:
  HMsc* m_hmsc;
public:
  AssignHMscListener(HMsc *hmsc)
    :m_hmsc(hmsc)
  {}
  void on_white_event_found(Event *e)
  {
    ConnectionNode *node = new ConnectionNode();
    m_hmsc->add_node(node);
    e->set_attribute("ACC_node", node);
    node->set_attribute("ACC_event", e);
  }
};

class MessageSuceessors: public SendReceivePairListener
{
public:
  void on_send_receive_pair(Event *send, Event *receive)
  {
    ConnectionNode *first, *second;
    first = send->get_attribute("ACC_node", second);
    second = receive->get_attribute("ACC_node", first);
    NodeRelationPtr nr = first->add_successor(second);
    nr->set_attribute("ACC_message", send->get_message().get());
    send->set_attribute("ACC_relation", nr.get());
  }
};

class InstanceSuccessors: public EventSuccessorListener
{
public:
  void on_event_successor(Event *event, Event *successor)
  {
    ConnectionNode *first, *second;
    first = event->get_attribute("ACC_node", second);
    second = successor->get_attribute("ACC_node", first);
    first->add_successor(second);
  }
};

class CleanHMscListener: public WhiteEventFoundListener
{
public:
  void on_white_event_found(Event *e)
  {
    ConnectionNode *node, *dummy;
    NodeRelation* nr;
    node = e->get_attribute<ConnectionNode*>("ACC_node", dummy);
    if(e->is_attribute_set("ACC_relation"))
    {
      nr = e->get_attribute<NodeRelation*>("ACC_relation", NULL);
      nr->remove_attribute<MscMessage*>("ACC_message");
      e->remove_attribute<NodeRelation*>("ACC_relation");
    }
    node->remove_attribute<Event*>("ACC_event");
    e->remove_attribute<ConnectionNode*>("ACC_node");
  }
};

class SCORDER_EXPORT AcyclicChecker
  : public Checker, public BMscChecker, public HMscReferenceChecker<AcyclicChecker>
{
  
protected:
  
  /**
   * Common instance.
   */
  static AcyclicCheckerPtr m_instance;
  
public:

  AcyclicChecker()
  {
  }
  
  ~AcyclicChecker()
  {
  }

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Acyclic"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"acyclic/acyclic.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether bmsc has acyclic events' dependecy
   */
  std::list<BMscPtr> check(BMscPtr bmsc, ChannelMapperPtr chm);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes()
  {
  }
  
  /**
   * Supports all mappers
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }
  
  static AcyclicCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = AcyclicCheckerPtr(new AcyclicChecker());
    return m_instance;
  }

};
 
// $Id: acyclic_checker.h 794 2010-05-21 06:46:05Z vacek $
