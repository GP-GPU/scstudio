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
 * $Id: checker_test.cpp 635 2010-02-28 18:07:48Z gotthardp $
 */

#include <iostream>
#include <stdexcept>
#include "data/checker.h"
#include "data/Z120/z120.h"

#ifdef WIN32
#define LIBRARY_SYMBOL GetProcAddress
#define LIBRARY_CLOSE FreeLibrary
#else
#include <dlfcn.h>
#define LIBRARY_SYMBOL dlsym
#define LIBRARY_CLOSE dlclose
#endif

MscPtr run_check(const FInitCheckers& init_checkers,
  const std::string& property, MscPtr msc)
{
  boost::shared_ptr<Checker> checker;

  Checker **checkers = init_checkers();
  for(Checker **fpos = checkers; *fpos != NULL; fpos++)
  {
    if((*fpos)->get_property_name() == TOWSTRING(property))
    {
      checker = boost::shared_ptr<Checker>(*fpos);
      // note: no break here as we have to delete the remaining pointers
    }
    else
      delete *fpos;
  }
  delete[] checkers;

  if(checker == NULL)
  {
    std::cerr << "ERROR: Checker for property " << property << " not found" << std::endl;
    throw std::runtime_error("invalid checker");
  }

  SRChannelMapperPtr srm(new SRChannelMapper());

  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);
  BMscCheckerPtr bmsc_checker = boost::dynamic_pointer_cast<BMscChecker>(checker);
  if(bmsc_checker != NULL && bmsc != NULL
    && checker->is_supported(srm))
  {
    std::list<BMscPtr> result = bmsc_checker->check(bmsc, srm);
    if(result.empty())
      return NULL;
    else
      return result.back();
  }

  HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);
  HMscCheckerPtr hmsc_checker = boost::dynamic_pointer_cast<HMscChecker>(checker);
  if(hmsc_checker != NULL && hmsc != NULL
    && checker->is_supported(srm))
  {
    std::list<HMscPtr> result = hmsc_checker->check(hmsc, srm);
    if(result.empty())
      return NULL;
    else
      return result.back();
  }

  std::cerr << "ERROR: No relevant checker for " << property << std::endl;
  throw std::runtime_error("invalid checker");
}

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
  if(argc < 5)
  {
    std::cerr << "Usage: " << argv[0] << " <library> <property> <msc> <satisfied>" << std::endl;
    return 1;
  }

#ifdef WIN32
  HINSTANCE module = LoadLibrary(argv[1]);
#else
  void *module = dlopen(argv[1], RTLD_LAZY);
#endif
  if(module == NULL)
  {
    std::cerr << "ERROR: Cannot open " << argv[1] << std::endl;
    return 1;
  }

  // note: this is a nasty hack to avoid the gcc warning:
  // ISO C++ forbids casting between pointer-to-function and pointer-to-object
  union
  {
    void *pointer;
    FInitCheckers function;
  } init_checkers;

  init_checkers.pointer = LIBRARY_SYMBOL(module, "init_checkers");
  if(init_checkers.pointer == NULL)
  {
    std::cerr << "ERROR: Initializer not found in " << argv[1] << std::endl;
    LIBRARY_CLOSE(module);
    return 1;
  }

  Z120 z120;

  StreamReportPrinter printer(std::wcerr);
  z120.set_printer(&printer);

  std::vector<MscPtr> msc;
  try
  {
    msc = z120.load_msc(argv[3]);
  }
  catch(std::exception& exc)
  {
    std::cerr << "EXCEPTION: " << exc.what() << std::endl;
  }

  if(msc.empty() || msc[0] == NULL)
  {
    std::cerr << "ERROR: Cannot open " << argv[3] << std::endl;
    LIBRARY_CLOSE(module);
    return 1;
  }

  int errors = 0;

  char *endptr;
  int satisfied = strtol(argv[4], &endptr, 10);
  if(*argv[4] == '\0' || *endptr != '\0')
  {
    std::cerr << "ERROR: Not a boolean value: " << argv[4] << std::endl;
    LIBRARY_CLOSE(module);
    return 1;
  }

  char *path = strdup(argv[3]);
  char *filename = extract_filename(path);

  try
  {
    MscPtr result = run_check(init_checkers.function, argv[2], msc[0]);
    if(result == NULL)
    {
      if(satisfied)
        std::cout << "OK: " << filename << " satisfied " << argv[2] << ", should be satisfied" << std::endl;
      else
      {
        std::cerr << "ERROR: " << filename << " satisfied " << argv[2] << ", should NOT be satisfied" << std::endl;
        errors = 1;
      }
    }
    else
    {
      std::cout << argv[2] << " violated" << std::endl;

      if(satisfied)
      {
        std::cerr << "ERROR: " << filename << " violated " << argv[2] << ", should NOT be violated" << std::endl;
        errors = 1;
      }
      else
        std::cout << "OK: " << filename << " violated " << argv[2] << ", should be violated" << std::endl;

      try
      {
        z120.save_msc(std::cout, L"counter_example", result);
      }
      catch(std::exception& exc)
      {
        std::cerr << "EXCEPTION: Cannot save the document: " << exc.what() << std::endl;
        errors = 1;
      }
    }
  }
  catch(std::exception &exc)
  {
    std::cerr << "ERROR: " << filename << " caused an exception: " << exc.what() << std::endl;
    errors = 1;
  }
  catch(...)
  {
    std::cerr << "ERROR: " << filename << " caused an exception" << std::endl;
    errors = 1;
  }

  free(path);
  LIBRARY_CLOSE(module);
  return errors;
}

// $Id: checker_test.cpp 635 2010-02-28 18:07:48Z gotthardp $
