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
 * $Id: addon.cpp 1019 2011-01-04 07:15:39Z xpekarc $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "addon.h"
#include "aboutdlg.h"
#include "SimulatorDlg.h"
#include "optionsdlg.h"
#include "instancesfielddlg.h"
#include "document.h"
#include "extract.h"
#include "errors.h"
#include "resource.h"
#include "pageutils.h"
#include "messageSnapping.h"
#include "messageJump.h"

// include command line parsing library SimpleOpt
// http://code.jellycan.com/simpleopt
#include <SimpleOpt.h>

// set VISIOSDK_ROOT e.g. to "C:\Program Files\Microsoft Office\Visio11\SDK\"
// include libraries from the Microsoft Office Visio 2003 SDK
// http://www.microsoft.com/downloads/details.aspx?familyid=557120bd-b0bb-46e7-936a-b8539898d44d
extern "C"
{
#include <Vao.c>
}
#include <Vaddon.cpp>
#include <Addsink.cpp>

const int MIN_VISIO_VERSION = 11;

CStudioAddon scstudio(_T(ADDON_NAME), IDS_ADDON_NAME);

const unsigned short visEvtBeforeDocumentClose = Visio::visEvtDel | Visio::visEvtDoc;
const unsigned short visEvtPageAdded = Visio::visEvtAdd | Visio::visEvtPage;
const unsigned short visEvtCellChanged = Visio::visEvtMod | Visio::visEvtCell;
const unsigned short visEvtConnectionsAdded = Visio::visEvtAdd | Visio::visEvtConnect;
const unsigned short visEvtConnectionsDeleted = Visio::visEvtDel | Visio::visEvtConnect;

CStudioAddon::CStudioAddon(LPCTSTR pName, UINT uIDLocalName)
: VAddon(VAO_AOATTS_ISACTION | VAO_AOATTS_HASABOUT, VAO_ENABLEALWAYS, 0, 0, pName, uIDLocalName)
{
  m_pIAddonSink = NULL;
  m_keyButtonState = 0;
  
  m_mousePosX = m_mousePosY = 0.0;
  m_mouseRelPosX = m_mouseRelPosY = 0.0;
  
  m_bCellChanged = false;
  m_bShapeDeleted = false;
  
  m_bSnap = false;
  m_bMoving = false;
  m_bBeginChanged = false;
  m_bEndChanged = false;
  m_bCtrlDown = false;
  m_bArrowKeyDown = false;
  m_bOnDropShape = false;
  m_bKeyDown = false;
  m_bShapeChanged = false;

  m_iJumpType = 0;
};

VAORC CStudioAddon::About(LPVAOV2LSTRUCT pV2L)
{
  TRACE("CStudioAddon::About() called");
  CAboutDlg dlg;

  std::wstringstream message;
  message
    << GetVersionInfo(_T("\\StringFileInfo\\000004e4\\ProductName"))
    << ", version " << GetVersionInfo(_T("\\StringFileInfo\\000004e4\\ProductVersion"))
    << " (" << GetVersionInfo(_T("\\StringFileInfo\\000004e4\\PrivateBuild")) << ")";
  dlg.m_version = message.str();

  dlg.DoModal();
  return VAORC_SUCCESS;
}

