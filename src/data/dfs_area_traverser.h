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
 * $Id: dfs_area_traverser.h 124 2008-11-29 11:22:31Z gotthardp $
 */

#ifndef _DFS_AREA_TRAVERSER_H
#define _DFS_AREA_TRAVERSER_H

#include "data/dfs_instance_events_traverser.h"

/**
 * Processes EventAreas' events in sequence and do not follow matching events 
 * during traversing.
 *
 * Single EventAreas are traversed in depth first search manner.
 */
class SCMSC_EXPORT DFSAreaTraverser:public DFSEventsTraverser
{

public:
  
  DFSAreaTraverser(const std::string& color="color");

  void traverse(BMscPtr bmsc);
  
protected:
  
  void traverse_area(EventArea* area);
  
  /**
   * Doesn't continue in traversing next EventArea in case of event hasn't got
   * any successors.
   */
  void traverse_coregion_event(CoregionEvent* event);

  void traverse_strict_event(StrictEvent* event);
  
};

#endif /* _DFS_AREA_TRAVERSER_H */

// $Id: dfs_area_traverser.h 124 2008-11-29 11:22:31Z gotthardp $
