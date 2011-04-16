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
 * $Id: dfs_instance_events_traverser.h 136 2008-12-25 15:59:43Z gotthardp $
 */

#ifndef _DFS_INSTANCE_EVENTS_TRAVERSER_H
#define _DFS_INSTANCE_EVENTS_TRAVERSER_H

#include "data/dfs_events_traverser.h"

typedef boost::shared_ptr<EventPList> EventPListPtr;

/**
 * Processes only Instances' Events during depth first search.
 *
 * I.e. this traverser doesn't follow matching events during traversing.
 */
class SCMSC_EXPORT DFSInstanceEventsTraverser:public DFSEventsTraverser
{

public:
  
  DFSInstanceEventsTraverser(const std::string& color="color");
  
  void traverse(Instance* instance);

  using DFSEventsTraverser::traverse;
  
  static EventPListPtr topology_order(Instance* i);
  
protected:
  
  /**
   * Behaves as same as DFSEventsTraverser::traverse_strict_event(StrictEvent* event) 
   * except following matching events.
   */
  void traverse_strict_event(StrictEvent* event);
  
  /**
   * Behaves as same as DFSEventsTraverser::traverse_coregion_event(CoregionEvent* event) 
   * except following matching events.
   */
  void traverse_coregion_event(CoregionEvent* event);
  
};

#endif /* _DFS_INSTANCE_EVENTS_TRAVERSER_H */

// $Id: dfs_instance_events_traverser.h 136 2008-12-25 15:59:43Z gotthardp $
