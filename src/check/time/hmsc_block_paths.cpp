#include "check/time/hmsc_block_paths.h"


/*
void BlockPathFoundListener::on_path_found(MscElementPList& path)
{
  std::cout << "mam cestu" << std::endl;
  //from hmsc path to bmsc ??
  //then tighten ??
}
*/


void AllPathsAllBlocks::all_paths_all_blocks()
{
//  m_hmsc= AllPathsAllBlocks::m_hmsc;
//  m_list_of_blocks= AllPathsAllBlocks::m_list_of_blocks;
  int i=0;
  for (std::list<Block>::iterator it = m_list_of_blocks.begin(); it != m_list_of_blocks.end(); it++)  //iterate blocks
    {
      i++;
//      std::cout << "Cesty v " << i << " bloku" << std::endl;
      m_last.insert(it->get_end());
      AllPaths allpaths = AllPaths(m_hmsc, it->get_begin(), m_last, 1);

      allpaths.add_path_found_listener(m_listener);
      allpaths.traverse();
      m_last.clear();
      //don't know exatly what is result of allpaths.traverse()  -- maybe list of path found listener??
    }
}
