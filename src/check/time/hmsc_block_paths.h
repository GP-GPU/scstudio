#ifndef _HMSC_BLOCKPATHS_H_
#define _HMSC_BLOCKPATHS_H_

#include "check/time/hmsc_all_paths.h"
#include "check/time/find_block.h"

class SCTIME_EXPORT BlockPathFoundListener:public PathFoundListener
{
public:
  BlockPathFoundListener()
  {}

  virtual ~BlockPathFoundListener()
  {}

  virtual void on_path_found(MscElementPList& path)=0;
};

class AllPathsAllBlocks
{
private:
  HMscPtr m_hmsc;
  HMscNodePtr m_first;
  HMscNodePtrSet m_last;
  PathFoundListener* m_listener;
//  int m_occurence;
  std::list<Block> m_list_of_blocks;


public:
  AllPathsAllBlocks(std::list<Block> list_of_blocks, HMscPtr hmsc):m_hmsc(hmsc),m_list_of_blocks(list_of_blocks)//(Block* block, HMscPtr hmsc)
  {
  }

  void all_paths_all_blocks();

  void set_listener(PathFoundListener* l)
  {
    m_listener = l;
  }

};

#endif // _HMSC_BLOCKPATHS_H_
