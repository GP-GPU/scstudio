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
 * $Id: race_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
 */

#include <iostream>

#include "data/msc.h"
#include "check/race/race_checker.h"

#define RETURN_IF_FAILED(res) if(!(res)) return 1;

bool print_result(const HMscPtr hmsc,const bool is_race_free,const std::string& mapper_name)
{
  bool result;
  if(is_race_free)
    if(hmsc.get())
    {
      result =false;
      std::cout << "ERROR: HMsc is supposed to be race free for " << mapper_name
                << " channel mapper but it isn't" << std::endl;
    }
    else
    {
      result = true;
      std::cout << "OK: HMsc is supposed to be race free for " << mapper_name
                << " channel mapper and it is" << std::endl;
    }
  else
    if(hmsc.get())
    {
      result = true;
      std::cout << "OK: HMsc is supposed not to be race free for " << mapper_name
                << " channel mapper and it isn't so" << std::endl;
    }
    else
    {
      result =false;
      std::cout << "ERROR: HMsc is supposed not to be race free for " << mapper_name
                << " channel mapper but it is" << std::endl;
    }
  std::cout << std::endl;
  return result;
}

bool check(HMscPtr hmsc,const bool is_sr_race_free,const bool is_srl_race_free)
{
  static RaceChecker ch;
  SRChannelMapperPtr srm = SRChannelMapper::instance();
  SRMChannelMapperPtr srlm = SRMChannelMapper::instance();
	std::list<HMscPtr> res;
	HMscPtr presult;
	res = ch.check(hmsc,srm);
	if(res.empty())
		presult = NULL;
	else
		presult = res.back();

  if(!print_result(presult,is_sr_race_free,"sender-receiver"))
		return false;

	res = ch.check(hmsc,srlm);
	if(res.empty())
		presult = NULL;
	else
		presult = res.back();
  if(!print_result(presult,is_srl_race_free,"sender-receiver-label"))
  	return false;
	return true;
}

bool HMscA()
{
  std::cout << "HMscA:" << std::endl;
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
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1_1);
  hmsc1->add_node(r1_2);
  hmsc1->add_node(end1);
  start1->add_successor(r1_1.get());
  start1->add_successor(r1_2.get());
  r1_1->add_successor(end1.get());
  r1_2->add_successor(end1.get());

  r1_1->set_msc(bmsc);

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

  return check(hmsc1,true,true);
}

bool HMscB()
{
  std::cout << "HMscB:" << std::endl;
  //race in BMsc for both SRChannelMapper and SRLChannelMapper
  BMscPtr bmsc(new BMsc(L"BMsc"));
  InstancePtr instance1(new Instance(L"1"));
  InstancePtr instance2(new Instance(L"2"));
  InstancePtr instance3(new Instance(L"3"));
  bmsc->add_instance(instance1);
  bmsc->add_instance(instance2);
  bmsc->add_instance(instance3);
  StrictOrderAreaPtr strict1(new StrictOrderArea());
  StrictOrderAreaPtr strict2(new StrictOrderArea());
  StrictOrderAreaPtr strict3(new StrictOrderArea());
  instance1->add_area(strict1);
  instance2->add_area(strict2);
  instance3->add_area(strict3);
  EventPtr e1 = strict1->add_event();
  EventPtr e2_1 = strict2->add_event();
  EventPtr e2_2 = strict2->add_event();
  EventPtr e3 = strict3->add_event();
  CompleteMessagePtr m1 = new CompleteMessage(L"hi");
  m1->glue_events(e1, e2_1);
  CompleteMessagePtr m2 = new CompleteMessage(L"hi");
  m2->glue_events(e3, e2_2);

  HMscPtr hmsc1(new HMsc(L"HMsc"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1);
  hmsc1->add_node(end1);
  start1->add_successor(r1.get());
  r1->add_successor(end1.get());

  r1->set_msc(bmsc);

  return check(hmsc1,false,false);
}

bool HMscC()
{
  std::cout << "HMscC:" << std::endl;
  BMscPtr bmsc(new BMsc(L"BMsc"));
  InstancePtr instance1(new Instance(L"1"));
  InstancePtr instance2(new Instance(L"2"));
  bmsc->add_instance(instance1);
  bmsc->add_instance(instance2);
  StrictOrderAreaPtr strict1(new StrictOrderArea());
  StrictOrderAreaPtr strict2(new StrictOrderArea());
  instance1->add_area(strict1);
  instance2->add_area(strict2);
  EventPtr e1_1 = strict1->add_event();
  EventPtr e1_2 = strict1->add_event();
  EventPtr e2_1 = strict2->add_event();
  EventPtr e2_2 = strict2->add_event();
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1_1, e2_1);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e1_2, e2_2);

  HMscPtr hmsc1(new HMsc(L"HMsc"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1);
  hmsc1->add_node(end1);
  start1->add_successor(r1.get());
  r1->add_successor(end1.get());

  r1->set_msc(bmsc);

  return check(hmsc1,true,false);
}

