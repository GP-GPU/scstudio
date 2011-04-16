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
 * Copyright (c) 2009 Matus Madzin <gotti@mail.muni.cz>
 *
 */

#include <string.h>
#include <iostream>

#include "data/Z120/z120.h"
#include "membership/membership_alg.h"

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
  if(argc < 4)
  {
    std::cerr << "Usage: " << argv[0] << " <filename> <filename> <satisfied>" << std::endl;
    return 1;
  }

  std::vector<std::wstring> focused_instances;
  
  for(int i = 4; i < argc; i++)
  {
    std::string s = argv[i];
    std::wstring temp(s.length(),L' ');
    std::copy(s.begin(), s.end(), temp.begin());

    focused_instances.push_back(temp);
  }

  Z120 z120;

  StreamReportPrinter printer(std::wcerr);
  z120.set_printer(&printer);

  int errors = 0;

  char *endptr;
  int satisfied = strtol(argv[3], &endptr, 10);
  if(*argv[3] == '\0' || *endptr != '\0')
  {
    std::cerr << "ERROR: Not a boolean value: " << argv[3] << std::endl;
    return 1;
  }

  std::vector<MscPtr> msc; 
  std::vector<MscPtr> msc_b;
  
  try
  {
    msc = z120.load_msc(argv[1]);
  }
  catch(std::exception& exc)
  {
    std::cerr << "EXCEPTION: " << exc.what() << std::endl;
    return 2;
  }

  try
  {
    msc_b = z120.load_msc(argv[2]);
  }
  catch(std::exception& exc)
  {
    std::cerr << "EXCEPTION: " << exc.what() << std::endl;
    return 2;
  }

  MembershipAlg mem;
  mem.set_printer(&printer);

  MscPtr result;

  result = mem.diff(msc[0], msc_b);

  if(result == NULL)
  {
std::cerr << "result was null" << std::endl;
    if(satisfied)
    {
      std::cerr << "ERROR: HMSC should contain bMSC" << std::endl;
      errors = 1;
    }
    else
      std::cerr << "OK: HMSC doesn't contain bMSC" << std::endl;
  }
  else
  {
    if(satisfied)
      std::cerr << "OK: HMSC contains bMSC" << std::endl;
    else
    {
      std::cerr << "ERROR: HMSC should not contain bMSC" << std::endl;
      errors = 1;
    }

    std::cout << std::endl;

    try
    {
      z120.save_msc(std::cout, L"msc_diff", result);
    }
    catch(std::exception& exc)
    {
      std::cerr << "EXCEPTION: Cannot save the document: " << exc.what() << std::endl;
      errors = 1;
    }
  }
  
  return errors;
}
