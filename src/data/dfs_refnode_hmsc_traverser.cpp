/*
 * scstudio - Sequence Chart Studio
 * http://scstudio.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied  warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2008 Jindra Babica <babica@mail.muni.cz>
 *
 * $Id: dfs_refnode_hmsc_traverser.cpp 455 2009-11-13 17:17:17Z kocianon $
 */

#include "dfs_refnode_hmsc_traverser.h"

bool DFSRefNodeHMscTraverser::traverse_successors(PredecessorNode* predecessor)
{
  bool end_found = false;
  HMscNodePListPtr successors = m_finder.find_successors(dynamic_cast<HMscNode*>(predecessor));
  m_finder.cleanup_traversing_attributes();

  //TODO: m_reached_elements should contain path to succ
  HMscNodePList::const_iterator succ;
  for(succ=successors->begin();succ!=successors->end();succ++)
  {
    end_found = traverse_node(*succ) || end_found;
  }
  return end_found;
}

void DFSRefNodeHMscTraverser::cleanup_traversing_attributes()
{
  DFSBMscGraphTraverser::cleanup_traversing_attributes();
  m_finder.cleanup_traversing_attributes();
}

// $Id: dfs_refnode_hmsc_traverser.cpp 455 2009-11-13 17:17:17Z kocianon $
