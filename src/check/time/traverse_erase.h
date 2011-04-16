#ifndef _TRAVERSE_ERASE_H_
#define _TRAVERSE_ERASE_H_

#include "data/dfs_bmsc_graph_traverser.h" //hmsc traverser
#include "data/dfs_events_traverser.h"
#include "export.h"


class SCTIME_EXPORT TraverseErase: public WhiteNodeFoundListener
{
public:
  TraverseErase() 
  {}
  virtual ~TraverseErase()
  {}

  virtual void on_white_node_found(HMscNode* n);
  void erase_hmsc_constraints(ReferenceNode* n);
  void traverse_erase_bmsc(BMscPtr bmsc);
};

class SCTIME_EXPORT BmscTraverseErase:public WhiteEventFoundListener  //dfs event traverser
{
public:
  BmscTraverseErase()
  {}
  virtual ~BmscTraverseErase()
  {}

  virtual void on_white_event_found(Event* e);
  void erase_bmsc_constraints(Event* e);
};

class EraseAllConstraints
{
public:
  EraseAllConstraints()
  {}

  void erase_all_constraints(HMscPtr hmsc)
  {
    DFSBMscGraphTraverser trav_erase;
    TraverseErase my_trav;
    trav_erase.add_white_node_found_listener(&my_trav);
    trav_erase.traverse(hmsc);
  }
};





#endif // _TRAVERSE_ERASE_H_