VAORC CStudioAddon::Run(LPVAOV2LSTRUCT pV2L)
{
  TRACE("CStudioAddon::Run() called");
  Visio::IVApplicationPtr vsoApp;

  try
  {
    // get a Visio::IVApplicationPtr for the Visio application
    if (!vsoApp.GetInterfacePtr())
    {
      // First try to attach to the currently running Visio application.
      // If that fails launch a new Visio application.
      if (pV2L != NULL && pV2L->lpApp != NULL)
        vsoApp = pV2L->lpApp;
      else
        if (!SUCCEEDED(vsoApp.GetActiveObject(Visio::CLSID_Application)))
          vsoApp.CreateInstance(Visio::CLSID_Application);

      if (!vsoApp.GetInterfacePtr())
        return VAORC_FAILURE;

      ResetState(vsoApp);
    }

    // get the major version number for Visio
    int majorVersion = 0;
    BSTR bstrVersion = NULL;
    if (SUCCEEDED(vsoApp->get_Version(&bstrVersion)))
      majorVersion = _ttoi(_bstr_t(bstrVersion));

    if (bstrVersion != NULL)
      SysFreeString(bstrVersion);

    // make sure that we're running against the correct version of Visio
    if (majorVersion == 0 || majorVersion < MIN_VISIO_VERSION)
    {
      DisplayException(vsoApp,
        LoadStringResource(IDS_ERROR_VISIO_VERSION), MB_OK | MB_ICONEXCLAMATION);

      return VAORC_SUCCESS;
    }

    // If the command-line argument is NULL, this add-on was called
    // by selecting the VSL from Visio Tools|Macros menu.
    if(pV2L->lpCmdLineArgs == NULL)
    {
      TRACE("CStudioAddon::Run() args=NULL");

      Visio::IVDocumentPtr vsoDocument = vsoApp->GetActiveDocument();
      // when no document is active
      if(vsoDocument == NULL)
      {
        try
        {
          vsoDocument = vsoApp->GetDocuments()->Add(VST_FILE_NAME);
        }
        catch (_com_error&)
        {
          DisplayException(vsoApp, _T("Failed to open a MSC document template."), MB_OK);
          return VAORC_FAILURE;
        }
      }

      RegisterPersistentEvents(vsoDocument);

      CDocumentMonitor *pDocumentMonitor = GetDocumentMonitor(vsoApp, vsoDocument);
      return VAORC_SUCCESS;
    }
    else
    {
      TCHAR* cmdline = _tcsdup(pV2L->lpCmdLineArgs);
      TRACE("CStudioAddon::Run() cmdline=\"" << cmdline << "\"");

      static const TCHAR* seps = _T(" \t");
      int argc = 0;
      TCHAR* argv[100];
      // read the command-line string
      TCHAR* token = _tcstok(cmdline, seps);
      while(argc < _countof(argv) && token != NULL)
      {
        argv[argc++] = token;
        token = _tcstok(NULL, seps);
      }

      enum
      {
        OPT_VISIO,
        OPT_EVENT,
        OPT_EVENTID,
        OPT_DOC,
        OPT_PAGE,
        OPT_SHAPE,
        OPT_SHAPEU
      };

      static CSimpleOpt::SOption const long_options[] =
      {
        { OPT_VISIO, _T("-visio"), SO_REQ_CMB },
        { OPT_EVENT, _T("-event"), SO_REQ_CMB },
        { OPT_EVENTID, _T("-eventid"), SO_REQ_CMB },
        { OPT_DOC, _T("-doc"), SO_REQ_CMB },
        { OPT_PAGE, _T("-page"), SO_REQ_CMB },
        { OPT_SHAPE, _T("-shape"), SO_REQ_CMB },
        { OPT_SHAPEU, _T("-shapeu"), SO_REQ_CMB },
        SO_END_OF_OPTIONS
      };

      // Parse the command-line arguments.
      CSimpleOpt args(argc, argv, long_options, SO_O_USEALL);

      int iVisio = 0;
      int iEvent = 0;
      int iDocumentIndex = 0;
      int iPageIndex = 0;
      _bstr_t sShape;
      _bstr_t sShapeU;

      while(args.Next())
      {
        if(args.LastError() != SO_SUCCESS)
        {
          TRACE("CStudioAddon::Run() bad command-line argument=" << args.OptionText());
          continue;
        }

        switch(args.OptionId())
        {
        case OPT_VISIO:
          iVisio = _tstol(args.OptionArg());
          break;
        case OPT_EVENT:
          iEvent = _tstol(args.OptionArg());
          break;
        case OPT_EVENTID:
          // FIXME: what is this parameter for?
          break;
        case OPT_DOC:
          iDocumentIndex = _tstol(args.OptionArg());
          break;
        case OPT_PAGE:
          iPageIndex = _tstol(args.OptionArg());
          break;
        case OPT_SHAPE:
          sShape = args.OptionArg();
          break;
        case OPT_SHAPEU:
          sShapeU = args.OptionArg();
          break;
        default:
          TRACE("CStudioAddon::Run() unexpected argument id=" << args.OptionId());
          break;
        }
      }
      free(cmdline);

      Visio::IVDocumentPtr vsoDocument;
      // if valid, use the document of the given index
      // note: the documents collection is one-based
      if(iDocumentIndex > 0 && iDocumentIndex <= vsoApp->Documents->Count)
      {
        TRACE("CStudioAddon::Run() using the document id=" << iDocumentIndex);
        vsoDocument = vsoApp->Documents->Item[iDocumentIndex];
      }
      else
      {
        TRACE("CStudioAddon::Run() using the active document");
        vsoDocument = vsoApp->GetActiveDocument();
      }

      RegisterPersistentEvents(vsoDocument);

      CDocumentMonitor *pDocumentMonitor = GetDocumentMonitor(vsoApp, vsoDocument);

      // event numbers 300-399 are reserved for transformers
      // note: formatter index is encoded in the event number
      if(iEvent >= 300 && iEvent <=399)
        pDocumentMonitor->OnMenuTransform(vsoApp, iEvent-300);

      // execute the relevant event handler
      switch(iEvent)
      {
      case 1: // DocumentCreate
      case 2: // DocumentOpen
        return VAORC_SUCCESS;

        // 1xx events
        // note: to keep stencil compatibility, the 1xx events must not be renumbered
      case 100:
        TRACE("CStudioAddon::Run() stencil event 'OnDrop'");
        return pDocumentMonitor->OnDropShape(iDocumentIndex, iPageIndex, sShapeU);
      case 101:
        TRACE("CStudioAddon::Run() reference action 'Open Reference'");
        return pDocumentMonitor->OnOpenReference(iDocumentIndex, iPageIndex, sShapeU);
      case CDocumentMonitor::MENU_ADD_INSTANCES: // 102
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Add Instances'");
        return DoInstancesField(pDocumentMonitor, vsoApp);
      case CDocumentMonitor::MENU_MESSAGE_SEQUENCE: // 103
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Message Sequence'");
        return pDocumentMonitor->OnMenuMessageSequence(vsoApp);
      case CDocumentMonitor::MENU_SELECT_NUMBERED_GROUP:	//104
        TRACE("CStudioAddon::Run() menu item 'context menu - select numbering group'");
        return pDocumentMonitor->OnMenuSelectNumberedGroup(vsoApp);
      case CDocumentMonitor::MENU_FLIP_MESSAGE_DIRECTION: //105
        TRACE("CStudioAddon::Run() menu item 'context menu - Flip message direction'");
        return pDocumentMonitor->OnMenuFlipMessageDirection(vsoApp);

        // 2xx events
      case CDocumentMonitor::MENU_WINDOWS_REPORTER:
        TRACE("CStudioAddon::Run() menu item 'Check--Windows--Verification Report'");
        return pDocumentMonitor->OnMenuWindowsReporter(vsoApp);
      case CDocumentMonitor::MENU_IMPORT:
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Import'");
        return pDocumentMonitor->OnMenuImport(vsoApp);
      case CDocumentMonitor::MENU_EXPORT:
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Export'");
        return pDocumentMonitor->OnMenuExport(vsoApp);
      case CDocumentMonitor::MENU_REPAINT:
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Repaint'");
        return pDocumentMonitor->OnMenuRepaint(vsoApp);
      case CDocumentMonitor::MENU_FIND_FLOW:
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Find Flow'");
        return pDocumentMonitor->OnMenuFindFlow(vsoApp);
      case CDocumentMonitor::MENU_SIMULATION_START:
        TRACE("CStudioAddon::Run() menu item 'Check--Start Simulation'");
        return pDocumentMonitor->OnMenuSimulationStart(vsoApp);
      case CDocumentMonitor::MENU_SIMULATION_STOP:
        TRACE("CStudioAddon::Run() menu item 'Check--Stop Simulation'");
        return pDocumentMonitor->OnMenuSimulationStop(vsoApp);
      case CDocumentMonitor::MENU_SIMULATOR_OPTIONS:
        TRACE("CStudioAddon::Run() menu item 'Check--Simulator Options'");
        return DisplaySimulatorOptions();
      case CDocumentMonitor::MENU_VERIFY:
        TRACE("CStudioAddon::Run() menu item 'Check--Verify'");
        return pDocumentMonitor->OnMenuVerify(vsoApp);
      case CDocumentMonitor::MENU_CHECK_OPTIONS:
        TRACE("CStudioAddon::Run() menu item 'Check--Options'");
        return DisplayCheckOptions();
      case CDocumentMonitor::MENU_SELECT_ALL_INSTANCES:
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Select--All Instances'");
        return pDocumentMonitor->OnMenuSelectAllInstances(vsoApp, SELECTION_REPLACE);
      case CDocumentMonitor::MENU_SELECT_ALL_MESSAGES:
        TRACE("CStudioAddon::Run() menu item 'Check--Drawing--Select--All Messages'");
        return pDocumentMonitor->OnMenuSelectAllMessages(vsoApp, SELECTION_REPLACE);
      case CDocumentMonitor::MENU_SELECT_ADD_ALL_INSTANCES:
        TRACE("CStudioAddon::Run() add all instances to the current selection");
        return pDocumentMonitor->OnMenuSelectAllInstances(vsoApp, SELECTION_ADD);
      case CDocumentMonitor::MENU_SELECT_ADD_ALL_MESSAGES:
        TRACE("CStudioAddon::Run() add all messages to the current selection");
        return pDocumentMonitor->OnMenuSelectAllMessages(vsoApp, SELECTION_ADD);
      case CDocumentMonitor::MENU_MESSAGE_JUMP_LEFT:
        TRACE("CStudioAddon::Run() jump message left");
        return pDocumentMonitor->OnMenuMessageJump(vsoApp, true, false);
      case CDocumentMonitor::MENU_MESSAGE_JUMP_RIGHT:
        TRACE("CStudioAddon::Run() jump message right");
        return pDocumentMonitor->OnMenuMessageJump(vsoApp, false, false);
      case CDocumentMonitor::MENU_MESSAGE_JUMP_LEFT_COPY:
        TRACE("CStudioAddon::Run() jump message left as copy");
        return pDocumentMonitor->OnMenuMessageJump(vsoApp, true, true);
      case CDocumentMonitor::MENU_MESSAGE_JUMP_RIGHT_COPY:
        TRACE("CStudioAddon::Run() jump message right as copy");
        return pDocumentMonitor->OnMenuMessageJump(vsoApp, false, true);
      case CDocumentMonitor::MENU_ENABLE_MESSAGE_ENUMERATION:
        TRACE("CStudioAddon::Run() enable message enumeration");
        return pDocumentMonitor->OnMenuEnableMessageEnumeration(vsoApp);
      case CDocumentMonitor::MENU_DISABLE_MESSAGE_ENUMERATION:
        TRACE("CStudioAddon::Run() disable message enumeration");
        return pDocumentMonitor->OnMenuDisableMessageEnumeration(vsoApp);
      case CDocumentMonitor::MENU_GLOBAL_SETTINGS:
        TRACE("CStudioAddon::Run() global settings");
        return pDocumentMonitor->OnMenuGlobalSettings(vsoApp);
			case CDocumentMonitor::MENU_BEAUTIFY_SETTINGS:
        TRACE("CStudioAddon::Run() beautify settings");
        return pDocumentMonitor->OnMenuBeautifySettings(vsoApp);
			case CDocumentMonitor::MENU_IMPORT_SETTINGS:
        TRACE("CStudioAddon::Run() import settings");
        return pDocumentMonitor->OnMenuImportSettings(vsoApp);

      default:
        TRACE("CStudioAddon::Run() unexpected event id=" << iEvent);
        return VAORC_FAILURE;
      }
    }
  }
  catch (_com_error &err)
  {
    _bstr_t message = err.Description();
    DisplayException(vsoApp, err.ErrorMessage(), MB_OK);
  }

  return VAORC_SUCCESS;
}

