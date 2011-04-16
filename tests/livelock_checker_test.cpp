#include "data/msc.h"
#include "check/liveness/livelock_checker.h"
#include <string>
#include <iostream>

#define RETURN_IF_FAILED(test) if(!(test)) return 1;

bool TestA()
{
  std::cout << "Test of livelock A:                " << std::endl;
  std::cout << "h1:                     h2:      " << std::endl;
  std::cout << "   start                   start " << std::endl;
  std::cout << "     |                       |   " << std::endl;
  std::cout << "    p1                       n1  " << std::endl;
  std::cout << "    / \\                    / |  " << std::endl;
  std::cout << "  p2   p3(h2)             n2--n3 " << std::endl;                                       
  std::cout << "   |     \\                      " << std::endl;
  std::cout << "   |     p4                      " << std::endl;
  std::cout << "   |    /                        " << std::endl;
  std::cout << "   end                           " << std::endl;
    
  HMscPtr h1(new HMsc(L"h1"));
  HMscPtr h2(new HMsc(L"h2"));
  BMscPtr bmsc(new BMsc());

  StartNodePtr start1 = new StartNode(); h1->set_start(start1);
  EndNodePtr end1(new EndNode);h1->add_node(end1);
  ReferenceNodePtr p1(new ReferenceNode());h1->add_node(p1);
  p1->set_msc(bmsc);
  ReferenceNodePtr p2(new ReferenceNode());h1->add_node(p2);
  p2->set_msc(bmsc);
  ReferenceNodePtr p3(new ReferenceNode());h1->add_node(p3);
  ReferenceNodePtr p4(new ReferenceNode());h1->add_node(p4);
  p4->set_msc(bmsc);

  StartNodePtr start2 = new StartNode(); h2->set_start(start2);
  ReferenceNodePtr n1(new ReferenceNode());h2->add_node(n1);
  n1->set_msc(bmsc);
  ReferenceNodePtr n2(new ReferenceNode());h2->add_node(n2);
  n2->set_msc(bmsc);
  ReferenceNodePtr n3(new ReferenceNode());h2->add_node(n3);
  n3->set_msc(bmsc);
  
  start1->add_successor(p1.get());
  p1->add_successor(p2.get());
  p1->add_successor(p3.get());
  p3->add_successor(p4.get());
  p2->add_successor(end1.get());
  p4->add_successor(end1.get());
  p3->set_msc(h2);

  start2->add_successor(n1.get());
  n1->add_successor(n2.get());
  n2->add_successor(n3.get());
  n3->add_successor(n1.get());
  
  ChannelMapperPtr chm;
  LivelockCheckerPtr live = LivelockChecker::instance();

  if(!live->check(h1,chm).empty())
  {
    std::cerr << "OK: h1 contains livelock" << std::endl;
    return true;
  }
  std::cerr << "ERROR: h1 doesn't contain livelock" << std::endl;
  return false;
}

bool TestB()
{
  std::cout << "Test of livelock B:" << std::endl;
  std::cout << "       start       " << std::endl;
  std::cout << "         |         " << std::endl;
  std::cout << "    p2 = p1        " << std::endl;                                       
  std::cout << "          \\       " << std::endl;
  std::cout << "          end      " << std::endl;
    
  HMscPtr h1(new HMsc(L"h1"));
  BMscPtr bmsc(new BMsc());

  StartNodePtr start1 = new StartNode(); h1->set_start(start1);
  EndNodePtr end1(new EndNode);h1->add_node(end1);
  ReferenceNodePtr p1(new ReferenceNode());h1->add_node(p1);
  p1->set_msc(bmsc);
  ReferenceNodePtr p2(new ReferenceNode());h1->add_node(p2);
  p2->set_msc(bmsc);
  
  start1->add_successor(p1.get());
  p1->add_successor(p2.get());
  p1->add_successor(end1.get());
  p2->add_successor(p1.get());
  
  ChannelMapperPtr chm;
  LivelockCheckerPtr live = LivelockChecker::instance();

  if(!live->check(h1,chm).empty())
  {
    std::cerr << "ERROR: h1 contains livelock" << std::endl;
    return false;
  }
  std::cerr << "OK: h1 doesn't contain livelock" << std::endl;
  return true;
}

bool TestC()
{
  std::cout << "Test of livelock C:" << std::endl;
  std::cout << "       start       " << std::endl;
  std::cout << "         |         " << std::endl;
  std::cout << "    p2 = p1        " << std::endl;                                       
  std::cout << "          \\       " << std::endl;
  std::cout << "          end      " << std::endl;
    
  HMscPtr h1(new HMsc(L"h1"));
  BMscPtr bmsc(new BMsc());

  StartNodePtr start1 = new StartNode(); h1->set_start(start1);
  EndNodePtr end1(new EndNode);h1->add_node(end1);
  ReferenceNodePtr p1(new ReferenceNode());h1->add_node(p1);
  p1->set_msc(bmsc);
  ReferenceNodePtr p2(new ReferenceNode());h1->add_node(p2);
  p2->set_msc(bmsc);
  
  start1->add_successor(p1.get());
  p1->add_successor(end1.get());
  p1->add_successor(p2.get());
  p2->add_successor(p1.get());
  
  ChannelMapperPtr chm;
  LivelockCheckerPtr live = LivelockChecker::instance();

  if(!live->check(h1,chm).empty())
  {
    std::cerr << "ERROR: h1 contains livelock" << std::endl;
    return false;
  }
  std::cerr << "OK: h1 doesn't contain livelock" << std::endl;
  return true;
}

int main() {
  RETURN_IF_FAILED(TestA());
  RETURN_IF_FAILED(TestB());
  RETURN_IF_FAILED(TestC());
  return 0;
}

// $Id: livelock_checker_test.cpp 711 2010-04-01 10:41:01Z vacek $
