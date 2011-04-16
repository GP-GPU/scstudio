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
 * $Id: acyclic_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
 */

#include "data/msc.h"
#include "check/order/acyclic_checker.h"

#include <iostream>

#define RETURN_IF_FAILED(res) if(!(res)) return 1;

bool print_result(const BMscPtr bmsc,const bool is_fifo,const std::string& mapper_name)
{
  bool result;
  if(is_fifo)
    if(bmsc.get())
    {
      result =false;
      std::cout << "ERROR: BMsc is supposed to be acyclic for " << mapper_name
                << " channels but it isn't" << std::endl;
    }
    else
    {
      result = true;
      std::cout << "OK: BMsc is supposed to be acyclic for " << mapper_name
                << " channels and it is" << std::endl;
    }
  else
    if(bmsc.get())
    {
      result = true;
      std::cout << "OK: BMsc is supposed not to be acyclic for " << mapper_name
                << " channels and it isnt't" << std::endl;
    }
    else
    {
      result =false;
      std::cout << "ERROR: BMsc is supposed not to be acyclic for " << mapper_name
                << " channels but it is" << std::endl;
    }
  std::cout << std::endl;
  return result;
}

bool check(BMscPtr bmsc,const bool is_sr_fifo,const bool is_srl_fifo)
{
  AcyclicCheckerPtr checker = AcyclicChecker::instance();
  SRChannelMapperPtr srm = SRChannelMapper::instance();
  SRMChannelMapperPtr srlm = SRMChannelMapper::instance();
	std::list<BMscPtr> result;
	BMscPtr presult;
	result = checker->check(bmsc,srm);
	if(result.empty())
		presult = NULL;
	else
		presult = result.back();
  if(!print_result(presult,is_sr_fifo,"sender-receiver"))
		return false;

	result = checker->check(bmsc,srlm);
	if(result.empty())
		presult = NULL;
	else
		presult = result.back();
  if(!print_result(presult,is_srl_fifo,"sender-receiver-label"))
		return false;
	return true;
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
  std::cout << "  p1           p2   " << std::endl;
  std::cout << "   |            |   " << std::endl;
  std::cout << "e1 O<--a---     |   " << std::endl;
  std::cout << "   |       \\   |   " << std::endl;
  std::cout << "e2 O-----a----->O e3" << std::endl;
  std::cout << "   |         \\ |   " << std::endl;
  std::cout << "   |           -O e4" << std::endl;
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
  m1->glue_events(e4, e1);
  CompleteMessagePtr m2 = new CompleteMessage(L"a");
  m2->glue_events(e2, e3);
  
  return check(myBmsc,false,false);
}

int main(int argc, char** argv) {
  RETURN_IF_FAILED(BMscA());
  RETURN_IF_FAILED(BMscB());
  return 0;
}

// $Id: acyclic_checker_test.cpp 585 2010-02-11 09:42:30Z vacek $