VAORC CStudioAddon::Unload(WORD wParam, LPVOID p)
{
  DocumentMonitorsMap::iterator documentsIter;
  // clean up the monitored document map
  while ((documentsIter = m_DocumentMonitors.begin()) != m_DocumentMonitors.end())
  {
    delete documentsIter->second;
    m_DocumentMonitors.erase(documentsIter);
  }

  // delete the marker Event
  if (m_vsoMarkerEvent != NULL)
    m_vsoMarkerEvent->Delete();

  // release the sink if there is one
  if (m_pIAddonSink)
  {
    m_pIAddonSink->Release();
    m_pIAddonSink = NULL;
  }

  return VAddon::Unload(wParam, p);
}

VAORC CStudioAddon::DisplaySimulatorOptions()
{
  TRACE("CStudioAddon::DisplaySimulatorOptions() called");
  CSimulatorDlg dlg;

  dlg.DoModal();
  return VAORC_SUCCESS;
}

VAORC CStudioAddon::DisplayCheckOptions()
{
  TRACE("CStudioAddon::DisplayCheckOptions() called");
  COptionsDlg dlg;

  dlg.DoModal();
  return VAORC_SUCCESS;
}

VAORC CStudioAddon::DoInstancesField(CDocumentMonitor* documentMonitor, Visio::IVApplicationPtr vsoApp)
{
  TRACE("CStudioAddon::DoInstancesField() called");

  CInstancesFieldDlg dlg(vsoApp, m_mousePosX, m_mousePosY);
  if (!dlg.DoModal()) return VAORC_SUCCESS;

  if (!documentMonitor->IsEmpty(vsoApp))
  {
    int res = MessageBox(GetActiveWindow(),
      _T("Current page is not empty.\n\nDraw the instances anyway?\n"),
      _T("The page is not empty"),
      MB_OKCANCEL | MB_ICONWARNING
      );
    if (res != IDOK) return VAORC_SUCCESS;
  }

  return documentMonitor->DrawInstancesField(
    vsoApp,
    dlg.m_instances_cnt,
    CPageUtils::ConvertUnits(vsoApp, dlg.m_instance_length, visPageUnits, visMillimeters),
    CPageUtils::ConvertUnits(vsoApp, dlg.m_instance_width, visPageUnits, visMillimeters),
    CPageUtils::ConvertUnits(vsoApp, dlg.m_start_pos_x, visPageUnits, visMillimeters),
    CPageUtils::ConvertUnits(vsoApp,
    (CPageUtils::GetPageHeight(vsoApp->ActivePage) - dlg.m_start_pos_y), // the drawing y-coordinates are upside down
    visPageUnits, visMillimeters),
    dlg.m_use_const_spacing,
    CPageUtils::ConvertUnits(vsoApp, (dlg.m_use_const_spacing ? dlg.m_spacing : dlg.m_total_width), visPageUnits, visMillimeters)
    );
}

