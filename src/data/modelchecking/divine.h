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
 * Copyright (c) 2009 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: divine.h 1026 2011-01-09 13:14:07Z vacek $
 */

#ifndef _DIVINE_H
#define _DIVINE_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "data/formatter.h"
#include "data/modelchecking/export.h"

#include "data/msc.h"
#include "data/node_finder.h"
#include "data/dfs_instance_events_traverser.h"
#include "data/dfs_refnode_hmsc_traverser.h"
#include "data/dfs_bmsc_graph_traverser.h"
#include "check/pseudocode/msc_duplicators.h"

class NonemptyNodeFinder: public NodeFinder
{
private:
	virtual bool is_terminal(HMscNode *node)
	{
		if(dynamic_cast<ConnectionNode*>(node))
			return false;
		if(dynamic_cast<EndNode*>(node))
			return false;
		if(dynamic_cast<ConditionNode*>(node))
			return false;
		if(dynamic_cast<StartNode*>(node))
			return false;

		ReferenceNode *ref = dynamic_cast<ReferenceNode*>(node);
		if(!ref)
			return false;

		for(InstancePtrList::const_iterator it = ref->get_bmsc()->get_instances().begin();
			it != ref->get_bmsc()->get_instances().end();
			it++)
			if((*it)->has_events())
				return true;
		return false;
	}
};
class SCMODELCHECKING_EXPORT Divine: public Formatter, public ExportFormatter
{
public:
  //! file extension used to distinguish this format
  // note: DLL in Windows cannot return pointers to static data
  virtual std::string get_extension() const
  { return "dve"; }
  //! human readable description of this format
  virtual std::string get_description() const
  { return "DiVinE format"; }

  //! Returns a list of preconditions for this format.
  virtual PreconditionList get_preconditions(MscPtr msc) const;

  //! export MSC document
  virtual int save_msc(std::ostream& stream, const std::wstring &name,
    const MscPtr& selected_msc, const std::vector<MscPtr>& msc = std::vector<MscPtr>());

  void reset(void)
  {
    m_message_map.clear();
    m_instance_map.clear();
    m_instance_names.clear();
		m_receive_counts.clear();
  }

  static const std::string node_id_attribute;

protected:

  std::map<std::wstring, unsigned> m_message_map;
  std::map<std::wstring, unsigned> m_instance_map;
  std::vector<std::string> m_instance_names;
  std::vector<unsigned> m_receive_counts;
};

class NodeCounterListener: public WhiteEventFoundListener
{
private:
  unsigned m_received;
  unsigned m_total;
public:
  NodeCounterListener()
    :m_received(0), m_total(0)
  {}
  void reset(void)
  {
    m_received = 0;
    m_total = 0;
  }
  void on_white_event_found(Event *e);
  unsigned get_received_count()
  {
    return m_received;
  }
  unsigned get_total_count()
  {
    return m_total;
  }
};


class MessageIdListener: public WhiteEventFoundListener
{
  std::map<std::wstring, unsigned>& m_message_map;
public:
  MessageIdListener(std::map<std::wstring, unsigned>& message_map)
    :m_message_map(message_map)
  {}
  void on_white_event_found(Event *e); 
};

class ProcessListener: public WhiteNodeFoundListener
{
private:
	std::string m_current_instance;
	

	std::ostream &m_stream;
  std::map<std::wstring, unsigned>& m_message_map;
  std::map<std::wstring, unsigned>& m_instance_map;
  bool m_states_only;
  void process_successor_transitions(HMscNode *n, std::string first_node);
public: 
	ProcessListener(std::ostream& stream,
    std::map<std::wstring, unsigned>& message_map,
    std::map<std::wstring, unsigned>& instance_map)
    :m_stream(stream),
     m_message_map(message_map),
     m_instance_map(instance_map),
		 m_states_only(true)
  {}
	void on_white_node_found(HMscNode *n);
	void set_instance(std::string instance)
	{
		m_current_instance = instance;
	}
	void set_states()
	{
		m_states_only = true;
	}
	void set_transitions()
	{
		m_states_only = false;
	}
};
class PreprocessListener: public WhiteNodeFoundListener
{
private:
  std::map<std::wstring, unsigned>& m_message_map;
  std::map<std::wstring, unsigned>& m_instance_map;
  std::vector<std::string>& m_instance_names;
  std::vector<unsigned>& m_receive_counts;
  unsigned m_node_number;
  unsigned m_instance_count;

public:
  void on_white_node_found(HMscNode *node);
  PreprocessListener(std::map<std::wstring, unsigned>& message_map,
    std::map<std::wstring, unsigned>& instance_map, std::vector<std::string>& instance_names,
    std::vector<unsigned>& receive_counts)
    :m_message_map(message_map),
     m_instance_map(instance_map), m_instance_names(instance_names),
     m_receive_counts(receive_counts)
  {
    m_node_number = 0;
    m_instance_count = 0;
  }
};

class CleanupListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *node)
	{
		node->remove_attribute<unsigned>(Divine::node_id_attribute);
	}

};


#endif /* _DIVINE_H */

// $Id: divine.h 1026 2011-01-09 13:14:07Z vacek $
