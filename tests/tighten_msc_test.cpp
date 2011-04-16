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
 * $Id: tighten_msc_test.cpp 686 2010-03-11 15:08:00Z kocianon $
 */

#include <iostream>
#include "check/time/tightening.h"
#include "data/Z120/z120.h"
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
  StrictOrderAreaPtr strict3(new StrictOrderArea());
  instance1->add_area(strict1);
  instance2->add_area(strict2);
  instance3->add_area(strict3);
  EventPtr e0 = strict1->add_event();
  EventPtr e1 = strict3->add_event();
  EventPtr e2 = strict2->add_event();
  EventPtr e3 = strict2->add_event();
  EventPtr e4 = strict1->add_event();
  EventPtr e5 = strict3->add_event();

  CompleteMessagePtr m1 = new CompleteMessage(L"hi1");
  m1->glue_events(e0, e1);
  CompleteMessagePtr m2 = new CompleteMessage(L"hi2");
  m2->glue_events(e5, e2);
  CompleteMessagePtr m3 = new CompleteMessage(L"hi3");
  m3->glue_events(e3, e4);

  MscTimeIntervalD in6(10,20);
  MscTimeIntervalD in7(0,0);
  MscTimeIntervalD in8(30,40);
  MscTimeIntervalD in9(60,D::infinity());//TODO: infi
  MscTimeIntervalD in10(20,30);
  MscTimeIntervalD in11(40,50);
  MscTimeIntervalD in12(60,120);
  MscTimeIntervalD in13(20,70);



  MscTimeIntervalSetD ins1;
  MscTimeIntervalSetD ins2;
  MscTimeIntervalSetD ins3;
  MscTimeIntervalSetD ins4;
  MscTimeIntervalSetD ins5;
  MscTimeIntervalSetD ins6;
  MscTimeIntervalSetD ins7;
  MscTimeIntervalSetD ins8;

  ins1.insert(in6);

  ins2.insert(in8);
  ins2.insert(in9);

  ins3.insert(in6);

  ins4.insert(in10);
  ins4.insert(in11);

  ins5.insert(in12);

  ins6.insert(in7);

  TimeRelationEventPtr rel1a = new TimeRelationEvent(ins1);
  rel1a->glue_events(e0.get(),e1.get());
  TimeRelationEventPtr rel6 = new TimeRelationEvent(ins6);
  rel6->glue_events(e1.get(),e5.get());
  TimeRelationEventPtr rel2 = new TimeRelationEvent(ins2);
  rel2->glue_events(e5.get(),e2.get());
  TimeRelationEventPtr rel1b = new TimeRelationEvent(ins1);
  rel1b->glue_events(e3.get(),e2.get());
  TimeRelationEventPtr rel4 = new TimeRelationEvent(ins4);
  rel4->glue_events(e3.get(),e4.get());
  TimeRelationEventPtr rel5 = new TimeRelationEvent(ins5);
  rel5->glue_events(e0.get(),e4.get());

  MscTimeIntervalSetD i;
  i.insert(MscTimeIntervalD(0,80));
/*
  BMscIntervalMatrixConverter conv(bmsc);
  std::cerr << conv.get_matrix() << std::endl;
  if(conv.is_leq(e3.get(),e4.get()))
    std::cerr << "ANO" << std::endl;
  else
    std::cerr << "NE" << std::endl;*/
//   MscSolveTCSP sol;
//   std::cerr << sol.solve(conv.get_matrix()) << std::endl;
//   sol.solve(conv.get_matrix());
 //  std::cerr << "tighten passed " << std::endl;
 //  MaxTightener tight(bmsc,&conv);
//    std::cerr << "tighten passed " << std::endl;
//   EventPList min;
//   EventPList max;
//   min.push_back(e0.get());
//   min.push_back(e3.get());
//   max.push_back(e2.get());
//   max.push_back(e4.get());
/*
   ConstMatrixPair pair;
   TightenBMsc tighten(bmsc);
   std::cerr << "tighten passed " << std::endl;
   pair = tighten.tighten_msc(i);*/
  /*
  BMscTighter tightener;
  BMscPtr result = tightener.transform(bmsc);
  */
  Z120 z120;
  // z120.save_msc(std::cout,L"original",bmsc);
  // z120.save_msc(std::cout,L"result",result);
/*
  std::vector<MscPtr> mscs = z120.load_msc("cons_neg_1.mpr");
  std::cout << mscs.size() << std::endl;
  BMscPtr b = boost::dynamic_pointer_cast<BMsc>(mscs[0]);
  z120.save_msc(std::cout,L"result",b);
  ChannelMapperPtr m;
  ConsistencyChecker checker;
  std::list<BMscPtr> re = checker.check(b,m);
  std::cout << re.size() << std::endl;
  
  if(re.size() > 0){
    std::cout << "vysledek" << std::endl;
   z120.save_msc(std::cout,L"result",re.back());
 }
 else
  std::cout << "neni" << std::endl;
*/
 //  tight.max_tightener(min,max,sol.solve(conv.get_matrix()),i,bmsc,conv);
 // std::pair<MscTimeIntervalSetD,IntervalSetMatrix> max_tightener(EventPList min, EventPList max,IntervalSetMatrix t_matrix,MscTimeIntervalSetD i,BMsc bmsc)
//
  //std::cerr << pair.first << std::endl << pair.second << std::endl;
  // z120.save_msc(std::cout,L"original",bmsc);
  // z120.save_msc(std::cout,L"result",result);
  std::vector<MscPtr> mscs = z120.load_msc("cons_neg_1.mpr");
  BMscPtr b = boost::dynamic_pointer_cast<BMsc>(mscs[0]);
  ConsistencyChecker con;
  ChannelMapperPtr mapper;
  con.check(b,mapper);
  BMscPtr n  = BMscDuplicator::duplicate(b);
  z120.save_msc(std::cout,L"original",b);
  z120.save_msc(std::cout,L"result",n);

  return 0;
}

// $Id: tighten_msc_test.cpp 686 2010-03-11 15:08:00Z kocianon $