HRESULT CStudioAddon::HandleVisioEvent(
                                       IUnknown *ipSink, short nEventCode, IDispatch *pSourceObj, long nEventID, 
                                       long nEventSeqNum, IDispatch *pSubjectObj, VARIANT vMoreInfo, VARIANT *pvResult)
{
  try
  {
    if (ipSink == NULL || pSourceObj == NULL || pSubjectObj == NULL || pvResult == NULL)
    {
      TRACE("CStudioAddon::HandleVisioEvent() unexpected NULL");
      return E_FAIL;
    }

    unsigned short event = nEventCode;
    switch (event)
    {
    case visEvtBeforeDocumentClose:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtBeforeDocumentClose");
      break;
    case visEvtPageAdded:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtPageAdded");
      break;

    case visEvtCellChanged:
      HandleCellChanged(pSubjectObj);
      break;

    case visEvtConnectionsAdded:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtConnectionsAdded");
      HandleConnectionsAdded(pSubjectObj);
      break;
    case visEvtConnectionsDeleted:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtConnectionsDeleted");
      break;

    case visEvtApp|visEvtMarker:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtMarker");
      HandleMarker(pSubjectObj);
      break;

    case Visio::visEvtCodeKeyDown:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeKeyDown");
      HandleKeyDown(pSubjectObj, pSourceObj);
      break;

    case Visio::visEvtCodeKeyUp:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeKeyUp");
      HandleKeyUp(pSubjectObj, pSourceObj);
      break;

    case Visio::visEvtCodeMouseMove:
      //TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeMouseMove");
      HandleMouseMove(pSubjectObj, pSourceObj);
      break;

    case Visio::visEvtCodeMouseDown:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeMouseDown");
      HandleMouseDown(pSubjectObj, pSourceObj);
      break;

    case Visio::visEvtCodeWinSelChange:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeWinSelChange");
      HandleWinSelChange(pSourceObj);
      break;

    case Visio::visEvtCodeShapeBeforeTextEdit:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeShapeBeforeTextEdit");
      HandleBeforeTextEdit(pSubjectObj, pSourceObj);
      break;

    case Visio::visEvtCodeShapeExitTextEdit:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeShapeExitTextEdit");
      HandleAfterTextEdit(pSubjectObj, pSourceObj);
      break;

    case Visio::visEvtDel+Visio::visEvtShape:
      TRACE("CStudioAddon::HandleVisioEvent() visEvtCodeShapeDelete");
      HandleBeforeShapeDeleted(pSubjectObj, pSourceObj);
      break;

    case visEvtApp+visEvtNonePending:
      //TRACE("CStudioAddon::HandleVisioEvent() visEvtNonePending");
      HandleNonePending(pSourceObj);
      break;

    case visEvtApp+visEvtIdle:
      HandleVisioIsIdle(pSourceObj);
      break;

    case visEvtMod+visEvtText:
      HandleTextChanged(pSubjectObj,pSourceObj);
      break;

    default:
      TRACE("CStudioAddon::HandleVisioEvent() unexpected event id="
        << std::ios::hex << event);
      break;
    }

    return NOERROR;
  }
  catch (_com_error &err)  // catch exceptions thrown by Visio
  {
    _bstr_t message = err.Description();
    return E_FAIL;
  }
}

