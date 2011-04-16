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
 * Copyright (c) 2008
 *
 * $Id: fifo_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
 */

#include "data/msc.h"
#include "check/order/fifo_checker.h"

#include <iostream>

bool BMscA();
bool BMscB();
bool BMscC();
bool BMscD();
bool BMscE();
bool BMscF();
bool BMscG();
bool BMscH(); //BMscA
bool BMscI(); //BMscB
bool BMscJ(); //BMscC
bool BMscK(); //BMscD
bool BMscL(); //BMscE
bool BMscM(); //BMscF
bool BMscN(); //BMscG
bool BMscO(); //BMscK
bool BMscP(); //BMscL
bool BMscR(); //BMscM
bool BMscQ();

#define RETURN_IF_FAILED(res) if(!(res)) return 1;

/*
 * 
 */
int main(int argc, char** argv) {
  RETURN_IF_FAILED(BMscA());
  RETURN_IF_FAILED(BMscB());
  RETURN_IF_FAILED(BMscC());
  RETURN_IF_FAILED(BMscD());
  RETURN_IF_FAILED(BMscE());
  RETURN_IF_FAILED(BMscF());
  RETURN_IF_FAILED(BMscG());
  RETURN_IF_FAILED(BMscH());
  RETURN_IF_FAILED(BMscI());
  RETURN_IF_FAILED(BMscJ());
  RETURN_IF_FAILED(BMscK());
  RETURN_IF_FAILED(BMscL());
  RETURN_IF_FAILED(BMscM());
  RETURN_IF_FAILED(BMscN());
  RETURN_IF_FAILED(BMscO());
  RETURN_IF_FAILED(BMscP());
  RETURN_IF_FAILED(BMscR());
  RETURN_IF_FAILED(BMscQ());
  return 0;
}

bool print_result(const BMscPtr bmsc,const bool is_fifo,const std::string& mapper_name)
{
  bool result;
  if(is_fifo)
    if(bmsc.get())
    {
      result =false;
      std::cout << "ERROR: BMsc is supposed to satisfy fifo for " << mapper_name
                << " channels but it doesn't satisfy it" << std::endl;
    }
    else
    {
      result = true;
      std::cout << "OK: BMsc is supposed to satisfy fifo for " << mapper_name
                << " channels and it does so" << std::endl;
    }
  else
    if(bmsc.get())
    {
      result = true;
      std::cout << "OK: BMsc is supposed not to satisfy fifo for " << mapper_name
                << " channels and it doesn't so" << std::endl;
    }
    else
    {
      result =false;
      std::cout << "ERROR: BMsc is supposed not to satisfy fifo for " << mapper_name
                << " channels but it satisfies it" << std::endl;
    }
  std::cout << std::endl;
  return result;
}

bool check(BMscPtr bmsc,const bool is_sr_fifo,const bool is_srl_fifo)
{
  FifoCheckerPtr checker = FifoChecker::instance();
  SRChannelMapperPtr srm = SRChannelMapper::instance();
  SRMChannelMapperPtr srlm = SRMChannelMapper::instance();
	std::list<BMscPtr> result;
	return (print_result((result = checker->check(bmsc,srm)).empty()?NULL:result.back(),is_sr_fifo,"sender-receiver") &&
  print_result((result = checker->check(bmsc,srlm)).empty()?NULL:result.back(),is_srl_fifo,"sender-receiver-label"));
}

bool BMscA() {
  std::cout << "Checking:" << std::endl;
  std::cout << "  p1           p2   " << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O-----a----->O e3" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e2 O<----a------O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;

  BMscPtr myBmsc(new BMsc());
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
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
  
  return check(myBmsc,true,true);
}

bool BMscB() {
  std::cout << "Checking:" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O-----a----->O e3" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e2 O-----a----->O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
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
  m2->glue_events(e2, e4);
  
  return check(myBmsc,true,true);
}

bool BMscC() {
  std::cout << "Checking:" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O       --a->O e3" << std::endl;
  std::cout << "   | \\    /     |  " << std::endl;
  std::cout << "   |  \\  /      |  " << std::endl;
  std::cout << "   |   /\\       |  " << std::endl;
  std::cout << "   |  /  \\      |  " << std::endl;
  std::cout << "e2 O       --a->O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;
  
  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr s1(new StrictOrderArea());
  i1->add_area(s1);
  EventAreaPtr s2(new StrictOrderArea());
  i2->add_area(s2);
  
  EventPtr e1 = s1->add_event();
  EventPtr e2 = s1->add_event();
  EventPtr e3 = s2->add_event();
  EventPtr e4 = s2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e4);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e2, e3);
  
  return check(myBmsc,false,false);
}

