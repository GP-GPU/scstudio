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
 * $Id: filefix.cpp 537 2009-12-28 16:45:51Z gotthardp $
 */

#include <stdafx.h>
#include <stdio.h>
#include <shlobj.h>

// include NSIS plug-in headers and library
// http://nsis.sourceforge.net/Examples/Plugin
#include "pluginapi.h"

extern "C" __declspec(dllexport)
void __cdecl RemoveMatchedFileLines(HWND hwndParent,
  int string_size, char *variables, stack_t **stacktop)
{
  HRESULT hr = S_OK;
  TCHAR filename[MAX_PATH+1];
  TCHAR string[MAX_PATH+1];

  EXDLL_INIT();

  popstringn(string, MAX_PATH);
  wchar_t needle[MAX_PATH+1];
  mbstowcs(needle, string, MAX_PATH);

  popstringn(filename, MAX_PATH);

  TCHAR path[MAX_PATH+1];
  // obtain local application data folder
  // typically C:\Documents and Settings\username\Local Settings\Application Data
  BOOL result = SHGetSpecialFolderPath(NULL, path, CSIDL_LOCAL_APPDATA, false);

  TCHAR source_path[MAX_PATH+1];
  _tcscpy_s(source_path, MAX_PATH, path);
  _tcscat_s(source_path, MAX_PATH, _T("\\"));
  _tcscat_s(source_path, MAX_PATH, filename);
  _tcscat_s(source_path, MAX_PATH, _T(".old"));

  TCHAR target_path[MAX_PATH+1];
  _tcscpy_s(target_path, MAX_PATH, path);
  _tcscat_s(target_path, MAX_PATH, _T("\\"));
  _tcscat_s(target_path, MAX_PATH, filename);

  // remove previous filename.old
  remove(source_path);
  // rename filename --> filename.old
  rename(target_path, source_path);

  FILE *infile;
  if(fopen_s(&infile, source_path, "rb") != 0) // filename.old
    return;

  FILE *outfile;
  if(fopen_s(&outfile, target_path, "wb") != 0) // filename
    return;

  while(!feof(infile))
  {
    static const size_t max_line = 1024;
    wchar_t line[max_line+1];

    size_t i = 0;
    // read a single line from the input file
    while(!feof(infile) && i < max_line)
    {
      wchar_t *pos = line+i;
      size_t res = fread(pos, sizeof(wchar_t), 1, infile);
      i += res;

      if(*pos == 0x0A)
        break;
    }

    // if not matched, write line to the output file
    if(wcsstr(line, needle) == NULL)
      fwrite(line, sizeof(wchar_t), i, outfile);
  }

  fclose(infile);
  fclose(outfile);

  // remove the filename.old
  remove(source_path);
}

// $Id: filefix.cpp 537 2009-12-28 16:45:51Z gotthardp $