void CStudioAddon::HandleCellChanged(Visio::IVCellPtr vsoCell)
{
  // ignore events when undoing or redoing
  if(vsoCell->Application->IsUndoingOrRedoing == VARIANT_TRUE)
    return;

  if(vsoCell->Section == visSectionControls)
  {
    // note: to keep stencil compatibility, the names must not be changed
    if(_tcsicmp(vsoCell->RowName, _T("mscHeadWidth")) == 0 && vsoCell->Column == visCtlY)
    {
      double value = vsoCell->ResultIU;
      TRACE("CStudioAddon::HandleCellChanged() mscHeadWidth changed to " << value);

      Visio::IVShapePtr vsoShape = vsoCell->Shape;
      // walk through connected shapes
      for(int i = 1; i <= vsoShape->FromConnects->Count; i++)
      {
        Visio::IVConnectPtr fromConnect = vsoShape->FromConnects->Item[i];
        // change width of all coregion boxes
        // note: the coregion is rotated by 90', width is the 'y' coordinate
        if(get_shape_type(fromConnect->FromSheet) == ST_BMSC_COREGION)
          fromConnect->FromSheet->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->ResultIU = value*2;
      }
    }
  }

  if((_tcsicmp(vsoCell->Name,_T("PinX")) == 0 || _tcsicmp(vsoCell->Name,_T("PinY")) == 0))
  {
    if(isMessageShape(vsoCell->Shape))
    {
      //Message enumeration
      if(_tcsicmp(vsoCell->Shape->Data1,_T("1")) == 0)
      {
        //add to set
        m_enumerationGroups.insert(vsoCell->Shape->Data3);
        m_bCellChanged = true;
      }
      //Message snapping
      if(!m_bSnap)
        m_bSnap = true;
    }
    m_bShapeChanged = true;
  }
  
  //HACK: cell changed so update mouse position (because when changing end points, MouseMove event won't trigger)  

  if((_tcsicmp(vsoCell->Name,_T("BeginX")) == 0) || (_tcsicmp(vsoCell->Name,_T("BeginY")) == 0))
    m_bBeginChanged = true;
  if((_tcsicmp(vsoCell->Name,_T("EndX")) == 0) || (_tcsicmp(vsoCell->Name,_T("EndY")) == 0))
    m_bEndChanged = true;
  if(_tcsicmp(vsoCell->Name,_T("LocPinX")) == 0)
    m_bMoving = true;
}

void CStudioAddon::HandleConnectionsAdded(Visio::IVConnectsPtr vsoConnects)
{
  // ignore events when undoing or redoing
  if(vsoConnects->Application->IsUndoingOrRedoing == VARIANT_TRUE)
    return;

  // dynamic connectors are positioned on Width*X
  // connectors must not move when an instance is begin enlarged

  // walk through added connectors and modify the positioning formula
  for(int i = 1; i <= vsoConnects->Count; i++)
  {
    Visio::IVConnectPtr vsoConnect = vsoConnects->Item[i]; // added connector
    if(vsoConnect == NULL)
      continue;

    // due to a bug in Visio 2003 the vsoConnect->ToCell->Formula is invalid
    // workaround: we process all connectors on a given instance;
    // the connector just being added is clearly one of them
    Visio::IVConnectsPtr fromConnects = vsoConnect->ToCell->Shape->FromConnects;
    for(int j = 1; j <= fromConnects->Count; j++)
    {
      Visio::IVConnectPtr fromConnect = fromConnects->Item[j];
      fromConnect->ToCell->Formula = fromConnect->ToCell->ResultStr[visNoCast];
    }
  }
}

void CStudioAddon::HandleMarker(Visio::IVApplicationPtr vsoApp)
{
  _bstr_t contextString = vsoApp->GetEventInfo(Visio::visEvtIdMostRecent);
  if(!contextString.length())
	  return;

  TRACE("CStudioAddon::HandleMarker() " << contextString);
  CDocumentMonitor *pDocumentMonitor = GetDocumentMonitor(vsoApp, vsoApp->ActiveDocument);
  if(pDocumentMonitor)
  {
    if(_stricmp((const char *)contextString, "SimulationResult") == 0)
      pDocumentMonitor->OnSimulationResult();
    else if(_stricmp((const char *)contextString, "SimulationError") == 0)
      pDocumentMonitor->OnSimulationError();
  }
}

void CStudioAddon::HandleKeyDown(Visio::IVKeyboardEventPtr vsoKeyboardEvent, Visio::IVApplicationPtr vsoApp)
{
  m_bKeyDown = true;

  long keyCode = vsoKeyboardEvent->KeyCode;
  long oldState = m_keyButtonState;
  m_keyButtonState = vsoKeyboardEvent->KeyButtonState;

  long modifiers = visKeyControl | visKeyShift;

  if (!(oldState & modifiers) && (m_keyButtonState & modifiers)) {
    GetDocumentMonitor(vsoApp, vsoApp->ActiveDocument)->ToggleToolbarItems(true);
  }

  //KeyDown handling
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;
  switch(keyCode)
  {
  case VK_LEFT:
  case VK_RIGHT:
    if(selection->Count == 1 && CMessageSnapping::isArrowKeysEnabled())
    {
      Visio::IVShapePtr shape = selection->Item[1];
      //If message isn't connected to both endpoints(not for LOST/FOUND), snap unplugged end to nearest instance
     /* MsgConnectedEndpoints conn = CShapeUtils::getConnectedEndpoints(shape);
      if(conn != MSCE_BOTH && conn != MSCE_NONE && get_shape_type(shape) == ST_BMSC_MESSAGE)
        CMessageSnapping::snapEndPointToClosestInstance(shape, (conn == MSCE_BEGIN) ? "EndX" : "BeginX", getMsgDirection::getMsgDirection(shape));*/
      //Get connected instances
      instances = CMessageSnapping::getConnectedInstances(shape);
      m_iJumpType = (keyCode == 0x25) ? 1 : 2;
    }
    break;
  }

  if(m_keyButtonState & visKeyControl)
    m_bCtrlDown = true;
}

void CStudioAddon::HandleKeyUp(Visio::IVKeyboardEventPtr vsoKeyboardEvent, Visio::IVApplicationPtr vsoApp)
{
  if (vsoKeyboardEvent->KeyCode == VK_ESCAPE)
  {
    ResetState(vsoApp);
  }
    
  // key code
  long keyCode = vsoKeyboardEvent->KeyCode;
  // buttons state
  long oldState = m_keyButtonState;
  m_keyButtonState = vsoKeyboardEvent->KeyButtonState;

  long modifiers = visKeyControl | visKeyShift;

  if ((oldState & modifiers) && !(m_keyButtonState & modifiers)) {
    GetDocumentMonitor(vsoApp, vsoApp->ActiveDocument)->ToggleToolbarItems(false);
  }

  //KeyUp handling
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;
  Visio::IVShapesPtr shapes = vsoApp->ActivePage->Shapes;
  switch(keyCode)
  {
  case VK_LEFT:
  case VK_RIGHT:
    if(selection->Count == 1 && CMessageSnapping::isArrowKeysEnabled()) //NOTE: Skip re-snapping if jumping is procceeding
      break;
  case VK_UP:
  case VK_DOWN:
    for(int i=1; i<=selection->Count; i++)
      CMessageSnapping::resnap(selection->Item[i], shapes, 0.001);
    break;
  }
  //Message snapping
  m_bCtrlDown = false;
}

