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
 * $Id: dllmodule.cpp 632 2010-02-28 16:38:28Z gotthardp $
 */

// implementation of DLL Exports

#include "stdafx.h"
#include "resource.h"
#include "eventsink.h"
#include "dllmodule.h"

namespace ATL
{

class CStudioDllModule : public CAtlDllModuleT< CStudioDllModule >
{
public :
  DECLARE_LIBID(LIBID_SCStudioLib)
  DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SCSTUDIO, "{F0C700DB-9781-48DA-83D2-E537117E2784}")
};

}; // namespace ATL

ATL::CStudioDllModule _AtlModule;

std::wstring LoadStringResource(UINT uiID)
{
  // Get the module instance.
  HINSTANCE hInstance = ATL::_AtlBaseModule.GetModuleInstance();

  TCHAR szBuffer[100];
  // Load the string from the string table.
  LoadString(hInstance, uiID, szBuffer, sizeof(szBuffer) / sizeof(TCHAR));
  return szBuffer;
}

std::wstring GetVersionInfo(const LPWSTR block)
{
  std::wstring csRet;
  HMODULE hLib = ATL::_AtlBaseModule.GetModuleInstance();

  HRSRC hVersion = FindResource(hLib, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
  if(hVersion != NULL)
  {
    HGLOBAL hGlobal = LoadResource(hLib, hVersion);
    if(hGlobal != NULL)
    {
      LPVOID versionInfo  = LockResource(hGlobal);
      if(versionInfo != NULL)
      {
        DWORD vLen;
        TCHAR *retbuf = NULL;

        if(VerQueryValueW(versionInfo, block, (LPVOID*)&retbuf, (UINT *)&vLen))
          csRet = retbuf;
      }
    }

    UnlockResource(hGlobal);
    FreeResource(hGlobal);
  }

  return csRet;
}

static const int _FILE_PATH_SIZE = _MAX_PATH * 4;

std::basic_string<TCHAR> GetVisioModuleFileName()
{
  TCHAR szPath[_FILE_PATH_SIZE];

  // get the full path including the Addon name
  GetModuleFileName(GetModuleHandle(LoadStringResource(IDS_VSL_NAME).c_str()),
    szPath, sizeof(szPath) / sizeof(TCHAR));

  return szPath;
}

std::basic_string<TCHAR> GetVisioModulePath()
{
  TCHAR szPath[_FILE_PATH_SIZE];
  TCHAR szDrive[_MAX_PATH];
  TCHAR szDir[_FILE_PATH_SIZE];
  TCHAR szFileName[_MAX_PATH];
  TCHAR szExt[_MAX_PATH];
  std::basic_string<TCHAR> strDir;

  // get the full path including the Addon name
  GetModuleFileName(GetModuleHandle(LoadStringResource(IDS_VSL_NAME).c_str()),
    szPath, sizeof(szPath) / sizeof(TCHAR));

  // extract the path name of the Addon
  if (szPath)
  {
    _tsplitpath(szPath, szDrive, szDir, szFileName, szExt);
    strDir = szDrive;
    strDir += szDir;
  }

  return strDir;
}

// Creates a locale string acceptable for setlocale
// see http://www.codeproject.com/KB/locale/CRTSynch.aspx
// public domain, written by Chris Grimes
LPCTSTR loadLocaleId(LCID lcid, _bstr_t& bstrRetBuf)
{
  TCHAR arcBuf[128];
  memset(arcBuf, 0, sizeof(arcBuf));

  // loading the English name fo the locale
  GetLocaleInfo( lcid, LOCALE_SENGLANGUAGE, arcBuf, 127);
  bstrRetBuf = arcBuf;
  memset(arcBuf, 0, sizeof(arcBuf));

  // loading the English name for the country/region
  GetLocaleInfo( lcid, LOCALE_SENGCOUNTRY, arcBuf, 127);
  if( *arcBuf )
  {
    bstrRetBuf += TEXT("_");
    bstrRetBuf += arcBuf;
  }

  // loading the code page
  memset(arcBuf, 0, sizeof(arcBuf));
  if( (GetLocaleInfo( lcid, LOCALE_IDEFAULTANSICODEPAGE, arcBuf, 127)
    || GetLocaleInfo( lcid, LOCALE_IDEFAULTCODEPAGE, arcBuf, 127))
    && *arcBuf )
  {
    bstrRetBuf += TEXT(".");
    bstrRetBuf += arcBuf;
  }

  return bstrRetBuf;
}

std::list<std::wstring> GetRegistryStrings(HKEY key, const TCHAR* subkey)
{
  std::list<std::wstring> result;

  HKEY hSubKey;
  if(RegOpenKeyEx(key, subkey, 0, KEY_READ, &hSubKey) != ERROR_SUCCESS)
  {
    throw RegistryValueNotFound();
  }

  TCHAR achClass[MAX_PATH];   // buffer for class name 
  DWORD cchClassName = MAX_PATH; // size of class string 
  DWORD cSubKeys;             // number of subkeys 
  DWORD cbMaxSubKey;          // longest subkey size 
  DWORD cchMaxClass;          // longest class string 
  DWORD cValues;              // number of values for key 
  DWORD cchMaxValue;          // longest value name 
  DWORD cbMaxValueData;       // longest value data 
  DWORD cbSecurityDescriptor; // size of security descriptor 
  FILETIME ftLastWriteTime;   // last write time 

  // get the class name and the value count
  if(RegQueryInfoKey(hSubKey,
    achClass,              // buffer for class name
    &cchClassName,         // size of class string
    NULL,                  // reserved
    &cSubKeys,             // number of subkeys
    &cbMaxSubKey,          // longest subkey size
    &cchMaxClass,          // longest class string
    &cValues,              // number of values for this key
    &cchMaxValue,          // longest value name
    &cbMaxValueData,       // longest value data
    &cbSecurityDescriptor,
    &ftLastWriteTime) != ERROR_SUCCESS)
  {
    throw RegistryValueNotFound();
  }

  // the size does not include the terminating NULL character
  TCHAR *valName = new TCHAR[cchMaxValue+1];
  DWORD valNameLength;
  DWORD dwType;
  TCHAR *valData = new TCHAR[cbMaxValueData+1]; 
  DWORD valDataLength; 

  // walk through all registry entries
  for(int i = cValues; i >= 0; i--)
  {
    valNameLength = cchMaxValue+1;
    valDataLength = cbMaxValueData+1;

    long res = RegEnumValue(hSubKey, i,
      valName,
      &valNameLength,
      NULL,
      &dwType,
      (LPBYTE)valData,
      &valDataLength);

    if(res != ERROR_SUCCESS)
      continue;

    result.push_back(valData);
  }

  delete[] valName;
  delete[] valData;

  RegCloseKey(hSubKey);
  return result;
}

std::list<std::wstring> GetRegistryStrings(const TCHAR* subkey)
{
  try
  {
    return GetRegistryStrings(HKEY_CURRENT_USER, subkey);
  }
  catch(RegistryValueNotFound)
  { }

  try
  {
    return GetRegistryStrings(HKEY_LOCAL_MACHINE, subkey);
  }
  catch(RegistryValueNotFound)
  { }

  return std::list<std::wstring>();
}

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//! DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE /* hInstance */, DWORD dwReason, LPVOID lpReserved)
{
  // early initialization of our subsystems
  if (dwReason == DLL_PROCESS_ATTACH)
  {
    _bstr_t localeName;
    loadLocaleId(GetThreadLocale(), localeName);
    // keep the C-runtime (CRT) in synch with the Windows API locale locale
    setlocale(LC_ALL, localeName);
  }

  return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

//! Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
  return _AtlModule.DllCanUnloadNow();
}

//! Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
  return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

//! Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
  // registers object, typelib and all interfaces in typelib
  return _AtlModule.DllRegisterServer();
}

//! Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
  return _AtlModule.DllUnregisterServer();
}

// $Id: dllmodule.cpp 632 2010-02-28 16:38:28Z gotthardp $
