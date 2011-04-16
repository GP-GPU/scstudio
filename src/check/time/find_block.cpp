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
 * Copyright (c) 2010 Martin Vodila <fprt77@mail.muni.cz>
 *
 * $Id: 
 */

#include "check/time/find_block.h"

void MyTraverse::on_white_node_found(HMscNode* n)
{
  n->set_attribute<int>("found_with_block_number",trav->m_last_block);
  n->set_attribute<TimeRelationRefNodePtrSet>("found_with_open_constraints",trav->m_open_constraints);
  update_constraints(n);
}

void MyTraverse::on_node_finished(HMscNode* n)
{
  for (std::list<Block>::iterator it = trav->m_list_of_blocks.begin(); it != trav->m_list_of_blocks.end(); it++)  //iterate blocks
  {
    if (((*it).get_number()==trav->m_last_block) && ((*it).get_if_end()==0)) //find the right one, and check if end node has been set
    {
      (*it).set_end(dynamic_cast<ReferenceNode*>(n)); //if it hasn't, then set it
      (*it).set_if_end(1);  //mark block, now we know that end node of block has been set and won't be changed
      continue;
    }
  }
  trav->m_last_block=n->get_attribute<int>("found_with_block_number",0);
  trav->m_open_constraints=n->get_attribute<TimeRelationRefNodePtrSet>("found_with_open_constraints",trav->m_open_constraints);
  n->remove_attribute<int>("found_with_block_number");
  n->remove_attribute<TimeRelationRefNodePtrSet>("found_with_open_constraints");
}

void MyTraverse::update_constraints(HMscNode* v)
{
  ReferenceNode* n= dynamic_cast<ReferenceNode*>(v);
  TimeRelationRefNodePtrSet node_constraints;
  TimeRelationRefNodePtrSet toerase;
  TimeRelationRefNodePtrSet toadd;
  TimeRelationRefNodePtrSet toadd2;
 // std::set<std::pair<TimeRelationRefNodePtr,int>> forerase;
  node_constraints.erase(node_constraints.begin(), node_constraints.end());

  if (n!=NULL)  //test if node v is also ReferenceNode
  {
    node_constraints.insert(n->get_time_relations_top().begin(), n->get_time_relations_top().end());  //fill with contraints in node v
    node_constraints.insert(n->get_time_relations_bottom().begin(), n->get_time_relations_bottom().end());  //fill with contraints in node v
    if (trav->m_open_constraints.empty()) 
    {
      trav->m_counter_of_blocks++;
      if (trav->m_counter_of_blocks!=0)
      {
        for (std::list<Block>::iterator it = trav->m_list_of_blocks.begin(); it != trav->m_list_of_blocks.end(); it++)  //iterate blocks
        {
          if (((*it).get_number()==trav->m_last_block)&&((*it).get_if_end()==0))  //find the one, we are looking for
          {
            (*it).set_if_end(1);  //find new block, so we must close previous block, to prevent any changes
          }
        }
      }
      Block block=  Block(trav->m_counter_of_blocks, n, n, 0, m_traverser->get_reached_elements());
      trav->m_list_of_blocks.push_back(block);
      trav->m_last_block=trav->m_counter_of_blocks;
      trav->m_open_constraints.insert(node_constraints.begin(), node_constraints.end());
    }
    else 
    {
      for (std::list<Block>::iterator it = trav->m_list_of_blocks.begin(); it != trav->m_list_of_blocks.end(); it++)  //iterate blocks
      {
        if (((*it).get_if_end()==0)&&((*it).get_number()==trav->m_last_block)) 	//find the right one, and check if end node has been set
        {
          (*it).set_end(n);		//if it hasn't, then set it as temporal
          continue;
        }
      }
      for (std::set<TimeRelationRefNodePtr>::iterator it = trav->m_open_constraints.begin(); it != trav->m_open_constraints.end(); it++) //itarate open constraints
      {
        for (std::set<TimeRelationRefNodePtr>::iterator it_2 = node_constraints.begin(); it_2 != node_constraints.end(); it_2++)  //iterate node constraints
        {
          if (*it==*it_2)
          {
              toerase.insert(*it_2);
          }
        }
      }

      for (std::set<TimeRelationRefNodePtr>::iterator it = trav->m_open_constraints.begin(); it != trav->m_open_constraints.end(); it++) //itarate open constraints
      {
        int x=0;
        for (std::set<TimeRelationRefNodePtr>::iterator it_2 = toerase.begin(); it_2 != toerase.end(); it_2++)  //iterate node constraints
        {
          if (*it==*it_2) x=1; 
        }
        if (x==0) toadd.insert(*it);
      }
      for (std::set<TimeRelationRefNodePtr>::iterator it = node_constraints.begin(); it != node_constraints.end(); it++) //itarate open constraints
      {
        int x=0;
        for (std::set<TimeRelationRefNodePtr>::iterator it_2 = toerase.begin(); it_2 != toerase.end(); it_2++)  //iterate node constraints
        {
          if (*it==*it_2) x=1; 
        }
        if (x==0) toadd2.insert(*it);
      }

      trav->m_open_constraints.clear();
      trav->m_open_constraints.insert(toadd.begin(),toadd.end());
      trav->m_open_constraints.insert(toadd2.begin(),toadd2.end());
      toadd.clear();
      toadd2.clear();
      node_constraints.clear();
    }
  }
  else if (dynamic_cast<EndNode*>(v)!=NULL)
       {
         for (std::list<Block>::iterator it = trav->m_list_of_blocks.begin(); it != trav->m_list_of_blocks.end(); it++)  //iterate blocks
         {
           if (((*it).get_if_end()==0)&&((*it).get_number()==trav->m_last_block)) 	//find the right one, and check if end node has been set
           {
             (*it).set_if_end(1);		//close changing end_node of last_block
             continue;
           }
         }
       }
         else if (!(dynamic_cast<ConnectionNode*>(v)
                 || dynamic_cast<StartNode*>(v)
                 || dynamic_cast<ConditionNode*>(v)))
              {
                throw std::runtime_error("Unknown type");
              }
}