void CStudioAddon::HandleMouseMove(Visio::IVMouseEventPtr vsoMouseEvent, Visio::IVApplicationPtr vsoApp)
{
  m_mousePosX = vsoMouseEvent->x;
  m_mousePosY = vsoMouseEvent->y;
}

void CStudioAddon::HandleMouseDown(Visio::IVMouseEventPtr vsoMouseEvent, Visio::IVApplicationPtr vsoApp)
{
  //Message snapping
  if(vsoApp->ActiveWindow->Selection->Count == 1)
  {
    Visio::IVShapePtr shape = vsoApp->ActiveWindow->Selection->Item[1];
    m_mouseRelPosX = m_mousePosX - shape->Cells["PinX"]->Result[""];
    m_mouseRelPosY = m_mousePosY - shape->Cells["PinY"]->Result[""];
  }
}

void CStudioAddon::HandleWinSelChange(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;

  if(selection->Count == 1)
  {
    //Message snapping
    Visio::IVShapePtr shape = selection->Item[1];
    m_mouseRelPosX = m_mousePosX - shape->Cells["PinX"]->Result[""];
    m_mouseRelPosY = m_mousePosY - shape->Cells["PinY"]->Result[""];
  }

  m_oldSelections[vsoApp] = m_curSelections[vsoApp];
  m_curSelections[vsoApp] = selection;

  if (GetState(vsoApp) == STATE_MESSAGE_SEQUENCE_WAITING_FOR_SEL_CHANGE)
  {
    CDocumentMonitor* docMon = GetDocumentMonitor(vsoApp, vsoApp->ActiveDocument);
    docMon->OnMenuMessageSequence(vsoApp, m_oldSelections[vsoApp]);
  }
}

void CStudioAddon::HandleNonePending(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;
  ////////////////////////////////////////////////////
  //Message jumping
  if(m_iJumpType && selection->Count == 1 && CMessageSnapping::isArrowKeysEnabled() && isMessageShape(selection->Item[1]))
  {
    Visio::IVShapesPtr shapes = vsoApp->ActivePage->Shapes;
    Visio::IVShapePtr shape = selection->Item[1];

    long scopeId = vsoApp->BeginUndoScope("Jump");
    if(!instances.size()) //If message is not connected, snap it first
    {
      CMessageSnapping::snap(shape, CShapeUtils::getShapeCell(shape, "PinX"), CShapeUtils::getShapeCell(shape, "PinY"), MSSNAP_PRESERVE_VERTICAL);
      instances = CMessageSnapping::getConnectedInstances(shape);
    }
    if(!CMessageJump::jump(shape, instances, (m_iJumpType == 1) ? MSJUMP_LEFT : MSJUMP_RIGHT, false))
    {
      MscPoint msgVec;
      CMessageSnapping::getMsgVector(shape, &msgVec);
      CMessageSnapping::resnap(shape, shapes, 0.1);
      MsgConnectedEndpoints conn = CShapeUtils::getConnectedEndpoints(shape);
      if(conn != MSCE_BOTH && conn != MSCE_NONE)
        CMessageSnapping::setMsgLength(shape, msgVec, (conn == MSCE_BEGIN) ? vis1DBeginX : vis1DEndX);
    }
    if(CMessageJump::getMsgNeedsResnap(shape))
      CMessageSnapping::resnap(shape, shapes, 0.0001);
    m_iJumpType = 0;
    instances.clear();
    vsoApp->EndUndoScope(scopeId, true);
  }
  ////////////////////////////////////////////////////
  //Message snapping
#define _u(x) CPageUtils::ConvertUnits(vsoApp, x, 0, visPageUnits)
  if(!m_bOnDropShape && m_bSnap && !m_bKeyDown && (selection->Count == 1) && CMessageSnapping::isEnabled())
  {    
    Visio::IVShapePtr msgShape = selection->Item[1];

    if(msgShape && isMessageShape(msgShape))
    {
      if(m_bBeginChanged && m_bEndChanged)  //NOTE: Trigger when moving with whole message
      {
        //Get relative position of the mouse from the shape Begin
        double newPosX = CShapeUtils::getShapeCell(msgShape,"PinX") + _u(m_mouseRelPosX);;
        double newPosY = CShapeUtils::getShapeCell(msgShape,"PinY") + _u(m_mouseRelPosY);
        CMessageSnapping::snap(msgShape, newPosX, newPosY,CMessageSnapping::getSnapType()); //Do regular snapping
      }
      else if(m_bBeginChanged ^ m_bEndChanged)  //NOTE: Trigger when pulling endpoints
        CMessageSnapping::snapEndPointToClosestInstance(msgShape, m_bBeginChanged ? "BeginX" : "EndX", 
                                                        CShapeUtils::getMsgDirection(msgShape));
    }
  }  
#undef _u

  ////////////////////////////////////////////////////
  //Message numbering
  if(m_bShapeDeleted || m_bCellChanged)
  {
    Visio::IVShapesPtr shapesPtr = vsoApp->ActivePage->Shapes;
    CDocumentMonitor* docMon = GetDocumentMonitor(vsoApp,vsoApp->ActiveDocument);

    for(std::set<_bstr_t>::iterator it = m_enumerationGroups.begin(); it != m_enumerationGroups.end(); it++)
      docMon->drawNumbers(shapesPtr,*it);

    m_enumerationGroups.clear();
    
    if(m_bShapeDeleted)
    {
      //Check whether there are still some numbered messages, otherwise erase enum info
      bool bNumberedMsg = false;
      for(int i=1; i<=vsoApp->ActivePage->Shapes->Count; i++)
      {
        Visio::IVShapePtr shape = vsoApp->ActivePage->Shapes->Item[i];
        if(isMessageShape(shape) && _tcsicmp(shape->Data1,_T("1")) == 0)
        {
          bNumberedMsg = true;
          break;
        }
      }
      if(!bNumberedMsg)
        CEnumerateUtils::eraseEnumInfo(vsoApp);
    }
    //Set variables to false to prevent this function from loop
    m_bShapeDeleted = m_bCellChanged = false;
  }

  //Set variables to false
  m_bSnap = m_bMoving = m_bBeginChanged = m_bEndChanged = false;
  //Reset OnDropShape state
  m_bOnDropShape = false;
  //Reset OnKeyDown
  m_bKeyDown = false;
}

