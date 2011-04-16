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
 * $Id: z120_test.cpp 419 2009-10-12 12:16:37Z gotthardp $
 */

#include <string.h>
#include <iostream>

#include "data/Z120/z120.h"

char* extract_filename(char* filename)
{
  char *last_name = filename;
  char *last_dot = NULL;

  for(char *pos = filename; *pos != '\0'; pos++)
  {
    if(*pos == '.')
      last_dot = pos;
    else if(*pos == '\\' || *pos == '/')
      last_name = pos+1;
  }

  if(last_dot != NULL)
    *last_dot = '\0'; // strip the extension

  return last_name;
}

int main(int argc, char** argv)
{
  if(argc < 3)
  {
    std::cerr << "Usage: " << argv[0] << " <filename> <satisfied>" << std::endl;
    return 1;
  }

  Z120 z120;

  StreamReportPrinter printer(std::wcerr);
  z120.set_printer(&printer);

  int errors = 0;

  char *endptr;
  int satisfied = strtol(argv[2], &endptr, 10);
  if(*argv[2] == '\0' || *endptr != '\0')
  {
    std::cerr << "ERROR: Not a boolean value: " << argv[2] << std::endl;
    return 1;
  }

  std::vector<MscPtr> msc = z120.load_msc(argv[1]);

  char *path = strdup(argv[1]);
  char *filename = extract_filename(path);

  if(!msc.empty())
  {
    if(satisfied)
      std::cout << "OK: " << filename << " is correct, should be correct" << std::endl;
    else
    {
      std::cerr << "ERROR: " << filename << " is correct, should NOT be correct" << std::endl;
      errors = 1;
    }

    std::cout << std::endl;

    try
    {
      z120.save_msc(std::cout, TOWSTRING(filename), msc[0], msc);
    }
    catch(std::exception& exc)
    {
      std::cerr << "EXCEPTION: Cannot save the document: " << exc.what() << std::endl;
      errors = 1;
    }
  }
  else
  {
    if(satisfied)
    {
      std::cerr << "ERROR: Syntax error in " << filename << ", should NOT be erroneous" << std::endl;
      errors = 1;
    }
    else
      std::cout << "OK: Syntax error in " << filename << ", should be erroneous" << std::endl;
  }

  free(path);
  return errors;
}

// $Id: z120_test.cpp 419 2009-10-12 12:16:37Z gotthardp $
