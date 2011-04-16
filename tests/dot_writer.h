/* 
 * File:   dot_writer.h
 * Author: ondra
 *
 * Created on May 19, 2008, 8:52 PM
 */

#ifndef _DOT_WRITER_H
#define	_DOT_WRITER_H

#include <string>
#include <iostream>
#include <map>
#include <set>
#include "data/msc.h"
#include "data/dfs_hmsc_traverser.h"

class DotWriter;
typedef ReferenceNode* ReferenceNodeP;
typedef HMsc* HMscP;
class DotWriter : public WhiteNodeFoundListener
{
private:
  DFSHMscTraverser m_traverser;
  std::map<ReferenceNodeP,std::string> m_names_map;
  std::map<HMscP,std::string> m_start_map;
  std::set<std::string> m_ends_set;
  std::set<std::string> m_start_set;

public:

  DotWriter()
  {
    m_traverser.add_white_node_found_listener(this);
  }

  ~DotWriter()
  {
    //remove listener???
  }

  void head(std::string name)
  {
    std::cout << "digraph " << name << "{" << std::endl;
  }

  void end()
  {
    std::cout << "}" << std::endl;
  }

  void settings()
  {
    std::set<std::string>::iterator it;
    for (it = m_ends_set.begin(); it != m_ends_set.end(); it++)
    {
      std::cout << (*it) << " [shape =triangle];" << std::endl;
    }
    for (it = m_start_set.begin(); it != m_start_set.end(); it++)
    {
      std::cout << (*it) << " [shape =invtriangle];" << std::endl;
    }
  }

  void on_white_node_found(ReferenceNode* n)
  {
    ReferenceNodePSet succ = n->get_successors();
    ReferenceNodePSet::iterator it;
    for (it = succ.begin(); it != succ.end(); it++)
    {
      std::cout << this->return_name(n) << " ";
      std::cout << "->" << " ";
      std::cout << this->return_name(*it);
      std::cout << ";" << std::endl;
    }
    if (n->get_hmsc().get())
    {
      std::string start = this->return_start(n->get_hmsc());
      std::cout << this->return_name(n) << " ";
      std::cout << "->" << " ";
      std::cout << start;
      std::cout << ";" << std::endl;

      succ = n->get_hmsc()->get_start()->get_successors();
      for (it = succ.begin(); it != succ.end(); it++)
      {
        std::cout << start << " ";
        std::cout << "->" << " ";
        std::cout << this->return_name(*it);
        std::cout << ";" << std::endl;
      }
    }
    if (n->is_end_node())
    {
      std::cout << this->return_name(n) << " ";
      std::cout << "->" << " ";
      std::cout << this->return_end(n);
      std::cout << ";" << std::endl;
    }
  }

  void print(HMscPtr h, std::string name)
  {
    this->head(name);
    ReferenceNodePSet succ;
    ReferenceNodePSet::iterator it;
    succ = h->get_start()->get_successors();
    std::string start = this->return_start(h);
    for (it = succ.begin(); it != succ.end(); it++)
    {
      std::cout << start << " ";
      std::cout << "->" << " ";
      std::cout << this->return_name(*it);
      std::cout << ";" << std::endl;
    }
    m_traverser.traverse(h);
    this->settings();
    this->end();
    this->leon_cleaner();
  }

  std::string return_name(ReferenceNode* n)
  {
    std::string name;
    std::map<ReferenceNodeP, std::string>::iterator it;
    it = m_names_map.find(n);
    if (it != m_names_map.end())
    {
      name = it->second;
    }
    else
    {
      if (n->get_original())
        name = "o_" + ((ReferenceNode*) n->get_original())->get_label();
      else
        name += n->get_label();
      m_names_map.insert(std::pair<ReferenceNodeP, std::string > (n, name));
    }
    return name;
  }

  std::string return_start(HMscPtr h)
  {
    std::string name;
    std::map<HMscP, std::string>::iterator it;
    it = m_start_map.find(h.get());

    if (it != m_start_map.end())
    {
      name = it->second;
    }
    else
    {
      name += "start_";
      if (h->get_original())
        name += "o_" + ((HMsc*) h->get_original())->get_label();
      else
        name += h->get_label();
      m_start_map.insert(std::pair<HMscP, std::string > (h.get(), name));
      m_start_set.insert(name);
    }
    return name;
  }

  std::string return_end(ReferenceNodeP n)
  {
    std::string name;
    name += "end_";
    if (n->get_original())
      name += "o_" + ((ReferenceNode*) n->get_original())->get_label();
    else
      name += n->get_label();
    m_ends_set.insert(name);
    return name;
  }

   void leon_cleaner()
   {
     m_ends_set.clear();
     m_start_set.clear();
     m_start_map.clear();
     m_names_map.clear();
   }
};


#endif	/* _DOT_WRITER_H */
