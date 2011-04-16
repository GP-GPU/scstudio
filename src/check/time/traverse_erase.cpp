#include "check/time/traverse_erase.h"


void TraverseErase::on_white_node_found(HMscNode* n)
{
  ReferenceNode* v= dynamic_cast<ReferenceNode*>(n);
  if (v!=NULL)
  {
    erase_hmsc_constraints(v);
//    v->empty_time_relations();
    BMscPtr bmsc= v->get_bmsc();
    if (bmsc!=NULL)
    {
      traverse_erase_bmsc(bmsc);
    }
  }
}

void TraverseErase::erase_hmsc_constraints(ReferenceNode* n)
{
  TimeRelationRefNodePtrSet node_constraints_bottom;
  TimeRelationRefNodePtrSet node_constraints_top;
  node_constraints_top.insert(n->get_time_relations_top().begin(), n->get_time_relations_top().end());  //fill with contraints in node v
  node_constraints_bottom.insert(n->get_time_relations_bottom().begin(), n->get_time_relations_bottom().end());  //fill with contraints in node v
  for (std::set<TimeRelationRefNodePtr>::iterator it = node_constraints_bottom.begin(); it != node_constraints_bottom.end(); it++)  //iterate node constraints
  {
    it->get()->clear_interval_set();
  }
  for (std::set<TimeRelationRefNodePtr>::iterator it = node_constraints_top.begin(); it != node_constraints_top.end(); it++)  //iterate node constraints
  {
    it->get()->clear_interval_set();
  }

std::cout << "Tu vymazane hmsc obmedz" << std::endl;
  //erase top and bottom contraints
}

void TraverseErase::traverse_erase_bmsc(BMscPtr bmsc)
{
  DFSEventsTraverser trav_bmsc_erase;
  BmscTraverseErase my_bmsc_trav;
  trav_bmsc_erase.add_white_event_found_listener(&my_bmsc_trav);
  trav_bmsc_erase.traverse(bmsc);
}

void BmscTraverseErase::on_white_event_found(Event* e)
{
  erase_bmsc_constraints(e);
}

void BmscTraverseErase::erase_bmsc_constraints(Event* e)
{
//  e->empty_time_relations();
  TimeRelationEventPtrList list =  e->get_time_relations();
  TimeRelationEventPtrList::iterator it;
  for(it = list.begin();it!=list.end();it++){
    it->get()->clear_interval_set();
  }
  std::cout << "Tu vymazane bmsc obmedz" << std::endl;
}







