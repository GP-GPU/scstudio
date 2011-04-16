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
 * $Id: name_checker.h 1052 2011-02-27 17:46:37Z vacek $
 */

#include <vector>
#include <map>
#include "data/checker.h"
#include "data/msc.h"
#include "check/structure/export.h"
#include "data/dfs_hmsc_traverser.h"
#include "data/dfs_bmsc_graph_traverser.h"
#include "check/pseudocode/msc_duplicators.h"


class NameChecker;


typedef boost::shared_ptr<NameChecker> NameCheckerPtr;


class InconsistentNamesException: public std::exception
{
public:

  virtual const char* what() const throw()
  {
    return "Inconsistent names.";
  }
};

class DuplicateNamesException: public std::exception
{
public:
  DuplicateNamesException(const std::wstring& name)
    : m_name(name)
  { }

  virtual ~DuplicateNamesException() throw()
  { }

  const std::wstring& get_name() const
  {
    return m_name;
  }

  virtual const char* what() const throw()
  {
    return "Duplicate names.";
  }

private:
  std::wstring m_name;
};

class FindFirstNodeListener:public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *n);
};

class FirstNodeFoundException: public std::exception
{
public:
  virtual const char* what() const throw()
  {
    return "First node found.";
  }
};


class NameListener: public WhiteNodeFoundListener
{
private:
  bool m_first_node;
  std::vector<std::wstring> m_instance_names;

public:
  void on_white_node_found(HMscNode *n);
  NameListener()
    :m_first_node(true)
  {}
  
};

class NodeAdder: public WhiteNodeFoundListener
{
  HMscPtr m_where_to_add;
public:
  void on_white_node_found(HMscNode *n);
  NodeAdder(HMscPtr &where_add)
  {
    m_where_to_add = where_add;
  }
};

class SCSTRUCTURE_EXPORT NameChecker: public Checker, public HMscChecker, public BMscChecker
{
protected:
  
  /**
   * Common instance.
   */
  static NameCheckerPtr m_instance;

  HMscPtr create_duplicate_counter_example(const MscElementPListList& path, const std::wstring &label);

  HMscPtr create_inconsistent_counter_example(const MscElementPListList& path1, const MscElementPListList& path2);

  BMscGraphDuplicator m_graph_duplicator;

  
        
public:
    
  NameChecker(){};

  /**
   * Human readable name of the property being checked.
   */
  // note: DLL in Windows cannot return pointers to static data
  virtual std::wstring get_property_name() const
  { return L"Unique instance names"; }

  /**
   * Ralative path to a HTML file displayed as help.
   */
  virtual std::wstring get_help_filename() const
  { return L"unique_instance/unique_instance.html"; }

  virtual PreconditionList get_preconditions(MscPtr msc) const;

  /**
   * Checks whether a given hmsc contains consistent set of instances.
   */
  std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr chm);

  std::list<BMscPtr> check(BMscPtr bmsc, ChannelMapperPtr chm);

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
  
  
  static NameCheckerPtr instance()
  {
    if(!m_instance.get())
      m_instance = NameCheckerPtr(new NameChecker());
    return m_instance;
  }

};
 
// $Id: name_checker.h 1052 2011-02-27 17:46:37Z vacek $