void CStudioAddon::HandleVisioIsIdle(Visio::IVApplicationPtr vsoApp)
{
  ////////////////////////////////////////////////////
  //Block instance rotation
  if(m_bShapeChanged)
  {
    if(vsoApp->ActiveWindow->Selection->Count > 0)
    {
      for(int i=1;i<=vsoApp->ActiveWindow->Selection->Count; i++)
        if(get_shape_type(vsoApp->ActiveWindow->Selection->Item[i]) == ST_BMSC_INSTANCE)
        {
          Visio::IVShapePtr shape = vsoApp->ActiveWindow->Selection->Item[i];
          if((CShapeUtils::getShapeCell(shape, "BeginX") != CShapeUtils::getShapeCell(shape, "EndX")) ||
            (CShapeUtils::getShapeCell(shape, "BeginY") < CShapeUtils::getShapeCell(shape, "EndY")))
          {
            if(GetRegistry<bool>(_T("Software\\Sequence Chart Studio\\Protection"), NULL, _T("PreventInstanceRotation"), 0))
            {
              vsoApp->Undo();
              MessageBeep(0);
            }
            break;
          }
        }
    }
    m_bShapeChanged = false;
  }
}

void CStudioAddon::HandleBeforeTextEdit(Visio::IVShapePtr shapePtr, Visio::IVApplicationPtr vsoApp)
{
  if(isMessageShape(shapePtr) && _tcsicmp(shapePtr->Data1,_T("1")) == 0)
  {
    GetDocumentMonitor(vsoApp,vsoApp->ActiveDocument)->ToogleNumeration(false);
    shapePtr->Text = shapePtr->Data2;
    m_bMessageTextEdited = true;
  }
}

void CStudioAddon::HandleAfterTextEdit(Visio::IVShapePtr shapePtr, Visio::IVApplicationPtr vsoApp)
{
  if(m_bMessageTextEdited)
  {
    shapePtr->Data2 = shapePtr->Text;
    GetDocumentMonitor(vsoApp,vsoApp->ActiveDocument)->drawNumbers(vsoApp->ActivePage->Shapes, shapePtr->Data3);	
    m_bMessageTextEdited = false;
  }
  GetDocumentMonitor(vsoApp,vsoApp->ActiveDocument)->ToogleNumeration(true);
}

void CStudioAddon::HandleBeforeShapeDeleted(Visio::IVShapePtr shapePtr, Visio::IVApplicationPtr vsoApp)
{
  if(isMessageShape(shapePtr) && _tcsicmp(shapePtr->Data1,_T("1")) == 0)
  {
    m_enumerationGroups.insert(shapePtr->Data3);
    m_bShapeDeleted = true;
  }
}

void CStudioAddon::HandleTextChanged(Visio::IVShapePtr shapePtr,Visio::IVDocumentPtr vsoDocument)
{
}

void CStudioAddon::RegisterPersistentEvents(Visio::IVDocumentPtr vsoDocument)
{
  Visio::IVEventPtr vsoDocumentCreateEvent = NULL;
  Visio::IVEventPtr vsoDocumentOpenEvent = NULL;

  Visio::IVEventListPtr vsoDocumentEventList = vsoDocument->EventList;
  for(short i = 1; i <= vsoDocumentEventList->Count; i++)
  {
    Visio::IVEventPtr vsoEvent = vsoDocumentEventList->Item[i];

    if(vsoEvent == NULL || vsoEvent->Action != visActCodeRunAddon)
      continue;

    if(vsoEvent != NULL && _tcsicmp(vsoEvent->Target, _T(ADDON_NAME)) == 0)
    {
      if(vsoEvent->Event == visEvtCodeDocCreate)
        vsoDocumentCreateEvent = vsoEvent;
      else if(vsoEvent->Event == visEvtCodeDocOpen)
        vsoDocumentOpenEvent = vsoEvent;
    }
  }

  if(vsoDocumentCreateEvent == NULL)
    vsoDocumentCreateEvent = vsoDocumentEventList->Add(visEvtCodeDocCreate, visActCodeRunAddon, ADDON_NAME, ""); // event=1

  if(vsoDocumentOpenEvent == NULL)
    vsoDocumentOpenEvent = vsoDocumentEventList->Add(visEvtCodeDocOpen, visActCodeRunAddon, ADDON_NAME, ""); // event=2
}

