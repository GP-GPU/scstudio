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
 * $Id: dfs_hmsc_traverser.h 124 2008-11-29 11:22:31Z gotthardp $
 */

#ifndef _DFS_HMSC_TRAVERSER_H
#define _DFS_HMSC_TRAVERSER_H

#include "data/dfs_bmsc_graph_traverser.h"


/**
 * Traverses all HMscNodes in HMsc and referenced HMsc in depth 
 * first search manner. I.e. unlike DFSBMscGraphTraverser this traverser doesn't
 * care about presence of EndNodes in referenced HMsc in ReferenceNodes. 
 * Successors of the ReferenceNode are traversed no matter there is EndNode in
 * it's HMsc. Moreover each HMsc is traversed only one time.
 *
 * @warning Non-recursive HMsc is expected.
 */
class SCMSC_EXPORT DFSHMscTraverser:public DFSBMscGraphTraverser
{
  
protected:
  
  bool traverse_reference_node(ReferenceNode* node);
  
public:
  
  DFSHMscTraverser(
    const std::string& color_attribute = "dfs_hmsc_traverse_color"):
    DFSBMscGraphTraverser(color_attribute)
  {
    
  }
  
  void cleanup_traversing_attributes();
  
};

#endif /* _DFS_HMSC_TRAVERSER_H */

// $Id: dfs_hmsc_traverser.h 124 2008-11-29 11:22:31Z gotthardp $
