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
 * Copyright (c) 2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: engmann_test.cpp 294 2009-09-09 13:36:30Z gotthardp $
 */

#include <iostream>
#include "data/engmann/engmann.h"
#include "data/Z120/z120.h"

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;
  }

  Engmann engmann;

  StreamReportPrinter printer(std::wcerr);
  engmann.set_printer(&printer);

  std::vector<MscPtr> msc = engmann.load_msc(argv[1]);
  if(msc.empty())
  {
    std::cerr << "ERROR: Syntax error in " << argv[1] << std::endl;
    return 1;
  }

  Z120 z120;
  z120.save_msc(std::cout, TOWSTRING(argv[1]), msc[0]);

  return 0;
}

// $Id: engmann_test.cpp 294 2009-09-09 13:36:30Z gotthardp $
