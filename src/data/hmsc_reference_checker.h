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
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: hmsc_reference_checker.h 1029 2011-02-02 22:17:59Z madzin $
 */

#ifndef _HMSC_REFERENCE_CHECKER_H
#define _HMSC_REFERENCE_CHECKER_H

#include "data/checker.h"
#include "data/dfs_hmsc_traverser.h"
#include "check/pseudocode/msc_duplicators.h"

/**
 * Represents a property violation in a BMSC referenced from a given reference node.
 */
class ReferenceException
{
public:
  ReferenceException(ReferenceNodePtr reference, std::list<BMscPtr> counterexamples)
    : m_reference(reference), m_counterexamples(counterexamples)
  { }

  ReferenceNodePtr get_reference() const
  {
    return m_reference;
  }

  std::list<BMscPtr> get_counterexamples() const
  {
    return m_counterexamples;
  }

private:
  ReferenceNodePtr m_reference;
  std::list<BMscPtr> m_counterexamples;
};

class FindResultsListener: public WhiteNodeFoundListener
{
public:
  void on_white_node_found(HMscNode *node)
  {
    if(node->is_attribute_set("has_result"))
    {
      std::list<BMscPtr> dummy;
      std::list<BMscPtr> result = node->get_attribute("has_result", dummy);
      node->remove_attribute<std::list<BMscPtr> >("has_result");
      throw ReferenceException(dynamic_cast<ReferenceNode *>(node), result);
    }
  }
};

/**
 * Walks through all references in HMSC and invokes a given checker.
 */
class ReferenceWalker:public WhiteNodeFoundListener
{
public:
  ReferenceWalker(BMscChecker* bmsc_checker, ChannelMapperPtr mapper)
    : m_checker(bmsc_checker), m_mapper(mapper)
  { }

  void on_white_node_found(HMscNode* node)
  {
    ReferenceNodePtr reference_node = dynamic_cast<ReferenceNode*>(node);
    if(reference_node != NULL && reference_node->get_bmsc().get() != NULL)
    {
      BMscPtr bmsc = reference_node->get_bmsc();
			std::list<BMscPtr> b_result = m_checker->check(bmsc, m_mapper);
			if(!b_result.empty())
        node->set_attribute<std::list<BMscPtr> >("has_result", b_result);
				//throw ReferenceException(reference_node, b_result.front());
    }
  }

private:
  BMscChecker* m_checker;
  ChannelMapperPtr m_mapper;
};

/**
 * Abstract checker that verifies a property against every BMsc in HMsc.
 */
template <class BMSC_CHECKER>
class HMscReferenceChecker: public HMscChecker
{
public:
  std::list<HMscPtr> check(HMscPtr hmsc, ChannelMapperPtr chm)
  {
    HMscPtr example;
		std::list<HMscPtr> result;

    DFSBMscGraphTraverser traverser;
    // the descendant is expected to be BMscChecker
    ReferenceWalker walker(dynamic_cast<BMSC_CHECKER*>(this), chm);
    traverser.add_white_node_found_listener(&walker);
    traverser.traverse(hmsc);
    traverser.remove_all_listeners();
    traverser.cleanup_traversing_attributes();

    FindResultsListener re_fi_l;
    traverser.add_white_node_found_listener(&re_fi_l);
    bool result_found = true;
    std::list<BMscPtr>::iterator b_it;
    while(result_found)
    {
      result_found = false;
      try
      {
        traverser.traverse(hmsc);
      }
      catch (ReferenceException &error)
      {
        std::list<BMscPtr> results = error.get_counterexamples();
        for(b_it = results.begin(); b_it != results.end(); b_it++)
        {
          HMscPathDuplicator duplicator;
          example = duplicator.duplicate_path(traverser.get_reached_elements());
			    result.push_back(example);

          MscElementPtr element = duplicator.get_copy(error.get_reference().get());
          duplicator.cleanup_attributes();
          ReferenceNodePtr reference = boost::dynamic_pointer_cast<ReferenceNode>(element);
          reference->set_marked(MARKED);
          reference->set_msc(b_it->get());
        }
        traverser.cleanup_traversing_attributes();
        result_found = true;
      }
    }
    return result;
  }
};

#endif /* _HMSC_REFERENCE_CHECKER_H */

// $Id: hmsc_reference_checker.h 1029 2011-02-02 22:17:59Z madzin $
