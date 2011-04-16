#include "check/time/constrain_check.h" 
//#include "data/dot_viewer.h" 

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

  EndNodePtr end1(new EndNode());

  hmsc1->add_node(r1_1);
  hmsc1->add_node(r1_2);
  hmsc1->add_node(end1);

  start1->add_successor(r1_1.get());
  r1_1->add_successor(r1_2.get());
  r1_1->add_successor(r1_1.get());
//  r1_2->add_successor(r1_2.get());
//  r1_2->add_successor(end1.get());
//  r1_3->add_successor(r1_2.get());
  r1_2->add_successor(end1.get());

  r1_1->set_msc(bmsc);
  r1_2->set_msc(bmsc);

  MscTimeIntervalSetD inter;
  TimeRelationRefNodePtr node_relation_1 = new TimeRelationRefNode(inter);
  node_relation_1->glue_ref_nodes(true,r1_1.get(),false,r1_2.get());
 
  HMscTimeConstrainChecker constrain_checker;
  ChannelMapperPtr chm;
  HMscPtr result = constrain_checker.check(hmsc1,chm);
  int ret;
  if(result.get())
  {
    std::cerr << "Chyba v omezenich" << std::endl;
    ret=1;
  }
  else
  {
    std::cerr << "OK  sdfs" << std::endl;
    ret = 0;
  }
  std::cerr << "################333" << std::endl;
  
  HMscPtr hmsc2(new HMsc(L"HMsc2"));

  StartNodePtr start2 = new StartNode();
  hmsc2->set_start(start2);

  ReferenceNodePtr r1(new ReferenceNode());

  EndNodePtr end2(new EndNode());

  hmsc2->add_node(r1);
  hmsc2->add_node(end2);

  start2->add_successor(r1.get());
  r1->add_successor(end2.get());
 
  TimeRelationRefNodePtr node_relation_2 = new TimeRelationRefNode(inter);
  node_relation_2->glue_ref_nodes(false,r1.get(),true,r1.get());

  HMscTimeConstrainChecker constrain_checker2;
  HMscPtr resul = constrain_checker2.check(hmsc2,chm);
  if(resul.get())
  {
    std::cerr << "3 Chyba v omezenich" << std::endl;
    ret=1;
  }
  else
  {
    std::cerr << "3 OK  sdfs" << std::endl;
    ret = 0;
  }
////////////////////////////////////////////////////////
  HMscPtr hmsc3(new HMsc(L"HMsc2"));
  StartNodePtr start3 = new StartNode();
  EndNodePtr end3(new EndNode());

  hmsc3->set_start(start3);
  hmsc3->add_node(end3);
  start3->add_successor(end3.get());

  HMscTimeConstrainChecker checker3;
  HMscPtr res3 = checker3.check(hmsc3,chm);

  if(res3.get())
  {
    std::cerr << "3 Chyba v omezenich" << std::endl;
    StartNodePtr c_start = res3->get_start();
    c_start->has_successors();
    NodeRelationPtrSet node_rel = c_start->get_successors();
    NodeRelationPtrSet::iterator it;
    for(it=node_rel.begin();it!=node_rel.end();it++)
      (*it)->get_successor();
  }
  else
  {
    std::cerr << "3 OK  sdfs" << std::endl;
    ret = 0;
    
  }
////////////////////////////////////////////////////////

  HMscPtr hmsc4(new HMsc(L"HMsc4"));
  StartNodePtr start4 = new StartNode();
  ConnectionNodePtr con4 = new ConnectionNode();

  EndNodePtr end4(new EndNode());

  ReferenceNodePtr r4_1(new ReferenceNode());
  ReferenceNodePtr r4_2(new ReferenceNode());

  hmsc4->set_start(start4);
  hmsc4->add_node(end4);
  hmsc4->add_node(r4_1);
  hmsc4->add_node(r4_2);
  hmsc4->add_node(con4);

  start4->add_successor(r4_1.get());
  r4_1->add_successor(con4.get());

  con4->add_successor(r4_1.get());
  con4->add_successor(r4_2.get());

  r4_2->add_successor(end4.get());
//  std::iostream stream;
//  XDotViewer viewer;
//  viewer.view(hmsc4);
//  viewer.get(hmsc4);
 
  TimeRelationRefNodePtr node_relation_4 = new TimeRelationRefNode(inter);
  node_relation_4->glue_ref_nodes(false,r4_1.get(),true,r4_2.get());

  HMscTimeConstrainChecker checker4;
  HMscPtr res4 = checker4.check(hmsc4,chm);
  

  if(res3.get())
  {
    std::cerr << "4 Chyba v omezenich" << std::endl;
    StartNodePtr c_start = res4->get_start();
    c_start->has_successors();
    NodeRelationPtrSet node_rel = c_start->get_successors();
    NodeRelationPtrSet::iterator it;
    for(it=node_rel.begin();it!=node_rel.end();it++)
      (*it)->get_successor();
  }
  else
  {
    std::cerr << "4 OK  sdfs" << std::endl;
    ret = 0;
    
  }
  return ret;
  
}
