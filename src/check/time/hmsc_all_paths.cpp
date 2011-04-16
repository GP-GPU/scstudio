#include "hmsc_all_paths.h"

void AllPaths::traverse() 
{
  MscElementPList path_prefix2;
  m_nodes_set = m_hmsc->get_nodes();

  HMscNodePtrSet::iterator it;
  for(it=m_nodes_set.begin();it!=m_nodes_set.end();it++)
    set_number(*it,0);

  // TODO check origin of relation
  all_paths(m_first,path_prefix2);  

}

void AllPaths::all_paths(
HMscNodePtr node, 
MscElementPList path_prefix2
)
{
  HMscNodePtrSet::iterator last_it;
  std::list<PathFoundListener*>::iterator listener_it;  
  
  path_prefix2.push_back(node.get());

  
  //check whether node is the one of last nodes
  
  for(last_it=m_last.begin();last_it!=m_last.end();last_it++)
  {
    if(node == (*last_it)) //is OK that I compare counted pointer?
    {
      // call every listener from m_path_found_listeners
      for(listener_it = m_path_found_listeners.begin();
	  listener_it != m_path_found_listeners.end();listener_it++){
	(**listener_it).on_path_found(path_prefix2);
      }
      path_prefix2.pop_back();
      //keep poping until hmsc node is reached
//      std::cout << "b\n";
      while(path_prefix2.size() != 0 && !((HMscNode*)(path_prefix2.back())))
	path_prefix2.pop_back();
//      std::cout << "c\n";
     return; //TODO check- I think it should be here
    }
  }

   set_number(node,get_number(node)+1); //increment occurence counter of node
  
   //check whether node has successors
   PredecessorNode * pre;
   pre = dynamic_cast<PredecessorNode*>(node.get());

   if(pre == NULL) //node has no successors -> update attributes and finish
   {
    path_prefix2.pop_back();
    //keep poping until reference node is reached
    while(!dynamic_cast<HMscNode*>(path_prefix2.back()))
      path_prefix2.pop_back();
    set_number(node, 0);
    return;
   }
   
   
   //traverse all successors which do not occur in path_prefix more than m_occurence
   NodeRelationPtrVector set_succ = pre->get_successors();
   NodeRelationPtrVector::const_iterator rel;
   for(rel=set_succ.begin(); rel!=set_succ.end();rel++)
   {
     const NodeRelationPtr& node_relation = *rel;
     HMscNodePtr new_node(dynamic_cast<HMscNode*>(node_relation.get()->get_successor()));
     if(get_number(new_node)<m_occurence)
     {

	path_prefix2.push_back(node_relation.get()); //update attribute
        all_paths(new_node,path_prefix2);
	 
      }

    }
    
    //all successors have been traversed-> update attributes and finish.
    path_prefix2.pop_back();
    //keep poping until reference node is reached
    while(!dynamic_cast<HMscNode*>(path_prefix2.back()))
      path_prefix2.pop_back();
    set_number(node, get_number(node)-1);
    return;
}
