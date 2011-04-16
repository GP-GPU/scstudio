#include "data/msc.h"
#include "check/liveness/deadlock_checker.h"
#include <string>
#include <iostream>

int main() {
  std::cout << "Test of deadlock:                " << std::endl;
  std::cout << "h1:                     h2:      " << std::endl;
  std::cout << "   start                   start " << std::endl;
  std::cout << "     |                       |   " << std::endl;
  std::cout << "    p1                       n1  " << std::endl;
  std::cout << "    / \\                     |   " << std::endl;
  std::cout << "  p2   p3(h2)                n2  " << std::endl;                                       
  std::cout << "   |     \\                      " << std::endl;
  std::cout << "   |     p4                      " << std::endl;
  std::cout << "   |    /                        " << std::endl;
  std::cout << "   end                           " << std::endl;
    
  HMscPtr h1(new HMsc(L"h1"));
  HMscPtr h2(new HMsc(L"h2"));
  
  StartNodePtr start1 = new StartNode(); h1->set_start(start1);
  EndNodePtr end1(new EndNode);h1->add_node(end1);
  ReferenceNodePtr p1(new ReferenceNode());h1->add_node(p1);
  ReferenceNodePtr p2(new ReferenceNode());h1->add_node(p2);
  ReferenceNodePtr p3(new ReferenceNode());h1->add_node(p3);
  ReferenceNodePtr p4(new ReferenceNode());h1->add_node(p4);
  
  StartNodePtr start2 = new StartNode(); h2->set_start(start2);
  ReferenceNodePtr n1(new ReferenceNode());h2->add_node(n1);
  ReferenceNodePtr n2(new ReferenceNode());h2->add_node(n2);
  
  start1->add_successor(p1.get());
  p1->add_successor(p2.get());
  p1->add_successor(p3.get());
  p3->add_successor(p4.get());
  p2->add_successor(end1.get());
  p4->add_successor(end1.get());
  p3->set_msc(h2);

  start2->add_successor(n1.get());
  n1->add_successor(n2.get());
  
  ChannelMapperPtr chm;
  DeadlockCheckerPtr dead = DeadlockChecker::instance();

	std::list<HMscPtr> result;
	result = dead->check(h1,chm);
	HMscPtr path_h1 = result.empty()?NULL:result.back();
  if(path_h1.get())
  {
    std::cerr << "OK: h1 contains deadlock" << std::endl;
    return 0;
  }
  std::cerr << "ERROR: h1 doesn't contain deadlock" << std::endl;
  return 1;
}

// $Id: deadlock_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
