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
 * Copyright (c) 2007-2010 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id: dllmodule.h 632 2010-02-28 16:38:28Z gotthardp $
 */

#pragma once
#include <list>
#include <string>
#include "data/configurator.h"

#define SCSTUDIO_REGISTRY_ROOT _T("Software\\Sequence Chart Studio")

std::wstring LoadStringResource(UINT uiID);
std::wstring GetVersionInfo(const LPWSTR block);
std::basic_string<TCHAR> GetVisioModuleFileName();
std::basic_string<TCHAR> GetVisioModulePath();

class RegistryValueNotFound
{ };

template<class T, DWORD type>
T __GetRegistry(HKEY key, const TCHAR* subkey, const TCHAR* parameter)
{
  HKEY hPathKey;
  if(RegOpenKeyEx(key, subkey, 0, KEY_READ, &hPathKey) != ERROR_SUCCESS)
  {
    throw RegistryValueNotFound();
  }

  DWORD valueType;
  T value;
  DWORD valueLength = sizeof(T);

  if(RegQueryValueEx(hPathKey, parameter,
    NULL, &valueType, (LPBYTE)&value, &valueLength) != ERROR_SUCCESS)
  {
    throw RegistryValueNotFound();
  }

  if(valueType != type)
    throw RegistryValueNotFound();

  RegCloseKey(hPathKey);
  return value;
}

template<class T>
inline T GetRegistry(HKEY key, const TCHAR* subkey, const TCHAR* parameter)
{
  return __GetRegistry<T,REG_BINARY>(key, subkey, parameter);
}

template<>
inline DWORD GetRegistry<DWORD>(HKEY key, const TCHAR* subkey, const TCHAR* parameter)
{
  return __GetRegistry<DWORD,REG_DWORD>(key, subkey, parameter);
}

template<class T, DWORD type>
int __SetRegistry(HKEY key, const TCHAR* subkey, const TCHAR* parameter, T value)
{
  HKEY hPathKey;
  if(RegCreateKeyEx(
    key,
    subkey,
    0,
    NULL,
    REG_OPTION_NON_VOLATILE,
    KEY_ALL_ACCESS,
    NULL,
    &hPathKey,
    NULL) != ERROR_SUCCESS)
  {
    return 0;
  }

  if(RegSetValueEx(hPathKey,
    parameter,
    0, // must be zero 
    type, // value type 
    (LPBYTE)&value, sizeof(T)) != ERROR_SUCCESS)
  {
    return 0;
  }

  RegCloseKey(hPathKey);
  return 1;
}

template<class T>
inline int SetRegistry(HKEY key, const TCHAR* subkey, const TCHAR* parameter, T value)
{
  return __SetRegistry<T,REG_BINARY>(key, subkey, parameter, value);
}

template<>
inline int SetRegistry(HKEY key, const TCHAR* subkey, const TCHAR* parameter, DWORD value)
{
  return __SetRegistry<DWORD,REG_DWORD>(key, subkey, parameter, value);
}

template<class T>
T GetRegistry(const TCHAR* root, const TCHAR* path, const TCHAR* parameter, T default_value)
{
  TCHAR subkey[MAX_PATH];
  // construct the subkey address
  _tcscpy(subkey, root);
  if(path != NULL && *path != 0)
  {
    _tcscat(subkey, _T("\\"));
    _tcscat(subkey, path);
  }

  try
  {
    return GetRegistry<T>(HKEY_CURRENT_USER, subkey, parameter);
  }
  catch(RegistryValueNotFound)
  { }

  try
  {
    return GetRegistry<T>(HKEY_LOCAL_MACHINE, subkey, parameter);
  }
  catch(RegistryValueNotFound)
  { }

  return default_value;
}

std::list<std::wstring> GetRegistryStrings(const TCHAR* subkey);

class RegistryConfigProvider : public ConfigProvider
{
public:
  virtual long get_config_long(const std::wstring& section, const std::wstring& parameter, long def = 0) const
  {
    return GetRegistry<DWORD>(SCSTUDIO_REGISTRY_ROOT, section.c_str(), parameter.c_str(), def);
  }

  virtual float get_config_float(const std::wstring& section, const std::wstring& parameter, float def = 0.0) const
  {
    return GetRegistry<float>(SCSTUDIO_REGISTRY_ROOT, section.c_str(), parameter.c_str(), def);
  }
};

// $Id: dllmodule.h 632 2010-02-28 16:38:28Z gotthardp $
