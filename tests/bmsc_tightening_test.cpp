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
 * Copyright (c)
 * 
 * $Id:
 */

#include <iostream>
#include "check/pseudocode/msc_duplicators.h"
#include "check/time/tightening.h"

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
  StrictOrderAreaPtr strict3(new StrictOrderArea());
  instance1->add_area(strict1);
  instance2->add_area(strict2);
  instance3->add_area(strict3);

  EventPtr e0 = strict1->add_event();
  EventPtr e40 = strict1->add_event();

  EventPtr e30 = strict2->add_event();
  EventPtr e20 = strict2->add_event();

  EventPtr e11 = strict3->add_event();
  EventPtr e10 = strict3->add_event();

  CompleteMessagePtr m1 = new CompleteMessage(L"hi1");
  m1->glue_events(e0, e11);
  CompleteMessagePtr m2 = new CompleteMessage(L"hi2");
  m2->glue_events(e10, e20);
  CompleteMessagePtr m3 = new CompleteMessage(L"hi3");
  m3->glue_events(e30, e40);

  MscTimeIntervalD in1(10,20);
  MscTimeIntervalD in2(0,0);
  MscTimeIntervalD in3(30,40);
  MscTimeIntervalD in4(60,D::infinity());//TODO: infi
  MscTimeIntervalD in5(20,30);   
  MscTimeIntervalD in6(40,50);

  MscTimeIntervalD in12(60,70);
  MscTimeIntervalD in13(20,70);



  MscTimeIntervalSetD ins1;
  MscTimeIntervalSetD ins2;
  MscTimeIntervalSetD ins3;
  MscTimeIntervalSetD ins4;
  MscTimeIntervalSetD ins5;
  MscTimeIntervalSetD ins6;
  MscTimeIntervalSetD ins7;
  MscTimeIntervalSetD ins8;

  ins1.insert(in1);
                
  ins2.insert(in2);

//  ins2.insert(in9); 
                      
  ins3.insert(in3);     
  ins3.insert(in4);     

                          
  ins4.insert(in5);        
  ins4.insert(in6); 

  ins5.insert(in12);

//  ins6.insert(in7);
   
  TimeRelationEventPtr rel1a = new TimeRelationEvent(ins1);
  rel1a->glue_events(e0.get(),e11.get());

  TimeRelationEventPtr rel6 = new TimeRelationEvent(ins2);
  rel6->glue_events(e11.get(),e10.get());

  TimeRelationEventPtr rel2 = new TimeRelationEvent(ins3);
  rel2->glue_events(e10.get(),e20.get());

  TimeRelationEventPtr rel7 = new TimeRelationEvent(ins1);
  rel7->glue_events(e30.get(),e20.get());

  TimeRelationEventPtr rel1b = new TimeRelationEvent(ins4);
  rel1b->glue_events(e30.get(),e40.get());

  TimeRelationEventPtr rel5 = new TimeRelationEvent(ins5);
  rel5->glue_events(e0.get(),e40.get());

  MscTimeIntervalSetD i;
  i.insert(MscTimeIntervalD(0,80));

  BMscDuplicator duplicator;
  BMscPtr bmsc_duplicated = duplicator.duplicate_bmsc(bmsc);


  BMscIntervalMatrixConverter conv2(bmsc_duplicated);
  std::cerr << conv2.get_matrix() << std::endl;
   MscSolveTCSP sol2;
   std::cerr << sol2.solve(conv2.get_matrix()) << std::endl;


  std::cerr << "##################################################" << std::endl;


  BMscIntervalMatrixConverter conv(bmsc);
  std::cerr << conv.get_matrix() << std::endl;
//  if(conv.is_leq(e3.get(),e4.get()))
//    std::cerr << "ANO" << std::endl;
//  else
//    std::cerr << "NE" << std::endl;
   MscSolveTCSP sol;
   std::cerr << sol.solve(conv.get_matrix()) << std::endl;
//   sol.solve(conv.get_matrix());
//   MaxTightener tight;
//   EventPList min;
//   EventPList max;
//   min.push_back(e0.get());
//   min.push_back(e3.get());
//   max.push_back(e2.get());
//   max.push_back(e4.get());

   std::pair<MscTimeIntervalSetD,IntervalSetMatrix> pair;
   TightenBMsc tighten(bmsc);
   std::cerr << "tighten passed " << std::endl;
   pair = tighten.tighten_msc(i);
   

 //  tight.max_tightener(min,max,sol.solve(conv.get_matrix()),i,bmsc,conv);
//   std::pair<MscTimeIntervalSetD,IntervalSetMatrix> max_tightener(EventPList min, EventPList max,IntervalSetMatrix t_matrix, \
                  MscTimeIntervalSetD i,BMsc bmsc)
//
  std::cerr << pair.first << std::endl << pair.second << std::endl;
  return 0;
}

// $Id: bmsc_tightening_test.cpp 405 2009-10-05 10:04:45Z kocianon $
