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
 * Copyright (c) 2008 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: universal_boundedness_checker.h 789 2010-05-16 20:44:30Z vacek $
 */

#include <vector>
#include <map>
#include "data/checker.h"
#include "data/msc.h"
#include "check/boundedness/export.h"
#include "data/elementary_cycles_traverser.h"
#include "data/dfs_hmsc_traverser.h"
#include "check/pseudocode/communication_graph.h"
#include "check/pseudocode/msc_duplicators.h"

class UniversalBoundednessChecker;


typedef boost::shared_ptr<UniversalBoundednessChecker> UniversalBoundednessCheckerPtr;

class BoundednessAssignListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n);
  BoundednessAssignListener(const std::map<std::wstring, unsigned> &map)
    :m_node_number(0), m_label_map(map)
  {}
  unsigned get_vertex_count(void)
  {
    return m_node_number;
  }
private:
  unsigned m_node_number;
  const std::map<std::wstring, unsigned>& m_label_map;
};

class UnboundedCycleException: public std::exception
{
  MscElementPListList m_unbounded_cycles;
public: 
  MscElementPListList get_unbounded_cycles(void)
  {
    return m_unbounded_cycles;
  }
  virtual const char* what() const throw()
  {
    return "An unbounded cycle has been found.";
  }
  UnboundedCycleException(MscElementPListList cycles)
    :m_unbounded_cycles(cycles)
  {}
  ~UnboundedCycleException() throw ()
 {
 }
};

class ResultCatcher: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n)
  {
    if(n->is_attribute_set("boundedness_result"))
    {
      MscElementPListList result, dummy;
      result = n->get_attribute("boundedness_result", dummy);
      n->remove_attribute<MscElementPListList>("boundedness_result");
      throw UnboundedCycleException(result);
    }
  }
};


class BoundednessListener: public ElementaryCycleListener
{
private:
  unsigned int m_vertex_count;
public:
  BoundednessListener(unsigned int vertex_count)
    :m_vertex_count(vertex_count)
  {}
  void on_elementary_cycle_found(const MscElementPList &cycle);

};


class BoundednessCleanupListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n);
};


class NameCollector: public WhiteNodeFoundListener
{
private:
  std::map<std::wstring, unsigned> m_instance_map;
  

public:
  void on_white_node_found(HMscNode *n);
  unsigned get_instance_count(void)
  {
    return m_instance_map.size();
  }
  const std::map<std::wstring, unsigned> & get_instance_map(void)
  {
    return m_instance_map;
  }
  
};

class SCBOUNDEDNESS_EXPORT UniversalBoundednessChecker: public Checker, public HMscChecker
{
protected:
  
  /**
   * Common instance.
   */
  static UniversalBoundednessCheckerPtr m_instance;

  HMscPtr create_counter_example(const MscElementPList& to_cycle, const MscElementPList& cycle);

  //BMscGraphDuplicator m_graph_duplicator;
  
        
public:
    
  UniversalBoundednessChecker(){};

  static const std::string com_graph_attribute;

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Universal Boundedness"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"boundedness/boundedness.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether hmsc satisfy universal boundedness property.
   */
  std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr chm);

  /**
   * Cleans up no more needed attributes.
   */
  void cleanup_attributes();
  
  /**
   * Supports all mappers
   */
  bool is_supported(ChannelMapperPtr chm)
  {
    return true;
  }
  
  
  static UniversalBoundednessCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = UniversalBoundednessCheckerPtr(new UniversalBoundednessChecker());
    return m_instance;
  }

};
 
// $Id: universal_boundedness_checker.h 789 2010-05-16 20:44:30Z vacek $
