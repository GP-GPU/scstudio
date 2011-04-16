

#include "data/msc.h"
#include "check/time/find_block.h"
#include "check/time/traverse_erase.h"
#include "check/time/tightening.h"
#include "check/time/hmsc_block_paths.h"
#include "check/liveness/deadlock_checker.h"
#include <string>
#include <iostream>


int main(int argc, char** argv)
{


  std::cout << "Test of find_block:              " << std::endl;
  std::cout << "h1:                              " << std::endl;
  std::cout << "   start                         " << std::endl;
  std::cout << "     |                           " << std::endl;
  std::cout << "    p1                           " << std::endl;
  std::cout << "    / \\                         " << std::endl;
  std::cout << "  p2   p3                        " << std::endl;                                       
  std::cout << "   |     \\                      " << std::endl;
  std::cout << "  p5     p4                      " << std::endl;
  std::cout << "   |    /                        " << std::endl;
  std::cout << "    \\ /                         " << std::endl;
  std::cout << "     p6                          " << std::endl;
  std::cout << "     |                           " << std::endl;
  std::cout << "   end1                          " << std::endl;

  HMscPtr h1(new HMsc(L"h1"));



  StartNodePtr start1 = new StartNode();
  h1->set_start(start1);
  EndNodePtr end1(new EndNode);
  h1->add_node(end1);
  ReferenceNodePtr p1(new ReferenceNode());h1->add_node(p1);
  ReferenceNodePtr p2(new ReferenceNode());h1->add_node(p2);
  ReferenceNodePtr p3(new ReferenceNode());h1->add_node(p3);
  ReferenceNodePtr p4(new ReferenceNode());h1->add_node(p4);
  ReferenceNodePtr p5(new ReferenceNode());h1->add_node(p5);
  ReferenceNodePtr p6(new ReferenceNode());h1->add_node(p6);
  
  
  start1->add_successor(p1.get());
  p1->add_successor(p2.get());
  p1->add_successor(p3.get());
  p3->add_successor(p4.get());
  p2->add_successor(p5.get());
  p5->add_successor(p6.get());
  p4->add_successor(p6.get());
  p6->add_successor(end1.get());
  

  MscTimeIntervalD in6(10,20);
  MscTimeIntervalSetD ins1;
  ins1.insert(in6);

  MscTimeIntervalD in7(10,20);
  MscTimeIntervalSetD ins2;
  ins2.insert(in7); 
  
  TimeRelationRefNodePtr rel1a = new TimeRelationRefNode(ins1);
  rel1a->glue_ref_nodes(0,p2.get(),0,p5.get());

  TimeRelationRefNodePtr rel1a2 = new TimeRelationRefNode(ins2);
  rel1a2->glue_ref_nodes(0,p3.get(),0,p4.get()); 

  TraverseAndMarkBlocks* trav_mark = new TraverseAndMarkBlocks();
  trav_mark->travers_and_mark_blocks(h1);
  
  for (std::list<Block>::iterator it = trav_mark->m_list_of_blocks.begin(); it != trav_mark->m_list_of_blocks.end(); it++)
    {
    std::cout << "Blok:" << (*it).get_number() << " if_end:" << (*it).get_if_end() << std::endl;
    }
//  AllPathsAllBlocks* apab= new AllPathsAllBlocks(trav_mark->m_list_of_blocks, h1);
//  apab->all_paths_all_blocks();









  EraseAllConstraints* eac= new EraseAllConstraints();
  eac->erase_all_constraints(h1);

  delete trav_mark;
  delete eac;

  return 0;
}
