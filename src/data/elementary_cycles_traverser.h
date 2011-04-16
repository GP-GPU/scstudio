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
 * Copyright (c) 2010 Vaclav Vacek <vacek@ics.muni.cz>
 *
 * $Id: elementary_cycles_traverser.h 746 2010-05-08 12:14:32Z gotthardp $
 */

#ifndef _ELEMENTARY_CYCLES_TRAVERSER_H
#define _ELEMENTARY_CYCLES_TRAVERSER_H

#include "data/dfs_hmsc_traverser.h"

class AssignListener: public WhiteNodeFoundListener
{
public:
  AssignListener(void)
    :m_node_number(0)
  {}
  void on_white_node_found(HMscNode *n)
  {
    n->set_attribute("Cycles_number", m_node_number++);  
  }
  unsigned get_vertex_count(void)
  {
    return m_node_number;
  }
private:
  unsigned m_node_number;
};

class CleanupListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    n->remove_attribute<unsigned>("Cycles_number");
  }
};

class TarjanCycles
{
public:
  MscElementPListList circuit_enumeration(HMscNode *n);
  TarjanCycles(unsigned vertex_count, std::string restriction)
    :m_restriction(restriction), m_vertex_count(vertex_count)
  {}
private:
  std::string m_restriction;
  unsigned m_s; //the value of the root of the backtracking tree
  MscElementPListList m_result; //needs to be shared among procedures
  std::vector<unsigned> m_point_stack;
  std::vector<unsigned> m_marked_stack;
  MscElementPList m_current_path;
  std::vector<bool> m_mark;
  bool backtrack(HMscNode *n);
  unsigned m_vertex_count;
};

class SCMSC_EXPORT ElementaryCycleListener
{
public:
  virtual ~ElementaryCycleListener()
  {}
  virtual void on_elementary_cycle_found(const MscElementPList &cycle) = 0;
};

typedef std::list<ElementaryCycleListener*> ElementaryCycleListenerPList;

class SCMSC_EXPORT ElementaryCyclesTraverser
{
  
protected:
  ElementaryCycleListenerPList m_cycle_listeners;
  void cycle_found(const MscElementPList &cycle);
  virtual bool is_applicable(HMscNode *n)
  {
    return true;
  }
  DFSHMscTraverser m_traverser;
  bool m_restriction_enabled;
  std::string m_restriction_name;

public:
  virtual ~ElementaryCyclesTraverser()
  {}

   void add_cycle_listener(ElementaryCycleListener* l)
   {
     m_cycle_listeners.push_back(l);
   }

   void remove_cycle_listeners()
   {
      m_cycle_listeners.erase(
      m_cycle_listeners.begin(),m_cycle_listeners.end());
   }
   void traverse(HMscPtr hmsc);
   ElementaryCyclesTraverser(void)
     :m_traverser("EC_color"), m_restriction_enabled(false), m_restriction_name("")
   {}
   void cleanup_traversing_attributes(void)
   {
     m_traverser.cleanup_traversing_attributes();
   }
   void enable_restriction(std::string name)
   {
     m_restriction_name = name;
     if(name != "")
       m_restriction_enabled = true;
   }
   void disable_restriction(void)
   {
     m_restriction_name = "";
     m_restriction_enabled = false;
   }

   bool is_restriction_enabled(void)
   {
     return m_restriction_enabled;
   }
};

class CycleListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n);
  CycleListener(unsigned vertex_count, const ElementaryCycleListenerPList &listeners, const std::string &restriction)
    :m_vertex_count(vertex_count), m_restriction(restriction), m_listeners(listeners) 
  {}

private:
  unsigned m_vertex_count;
  std::string m_restriction;
  const ElementaryCycleListenerPList &m_listeners;
};

#endif /* _ELEMENTARY_CYCLES_TRAVERSER_H */

// $Id: elementary_cycles_traverser.h 746 2010-05-08 12:14:32Z gotthardp $
