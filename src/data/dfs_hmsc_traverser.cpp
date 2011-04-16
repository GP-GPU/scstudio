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
 * $Id: dfs_hmsc_traverser.cpp 123 2008-11-28 23:39:47Z gotthardp $
 */

#include "data/dfs_hmsc_traverser.h"

bool DFSHMscTraverser::traverse_reference_node(ReferenceNode* ref_node)
{
  HMscPtr hmsc = ref_node->get_hmsc();
  if(hmsc.get()!=NULL)
  {
    m_reached_elements.push_back(MscElementPList());
    traverse_node(hmsc->get_start().get());
    m_reached_elements.pop_back();
    
  }
  return traverse_successors(ref_node);
}

void DFSHMscTraverser::cleanup_traversing_attributes()
{
  cleanup_top_attributes();
  while(!m_reached_elements.empty())
  {
    m_reached_elements.pop_back();
  }
}

// $Id: dfs_hmsc_traverser.cpp 123 2008-11-28 23:39:47Z gotthardp $
