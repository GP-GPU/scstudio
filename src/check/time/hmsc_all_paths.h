#ifndef _AllPaths_
#define _AllPaths_

#include "time_pseudocode.h"
#include "data/session_attribute.h"

//typedef std::pair<std::list<HMscNodePtr>,bool> HMscPath;
//typedef std::pair<std::list<HMscPath>,bool> HMscAllPaths;

class AllPaths;
class PathFoundListener;


class AllPaths
{
private:
  HMscNodePtrSet m_nodes_set;
  HMscPtr m_hmsc;
  HMscNodePtr m_first;
  HMscNodePtrSet m_last;
  int m_occurence; // how many times node can occur in path
  std::list<PathFoundListener*> m_path_found_listeners;
  SessionAttribute<int> m_attr_number;


  //preconditions: every path from node first to end node should go through some node from last
public:
  AllPaths(HMscPtr hmsc, //TimeRelationRefNodePtr relation,
	   HMscNodePtr first, HMscNodePtrSet last, int occurence,
    const std::string& number="AllPathAlg")
    :m_hmsc(hmsc),m_first(first),m_last(last),m_occurence(occurence),m_attr_number("attr_number",-1)
  {

  }

  ~AllPaths()
  {
    m_attr_number.clean_up();
  }

  void set_number(HMscNodePtr e,int number)
  {
    return m_attr_number.set(e.get(), number);
  }

  int get_number(HMscNodePtr e)
  {
    return m_attr_number.get(e.get());
  }


  /**
   * Traverses given BMscGraph and calls on_path_found method of every added PathFoundListener on all paths, such:
   * - the path starts from node first
   * - the path contains one of the nodes from lastand the node is last node of the path
   * - the path does not contain more than occurence times of same node.
   */
  static void traverse(
  HMscPtr hmsc,
  HMscNodePtr first,
  HMscNodePtrSet last,
  int occurence
  )
  {
    AllPaths allpaths(hmsc,first,last,occurence);
    allpaths.traverse();
  }

  void traverse();

  void all_paths(HMscNodePtr, MscElementPList);

  /**
   * Adds PathFoundListener
   */
  void add_path_found_listener(PathFoundListener* l)
  {
    m_path_found_listeners.push_back(l);
  }
};


/**
 * Listener of found path
 */
class SCTIME_EXPORT PathFoundListener
{
  public:

  virtual ~PathFoundListener()
  {

  }

  virtual void on_path_found(MscElementPList& path)=0;

};

#endif

