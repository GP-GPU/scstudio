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
 * Copyright (c) LK
 *
 * $Id: tighten_hmsc_test.cpp 750 2010-05-08 16:31:21Z gotthardp $
 */

#include <iostream>
#include "check/time/tightening.h"
#include "data/Z120/z120.h"

#include "check/time/time_trace_race.h"
#include "data/checker.h"
#include "check/time/constraint_syntax.h"
#include "check/time/tightening.h"

#include <list>



int main(int argc, char** argv) {

  BMscPtr bmsc(new BMsc(L"BMsc"));
  InstancePtr instance1(new Instance(L"1"));
  InstancePtr instance2(new Instance(L"2"));
  InstancePtr instance3(new Instance(L"3"));
  bmsc->add_instance(instance1);
  bmsc->add_instance(instance2);
  bmsc->add_instance(instance3);
  StrictOrderAreaPtr strict1(new StrictOrderArea());
  StrictOrderAreaPtr strict2(new StrictOrderArea());
  instance1->add_area(strict1);
  instance2->add_area(strict2);
  EventPtr e0 = strict1->add_event();
  EventPtr e1 = strict2->add_event();

  CompleteMessagePtr m1 = new CompleteMessage(L"hi1");
  m1->glue_events(e1, e0);


  BMscPtr bmsc2(new BMsc(L"BMsc2"));
  InstancePtr instance21(new Instance(L"1"));
  InstancePtr instance22(new Instance(L"2"));
  InstancePtr instance23(new Instance(L"3"));
  bmsc2->add_instance(instance21);
  bmsc2->add_instance(instance22);
  bmsc2->add_instance(instance23);
  StrictOrderAreaPtr strict21(new StrictOrderArea());
  StrictOrderAreaPtr strict22(new StrictOrderArea());
  instance22->add_area(strict21);
  instance23->add_area(strict22);
  EventPtr e20 = strict21->add_event();
  EventPtr e21 = strict22->add_event();

  CompleteMessagePtr m21 = new CompleteMessage(L"hi1");
  m21->glue_events(e20, e21);


  MscTimeIntervalD in6(0,5);
  MscTimeIntervalD in8(5,10);
  MscTimeIntervalD in9(7,D::infinity());
  MscTimeIntervalD in10(6,20);



  MscTimeIntervalSetD ins1;
  MscTimeIntervalSetD ins2;
  MscTimeIntervalSetD ins3;

  ins1.insert(in6);
  ins1.insert(in9);

  ins2.insert(in10);

  ins3.insert(in8);


  TimeRelationEventPtr rel1 = new TimeRelationEvent(ins1);
  rel1->glue_events(e1.get(),e0.get());
  TimeRelationEventPtr rel2 = new TimeRelationEvent(ins2);
  rel2->glue_events(e20.get(),e21.get());


  Z120 z120;

  z120.save_msc(std::cout,L"original 1",bmsc);
  z120.save_msc(std::cout,L"original 2",bmsc2);

//    ChannelMapperPtr mapper;
//    SRChannelMapperPtr srm = SRChannelMapper::instance(); //sender-receiver
// //   SRMChannelMapperPtr srlm = SRMChannelMapper::instance(); //sender-receiver-label

  HMscPtr h1(new HMsc(L"h1"));

  StartNodePtr start1 = new StartNode(); h1->set_start(start1);
  EndNodePtr end1(new EndNode);h1->add_node(end1);
  ReferenceNodePtr p1(new ReferenceNode());h1->add_node(p1);
  ReferenceNodePtr p2(new ReferenceNode());h1->add_node(p2);

  start1->add_successor(p1.get());
  p1->add_successor(p2.get());
  p1->add_successor(p1.get());
  p2->add_successor(p2.get());
  p2->add_successor(end1.get());
  p1->set_msc(bmsc);
  p2->set_msc(bmsc2);


  TimeRelationRefNodePtr rel3 = new TimeRelationRefNode(ins3);
  rel3->glue_ref_nodes(0,p1.get(),1,p2.get());

  std::cout << " " << std::endl;

  std::list<MscElement*> path_list;
  path_list.push_back(p1.get());
  path_list.push_back(p2.get());

  HMscTighter* tighter = new HMscTighter();
  //IntervalSetMatrix result =
  tighter->on_path_found(path_list);

  std::cout<< "matrix result:" << std::endl;
 // std::cout<< result << std::endl;



  HMscFlatPathToBMscDuplicator duplicator;
  bmsc2 = duplicator.duplicate_path(path_list);
  p1->set_msc(bmsc2);
  path_list.clear();
  path_list.push_back(p1.get());
  rel3->glue_ref_nodes(0,p1.get(),1,p1.get());
 // result = tighter->tight_path(path_list);
 // std::cout<< result << std::endl;




 std::cout<< "test finished" << std::endl;

  return 0;
}


// $Id: tighten_hmsc_test.cpp 750 2010-05-08 16:31:21Z gotthardp $
