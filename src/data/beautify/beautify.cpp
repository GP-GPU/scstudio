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
 * $Id: beautify.cpp 1019 2011-01-04 07:15:39Z xpekarc $
 */

#include "beautify.h"
#include "length_optimizer.h"
#include "instance_sequencer.h"
#include "layout_optimizer.h"

Beautify::Beautify()
{
	m_is_imported = false;
}

Transformer::PreconditionList Beautify::get_preconditions(MscPtr msc) const
{
  Transformer::PreconditionList result;
  // no preconditions
  return result;
}

//! Transform a MSC drawing.
MscPtr Beautify::transform(MscPtr msc)
{
  std::set<std::wstring> processed;

  // list of MSC to be processed
  // new references may be added to m_printing by save_hmsc()
  m_processing.push_back(msc);

  for(std::list<MscPtr>::const_iterator pos = m_processing.begin();
    pos != m_processing.end(); pos++)
  {
    if(*pos == NULL)
      continue;

    // if not already processed
    if(processed.find((*pos)->get_label()) == processed.end())
    {
      int result = 0;

      BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(*pos);
      if(bmsc)
        result = transform_bmsc(bmsc);

      HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(*pos);
      if(hmsc)
        result = transform_hmsc(hmsc);

      if(!result)
        return NULL; // failure

      processed.insert((*pos)->get_label());
    }
  }

  m_processing.clear();
  return msc; // success
}

int Beautify::transform_bmsc(BMscPtr bmsc)
{
  if(bmsc->get_instances().empty())
    return 1; //success

   // (1) optimize instance order
  InstanceSequencer sequencer(this->get_config_provider());
	if(this->get_is_imported())
		sequencer.after_import_process(bmsc);
	else
		sequencer.process(bmsc);

	// (3) optimalize instance length
  LengthOptimizer length_opt(this->get_config_provider());
	if(!this->get_is_imported())
		length_opt.process(bmsc);
  
  // (2) optimize event positions
	// generate graphical layout information
  LayoutOptimizer optimizer(this->get_config_provider());

	if(this->get_is_imported())
	{
		if(!optimizer.after_import_process(bmsc))
			return 0; //failure
	}
	else
	{
		// generate graphical layout information
		if(!optimizer.process(bmsc))
			return 0; // failure
	}

	this->set_is_imported(false);

  
  
  return 1; // success
}

int Beautify::transform_hmsc(HMscPtr hmsc)
{
  Coordinate tabsize_x = 0;
  Coordinate tabsize_y = 20;
  Coordinate act_position_x = 40;
  Coordinate act_position_y =  0;

  PtrIDMap<HMscNodePtr> node_id_map; // node identifiers
  // nodes to be processed; this is to avoid recursion
  std::list<HMscNodePtr> node_stack;

  // initialize the stack with the start node
  push_back_if_unique<HMscNodePtr>(node_stack, hmsc->get_start());

  // process all nodes in the stack
  for(std::list<HMscNodePtr>::const_iterator npos = node_stack.begin();
    npos != node_stack.end(); npos++)
  {
    HMscNodePtr act_node = boost::dynamic_pointer_cast<HMscNode>(*npos);
    if(act_node)
    {
      act_position_x += tabsize_x;
      act_position_y += tabsize_y;

      act_node->set_position(MscPoint(act_position_x, act_position_y));
    }

    ReferenceNodePtr reference_node = boost::dynamic_pointer_cast<ReferenceNode>(*npos);
    if(reference_node)
    {
      MscPtr msc = reference_node->get_msc();
      if(msc != NULL)
        m_processing.push_back(msc);
    }

    PredecessorNode *predecessor_node = dynamic_cast<PredecessorNode*>(npos->get());
    if(predecessor_node != NULL)
    {
      for(NodeRelationPtrVector::const_iterator spos = predecessor_node->get_successors().begin();
        spos != predecessor_node->get_successors().end(); spos++)
      {
        SuccessorNode *successor = (*spos)->get_successor();
        HMscNode *successor_node = dynamic_cast<HMscNode*>(successor);
  
        // add successors of this node to the stack
        // note: std::list<>::push_back doesn't invalidate iterators
        push_back_if_unique<HMscNodePtr>(node_stack, successor_node);
      }
    }
  }

  return 1; // success
}

// $Id: beautify.cpp 1019 2011-01-04 07:15:39Z xpekarc $
