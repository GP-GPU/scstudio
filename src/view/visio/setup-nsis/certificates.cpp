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
 * $Id: certificates.cpp 198 2009-03-01 22:24:16Z gotthardp $
 */

#include <stdafx.h>
#include <atlbase.h>

// include NSIS plug-in headers and library
// http://nsis.sourceforge.net/Examples/Plugin
#include "pluginapi.h"

#pragma warning (disable : 4192)

// include Microsoft CAPICOM 2.0
// http://go.microsoft.com/fwlink/?linkid=84567
#import "capicom.dll"

extern "C" __declspec(dllexport)
void __cdecl InstallPublisherCertificate(HWND hwndParent,
  int string_size, char *variables, stack_t **stacktop)
{
  HRESULT hr = S_OK;
  TCHAR filename[MAX_PATH+1];

  EXDLL_INIT();
  popstringn(filename, MAX_PATH);

  try
  {
    // load certificate
    CAPICOM::ICertificate2Ptr pICertificate(__uuidof(CAPICOM::Certificate));
    hr = pICertificate->Load(filename, _T(""),
      CAPICOM::CAPICOM_KEY_STORAGE_DEFAULT, CAPICOM::CAPICOM_CURRENT_USER_KEY);

    CAPICOM::IStorePtr pIRootStore(__uuidof(CAPICOM::Store));
    // open a store for root certificates
    if (FAILED(hr = pIRootStore->Open(
      CAPICOM::CAPICOM_CURRENT_USER_STORE, _T("ROOT"), 
      CAPICOM::CAPICOM_STORE_OPEN_READ_WRITE)))
    {
      ATLTRACE(_T("Error [%#x]: pIStore->Open() failed at line %d.\n"), hr, __LINE__);
      throw hr;
    }

    hr = pIRootStore->Add(pICertificate);

    CAPICOM::IStorePtr pITrustedPublisherStore(__uuidof(CAPICOM::Store));
    // open a certificate store for trusted publishers
    if (FAILED(hr = pITrustedPublisherStore->Open(
      CAPICOM::CAPICOM_CURRENT_USER_STORE, _T("TrustedPublisher"), 
      CAPICOM::CAPICOM_STORE_OPEN_READ_WRITE)))
    {
      ATLTRACE(_T("Error [%#x]: pIStore->Open() failed at line %d.\n"), hr, __LINE__);
      throw hr;
    }

    hr = pITrustedPublisherStore->Add(pICertificate);
  }
  catch (_com_error e) 
  {
    hr = e.Error();
    ATLTRACE(_T("Error [%#x]: %s.\n"), hr, e.ErrorMessage());
  }
  catch (HRESULT hr)
  {
    ATLTRACE(_T("Error [%#x]: CAPICOM error.\n"), hr);
  }
  catch(...)
  {
    hr = CAPICOM::CAPICOM_E_UNKNOWN;
    ATLTRACE(_T("Unknown error.\n"));
  }

  pushint(hr);
}

// $Id: certificates.cpp 198 2009-03-01 22:24:16Z gotthardp $