CDocumentMonitor *CStudioAddon::GetDocumentMonitor(Visio::IVApplicationPtr vsoApp, Visio::IVDocumentPtr vsoDocument)
{
  CDocumentMonitor *pDocumentMonitor = NULL;

  TRACE("CStudioAddon::GetDocumentMonitor() called for " << vsoDocument->Name);
  long nID = vsoDocument->GetID();

  DocumentMonitorsMap::iterator documentIter = m_DocumentMonitors.find(nID);
  if (documentIter != m_DocumentMonitors.end())
  {
    // the document monitor already exists
    return documentIter->second;
  }

  try
  {
    Visio::IVEventPtr vsoEvent;
    Visio::IVEventListPtr vsoApplicationEventList;

    Visio::IVEventListPtr vsoDocumentEventList = vsoDocument->EventList;

    // create a sink object which point back to this class as the event handler
    if(m_pIAddonSink == NULL)
      CoCreateAddonSinkForHandler(0, this, &m_pIAddonSink);

    if(m_pIAddonSink)
    {
      _variant_t varSink = m_pIAddonSink;

      // If the add-on is not already listening to the marker event, 
      // create an event object for marker events.  Marker events are
      // sourced from the Application object. 
      if (m_vsoMarkerEvent == NULL)
      {
        vsoApplicationEventList = vsoApp->EventList;
        m_vsoMarkerEvent = vsoApplicationEventList->AddAdvise(visEvtApp|visEvtMarker, varSink, _T(""), _T(""));
      }

      // register key & mouse listening
      vsoApp->EventList->AddAdvise(Visio::visEvtCodeKeyDown, varSink, _T(""), _T("KeyDown"));
      vsoApp->EventList->AddAdvise(Visio::visEvtCodeKeyUp, varSink, _T(""), _T("KeyUp"));
      vsoApp->EventList->AddAdvise(Visio::visEvtCodeMouseMove, varSink, _T(""), _T("MouseMove"));
      vsoApp->EventList->AddAdvise(Visio::visEvtCodeMouseDown, varSink, _T(""), _T("MouseDown"));
      // Message enumeration events
      vsoApp->EventList->AddAdvise(Visio::visEvtCodeShapeBeforeTextEdit, varSink, _T(""), _T("BeforeTextEdit"));
      vsoApp->EventList->AddAdvise(Visio::visEvtCodeShapeExitTextEdit, varSink, _T(""), _T("AfterTextEdit"));
      vsoApp->EventList->AddAdvise(visEvtApp+visEvtNonePending, varSink, _T(""), _T("NonePending"));
      vsoApp->EventList->AddAdvise(visEvtDel+visEvtShape, varSink, _T(""), _T("ShapeDeleted"));

      vsoApp->EventList->AddAdvise(visEvtCodeWinSelChange, varSink, _T(""), _T("SelectionChanges"));
      vsoApp->EventList->AddAdvise(visEvtApp+visEvtIdle, varSink, _T(""), _T("VisioIsIdle"));


      // Create a monitor class to keep track of this document and the Events 
      // being monitored for this document.  
      pDocumentMonitor = new CDocumentMonitor(this, vsoApp, vsoDocument);
      pDocumentMonitor->InitMenu();
      pDocumentMonitor->InitToolbar();

      // register BeforeDocumentClose
      vsoEvent = vsoDocumentEventList->AddAdvise(visEvtBeforeDocumentClose, varSink, _T(""), _T(""));
      pDocumentMonitor->m_vsoBeforeDocumentClosedEvent = vsoEvent;
      // register PageAdded
      vsoEvent = vsoDocumentEventList->AddAdvise(visEvtPageAdded, varSink, _T(""), _T(""));
      pDocumentMonitor->m_vsoPageAddedEvent = vsoEvent;

      vsoDocumentEventList->AddAdvise(visEvtMod+visEvtText, varSink, _T(""), _T("TextChanged"));
      vsoDocumentEventList->AddAdvise(visEvtCellChanged, varSink, _T(""), _T("CellChanged"));
      vsoDocumentEventList->AddAdvise(visEvtConnectionsAdded, varSink, _T(""), _T("ConnectionsAdded"));
      vsoDocumentEventList->AddAdvise(visEvtConnectionsDeleted, varSink, _T(""), _T("ConnectionsDeleted"));

      // add this document to our monitored documents list
      m_DocumentMonitors.insert(DocumentMonitorsMap::value_type(nID, pDocumentMonitor));

      return pDocumentMonitor;
    }
  }
  catch (_com_error &)  // catch exceptions thrown by Visio
  {
    delete pDocumentMonitor;
  }

  return NULL;
}

void CStudioAddon::StopDocumentMonitor(Visio::IVApplicationPtr vsoApp, Visio::IVDocumentPtr vsoDocument)
{
  long nID = vsoDocument->GetID();

  // the document is closing so stop monitoring its events
  DocumentMonitorsMap::iterator documentIter = m_DocumentMonitors.find(nID);
  if (documentIter != m_DocumentMonitors.end())
  {
    delete documentIter->second;
    m_DocumentMonitors.erase(documentIter);    
  }
}

void CStudioAddon::SetState(Visio::IVApplicationPtr vsoApp, TAddonState state, const TCHAR* message)
{
  m_states[vsoApp] = state;
  if (message != NULL) {
    //MessageBox(GetActiveWindow(), message, _T("Status bar"), MB_OK); // FIXME: set status bar text
  }
}

void CStudioAddon::ResetState(Visio::IVApplicationPtr vsoApp)
{
  m_states[vsoApp] = STATE_INIT;
  /* // does not work:
  CDocumentMonitor* docMon = GetDocumentMonitor(vsoApp, vsoApp->ActiveDocument);
  Visio::IVUIObjectPtr toolbars = docMon->GetMostCustomToolbars();
  toolbars->StatusBars->ItemAtID[Visio::visUIObjSetDrawing]->StatusBarItems->Item[1]->Caption = _T("Wheee!");
  */
  //MessageBox(GetActiveWindow(), _T("Reseting status bar"), _T("Status bar"), MB_OK); // FIXME: reset status bar
}

// $Id: addon.cpp 1019 2011-01-04 07:15:39Z xpekarc $
