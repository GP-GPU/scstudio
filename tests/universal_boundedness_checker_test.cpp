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
 * Copyright (c) 2009 Václav Vacek <vacek@ics.muni.cz>
 *
 * $Id: universal_boundedness_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
 */

#include "data/msc.h"
#include "check/boundedness/universal_boundedness_checker.h"


#include <iostream>

#define RETURN_IF_FAILED(res) if(!(res)) return 1;

bool print_result(const HMscPtr hmsc,const bool is_bounded)
{
  bool result;
  if(is_bounded)
    if(hmsc.get())
    {
      result = false;
      std::cout << "ERROR: HMsc is supposed to be universally bounded but it isn't." 
                << std::endl;
    }
    else
    {
      result = true;
      std::cout << "OK: HMsc is supposed to be universally bounded and it is."
                << std::endl;
    }
  else
    if(hmsc.get())
    {
      result = true;
      std::cout << "OK: HMsc is supposed not to be universally bounded and it isn't."
                << std::endl;
    }
    else
    {
      result = false;
      std::cout << "ERROR: HMsc is supposed not to be universally bounded but it is."
                << std::endl;
    }
  std::cout << std::endl;

  return result;
}

bool check(HMscPtr hmsc,const bool is_bounded)
{
  UniversalBoundednessCheckerPtr checker = UniversalBoundednessChecker::instance();

  SRChannelMapperPtr srm = SRChannelMapper::instance();
	HMscPtr presult;
	std::list<HMscPtr> result;
	result = checker->check(hmsc,srm);
	if(result.empty())
		presult = NULL;
	else
		presult = result.back();

  return print_result(presult,is_bounded);  
}

bool HMscA() {
  std::cout << "Checking this bmsc in a cycle:" << std::endl;
  std::cout << "  p1           p2   " << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O-----a----->O e3" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e2 O<----a------O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;

  BMscPtr bmsc1(new BMsc());
  
  InstancePtr i1(new Instance(L"p1"));
  bmsc1->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  bmsc1->add_instance(i2);
  
  EventAreaPtr a1(new StrictOrderArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e4, e2);

  HMscPtr hmsc1(new HMsc(L"HMsc1"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1_1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1_1);
  hmsc1->add_node(end1);
  start1->add_successor(r1_1.get());
  r1_1->add_successor(end1.get());
  r1_1->add_successor(r1_1.get());
  r1_1->set_msc(bmsc1);
  
  return check(hmsc1,true);
}

bool HMscB() {
  std::cout << "Checking these bmscs in a cycle:" << std::endl;
  std::cout << "  p1           p2   " << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O-----a----->O e2" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << std::endl;
  std::cout << "  p1           p2   " << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e3 O<----a------O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "   |            |   " << std::endl;

  BMscPtr bmsc2(new BMsc());
  
  InstancePtr i1(new Instance(L"p1"));
  bmsc2->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  bmsc2->add_instance(i2);
  
  EventAreaPtr a1(new StrictOrderArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e2);

  //////////////
  BMscPtr bmsc3(new BMsc());
  
  InstancePtr i3(new Instance(L"p1"));
  bmsc3->add_instance(i3);
  InstancePtr i4(new Instance(L"p2"));
  bmsc3->add_instance(i4);
  
  EventAreaPtr a3(new StrictOrderArea());
  i3->add_area(a3);
  EventAreaPtr a4(new StrictOrderArea());
  i4->add_area(a4);
  
  EventPtr e3 = a3->add_event();
  EventPtr e4 = a4->add_event();
  
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e4, e3);
  
  HMscPtr hmsc2(new HMsc(L"HMsc2"));
  StartNodePtr start1 = new StartNode();
  hmsc2->set_start(start1);
  ReferenceNodePtr r1_1(new ReferenceNode());
  ReferenceNodePtr r1_2(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc2->add_node(r1_1);
  hmsc2->add_node(r1_2);
  hmsc2->add_node(end1);
  start1->add_successor(r1_1.get());
  r1_1->add_successor(r1_2.get());
  r1_2->add_successor(end1.get());
  r1_2->add_successor(r1_1.get());
  r1_1->set_msc(bmsc2);
  r1_2->set_msc(bmsc3);
  
  return check(hmsc2,true);
}

bool HMscC() {
  std::cout << "Checking this bmsc in a cycle:" << std::endl;
  std::cout << "  p1           p2   " << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O-----a----->O e2" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "   |            |   " << std::endl;

  BMscPtr bmsc1(new BMsc());
  
  InstancePtr i1(new Instance(L"p1"));
  bmsc1->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  bmsc1->add_instance(i2);
  
  EventAreaPtr a1(new StrictOrderArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e2);


  HMscPtr hmsc1(new HMsc(L"HMsc1"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1_1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1_1);
  hmsc1->add_node(end1);
  start1->add_successor(r1_1.get());
  r1_1->add_successor(end1.get());
  r1_1->add_successor(r1_1.get());
  r1_1->set_msc(bmsc1);
  
  return check(hmsc1,false);
}

int main(int argc, char** argv) {
  
  RETURN_IF_FAILED(HMscA());

  RETURN_IF_FAILED(HMscB());

  RETURN_IF_FAILED(HMscC());

  ;
  /*CommunicationGraph tester;
  std::vector<std::vector<unsigned> > test_graph, test_graph_matrix;
  const int g_s = 6;
  test_graph.resize(g_s);
  
  
  test_graph[0].push_back(1);
  test_graph[1].push_back(2);
  test_graph[2].push_back(3);
  test_graph[2].push_back(5);
  test_graph[3].push_back(1);
  test_graph[3].push_back(4);
  test_graph[4].push_back(5);
  test_graph[5].push_back(1);
  test_graph[5].push_back(0);

  unsigned i, j;
  test_graph_matrix.resize(g_s);
  for(i = 0; i < g_s; i++)
  {
    test_graph_matrix.at(i).resize(g_s);
    for(j = 0; j < g_s; j++)
      test_graph_matrix.at(i).at(j) = 0;
    for(j = 0; j < test_graph.at(i).size(); j++)
      test_graph_matrix.at(i).at(test_graph.at(i).at(j)) = 1;
  }


  BMscPtr bmsc2(new BMsc());
  
  InstancePtr i1(new Instance("p1"));
  bmsc2->add_instance(i1);
  InstancePtr i2(new Instance("p2"));
  bmsc2->add_instance(i2);
  
  EventAreaPtr a1(new StrictOrderArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage("a");
  m1->glue_events(e1, e2);

  HMscPtr hmsc2(new HMsc("HMsc1"));
  StartNodePtr start1 = new StartNode();
  hmsc2->set_start(start1);
  ReferenceNodePtr r1_1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc2->add_node(r1_1);
  hmsc2->add_node(end1);
  start1->add_successor(r1_1.get());
  r1_1->add_successor(end1.get());
  r1_1->add_successor(r1_1.get());
  r1_1->set_msc(bmsc2);

  tester.create_from_bmsc(bmsc2);
  std::cout << tester.is_strongly_connected() << std::endl;
  return 0;*/
}

// $Id: universal_boundedness_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
