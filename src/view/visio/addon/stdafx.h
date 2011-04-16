// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER        // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501    // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT    // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501  // Change this to the appropriate value to target other versions of Windows.
#endif            

#ifndef _WIN32_WINDOWS    // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE      // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600  // Change this to the appropriate value to target other versions of IE.
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _WTL_NEW_PAGE_NOTIFY_HANDLERS

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
#define _ATL_USE_DDX_FLOAT  // allow floating point DDX macros

#include "resource.h"
#include "config.h"

#include <Vaddon.h>
#include <Visiwrap.h>

#include <atlbase.h>
#include <atlapp.h>


// adding common controls manifest to get XP look
#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Import the Visio type library with named GUIDs for the type library entries.
// The namespace is "Visio".
#import "Visio.tlb" named_guids

#if(EXCEL_FOUND)

// Import Office type library
#import EXCEL_MSO \
  rename("DocumentProperties", "DocumentPropertiesXL") \
  rename("RGB", "RBGXL")

// Import VBA type library
#import EXCEL_VBA

// Import Excel type library
#import EXCEL_EXECUTABLE \
  rename("DialogBox", "DialogBoxXL") rename("RGB", "RBGXL") \
  rename("DocumentProperties", "DocumentPropertiesXL") \
  rename("ReplaceText", "ReplaceTextXL") \
  rename("CopyFile", "CopyFileXL") \
  exclude("IFont", "IPicture") no_dual_interfaces

#endif // EXCEL_FOUND

// $Id: stdafx.h 829 2010-07-09 11:00:32Z mbezdeka $
