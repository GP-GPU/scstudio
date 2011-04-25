#include "check/pycheck/livelock_checker_visio.h"
#include "check/pycheck/fifo_checker_visio.h"
#include "check/pycheck/universal_checker_visio.h"
#include "data/msc.h"
#include "data/Z120/z120.h"
#include <vector>
#include <iostream>

int main(){
  std::vector<MscPtr> msc;
/*  HMscPtr h1(new HMsc(L"h1"));
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
*/
  HMscPtr h1(new HMsc(L"h1"));
  HMscPtr h2(new HMsc(L"h2"));
  
  StartNodePtr start1 = new StartNode(); h1->set_start(start1);
  EndNodePtr end1(new EndNode);h1->add_node(end1);
  ReferenceNodePtr p1(new ReferenceNode());h1->add_node(p1);
  ReferenceNodePtr p2(new ReferenceNode());h1->add_node(p2);
  ReferenceNodePtr p3(new ReferenceNode());h1->add_node(p3);
  ReferenceNodePtr p4(new ReferenceNode());h1->add_node(p4);
  
  StartNodePtr start2 = new StartNode(); h2->set_start(start2);
  EndNodePtr end2(new EndNode);h2->add_node(end2);
  ReferenceNodePtr n1(new ReferenceNode());h2->add_node(n1);
  ReferenceNodePtr n2(new ReferenceNode());h2->add_node(n2);
  ReferenceNodePtr n3(new ReferenceNode());h2->add_node(n3);
  ReferenceNodePtr n4(new ReferenceNode());h2->add_node(n4);
  
  start1->add_successor(p1.get());
  p1->add_successor(p2.get());
  p1->add_successor(p3.get());
  p3->add_successor(p4.get());
  p2->add_successor(end1.get());
  p4->add_successor(end1.get());
  p3->set_msc(h2);

  start2->add_successor(n1.get());
  start2->add_successor(n4.get());
  n1->add_successor(n2.get());
  n2->add_successor(n1.get());
  n2->add_successor(n3.get());
  n3->add_successor(n1.get());
  n4->add_successor(n3.get());
  n3->add_successor(end2.get());
  n4->add_successor(end2.get());

//  msc.push_back(h1);

  BMscPtr myBmsc1(new BMsc(L"b1"));
  
  InstancePtr i1(new Instance(L"p1"));
  myBmsc1->add_instance(i1);
  InstancePtr i2(new Instance(L"p2"));
  myBmsc1->add_instance(i2);
  
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

  

  BMscPtr myBmsc2(new BMsc(L"b2"));
  
  InstancePtr i21(new Instance(L"p21"));
  myBmsc2->add_instance(i21);
  InstancePtr i22(new Instance(L"p22"));
  myBmsc2->add_instance(i22);
  
  EventAreaPtr ca1(new CoregionArea());
  i21->add_area(ca1);
  EventAreaPtr ca2(new CoregionArea());
  i22->add_area(ca2);
  
  EventPtr e21 = ca1->add_event();
  EventPtr e22 = ca1->add_event();
  EventPtr e23 = ca2->add_event();
  EventPtr e24 = ca2->add_event();
  
  CompleteMessagePtr m21 = new CompleteMessage(L"a");
  m21->glue_events(e21, e23);
  CompleteMessagePtr m22 = new CompleteMessage(L"a");
  m22->glue_events(e22, e24);

  BMscPtr myBmsc3(new BMsc(L"b3"));
  
  InstancePtr i31(new Instance(L"p31"));
  myBmsc3->add_instance(i31);
  InstancePtr i32(new Instance(L"p32"));
  myBmsc3->add_instance(i32);
  
  EventAreaPtr a31(new CoregionArea());
  i31->add_area(a31);
  EventAreaPtr a32(new CoregionArea());
  i32->add_area(a32);
  
  EventPtr e31 = a31->add_event();
  EventPtr e32 = a31->add_event();
  EventPtr e33 = a32->add_event();
  EventPtr e34 = a32->add_event();

  ((CoregionEvent*)e34.get())->add_successor((CoregionEvent*)e33.get());
  
  CompleteMessagePtr m31 = new CompleteMessage(L"a");
  m31->glue_events(e31, e33);
  CompleteMessagePtr m32 = new CompleteMessage(L"b");
  m32->glue_events(e32, e34);

  SRChannelMapperPtr chm(new SRChannelMapper);

  PyBFIFOChecker *pfifo;
  try{
    pfifo = new PyBFIFOChecker();
  }
  catch(int e){
    return e;
  }

  Z120 z;
  std::vector<MscPtr> mscs = z.load_msc("tests/acyclic/acyclic1.mpr");
  std::list<BMscPtr> bmscs = pfifo->check(boost::dynamic_pointer_cast<BMsc>(mscs[0]), chm);
  std::cout << bmscs.size() << std::endl;
  if(bmscs.back() == myBmsc1)
    std::cout << "It really works\n";
  bmscs = pfifo->check(myBmsc2, chm);
  std::cout << bmscs.size() << std::endl;
  bmscs = pfifo->check(myBmsc3, chm);
  std::cout << bmscs.size() << std::endl;

  PyHUniversalChecker *plive;
  try{
    plive = new PyHUniversalChecker();
  }
  catch(int e){
    return e;
  }

  std::list<HMscPtr> hmscs = plive->check(h1, chm);
  std::cout << hmscs.size() << std::endl;
  hmscs = plive->check(h2, chm);
  std::cout << hmscs.size() << std::endl;
//  exp.save_msc(std::cout, L"mymsc", h1, msc);
  delete plive;
  delete pfifo;
  return 0;
}
