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
 * Copyright (c) 2008-2009 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: z120_load.cpp 626 2010-02-26 10:50:31Z gotthardp $
 */

#include "z120.h"
// Process "Z120.g" with ANTLR 3.1.1 to produce the following files.
#include "Z120Lexer.h"
#include "Z120Parser.h"

std::vector<MscPtr> Z120::load_msc(const std::string &filename)
{
  std::vector<MscPtr> result;
  
  pANTLR3_INPUT_STREAM input =
    antlr3AsciiFileStreamNew((pANTLR3_UINT8)filename.c_str());
  if (input == NULL || input < 0)
  {
    print_report(RS_ERROR,
      stringize() << "Cannot open file '" << TOWSTRING(filename) << "'.");
    return result;
  }

  pZ120Lexer lxr = Z120LexerNew(input);
  if (lxr == NULL)
    return result;

  pANTLR3_COMMON_TOKEN_STREAM tstream =
    antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
  if (tstream == NULL)
    return result;

  pZ120Parser psr = Z120ParserNew(tstream);
  if (psr == NULL)
    return result;
  
  s_Msc** my_mscs = psr->textual_msc_file(psr, static_cast<s_Z120*>(this));
  if (my_mscs == NULL)
  {
    print_report(RS_ERROR, stringize() << "Error 22: Syntax error ");
    return result;
  }
 
  for(int i = 0; my_mscs[i] != NULL; i++)
  {
    Msc* my_msc = static_cast<Msc*>(my_mscs[i]);
    if(my_msc == NULL)
      continue;

    result.push_back(my_msc);
    // my_msc is an extern "C" pointer to an object pointed by smart pointers
    // the counter was increased in get_msc_fun() to avoid premature delete
    intrusive_ptr_release(my_msc);
  }
  
  delete[] my_mscs;

  psr->free(psr); psr = NULL;
  tstream->free(tstream); tstream = NULL;
  lxr->free(lxr); lxr = NULL;
  input->close(input); input = NULL;

  return result;
}

ImportFormatter::TransformationList Z120::get_transformations(MscPtr msc) const
{
  ImportFormatter::TransformationList result;
  result.push_back(L"Beautify");
  return result;
}

// $Id: z120_load.cpp 626 2010-02-26 10:50:31Z gotthardp $
