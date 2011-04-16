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
 * Copyright (c) 2010 Martin Vodila <fprt77@mail.muni.cz>
 *
 * $Id:
 */

#ifndef _FIND_BLOCK_H
#define _FIND_BLOCK_H

#include "data/time.h"
#include "data/msc.h"
#include "data/dfs_bmsc_graph_traverser.h"
#include "check/pseudocode/msc_duplicators.h"
#include "export.h"

#include <list>

class TraverseAndMarkBlocks;


//complete virtual methods from WNFlistener and NFlistener
class SCTIME_EXPORT MyTraverse:public WhiteNodeFoundListener, public NodeFinishedListener
{
  DFSBMscGraphTraverser* m_traverser;
public:
  TraverseAndMarkBlocks* trav;
  MyTraverse(DFSBMscGraphTraverser* traverser)
  {
    m_traverser = traverser;
    trav = NULL;
  }

  MyTraverse(TraverseAndMarkBlocks* t,DFSBMscGraphTraverser* traverser)
  {
    m_traverser = traverser;
    trav=t;
  }
  virtual ~MyTraverse()
  {}

  virtual void on_white_node_found(HMscNode* n);
  virtual void on_node_finished(HMscNode* n);
  void update_constraints(HMscNode* n);
};




class SCTIME_EXPORT Block
{
private:
  MscElementPListList m_path_to_block;
  int m_number;  //number of block
  ReferenceNode* m_begin;  //begin node of block
  ReferenceNode* m_end;  //end node of block
  bool m_if_end;  //test variable - if true, then end has been set
public:
  Block(int n, ReferenceNode* begin, ReferenceNode* end, bool if_end, MscElementPListList list)
    :m_number(n),m_begin(begin),m_end(end),m_if_end(if_end)
  {
    m_path_to_block.clear();
//    std::cout<<"size of path (list of lists) we got: "<<list.size()<<" size of the first list: "<<std::endl;
//    if(list.size() == 0) std::cout<<"is empty"<<std::endl;
//    else std::cout<<list.back().size()<<std::endl;
    for(MscElementPListList::iterator it = list.begin();it!=list.end();it++)
      m_path_to_block.push_back(MscElementPList(it->begin(),it->end()));
  }

  const MscElementPListList& get_path_to_block()
  {
    return m_path_to_block;
  }

  int get_number()
  {
    return m_number;
  }
  void set_number(int n)
  {
    m_number=n;
  }
  ReferenceNode* get_begin()
  {
    return m_begin;
  }
  void set_begin(ReferenceNode* b)
  {
    m_begin=b;
  }
  ReferenceNode* get_end()
  {
   return m_end;
  }
  void set_end(ReferenceNode* e)
  {
    m_end=e;
  }
  bool get_if_end()
  {
    return m_if_end;
  }
  void set_if_end(bool i)
  {
    m_if_end=i;
  }
};


class SCTIME_EXPORT TraverseAndMarkBlocks
{
private:

public:
  std::list<Block> m_list_of_blocks;
  int m_last_block;
  int m_counter_of_blocks;
  TimeRelationRefNodePtrSet m_open_constraints;

//constructor - initializes variables
  TraverseAndMarkBlocks():m_last_block(0),m_counter_of_blocks(0)
  {
  }


/*
At first duplicates graph,
then travers graph, using only white_node_found_listener and node_finished_listener.
*/
  void travers_and_mark_blocks(HMscPtr m_msc)
  {
    //BMscGraphDuplicator* duplicator= new BMscGraphDuplicator();
    //m_msc=duplicator->duplicate_hmsc(m_msc);
    DFSBMscGraphTraverser traverser_block;
    MyTraverse my_trav(this,&traverser_block);
    traverser_block.add_white_node_found_listener(&my_trav);
    traverser_block.add_node_finished_listener(&my_trav);
    traverser_block.traverse(m_msc);

  }
};

#endif /* _FIND_BLOCK_H */

