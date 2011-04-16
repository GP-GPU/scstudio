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
 * $Id: acyclic_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "check/order/acyclic_checker.h"
#include "check/pseudocode/msc_duplicators.h"

AcyclicCheckerPtr AcyclicChecker::m_instance;

Checker::PreconditionList AcyclicChecker::get_preconditions(MscPtr msc) const
{
  Checker::PreconditionList result;
  // no preconditions
  return result;
}


std::list<BMscPtr> AcyclicChecker::check(BMscPtr bmsc, ChannelMapperPtr chm)
{
  HMscPtr hmsc = new HMsc();
  StartNodePtr start = new StartNode();
  hmsc->set_start(start);
  AssignHMscListener asil(hmsc.get());
  MessageSuceessors mesul;
  InstanceSuccessors isul;
  CleanHMscListener chli;


  DFSEventsTraverser traverser;
  ElementaryCyclesTraverser cycle_traverser;
  FindCycles ficel;
  cycle_traverser.add_cycle_listener(&ficel);
  traverser.add_white_event_found_listener(&asil);
  traverser.traverse(bmsc);
  traverser.remove_all_listeners();
  traverser.add_send_receive_pair_listener(&mesul);
  traverser.add_event_successor_listener(&isul);
  traverser.traverse(bmsc);
  traverser.remove_all_listeners();

  InstancePtrList::const_iterator it;

  BMscDuplicator duplicator;
  for(it = bmsc->get_instances().begin(); it != bmsc->get_instances().end(); it++)
  {
    ConnectionNode *dummy;
    EventAreaPtr eap = (*it)->get_first();
    while(eap)
    {
      if(eap->is_empty())
        eap = eap->get_next();
      else
        break;
    }
    if(eap)
    {
      StrictOrderArea *seap = dynamic_cast<StrictOrderArea*>(eap.get());
      if(seap)
        start->add_successor(seap->get_first()->get_attribute("ACC_node", dummy));
      else
      {
        CoregionArea *ceap = dynamic_cast<CoregionArea*>(eap.get());
        CoregionEventPVector::const_iterator cit;
        for(cit = ceap->get_minimal_events().begin(); cit != ceap->get_minimal_events().end(); cit++)
          start->add_successor((*cit)->get_attribute("ACC_node", dummy));
      }
    }
  }
  cycle_traverser.traverse(hmsc);
  std::list<BMscPtr> res;
  MscElementPListList::const_iterator cyit;
  MscElementPList::const_iterator eit, laeit; //element iterator, lookahead;

  for(cyit = ficel.get_cycles().begin(); cyit != ficel.get_cycles().end(); cyit++)
  {
    BMscPtr example = duplicator.duplicate_bmsc(bmsc); 
    for(eit = cyit->begin(); eit != cyit->end(); eit++)
    {
      ConnectionNode *cono;
      NodeRelation *nore;

      cono = dynamic_cast<ConnectionNode*>((*eit));
      if(cono)
      {
        Event *e, *dummy;
        e = cono->get_attribute("ACC_event", dummy);
        duplicator.get_copy(e)->set_marked(MARKED);
      }
      nore = dynamic_cast<NodeRelation*>((*eit));
      if(nore)
      {
        MscMessage *m;
        if(nore->is_attribute_set("ACC_message"))
        {
          m = nore->get_attribute<MscMessage*>("ACC_message", NULL);
          if(m)
            duplicator.get_copy(m)->set_marked(MARKED);
        }
      }
    }
    res.push_back(example);
    duplicator.cleanup_attributes();
  }

  CleanHMscListener cleaner;
  traverser.add_white_event_found_listener(&cleaner);

  traverser.traverse(bmsc);
  return res;
}

// $Id: acyclic_checker.cpp 1029 2011-02-02 22:17:59Z madzin $