bool BMscD() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |      " << std::endl;
  std::cout << "<coregion>        <coregion> " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "|     e1  ----a---> e3     | " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "|     e2  ----a---> e4     | " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "<coregion>        <coregion> " << std::endl;
  std::cout << "    |                  |     " << std::endl;
  
  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr ca1(new CoregionArea());
  i1->add_area(ca1);
  EventAreaPtr ca2(new CoregionArea());
  i2->add_area(ca2);
  
  EventPtr e1 = ca1->add_event();
  EventPtr e2 = ca1->add_event();
  EventPtr e3 = ca2->add_event();
  EventPtr e4 = ca2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e2, e4);
  
  return check(myBmsc,true,true);
}

bool BMscE() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  -----a----->O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  -----a----->O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;
  
  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr c(new CoregionArea());
  i1->add_area(c);
  EventAreaPtr s(new StrictOrderArea());
  i2->add_area(s);
  
  EventPtr e1 = c->add_event();
  EventPtr e2 = c->add_event();
  EventPtr e3 = s->add_event();
  EventPtr e4 = s->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e2, e4);
  
  return check(myBmsc,false,false);
}

bool BMscF() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  <-----a-----O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  <-----a-----O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;
  
  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr ca1(new CoregionArea());
  i1->add_area(ca1);
  EventAreaPtr soa1(new StrictOrderArea());
  i2->add_area(soa1);
  
  EventPtr e1 = ca1->add_event();
  EventPtr e2 = ca1->add_event();
  EventPtr e3 = soa1->add_event();
  EventPtr e4 = soa1->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e3, e1);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e4, e2);
  
  return check(myBmsc,true,true);
}

bool BMscG() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  <-----a-----O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  ------a---->O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr ca1(new CoregionArea());
  i1->add_area(ca1);
  EventAreaPtr soa1(new StrictOrderArea());
  i2->add_area(soa1);
  
  EventPtr e1 = ca1->add_event();
  EventPtr e2 = ca1->add_event();
  EventPtr e3 = soa1->add_event();
  EventPtr e4 = soa1->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e3, e1);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e2, e4);
  
  return check(myBmsc,true,true);
}

bool BMscH() {
  std::cout << "Checking:" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O-----b----->O e3" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e2 O<----a------O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;
  
  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr s1(new CoregionArea());
  i1->add_area(s1);
  EventAreaPtr s2(new StrictOrderArea());
  i2->add_area(s2);
  
  EventPtr e1 = s1->add_event();
  EventPtr e2 = s1->add_event();
  EventPtr e3 = s2->add_event();
  EventPtr e4 = s2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"b");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e4, e2);
  
  return check(myBmsc,true,true);
}

bool BMscI() {
  std::cout << "Checking:" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O-----a----->O e3" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e2 O-----b----->O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr s1(new StrictOrderArea());
  i1->add_area(s1);
  EventAreaPtr s2(new StrictOrderArea());
  i2->add_area(s2);
  
  EventPtr e1 = s1->add_event();
  EventPtr e2 = s1->add_event();
  EventPtr e3 = s2->add_event();
  EventPtr e4 = s2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e2, e4);
  
  return check(myBmsc,true,true);
}

bool BMscJ() {
  std::cout << "Checking:" << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O       --a->O e3" << std::endl;
  std::cout << "   | \\    /     |  " << std::endl;
  std::cout << "   |  \\  /      |  " << std::endl;
  std::cout << "   |   /\\       |  " << std::endl;
  std::cout << "   |  /  \\      |  " << std::endl;
  std::cout << "e2 O       --b->O e4" << std::endl;
  std::cout << "   |            |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new StrictOrderArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e2, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e1, e4);
  
  return check(myBmsc,false,true);
}

bool BMscK() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |      " << std::endl;
  std::cout << "<coregion>        <coregion> " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "|     e1  ----a---> e3<-   | " << std::endl;
  std::cout << "|        |        |     |  | " << std::endl;
  std::cout << "|     e2  ----b---> e4--   | " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "<coregion>        <coregion> " << std::endl;
  std::cout << "    |                  |     " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new CoregionArea());
  i1->add_area(a1);
  EventAreaPtr a2(new CoregionArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();

  ((CoregionEvent*)e4.get())->add_successor((CoregionEvent*)e3.get());
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e2, e4);
  
  return check(myBmsc,false,true);
}

bool BMscL() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  -----a----->O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  -----b----->O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new CoregionArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e2, e4);
  
  return check(myBmsc,false,true);
}


bool BMscM() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  <-----a-----O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  <-----b-----O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new CoregionArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e3, e1);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e4, e2);
  
  return check(myBmsc,true,true);
}

