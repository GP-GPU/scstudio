#include "check/time/hmsc_all_paths.h"
#include "check/time/hmsc_tighten.h"

#include <iostream>
int main()
{
  BMscPtr bmsc(new BMsc(L"BMsc"));
  InstancePtr instance1(new Instance(L"1"));
  InstancePtr instance2(new Instance(L"2"));
  bmsc->add_instance(instance1);
  bmsc->add_instance(instance2);
  StrictOrderAreaPtr strict1(new StrictOrderArea());
  StrictOrderAreaPtr strict2(new StrictOrderArea());
  instance1->add_area(strict1);
  instance2->add_area(strict2);
  EventPtr e1 = strict1->add_event();
  EventPtr e2 = strict2->add_event();
  CompleteMessagePtr m1 = new CompleteMessage(L"hi");
  m1->glue_events(e1, e2);

  HMscPtr hmsc1(new HMsc(L"HMsc1"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1_1(new ReferenceNode());
  ReferenceNodePtr r1_2(new ReferenceNode());
  ReferenceNodePtr r1_3(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1_1);
  hmsc1->add_node(r1_2);
  hmsc1->add_node(r1_3);
  hmsc1->add_node(end1);
  start1->add_successor(r1_1.get());
  r1_1->add_successor(r1_3.get());
  r1_1->add_successor(r1_2.get());
  r1_2->add_successor(r1_2.get());
  r1_2->add_successor(end1.get());
  r1_2->add_successor(r1_1.get());
  r1_3->add_successor(r1_2.get());

  r1_1->set_msc(bmsc);
  r1_2->set_msc(bmsc);
  r1_3->set_msc(bmsc);

//  AllPaths all_paths;
  MscTimeIntervalSetD set;
  set.insert(MscTimeIntervalD(20,40));
  HMscAllPaths paths = AllPaths::get_all_paths(hmsc1,TimeRelationRefNode::create(false,r1_1.get(),r1_2.get(),true,set));

  std::cerr << "Kabonga: " << paths.first.size() << " " << paths.second << std::endl;

  BMscPtr bmsc1(new BMsc(L"BMsc "));
  InstancePtr inst1(new Instance(L"1"));
  InstancePtr inst2(new Instance(L"2"));
  bmsc1->add_instance(inst1);
  bmsc1->add_instance(inst2);

  StrictOrderAreaPtr str1(new StrictOrderArea());
  StrictOrderAreaPtr str2(new StrictOrderArea());
  inst1->add_area(str1);
  inst2->add_area(str2);
  EventPtr ev1 = str1->add_event();
  EventPtr ev2 = str2->add_event();
  CompleteMessagePtr mess1 = new CompleteMessage(L"hi");
  mess1->glue_events(ev1, ev2);


  BMscPtr bmsc2(new BMsc(L"BMsc "));
  InstancePtr inst3(new Instance(L"1"));
  InstancePtr inst4(new Instance(L"2"));
  bmsc2->add_instance(inst3);
  bmsc2->add_instance(inst4);

  StrictOrderAreaPtr str3(new StrictOrderArea());
  StrictOrderAreaPtr str4(new StrictOrderArea());
  inst3->add_area(str3);
  inst4->add_area(str4);
  EventPtr ev3 = str3->add_event();
  EventPtr ev4 = str4->add_event();
  CompleteMessagePtr mess2 = new CompleteMessage(L"hi");
  mess2->glue_events(ev4, ev3);

  MscTimeIntervalD interval1(4,8);
  MscTimeIntervalD interval2(5,10);
  MscTimeIntervalD interval3(1,2);
  MscTimeIntervalD interval4(3,4);
  MscTimeIntervalD interval5(0,13);


  MscTimeIntervalSetD set1;
  MscTimeIntervalSetD set2;
  MscTimeIntervalSetD set3;
  MscTimeIntervalSetD set4;

  set1.insert(interval1);
  set2.insert(interval2);
  set3.insert(interval3);
  set3.insert(interval4);
  set4.insert(interval5);
  std::list<TimeRelationRefNodePtr> constr;
  ReferenceNodePtr r1(new ReferenceNode());
  ReferenceNodePtr r2(new ReferenceNode());
  constr.push_back(TimeRelationRefNode::create(true,r1.get(),r2.get(),true,set3)); 
  TimeRelationEvent::create(ev1.get(),ev2.get(),set1);
  TimeRelationEvent::create(ev4.get(),ev3.get(),set2);

  std::list<BMscPtr> list_bmsc;
  list_bmsc.push_back(bmsc1);
  list_bmsc.push_back(bmsc2);

  std::cerr << "tighten: " <<std::endl;
  TightenSetOfPaths tighten;
  std::cerr << "dsdfsdf: " << tighten.tightenMsgPath(list_bmsc,constr,set4) << std::endl;
  
/*
  HMscPtr hmsc2(new HMsc(L"HMsc2"));
  StartNodePtr start2 = new StartNode();
  hmsc2->set_start(start2);
  ReferenceNodePtr r2_1(new ReferenceNode());
  ReferenceNodePtr r2_2(new ReferenceNode());
  EndNodePtr end2(new EndNode());
  hmsc2->add_node(r2_1);
  hmsc2->add_node(r2_2);
  hmsc2->add_node(end2);
  start2->add_successor(r2_1.get());
  start2->add_successor(r2_2.get());
  r2_1->add_successor(end2.get());
  r2_2->add_successor(end2.get());

  r1_2->set_msc(hmsc2);
  r2_1->set_msc(bmsc);

  HMscPtr hmsc3(new HMsc(L"HMsc3"));
  StartNodePtr start3 = new StartNode();
  hmsc3->set_start(start3);
  ReferenceNodePtr r3_1(new ReferenceNode());
  hmsc3->add_node(r2_1);
  start3->add_successor(r3_1.get());

  r2_2->set_msc(hmsc3);
  r3_1->set_msc(bmsc);
  */
  return 0;
}
