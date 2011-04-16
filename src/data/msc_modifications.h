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
 * $Id: msc_modifications.h 123 2008-11-28 23:39:47Z gotthardp $
 */

#ifndef _MSC_MODIFICATIONS_H
#define _MSC_MODIFICATIONS_H

#include <set>
#include "data/msc.h"

/**
 * General base class for modification of MSC.
 */
class MscModification{

};

typedef counted_ptr<MscModification> MscModificationPtr;
typedef std::set<MscModificationPtr> MscModificationPtrSet;

typedef counted_ptr<CoregionEvent> CoregionEventPtr;
typedef std::set<CoregionEventPtr> CoregionEventPtrSet;

typedef counted_ptr<StrictEvent> StrictEventPtr;
typedef std::set<StrictEventPtr> StrictEventPtrSet;

/**
 * Container of modifications.
 *
 * Encapsulates container of modifications performed in previous 
 * version of Msc. These modifications are ordered in descending 
 * order by level of hierarchical structure which they reside in.
 * However, modifications can be accessed in both directions.
 *
 * Purpose of this container will be seen when we use iterative 
 * type of checking algorithms of particular property of Msc.
 * This type of algorithms is capable to use results from previous
 * version of Msc to decrease computing complexity. Withou this
 * approach (container of modifications) it would be neccessary to 
 * involve searching of modifications into complexity.
 */
class MscModificationContainer{

  /**
   * Container of sorted modifications.
   */
  MscModificationPtrSet m_modifications;

};



/**
 * Base class for modifications in EventArea.
 */
class EventAreaModification:public MscModification{

  /**
   * Events which were removed from EventArea.
   */
  std::set<Event> m_removed_events;

  /**
   * Events which are new in EventArea.
   */
  std::set<Event> m_new_events;

};

/**
 * Modifications which are performed in StrictOrderArea.
 */
class StrictOrderModification:public EventAreaModification{
  
  /**
   * Previous version of StrictOrderArea.
   */
  StrictOrderAreaPtr m_previous;

  /**
   * Events with modified successor.
   */
  StrictEventPtrSet m_modified_events;
  
  /**
   * New first event.
   */
  StrictEventPtr m_first;

};

/**
 * Modifications which are performed in CoregionArea.
 */
class CoregionModification:public EventAreaModification{

  /**
   * Previous version of CoregionArea.
   */
  CoregionAreaPtr m_previous;

  /**
   * Events which are newly minimimal in modified version.
   */
  CoregionEventPtrSet m_minimal_events;

  /**
   * Events with modified successors.
   */
  CoregionEventPtrSet m_modified_events;

};

/**
 * Modification of instance.
 */
class InstanceModification:public MscModification{

  /**
   * Previous version of Instance.
   */
  InstancePtr m_previous;

  /**
   * Removed EventAreas from Instance.
   */
  std::set<EventAreaPtr> m_removed_areas;

  /**
   * New EventAreas at instance.
   */
  std::set<EventAreaPtr> m_new_areas;

  /**
   * Modified first area, undefined if no change.
   */
  EventAreaPtr m_first_area;

};

/**
 * Modification of BMsc.
 */
class BMscModification: public MscModification
{

  /**
   * Previous version of BMsc.
   */
  BMscPtr m_previous;

  /**
   * Removed Instances.
   */
  std::set<EventAreaPtr> m_removed_instances;

  /**
   * New Instances.
   */
  std::set<EventAreaPtr> m_new_instances;

};

/**
 * Modification of ReferenceNode.
 */
class ReferenceNodeModification: public MscModification
{

  /**
   * Previous version of ReferenceNode.
   */
  ReferenceNodePtr m_previous;

  /**
   * Removed successors.
   */
  std::set<ReferenceNodePtr> m_removed_successors;

  /**
   * New successors.
   */
  std::set<ReferenceNodePtr> m_new_successors;

  /**
   * Changed Msc of the ReferenceNode.
   *
   * If NULL then Msc points to the same one (doesn't care if it was
   * internally modified).
   */
  MscPtr m_changed_msc;
};

/**
 * Modification of HMsc
 */
class HMscModification:public MscModification{

  /**
   * Previous version of HMsc.
   */
  HMscPtr m_previous;
  
  /**
   * Removed successors of StartNode.
   */
  std::set<ReferenceNodePtr> m_removed_beginners;

  /**
   * New successors of StartNode.
   */
  std::set<ReferenceNodePtr> m_new_beginners;

  /**
   * Removed nodes.
   */
  std::set<ReferenceNodePtr> m_removed_nodes;

  /**
   * New nodes.
   */
  std::set<ReferenceNodePtr> m_new_nodes;

};

#endif /* _MSC_MODIFICATIONS_H */

// $Id: msc_modifications.h 123 2008-11-28 23:39:47Z gotthardp $