bool BMscN() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  <-----a-----O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  ------b---->O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new CoregionArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e3, e1);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e2, e4);
  
  return check(myBmsc,true,true);
}

bool BMscO() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |      " << std::endl;
  std::cout << "<coregion>        <coregion> " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "|     e1  ----a---> e3     | " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "|     e2  ----b---> e4     | " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "|     e5  ----a---> e6     | " << std::endl;
  std::cout << "|        |        |        | " << std::endl;
  std::cout << "<coregion>        <coregion> " << std::endl;
  std::cout << "    |                  |     " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new CoregionArea());
  i1->add_area(a1);
  EventAreaPtr a2(new CoregionArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  EventPtr e5 = a1->add_event();
  EventPtr e6 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e2, e4);
  CompleteMessagePtr m3 = new CompleteMessage(L"a");
  m3->glue_events(e5, e6);
  
  return check(myBmsc,true,true);
}

bool BMscP() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  -----a----->O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  -----b----->O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e5  -----a----->O e6" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new CoregionArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  EventPtr e5 = a1->add_event();
  EventPtr e6 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1, e3);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e2, e4);
  CompleteMessagePtr m3 = new CompleteMessage(L"a");
  m3->glue_events(e5, e6);
  
  return check(myBmsc,false,false);
}

bool BMscR() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |                 |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e1  <-----a-----O e3" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e2  <-----b-----O e4" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "|     e5  <-----a-----O e6" << std::endl;
  std::cout << "|        |            |   " << std::endl;
  std::cout << "<coregion>            |   " << std::endl;
  std::cout << "    |                 |   " << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  
  EventAreaPtr a1(new CoregionArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a1->add_event();
  EventPtr e3 = a2->add_event();
  EventPtr e4 = a2->add_event();
  EventPtr e5 = a1->add_event();
  EventPtr e6 = a2->add_event();
  
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e3, e1);
  CompleteMessagePtr m2 = new CompleteMessage(L"b");
  m2->glue_events(e4, e2);
  CompleteMessagePtr m3 = new CompleteMessage(L"a");
  m3->glue_events(e6, e5);
  
  return check(myBmsc,true,true);
}

bool BMscQ() {
  std::cout << "Checking:" << std::endl;
  std::cout << "    |           |           |           |" << std::endl;
  std::cout << "    o-----a---->o           |           |" << std::endl;
  std::cout << "    |           |           |           |" << std::endl;
  std::cout << "    |           o----b----->o           |" << std::endl;
  std::cout << "    |           |           |           |" << std::endl;
  std::cout << "    |           o<--b ack---o           |" << std::endl;
  std::cout << "    |           |           |           |" << std::endl;
  std::cout << "    |<--a ack---o           |           |" << std::endl;
  std::cout << "    |           |           |           |" << std::endl;
  std::cout << "    |           o----c----------------->o" << std::endl;
  std::cout << "    |           |           |           |" << std::endl;
  std::cout << "    |           o<-----------------d----o" << std::endl;
  std::cout << "    |           |           |           |" << std::endl;

  BMscPtr myBmsc(new BMsc);
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc->add_instance(i2);
  InstancePtr i3(new Instance(L"p3"));
  myBmsc->add_instance(i3);
  InstancePtr i4(new Instance(L"p4"));
  myBmsc->add_instance(i4);
  
  EventAreaPtr a1(new StrictOrderArea());
  i1->add_area(a1);
  EventAreaPtr a2(new StrictOrderArea());
  i2->add_area(a2);
  EventAreaPtr a3(new StrictOrderArea());
  i3->add_area(a3);
  EventAreaPtr a4(new StrictOrderArea());
  i4->add_area(a4);
  
  EventPtr e1 = a1->add_event();
  EventPtr e2 = a2->add_event();
  CompleteMessagePtr m1 = new CompleteMessage(L"a");
  m1->glue_events(e1,e2);

  e1 = a2->add_event();
  e2 = a3->add_event();
  m1 = new CompleteMessage(L"b");
  m1->glue_events(e1,e2);

  e1 = a3->add_event();
  e2 = a2->add_event();
  m1 = new CompleteMessage(L"b ack");
  m1->glue_events(e1,e2);

  e1 = a2->add_event();
  e2 = a1->add_event();
  m1 = new CompleteMessage(L"a ack");
  m1->glue_events(e1,e2);

  e1 = a2->add_event();
  e2 = a4->add_event();
  m1 = new CompleteMessage(L"c");
  m1->glue_events(e1,e2);

  e1 = a4->add_event();
  e2 = a2->add_event();
  m1 = new CompleteMessage(L"d");
  m1->glue_events(e1,e2);
  
  return check(myBmsc,true,true);
}

// $Id: fifo_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
