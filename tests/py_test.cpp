#include "check/pycheck/universal_checker_visio.h"
#include "data/msc.h"
#include "data/Z120/z120.h"
#include <vector>
#include <iostream>

#define DFILE "tests/deadlock/nodeadlock4.mpr"

int main(int argc, char *argv[]){
  std::string sinp = std::string(DFILE);
  if(argc == 2)
    sinp = std::string(argv[1]);
  std::cout << "Importing file: " << sinp << std::endl;
  
  SRChannelMapperPtr chm(new SRChannelMapper);

  PyBUniversalChecker *pbuni;
  try{
    pbuni = new PyBUniversalChecker();
  }
  catch(int e){
    return e;
  }

  PyHUniversalChecker *phuni;
  try{
    phuni = new PyHUniversalChecker();
  }
  catch(int e){
    return e;
  }
  std::list<BMscPtr> bmscs;
  std::list<HMscPtr> hmscs;
  Z120 z;
  std::vector<MscPtr> mscs = z.load_msc(sinp);

  std::cout << "Going to run checkers " << mscs.size() << std::endl;
  if(!mscs.size()){
    std::cout << "Nothing to test" << std::endl;
    return 3;
  }
  if(boost::dynamic_pointer_cast<BMsc>(mscs[0]))
    bmscs = pbuni->check(boost::dynamic_pointer_cast<BMsc>(mscs[0]), chm);
  if(boost::dynamic_pointer_cast<HMsc>(mscs[0]))
    hmscs = phuni->check(boost::dynamic_pointer_cast<HMsc>(mscs[0]), chm);
  std::cout << bmscs.size() << std::endl;
  std::cout << hmscs.size() << std::endl;
  delete pbuni;
  delete phuni;
  return 0;
}