bool HMscD()
{
  std::cout << "HMscD:" << std::endl;
  BMscPtr bmsc1(new BMsc(L"BMsc"));
  InstancePtr instance1_1(new Instance(L"1"));
  InstancePtr instance1_2(new Instance(L"2"));
  bmsc1->add_instance(instance1_1);
  bmsc1->add_instance(instance1_2);
  StrictOrderAreaPtr strict1_1(new StrictOrderArea());
  StrictOrderAreaPtr strict1_2(new StrictOrderArea());
  instance1_1->add_area(strict1_1);
  instance1_2->add_area(strict1_2);
  EventPtr e1 = strict1_1->add_event();
  EventPtr e2 = strict1_2->add_event();
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e2);

  BMscPtr bmsc2(new BMsc(L"BMsc"));
  InstancePtr instance2_1(new Instance(L"1"));
  InstancePtr instance2_2(new Instance(L"2"));
  bmsc2->add_instance(instance2_1);
  bmsc2->add_instance(instance2_2);
  StrictOrderAreaPtr strict2_1(new StrictOrderArea());
  StrictOrderAreaPtr strict2_2(new StrictOrderArea());
  instance2_1->add_area(strict2_1);
  instance2_2->add_area(strict2_2);
  EventPtr e3 = strict2_1->add_event();
  EventPtr e4 = strict2_2->add_event();
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e3, e4);

  HMscPtr hmsc1(new HMsc(L"HMsc"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1(new ReferenceNode());
  ReferenceNodePtr r2(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1);
  hmsc1->add_node(r2);
  hmsc1->add_node(end1);
  start1->add_successor(r1.get());
  r1->add_successor(r2.get());
  r2->add_successor(end1.get());

  r1->set_msc(bmsc1);
  r2->set_msc(bmsc2);

  return check(hmsc1,true,false);
}

bool HMscE()
{
  std::cout << "HMscE:" << std::endl;

  BMscPtr bmsc1(new BMsc(L"BMsc"));
  InstancePtr instance1(new Instance(L"1"));
  InstancePtr instance2(new Instance(L"2"));
  bmsc1->add_instance(instance1);
  bmsc1->add_instance(instance2);
  StrictOrderAreaPtr strict(new StrictOrderArea());
  CoregionAreaPtr coregion(new CoregionArea());

  instance1->add_area(coregion);
  instance2->add_area(strict);

  

  EventPtr e1 = strict->add_event();
  EventPtr e2 = strict->add_event();

  CoregionEventPtr e3(new CoregionEvent());
  CoregionEventPtr e4(new CoregionEvent());
  coregion->add_event(e3);
  coregion->add_event(e4);
  e3->add_successor(e4.get());

  CompleteMessagePtr m1 = new CompleteMessage(L"b1");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"d1");
  m2->glue_events(e4, e2);

  HMscPtr hmsc1(new HMsc(L"HMsc"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1);
  hmsc1->add_node(end1);
  start1->add_successor(r1.get());
  r1->add_successor(end1.get());

  r1->set_msc(bmsc1);

  return check(hmsc1,true,true);
}

bool HMscF()
{
  std::cout << "HMscF:" << std::endl;

  BMscPtr bmsc1(new BMsc(L"BMsc"));
  InstancePtr instance1(new Instance(L"NAME"));
  InstancePtr instance2(new Instance(L"NAME"));
  InstancePtr instance3(new Instance(L"NAME"));
  bmsc1->add_instance(instance1);
  bmsc1->add_instance(instance2);
  bmsc1->add_instance(instance3);

  StrictOrderAreaPtr strict1(new StrictOrderArea());
  instance1->add_area(strict1);

  StrictOrderAreaPtr strict2(new StrictOrderArea());
  instance2->add_area(strict2);

  StrictOrderAreaPtr strict3(new StrictOrderArea());
  instance3->add_area(strict3);  

  EventPtr e1 = strict1->add_event();
  EventPtr e2 = strict2->add_event();
  CompleteMessagePtr m = new CompleteMessage(L"NAME");
  m->glue_events(e1,e2);

  e1 = strict2->add_event();
  e2 = strict3->add_event();
  m = new CompleteMessage(L"NAME");
  m->glue_events(e1,e2);

  e1 = strict2->add_event();
  e2 = strict1->add_event();
  m = new CompleteMessage(L"NAME");
  m->glue_events(e1,e2);

  e1 = strict3->add_event();
  e2 = strict1->add_event();
  m = new CompleteMessage(L"NAME");
  m->glue_events(e1,e2);

  HMscPtr hmsc1(new HMsc(L"HMsc"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1);
  hmsc1->add_node(end1);
  start1->add_successor(r1.get());
  r1->add_successor(end1.get());

  r1->set_msc(bmsc1);

  return check(hmsc1,false,false);
}

bool HMscG()
{
  std::cout << "HMscG:" << std::endl;
  BMscPtr bmsc1(new BMsc(L"BMsc1"));
  InstancePtr instance1_1(new Instance(L"1"));
  InstancePtr instance1_2(new Instance(L"2"));
  bmsc1->add_instance(instance1_1);
  bmsc1->add_instance(instance1_2);
  StrictOrderAreaPtr strict1_1(new StrictOrderArea());
  StrictOrderAreaPtr strict1_2(new StrictOrderArea());
  instance1_1->add_area(strict1_1);
  instance1_2->add_area(strict1_2);
  EventPtr e1 = strict1_1->add_event();
  EventPtr e2 = strict1_2->add_event();
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e2);

  BMscPtr bmsc2(new BMsc(L"BMsc2"));
  InstancePtr instance2_1(new Instance(L"3"));
  InstancePtr instance2_2(new Instance(L"1"));
  bmsc2->add_instance(instance2_1);
  bmsc2->add_instance(instance2_2);
  StrictOrderAreaPtr strict2_1(new StrictOrderArea());
  StrictOrderAreaPtr strict2_2(new StrictOrderArea());
  instance2_1->add_area(strict2_1);
  instance2_2->add_area(strict2_2);
  EventPtr e3 = strict2_1->add_event();
  EventPtr e4 = strict2_2->add_event();
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e3, e4);

  HMscPtr hmsc1(new HMsc(L"HMsc"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1(new ReferenceNode());
  ReferenceNodePtr r2(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1);
  hmsc1->add_node(r2);
  hmsc1->add_node(end1);
  start1->add_successor(r1.get());
  r1->add_successor(r2.get());
  r2->add_successor(end1.get());

  r1->set_msc(bmsc1);
  r2->set_msc(bmsc2);

  return check(hmsc1,false,false);
}

bool HMscH()
{
  std::cout << "HMscH:" << std::endl;
  BMscPtr bmsc1(new BMsc(L"BMsc1"));
  InstancePtr instance1_1(new Instance(L"1"));
  InstancePtr instance1_2(new Instance(L"2"));
  bmsc1->add_instance(instance1_1);
  bmsc1->add_instance(instance1_2);
  StrictOrderAreaPtr strict1_1(new StrictOrderArea());
  StrictOrderAreaPtr strict1_2(new StrictOrderArea());
  instance1_1->add_area(strict1_1);
  instance1_2->add_area(strict1_2);
  EventPtr e1 = strict1_1->add_event();
  EventPtr e2 = strict1_2->add_event();
  CompleteMessagePtr m1 = new CompleteMessage(L"b");
  m1->glue_events(e2, e1);

  BMscPtr bmsc2(new BMsc(L"BMsc2"));
  InstancePtr instance2_1(new Instance(L"1"));
  InstancePtr instance2_2(new Instance(L"2"));
  InstancePtr instance2_3(new Instance(L"3"));
  bmsc2->add_instance(instance2_1);
  bmsc2->add_instance(instance2_2);
  bmsc2->add_instance(instance2_3);
  StrictOrderAreaPtr strict2_1(new StrictOrderArea());
  StrictOrderAreaPtr strict2_2(new StrictOrderArea());
  StrictOrderAreaPtr strict2_3(new StrictOrderArea());
  instance2_1->add_area(strict2_1);
  instance2_2->add_area(strict2_2);
  instance2_3->add_area(strict2_3);

  EventPtr s = strict2_1->add_event();
  EventPtr r = strict2_3->add_event();
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(s, r);

  s = strict2_3->add_event();
  r = strict2_2->add_event();
  m2 = new CompleteMessage(L"b");
  m2->glue_events(s,r);

  HMscPtr hmsc1(new HMsc(L"HMsc"));
  StartNodePtr start1 = new StartNode();
  hmsc1->set_start(start1);
  ReferenceNodePtr r1(new ReferenceNode());
  ReferenceNodePtr r2(new ReferenceNode());
  EndNodePtr end1(new EndNode());
  hmsc1->add_node(r1);
  hmsc1->add_node(r2);
  hmsc1->add_node(end1);
  start1->add_successor(r1.get());
  r1->add_successor(r2.get());
  r2->add_successor(end1.get());

  r1->set_msc(bmsc1);
  r2->set_msc(bmsc2);

  return check(hmsc1,true,true);
}

int main(int argc, char** argv) {
  RETURN_IF_FAILED(HMscA());
  RETURN_IF_FAILED(HMscB());
  RETURN_IF_FAILED(HMscC());
  RETURN_IF_FAILED(HMscD());
  RETURN_IF_FAILED(HMscE());
  RETURN_IF_FAILED(HMscF());
  RETURN_IF_FAILED(HMscG());
  RETURN_IF_FAILED(HMscH());
  return 0;
}

// $Id: race_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
