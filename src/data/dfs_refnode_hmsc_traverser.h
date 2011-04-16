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
 * $Id: dfs_refnode_hmsc_traverser.h 307 2009-09-11 07:48:48Z vacek $
 */

#ifndef _DFS_REFNODE_HMSC_TRAVERSER_H
#define _DFS_REFNODE_HMSC_TRAVERSER_H

#include "data/dfs_bmsc_graph_traverser.h"
#include "data/node_finder.h"


/**
 * Traverses nodes in HMsc and referenced HMsc in depth first 
 * search manner like DFSBMscGraphTraverser. Unlike DFSBMscGraphTraverser 
 * this traverser handles ConnectionNodes like it would be
 * only edge of graph. I.e. user of this traverser and connected 
 * listeners to this traverser can suppose that HMsc is graph 
 * consisting of StartNodes, EndNodes and ReferenceNodes.
 */
class SCMSC_EXPORT DFSRefNodeHMscTraverser:public DFSBMscGraphTraverser
{
  
protected:
  
  bool traverse_successors(PredecessorNode* predecessor);
  NodeFinder m_finder;
  
public:
  
  DFSRefNodeHMscTraverser(
    const std::string& color_attribute = "DFSRNHMT_color"):
    DFSBMscGraphTraverser(color_attribute)
  {
    
  }
  
  void cleanup_traversing_attributes();
  
};

#endif /* _DFS_REFNODE_HMSC_TRAVERSER_H */

// $Id: dfs_refnode_hmsc_traverser.h 307 2009-09-11 07:48:48Z vacek $
