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
 * $Id: document.cpp 1070 2011-03-29 07:27:45Z madzin $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "addon.h"
#include "document.h"
#include "errors.h"
#include "extract.h"
#include "visualize.h"
#include "finddlg.h"

//Dialog classes
#include "messagesequencedlg.h"
#include "enumerationDlg.h"
#include "GlobalSettingsDlg.h"
#include "beautifySettingsDlg.h"
#include "importSettingsDlg.h"

#include "messageSnapping.h"
#include "messageJump.h"

#include <fstream>

#include "data/msc.h"
#include "data/beautify/beautify.h"
#include "data/transformer.h"
#include "data/beautify/export.h"

// Include libraries from the Windows Template Library (WTL).
// http://wtl.sourceforge.net
// Install WTL81 under e.g. "C:\Program Files\Microsoft Visual Studio 9.0\VC\"
#include <atldlgs.h>

#include <htmlhelp.h>

Visio::IVPagePtr GetOrCreatePage(Visio::IVPagesPtr pages, const TCHAR* name)
{
  Visio::IVPagePtr page;

  // search for an existing page of the given name
  for(int i = 1; i <= pages->Count; i++)
  {
    page = pages->Item[i];
    if(_tcsicmp(page->Name, name) == 0)
      return page;
  }

  // create a new page
  page = pages->Add();
  page->Name = name;

  return page;
}

CDocumentMonitor::CDocumentMonitor(CStudioAddon *addon,
  Visio::IVApplicationPtr vsoApp, Visio::IVDocumentPtr vsoDocument)
{
  m_addon = addon;
  m_vsoApp = vsoApp;
  m_vsoDocument = vsoDocument;

  m_reportVisible = false;
  m_reportView = new CReportView(this);

  beautifyIndex = -1;
  tightenTimeIndex = -1;

  m_configProvider = new RegistryConfigProvider();

  LoadModulesFromRegistry(HKEY_LOCAL_MACHINE);
  LoadModulesFromRegistry(HKEY_CURRENT_USER);
}

CDocumentMonitor::~CDocumentMonitor()
{
  // terminate all module functions
  m_formatters.clear();
  m_checkers.clear();
  m_membership = SearcherPtr(); // NULL
  m_simulator = SimulatorPtr(); // NULL
  m_transformers.clear();
  // close extension modules
  for(std::vector<HINSTANCE>::const_iterator mpos = m_open_modules.begin();
    mpos != m_open_modules.end(); mpos++)
  {
    FreeLibrary(*mpos);
  }

  delete m_configProvider;

  if (m_vsoBeforeDocumentClosedEvent != NULL)
    m_vsoBeforeDocumentClosedEvent->Delete();

  if (m_vsoPageAddedEvent != NULL)
    m_vsoPageAddedEvent->Delete();
}

int CDocumentMonitor::LoadModulesFromRegistry(HKEY hKey)
{
  // step 1: read the module directory path
  HKEY hPathKey;
  if(RegOpenKeyEx(hKey, SCSTUDIO_REGISTRY_ROOT,
    0, KEY_READ, &hPathKey) != ERROR_SUCCESS)
  {
    return 1;
  }

  TCHAR oldPathName[MAX_PATH];
  DWORD oldPathLength = MAX_PATH;
  // store current directory
  GetCurrentDirectory(oldPathLength, oldPathName);

  DWORD newPathType;
  TCHAR newPathName[MAX_PATH];
  DWORD newPathLength = MAX_PATH;
  // set the current directory to the registry value
  // this is necessary to load DLL dependencies like scpseudocode.dll
  if(RegQueryValueEx(hPathKey, _T("ModulesPath"),
    NULL, &newPathType, (LPBYTE)newPathName, &newPathLength) == ERROR_SUCCESS)
  {
    SetCurrentDirectory(newPathName);
  }

  RegCloseKey(hPathKey);

  // step 2: load the modules
  std::list<std::wstring> module_names =
    GetRegistryStrings(SCSTUDIO_REGISTRY_ROOT _T("\\Modules"));

  for(std::list<std::wstring>::const_iterator npos = module_names.begin();
    npos != module_names.end(); npos++)
  {
    if(npos->empty())
      continue;

    // open the extension module
    HINSTANCE module = LoadLibrary(npos->c_str());
    if(module == NULL)
    {
      DisplayException(m_vsoApp, basic_stringize<TCHAR>()
        << _T("Cannot load module '") << *npos << _T("'"), MB_OK | MB_ICONEXCLAMATION);
      continue;
    }

    FInitFormatters init_formatters =
      (FInitFormatters)GetProcAddress(module, "init_formatters");
    FInitCheckers init_checkers =
      (FInitCheckers)GetProcAddress(module, "init_checkers");
    FInitMembership init_membership =
      (FInitMembership)GetProcAddress(module, "init_membership");
    FInitSimulator init_simulator =
      (FInitSimulator)GetProcAddress(module, "init_simulator");
    FInitTransformers init_transformers =
      (FInitTransformers)GetProcAddress(module, "init_transformers");

    if(init_formatters == NULL && init_checkers == NULL
      && init_membership == NULL && init_simulator == NULL && init_transformers == NULL)
    {
      DisplayException(m_vsoApp, basic_stringize<TCHAR>()
        << _T("Cannot open module '") << *npos << _T("'"), MB_OK | MB_ICONEXCLAMATION);
      // this is not our module
      FreeLibrary(module);
      continue;
    }

    if(init_formatters != NULL)
    {
      Formatter **formatters = init_formatters();
      // append new formatters to the list
      for(Formatter **fpos = formatters; *fpos != NULL; fpos++)
      {
        boost::shared_ptr<Formatter> formatter(*fpos);
        formatter->set_printer(m_reportView);
        m_formatters.push_back(formatter);
      }
      // delete the array
      // note: the formatters are managed by the boost:shared_ptr
      delete[] formatters;
    }

    if(init_checkers != NULL)
    {
      Checker **checkers = init_checkers();
      // append new checkers to the list
      for(Checker **fpos = checkers; *fpos != NULL; fpos++)
      {
        boost::shared_ptr<Checker> checker(*fpos);
        m_checkers.push_back(checker);
      }
      // delete the array
      // note: the checkers are managed by the boost:shared_ptr
      delete[] checkers;
    }

    if(init_membership != NULL)
    {
      m_membership = boost::shared_ptr<Searcher>(init_membership());
      m_membership->set_printer(m_reportView);
    }

    if(init_simulator != NULL)
    {
      m_simulator = boost::shared_ptr<Simulator>(init_simulator());
      m_simulator->set_config_provider(m_configProvider);
      m_simulator->set_printer(m_reportView);

      SYSTEM_INFO info;
      GetSystemInfo(&info);
      m_simulator->set_process_count(info.dwNumberOfProcessors);

      m_simulator->set_listener(this);
    }

    if(init_transformers != NULL)
    {
      Transformer **transformers = init_transformers();
      // append new transformers to the list
      for(Transformer **tpos = transformers; *tpos != NULL; tpos++)
      {
        boost::shared_ptr<Transformer> transformer(*tpos);
        transformer->set_config_provider(m_configProvider);
        transformer->set_printer(m_reportView);
        m_transformers.push_back(transformer);
      }
      // delete the array
      // note: the transformers are managed by the boost:shared_ptr
      delete[] transformers;
    }

    // keep the pointer to properly unload the module
    m_open_modules.push_back(module);
  }

  // put back the old value
  SetCurrentDirectory(oldPathName);

  return 0;
}

VAORC CDocumentMonitor::OnDropShape(int iDocumentIndex, int iPageIndex, _bstr_t sShapeU)
{
  m_addon->SetOnDropShapeState(true);
  // ignore events when undoing or redoing
  if(m_vsoApp->IsUndoingOrRedoing == VARIANT_TRUE)
    return VAORC_SUCCESS;

  Visio::IVDocumentPtr vsoDocument = m_vsoApp->Documents->Item[iDocumentIndex];
  Visio::IVShapePtr vsoShape = vsoDocument->Pages->Item[iPageIndex]->Shapes->ItemU[sShapeU];

  switch(get_shape_type(vsoShape))
  {
    case ST_BMSC_INSTANCE:
      if(vsoShape->Text.length() == 0)
        vsoShape->Text = _T("NAME");
      break;

    case ST_BMSC_MESSAGE:
    case ST_BMSC_MESSAGE_LOST:
    case ST_BMSC_MESSAGE_FOUND:
      if(vsoShape->Text.length() == 0)
        vsoShape->Text = _T("NAME");
      //Message flip
      if(!vsoShape->GetCellExists(_T("Actions.flipDirection"),0))
      {
        vsoShape->AddNamedRow(visSectionAction,_T("flipDirection"),visTagDefault);
        vsoShape->Cells["Actions.flipDirection.Action"]->FormulaU = _T("=RUNADDONWARGS(\"Sequence Chart Studio\",\"/event=105\")");
        vsoShape->Cells["Actions.flipDirection.Menu"]->FormulaU = _T("=\"Flip message direction\"");
      }
      //Message snapping
      if(CMessageSnapping::isEnabled() && (m_vsoApp->ActiveWindow->Selection->Count == 1)) //Check whether snapping is enabled
      {
        double posX = CShapeUtils::getShapeCell(vsoShape,"PinX");
        double posY = CShapeUtils::getShapeCell(vsoShape,"PinY");
        if(m_addon->GetCtrlKeyDown())
          posX += CPageUtils::ConvertUnits(m_vsoApp, m_addon->GetRelMousePosX(), 0, visPageUnits);

        CMessageSnapping::snap(vsoShape, posX, posY, CMessageSnapping::getSnapType());
      }
      //Message numbering
      autoEnumerate(vsoShape);
      break;

    case ST_COMMENT:
    case ST_TEXT:
      if(vsoShape->Text.length() == 0)
        vsoShape->Text = _T("NAME");
      break;

    case ST_HMSC_REFERENCE:
      if(vsoShape->Text.length() == 0)
        vsoShape->Text = _T("NAME");
      break;

    case ST_TIME_INTERVAL:
    case ST_TIME_DIRECTED:
      if(vsoShape->Text.length() == 0)
        vsoShape->Text = _T("[0,inf)");
      break;

    case ST_TIME_ABSOLUTE:
      if(vsoShape->Text.length() == 0)
        vsoShape->Text = _T("@0");
      break;
  }

  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnOpenReference(int iDocumentIndex, int iPageIndex, _bstr_t sShapeU)
{
  Visio::IVPagePtr page;

  Visio::IVDocumentPtr vsoDocument = m_vsoApp->Documents->Item[iDocumentIndex];
  Visio::IVShapePtr vsoShape = vsoDocument->Pages->Item[iPageIndex]->Shapes->ItemU[sShapeU];

  page = GetOrCreatePage(vsoDocument->Pages, vsoShape->Text);
  // switch to the found page
  m_vsoApp->ActiveWindow->Page = (IDispatch *)page;

  // if the page is empty, set zoom to 100%
  if (page->Shapes->Count == 0)
    m_vsoApp->ActiveWindow->Zoom = 1;

  return VAORC_SUCCESS;
}

Visio::IVUIObjectPtr CDocumentMonitor::GetMostCustomMenus()
{
  if(m_vsoDocument != NULL)
  {
    Visio::IVUIObjectPtr docMenus = m_vsoDocument->CustomMenus;
    if(docMenus != NULL)
      return docMenus->Clone; // document-specific menus are defined
  }

  Visio::IVUIObjectPtr appMenus = m_vsoApp->CustomMenus;
  if(appMenus != NULL)
    return appMenus->Clone; // application custom menus are defined

  return m_vsoApp->BuiltInMenus; // gets a clone
}

Visio::IVUIObjectPtr CDocumentMonitor::GetMostCustomToolbars()
{
  if(m_vsoDocument != NULL)
  {
    Visio::IVUIObjectPtr docToolbars = m_vsoDocument->CustomToolbars;
    if(docToolbars != NULL)
      return docToolbars->Clone;
  }

  Visio::IVUIObjectPtr appToolbars = m_vsoApp->CustomToolbars;
  if(appToolbars != NULL)
    return appToolbars->Clone;
  
  return m_vsoApp->GetBuiltInToolbars(0);
}

void CDocumentMonitor::InitMenu()
{
  Visio::IVUIObjectPtr vsoMenus = GetMostCustomMenus();

  // step 1: create custom menu
  Visio::IVMenuSetPtr menuSet = vsoMenus->MenuSets->ItemAtID[Visio::visUIObjSetDrawing];

  Visio::IVMenuPtr menu = menuSet->Menus->AddAt(5);
  menu->Caption = "&SCStudio";
  // many functions are not working in OLE window: repaint, import, error display
  // disable verification functions when opened from OLE window
  menu->Enabled = !m_vsoDocument->InPlace;

  std::basic_string<TCHAR> vslFileName = GetVisioModuleFileName();
  std::basic_string<TCHAR> vslIconFile;

  m_reportMenuItem = menu->MenuItems->Add();
  m_reportMenuItem->Caption = "Verification &Report";
  m_reportMenuItem->AddOnName = ADDON_NAME;
  m_reportMenuItem->AddOnArgs = stringize() << L"/event=" << MENU_WINDOWS_REPORTER;

  Visio::IVMenuItemPtr itemDrawing = menu->MenuItems->Add();
  itemDrawing->Caption = "&Drawing";
  itemDrawing->BeginGroup = true;
  itemDrawing->CmdNum = Visio::visCmdHierarchical;

  Visio::IVMenuItemPtr itemImport = itemDrawing->MenuItems->Add();
  itemImport->Caption = "&Import...";
  itemImport->AddOnName = ADDON_NAME;
  itemImport->AddOnArgs = stringize() << L"/event=" << MENU_IMPORT;
  // enable only if some formatters are available
  itemImport->Enabled = m_formatters.size() > 0;


  Visio::IVMenuItemPtr itemExport = itemDrawing->MenuItems->Add();
  itemExport->Caption = "&Export...";
  itemExport->AddOnName = ADDON_NAME;
  itemExport->AddOnArgs = stringize() << L"/event=" << MENU_EXPORT;
  // enable only if some formatters are available
  itemExport->Enabled = m_formatters.size() > 0;

  Visio::IVMenuItemPtr itemAddInstances = itemDrawing->MenuItems->Add();
  itemAddInstances->Caption = "&Add Instances...";
  itemAddInstances->AddOnName = ADDON_NAME;
  itemAddInstances->AddOnArgs = stringize() << L"/event=" << MENU_ADD_INSTANCES;
  itemAddInstances->BeginGroup = true;
  vslIconFile = vslFileName+_T(",19");
  itemAddInstances->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemMessageSequence = itemDrawing->MenuItems->Add();
  itemMessageSequence->Caption = "&Message Sequence...";
  itemMessageSequence->AddOnName = ADDON_NAME;
  itemMessageSequence->AddOnArgs = stringize() << L"/event=" << MENU_MESSAGE_SEQUENCE;
  vslIconFile = vslFileName+_T(",16");
  itemMessageSequence->IconFileName(vslIconFile.c_str());  

  Visio::IVMenuItemPtr itemFlipMsgDirection = itemDrawing->MenuItems->Add();
  itemFlipMsgDirection->Caption = "&Flip message Direction";
  itemFlipMsgDirection->AddOnName = ADDON_NAME;
  itemFlipMsgDirection->AddOnArgs = stringize() << L"/event=" << MENU_FLIP_MESSAGE_DIRECTION;
  vslIconFile = vslFileName+_T(",17");
  itemFlipMsgDirection->IconFileName(vslIconFile.c_str());

  itemMessageEnumerating = itemDrawing->MenuItems->Add();
  itemMessageEnumerating->Caption = "Message &numbering";
  //itemMessageEnumerating->BeginGroup = true;
  itemMessageEnumerating->CmdNum = Visio::visCmdHierarchical;

  Visio::IVMenuItemPtr itemSelect = itemDrawing->MenuItems->Add();
  itemSelect->Caption = "&Select";
  itemSelect->CmdNum = Visio::visCmdHierarchical;

  Visio::IVMenuItemPtr itemSelectInstances = itemSelect->MenuItems->Add();
  itemSelectInstances->Caption = "All &Instances";
  itemSelectInstances->AddOnName = ADDON_NAME;
  itemSelectInstances->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_ALL_INSTANCES;
  vslIconFile = vslFileName+_T(",2");
  itemSelectInstances->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemSelectMessages = itemSelect->MenuItems->Add();
  itemSelectMessages->Caption = "All &Messages";
  itemSelectMessages->AddOnName = ADDON_NAME;
  itemSelectMessages->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_ALL_MESSAGES;
  vslIconFile = vslFileName+_T(",3");
  itemSelectMessages->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemRepaint = itemDrawing->MenuItems->Add();
  itemRepaint->Caption = "&Repaint";
  itemRepaint->AddOnName = ADDON_NAME;
  itemRepaint->AddOnArgs = stringize() << L"/event=" << MENU_REPAINT;
  itemRepaint->BeginGroup = true;

  for(TransformerPtrList::const_iterator tpos = m_transformers.begin();
    tpos != m_transformers.end(); tpos++)
  {
    Visio::IVMenuItemPtr menuItem3x = itemDrawing->MenuItems->Add();
    menuItem3x->Caption = (*tpos)->get_name().c_str();
    menuItem3x->AddOnName = ADDON_NAME;
    // note: event numbers 300-399 are reserved for transformers
    menuItem3x->AddOnArgs =
      stringize() << L"/event=" << 300+(tpos-m_transformers.begin());
    //Make asociation with accelerator (Do it for every item you want shortcut to) and toolbar icon index
    vslIconFile = L"";
    if(!strcmp(menuItem3x->Caption,"Beautify"))
    {
      beautifyCmdNum = menuItem3x->CmdNum;
      beautifyIndex = tpos-m_transformers.begin();      
      vslIconFile = vslFileName+_T(",15");
    }
    else if(!strcmp(menuItem3x->Caption,"Tighten Time"))
    {
      tightenTimeIndex = tpos-m_transformers.begin();
      vslIconFile = vslFileName+_T(",18");
    }
    menuItem3x->IconFileName(vslIconFile.c_str());
  }

  Visio::IVMenuItemPtr itemEnableEnumerating = itemMessageEnumerating->MenuItems->Add();
  itemEnableEnumerating->Caption = "Message &numbering";
  itemEnableEnumerating->AddOnName = ADDON_NAME;
  itemEnableEnumerating->AddOnArgs = stringize() << L"/event=" << MENU_ENABLE_MESSAGE_ENUMERATION;
  vslIconFile = vslFileName+_T(",8");
  itemEnableEnumerating->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemDisableEnumerating = itemMessageEnumerating->MenuItems->Add();
  itemDisableEnumerating->Caption = "&Delete numbering";
  itemDisableEnumerating->AddOnName = ADDON_NAME;
  itemDisableEnumerating->AddOnArgs = stringize() << L"/event=" << MENU_DISABLE_MESSAGE_ENUMERATION;
  vslIconFile = vslFileName+_T(",9");
  itemDisableEnumerating->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemSelectNumberingGroup = itemMessageEnumerating->MenuItems->Add();
  itemSelectNumberingGroup->Caption = "&Select numbering group";
  itemSelectNumberingGroup->AddOnName = ADDON_NAME;
  itemSelectNumberingGroup->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_NUMBERED_GROUP;
  vslIconFile = vslFileName+_T(",14");
  itemSelectNumberingGroup->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemFindFlow = menu->MenuItems->Add();
  itemFindFlow->Caption = "&Find Flow...";
  itemFindFlow->AddOnName = ADDON_NAME;
  itemFindFlow->AddOnArgs = stringize() << L"/event=" << MENU_FIND_FLOW;
  // enable only if membership is available
  itemFindFlow->Enabled = m_membership != NULL;
  vslIconFile = vslFileName+_T(",1");
  itemFindFlow->IconFileName(vslIconFile.c_str());

  m_simulationStartMenuItem = menu->MenuItems->Add();
  m_simulationStartMenuItem->Caption = "&Start Simulation";
  m_simulationStartMenuItem->AddOnName = ADDON_NAME;
  m_simulationStartMenuItem->AddOnArgs = stringize() << L"/event=" << MENU_SIMULATION_START;
  // enable only if simulator is available
  m_simulationStartMenuItem->Enabled = m_simulator != NULL;
  vslIconFile = vslFileName+_T(",4");
  m_simulationStartMenuItem->IconFileName(vslIconFile.c_str());

  m_simulationStopMenuItem = menu->MenuItems->Add();
  m_simulationStopMenuItem->Caption = "&Stop Simulation";
  m_simulationStopMenuItem->AddOnName = ADDON_NAME;
  m_simulationStopMenuItem->AddOnArgs = stringize() << L"/event=" << MENU_SIMULATION_STOP;
  m_simulationStopMenuItem->Visible = false;
  // enable only if simulator is available
  m_simulationStopMenuItem->Enabled = m_simulator != NULL;
  vslIconFile = vslFileName+_T(",5");
  m_simulationStopMenuItem->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemSimulationOptions = menu->MenuItems->Add();
  itemSimulationOptions->Caption = "Simulator O&ptions...";
  itemSimulationOptions->AddOnName = ADDON_NAME;
  itemSimulationOptions->AddOnArgs = stringize() << L"/event=" << MENU_SIMULATOR_OPTIONS;

  Visio::IVMenuItemPtr itemVerify = menu->MenuItems->Add();
  itemVerify->Caption = "&Verify";
  itemVerify->AddOnName = ADDON_NAME;
  itemVerify->AddOnArgs = stringize() << L"/event=" << MENU_VERIFY;
  itemVerify->BeginGroup = true;
  vslIconFile = vslFileName+_T(",0");
  itemVerify->IconFileName(vslIconFile.c_str());

  Visio::IVMenuItemPtr itemCheckOptions = menu->MenuItems->Add();
  itemCheckOptions->Caption = "&Options...";
  itemCheckOptions->AddOnName = ADDON_NAME;
  itemCheckOptions->AddOnArgs = stringize() << L"/event=" << MENU_CHECK_OPTIONS;

  Visio::IVMenuItemPtr globalSettings = itemDrawing->MenuItems->Add();
  globalSettings->Caption = "&Settings...";
  globalSettings->AddOnName = ADDON_NAME;
  globalSettings->BeginGroup = true;
  globalSettings->AddOnArgs = stringize() << L"/event=" << MENU_GLOBAL_SETTINGS;

	Visio::IVMenuItemPtr importSettings = itemDrawing->MenuItems->Add();
  importSettings->Caption = "&Import settings...";
  importSettings->AddOnName = ADDON_NAME;
  importSettings->BeginGroup = true;
  importSettings->AddOnArgs = stringize() << L"/event=" << MENU_IMPORT_SETTINGS;

  Visio::IVMenuItemPtr beautifySettings = itemDrawing->MenuItems->Add();
  beautifySettings->Caption = "&Beautify settings...";
  beautifySettings->AddOnName = ADDON_NAME;
  beautifySettings->BeginGroup = true;
  beautifySettings->AddOnArgs = stringize() << L"/event=" << MENU_BEAUTIFY_SETTINGS;

	

  // step 2: create accelerators
  // note: the accelerators must be created in this custom menu set
  // note: the accelerators are bound with menu items via CmdNum property
  Visio::IVAccelTablePtr accelTable = vsoMenus->AccelTables->ItemAtID[Visio::visUIObjSetDrawing];
  //Select Instances
  Visio::IVAccelItemPtr accelItemSelectInstances = accelTable->AccelItems->Add();
  accelItemSelectInstances->CmdNum = itemSelectInstances->CmdNum;
  accelItemSelectInstances->Key = 'I';
  accelItemSelectInstances->Control = true;
  accelItemSelectInstances->Alt = true;
  //Select Messages
  Visio::IVAccelItemPtr accelItemSelectMessages = accelTable->AccelItems->Add();
  accelItemSelectMessages->CmdNum = itemSelectMessages->CmdNum;
  accelItemSelectMessages->Key = 'M';
  accelItemSelectMessages->Control = true;
  accelItemSelectMessages->Alt = true;
  //Add Instances
  Visio::IVAccelItemPtr accelItemAddInstances = accelTable->AccelItems->Add();
  accelItemAddInstances->CmdNum = itemAddInstances->CmdNum;
  accelItemAddInstances->Key = 'F';
  accelItemAddInstances->Control = true;
  accelItemAddInstances->Alt = true;
  //Message Sequence
  Visio::IVAccelItemPtr accelItemMessageSequence = accelTable->AccelItems->Add();
  accelItemMessageSequence->CmdNum = itemMessageSequence->CmdNum;
  accelItemMessageSequence->Key = 'S';
  accelItemMessageSequence->Control = true;
  accelItemMessageSequence->Alt = true;
  //Enable message numbering
  accelItemMessageEnumeration = accelTable->AccelItems->Add();
  accelItemMessageEnumeration->CmdNum = itemEnableEnumerating->CmdNum;
  accelItemMessageEnumeration->Key = 'E';
  accelItemMessageEnumeration->Control = true;
  accelItemMessageEnumeration->Alt = true;
  //Disable message numbering
  accelItemMessageEnumerationDisable = accelTable->AccelItems->Add();
  accelItemMessageEnumerationDisable->CmdNum = itemDisableEnumerating->CmdNum;
  accelItemMessageEnumerationDisable->Key = 'D';
  accelItemMessageEnumerationDisable->Control = true;
  accelItemMessageEnumerationDisable->Alt = true;
  //Select numbering group
  Visio::IVAccelItemPtr accelItemSelectNumberingGroup = accelTable->AccelItems->Add();
  accelItemSelectNumberingGroup->CmdNum = itemSelectNumberingGroup->CmdNum;
  accelItemSelectNumberingGroup->Key = 'N';
  accelItemSelectNumberingGroup->Control = true;
  accelItemSelectNumberingGroup->Alt = true;
  //Flip message direction
  Visio::IVAccelItemPtr accelItemFlipMsgDirection = accelTable->AccelItems->Add();
  accelItemFlipMsgDirection->CmdNum = itemFlipMsgDirection->CmdNum;
  accelItemFlipMsgDirection->Key = 0x08;
  accelItemFlipMsgDirection->Control = false;
  accelItemFlipMsgDirection->Alt = false;
  //Verify
  Visio::IVAccelItemPtr accelItemVerify = accelTable->AccelItems->Add();
  accelItemVerify->CmdNum = itemVerify->CmdNum;
  accelItemVerify->Key = 'C';
  accelItemVerify->Control = true;
  accelItemVerify->Alt = true;
  //Transforms shortcuts
  if(beautifyIndex >= 0)
  {
    Visio::IVAccelItemPtr accelItemBeutify = accelTable->AccelItems->Add();
    accelItemBeutify->CmdNum = beautifyCmdNum;
    accelItemBeutify->Key = 'B';
    accelItemBeutify->Control = true;
    accelItemBeutify->Alt = true;
  }

  m_vsoDocument->SetCustomMenus(vsoMenus);
}

void CDocumentMonitor::InitToolbar()
{
  Visio::IVUIObjectPtr vsoToolbars = GetMostCustomToolbars();
  Visio::IVToolbarSetPtr toolbarSet = vsoToolbars->ToolbarSets->ItemAtID[Visio::visUIObjSetDrawing];
  m_toolbar = toolbarSet->Toolbars->Add();

  m_toolbar->Caption = "Sequence Chart";
  // many functions are not working in OLE window: repaint, import, error display
  // disable verification functions when opened from OLE window
  m_toolbar->Enabled = !m_vsoDocument->InPlace;

  std::basic_string<TCHAR> vslFileName = GetVisioModuleFileName();
  std::basic_string<TCHAR> vslIconFile;

  m_selInstancesToolbarItem = m_toolbar->ToolbarItems->Add();
  m_selInstancesToolbarItem->Caption = "Select All Instances";
  m_selInstancesToolbarItem->AddOnName = ADDON_NAME;
  m_selInstancesToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_ALL_INSTANCES;
  vslIconFile = vslFileName+_T(",2");
  m_selInstancesToolbarItem->IconFileName(vslIconFile.c_str());
  m_selInstancesToolbarItem->BeginGroup = true;

  m_selMessagesToolbarItem = m_toolbar->ToolbarItems->Add();
  m_selMessagesToolbarItem->Caption = "Select All Messages";
  m_selMessagesToolbarItem->AddOnName = ADDON_NAME;
  m_selMessagesToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_ALL_MESSAGES;
  vslIconFile = vslFileName+_T(",3");
  m_selMessagesToolbarItem->IconFileName(vslIconFile.c_str());

  m_selAddInstancesToolbarItem = m_toolbar->ToolbarItems->Add();
  m_selAddInstancesToolbarItem->Caption = "Add All Instances To Selection";
  m_selAddInstancesToolbarItem->AddOnName = ADDON_NAME;
  m_selAddInstancesToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_ADD_ALL_INSTANCES;
  vslIconFile = vslFileName+_T(",6");
  m_selAddInstancesToolbarItem->IconFileName(vslIconFile.c_str());
  m_selAddInstancesToolbarItem->Visible = false;

  m_selAddMessagesToolbarItem = m_toolbar->ToolbarItems->Add();
  m_selAddMessagesToolbarItem->Caption = "Add All Messages To Selection";
  m_selAddMessagesToolbarItem->AddOnName = ADDON_NAME;
  m_selAddMessagesToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_ADD_ALL_MESSAGES;
  vslIconFile = vslFileName+_T(",7");
  m_selAddMessagesToolbarItem->IconFileName(vslIconFile.c_str());
  m_selAddMessagesToolbarItem->Visible = false;

  m_selAddMessagesToolbarItem = m_toolbar->ToolbarItems->Add();
  m_selAddMessagesToolbarItem->Caption = "Add All Messages To Selection";
  m_selAddMessagesToolbarItem->AddOnName = ADDON_NAME;
  m_selAddMessagesToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_ADD_ALL_MESSAGES;
  vslIconFile = vslFileName+_T(",7");
  m_selAddMessagesToolbarItem->IconFileName(vslIconFile.c_str());
  m_selAddMessagesToolbarItem->Visible = false;

  //Message Jumping Left
  m_msgJumpLeftToolbarItem = m_toolbar->ToolbarItems->Add();
  m_msgJumpLeftToolbarItem->BeginGroup = true;
  m_msgJumpLeftToolbarItem->Caption = "Jump Left";
  m_msgJumpLeftToolbarItem->AddOnName = ADDON_NAME;
  m_msgJumpLeftToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_MESSAGE_JUMP_LEFT;
  vslIconFile = vslFileName+_T(",10");
  m_msgJumpLeftToolbarItem->IconFileName(vslIconFile.c_str());
  //Message Jumping Right
  m_msgJumpRightToolbarItem = m_toolbar->ToolbarItems->Add();
  m_msgJumpRightToolbarItem->Caption = "Jump Right";
  m_msgJumpRightToolbarItem->AddOnName = ADDON_NAME;
  m_msgJumpRightToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_MESSAGE_JUMP_RIGHT;
  vslIconFile = vslFileName+_T(",11");
  m_msgJumpRightToolbarItem->IconFileName(vslIconFile.c_str());

  //Message Jumping Left Copy
  m_msgJumpLeftCopyToolbarItem = m_toolbar->ToolbarItems->Add();
  m_msgJumpLeftCopyToolbarItem->BeginGroup = true;
  m_msgJumpLeftCopyToolbarItem->Caption = "Jump Left As Copy";
  m_msgJumpLeftCopyToolbarItem->AddOnName = ADDON_NAME;
  m_msgJumpLeftCopyToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_MESSAGE_JUMP_LEFT_COPY;
  m_msgJumpLeftCopyToolbarItem->Visible = false;
  vslIconFile = vslFileName+_T(",12");
  m_msgJumpLeftCopyToolbarItem->IconFileName(vslIconFile.c_str());
  //Message Jumping Right Copy
  m_msgJumpRightCopyToolbarItem = m_toolbar->ToolbarItems->Add();
  m_msgJumpRightCopyToolbarItem->Caption = "Jump Right As Copy";
  m_msgJumpRightCopyToolbarItem->AddOnName = ADDON_NAME;
  m_msgJumpRightCopyToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_MESSAGE_JUMP_RIGHT_COPY;
  m_msgJumpRightCopyToolbarItem->Visible = false;
  vslIconFile = vslFileName+_T(",13");
  m_msgJumpRightCopyToolbarItem->IconFileName(vslIconFile.c_str());

  Visio::IVToolbarItemPtr itemFindFlow = m_toolbar->ToolbarItems->Add();
  itemFindFlow->BeginGroup = true;
  itemFindFlow->Caption = "Find Flow";
  itemFindFlow->AddOnName = ADDON_NAME;
  itemFindFlow->AddOnArgs = stringize() << L"/event=" << MENU_FIND_FLOW;
  // enable only if membership is available
  itemFindFlow->Enabled = m_membership != NULL;
  vslIconFile = vslFileName+_T(",1");
  itemFindFlow->IconFileName(vslIconFile.c_str());

  m_simulationStartToolbarItem = m_toolbar->ToolbarItems->Add();
  m_simulationStartToolbarItem->Caption = "Start Simulation";
  m_simulationStartToolbarItem->AddOnName = ADDON_NAME;
  m_simulationStartToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_SIMULATION_START;
  // enable only if simulator is available
  m_simulationStartToolbarItem->Enabled = m_simulator != NULL;
  vslIconFile = vslFileName+_T(",4");
  m_simulationStartToolbarItem->IconFileName(vslIconFile.c_str());

  m_simulationStopToolbarItem = m_toolbar->ToolbarItems->Add();
  m_simulationStopToolbarItem->Caption = "Stop Simulation";
  m_simulationStopToolbarItem->AddOnName = ADDON_NAME;
  m_simulationStopToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_SIMULATION_STOP;
  m_simulationStopToolbarItem->Visible = false;
  // enable only if simulator is available
  m_simulationStopToolbarItem->Enabled = m_simulator != NULL;
  vslIconFile = vslFileName+_T(",5");
  m_simulationStopToolbarItem->IconFileName(vslIconFile.c_str());

  m_enableMessageEnumerationToolbarItem = m_toolbar->ToolbarItems->Add();
  m_enableMessageEnumerationToolbarItem->BeginGroup = true;
  m_enableMessageEnumerationToolbarItem->Caption = "Message Numbering";
  m_enableMessageEnumerationToolbarItem->AddOnName = ADDON_NAME;
  m_enableMessageEnumerationToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_ENABLE_MESSAGE_ENUMERATION;
  m_enableMessageEnumerationToolbarItem->Visible = true;
  m_enableMessageEnumerationToolbarItem->Enabled = true;
  vslIconFile = vslFileName+_T(",8");
  m_enableMessageEnumerationToolbarItem->IconFileName(vslIconFile.c_str());

  m_disableMessageEnumerationToolbarItem = m_toolbar->ToolbarItems->Add();
  m_disableMessageEnumerationToolbarItem->Caption = "Delete Numbering";
  m_disableMessageEnumerationToolbarItem->AddOnName = ADDON_NAME;
  m_disableMessageEnumerationToolbarItem->AddOnArgs = stringize() << L"/event=" << MENU_DISABLE_MESSAGE_ENUMERATION;
  m_disableMessageEnumerationToolbarItem->Visible = true;
  m_disableMessageEnumerationToolbarItem->Enabled = true;
  vslIconFile = vslFileName+_T(",9");
  m_disableMessageEnumerationToolbarItem->IconFileName(vslIconFile.c_str());

  Visio::IVToolbarItemPtr selectNumberingGroup = m_toolbar->ToolbarItems->Add();
  selectNumberingGroup->Caption = "Select Numbering Group";
  selectNumberingGroup->AddOnName = ADDON_NAME;
  selectNumberingGroup->AddOnArgs = stringize() << L"/event=" << MENU_SELECT_NUMBERED_GROUP;
  selectNumberingGroup->Visible = true;
  selectNumberingGroup->Enabled = true;
  vslIconFile = vslFileName+_T(",14");
  selectNumberingGroup->IconFileName(vslIconFile.c_str());
  //Beautify
  if(beautifyIndex >= 0)
  {
    Visio::IVToolbarItemPtr itemBeautify = m_toolbar->ToolbarItems->Add();
    itemBeautify->Caption = "Beautify";
    itemBeautify->AddOnName = ADDON_NAME;
    itemBeautify->AddOnArgs = stringize() << L"/event=" << 300 + beautifyIndex;
    vslIconFile = vslFileName+_T(",15");
    itemBeautify->IconFileName(vslIconFile.c_str());
    itemBeautify->BeginGroup = true;
  }
  //Tighten time
  if(tightenTimeIndex >= 0)
  {
    Visio::IVToolbarItemPtr itemTightenTime = m_toolbar->ToolbarItems->Add();
    itemTightenTime->Caption = "Tighten Time";
    itemTightenTime->AddOnName = ADDON_NAME;
    itemTightenTime->AddOnArgs = stringize() << L"/event=" << 300 + tightenTimeIndex;
    vslIconFile = vslFileName+_T(",18");
    itemTightenTime->IconFileName(vslIconFile.c_str());
    itemTightenTime->BeginGroup = (beautifyIndex >= 0) ? false : true;
  }
  //Flip message direction
  Visio::IVToolbarItemPtr itemFlipMessageDir = m_toolbar->ToolbarItems->Add();
  itemFlipMessageDir->Caption = "Flip Message Direction";
  itemFlipMessageDir->AddOnName = ADDON_NAME;
  itemFlipMessageDir->AddOnArgs = stringize() << L"/event=" << MENU_FLIP_MESSAGE_DIRECTION;
  vslIconFile = vslFileName+_T(",17");
  itemFlipMessageDir->IconFileName(vslIconFile.c_str());
  itemFlipMessageDir->BeginGroup = true;

  //Message sequence
  Visio::IVToolbarItemPtr itemMsgSequence = m_toolbar->ToolbarItems->Add();
  itemMsgSequence->Caption = "Message Sequence";
  itemMsgSequence->AddOnName = ADDON_NAME;
  itemMsgSequence->AddOnArgs = stringize() << L"/event=" << MENU_MESSAGE_SEQUENCE;
  vslIconFile = vslFileName+_T(",16");
  itemMsgSequence->IconFileName(vslIconFile.c_str());

  //Add instance
  Visio::IVToolbarItemPtr itemAddInstances = m_toolbar->ToolbarItems->Add();
  itemAddInstances->Caption = "Add Instances";
  itemAddInstances->AddOnName = ADDON_NAME;
  itemAddInstances->AddOnArgs = stringize() << L"/event=" << MENU_ADD_INSTANCES;
  vslIconFile = vslFileName+_T(",19");
  itemAddInstances->IconFileName(vslIconFile.c_str());

  Visio::IVToolbarItemPtr itemVerify = m_toolbar->ToolbarItems->Add();
  itemVerify->Caption = "Verify";
  itemVerify->AddOnName = ADDON_NAME;
  itemVerify->AddOnArgs = stringize() << L"/event=" << MENU_VERIFY;
  vslIconFile = vslFileName+_T(",0");
  itemVerify->IconFileName(vslIconFile.c_str());
  itemVerify->BeginGroup = true;

  m_vsoDocument->SetCustomToolbars(vsoToolbars);
}

void CDocumentMonitor::ToggleToolbarItems(bool modifiersPressed)
{
  m_selInstancesToolbarItem->Visible = !modifiersPressed;
  m_selMessagesToolbarItem->Visible = !modifiersPressed;
  m_selAddInstancesToolbarItem->Visible = modifiersPressed;
  m_selAddMessagesToolbarItem->Visible = modifiersPressed;
  //Message jumping
  m_msgJumpLeftToolbarItem->Visible = !modifiersPressed;
  m_msgJumpRightToolbarItem->Visible = !modifiersPressed;
  m_msgJumpLeftCopyToolbarItem->Visible = modifiersPressed;
  m_msgJumpRightCopyToolbarItem->Visible = modifiersPressed;

  m_vsoDocument->CustomToolbars->UpdateUI();
}

void CDocumentMonitor::ToogleNumeration(bool enabled)
{
	m_enableMessageEnumerationToolbarItem->Enabled = enabled;
	m_disableMessageEnumerationToolbarItem->Enabled = enabled;
	itemMessageEnumerating->Enabled = enabled;
	
	accelItemMessageEnumeration->Control = enabled;
	accelItemMessageEnumerationDisable->Control = enabled;
	
	m_vsoDocument->CustomToolbars->UpdateUI();
	m_vsoDocument->CustomMenus->UpdateUI();
}

VAORC CDocumentMonitor::DrawInstancesField(Visio::IVApplicationPtr vsoApp, int instancesCnt, Coordinate instanceLength, Coordinate instanceWidth, Coordinate startX, Coordinate startY, bool useConstSpacing, Coordinate spacingOrWidth)
{
  TRACE("CDocumentMonitor::DrawInstancesField() Drawing instances field");

  if (instancesCnt < 0 || instanceLength < 0 || instanceWidth < 0)
  {
    TRACE("CDocumentMonitor::DrawInstancesField() Bad arguments (negative or out of drawing area)");
    return VAORC_FAILURE;
  }

  double pageWidth = CPageUtils::GetPageWidth(vsoApp->ActivePage);
  double pageHeight = CPageUtils::GetPageHeight(vsoApp->ActivePage);

  if (startX < 0 || startY < 0 || startX > pageWidth || startY > pageHeight)
  {
    MessageBox(GetActiveWindow(),
      _T("Start point is out of drawing area."), _T("Add Instances Error"),
      MB_OK | MB_ICONEXCLAMATION);
    return VAORC_FAILURE;
  }

  Visio::IVPagePtr vsoPage = vsoApp->ActivePage;
  int oldShapesCount = vsoPage->Shapes->Count;
  CDrawingVisualizer visualizer(m_vsoApp);
  Coordinate spacing = spacingOrWidth;

  // compute dynamic spacing
  if (!useConstSpacing && instancesCnt > 1)
  {
    spacing = (spacingOrWidth - instanceWidth) / (instancesCnt - 1);
  }

  // build a bmsc according to the parameters
  BMscPtr bmsc(new BMsc);

  for (int i=0; i<instancesCnt; i++)
  {
    Coordinate x = startX + instanceWidth/2 + i*spacing;
    Coordinate height = instanceLength;
    Coordinate width = instanceWidth;

    InstancePtr inst(new Instance(_T("NAME")));
    inst->set_line_begin(MscPoint(x, startY));
    inst->set_line_end(MscPoint(x, startY + height));
    inst->set_width(width);

    bmsc->add_instance(inst);
  }

  /* // FIXME: crashes, we will make visualize_bmsc public in the meantime
  std::vector<MscPtr> msc;
  msc.push_back(bmsc);
  visualizer.visualize_msc(vsoPage, msc);
  */
  visualizer.visualize_bmsc(vsoPage, bmsc);

  // select the newly created instances
  Visio::IVSelectionPtr selection = vsoPage->CreateSelection(Visio::visSelTypeEmpty, Visio::visSelModeSkipSuper);
  for(int i=oldShapesCount+1; i<=vsoPage->Shapes->Count; i++)
  {
    Visio::IVShapePtr shape = vsoPage->Shapes->Item[i];
    if (get_shape_type(shape) == ST_BMSC_INSTANCE)
      selection->Select(shape, Visio::visSelect);
  }
  vsoApp->ActiveWindow->Selection = selection;
  return VAORC_SUCCESS;
}

TransformerPtrList::const_iterator CDocumentMonitor::find_transformer(const std::wstring& name) const
{
  for(TransformerPtrList::const_iterator cpos = m_transformers.begin();
    cpos != m_transformers.end(); cpos++)
  {
    if(_wcsicmp((*cpos)->get_name().c_str(), name.c_str()) == 0)
      return cpos;
  }

  return m_transformers.end();
}

MscPtr CDocumentMonitor::run_transformers(MscPtr& msc, const TransformerPtrList& transformers)
{
  // initialize a list of user defined priorities
  // items listed in the same order as m_checkers
  RequiredCheckersList priorities;
  priorities.insert(priorities.begin(), m_checkers.size(), PrerequisiteCheck::PP_DISREGARDED);

  // examine preconditions of all required transformers
  for(TransformerPtrList::const_iterator tpos = transformers.begin();
    tpos != transformers.end(); tpos++)
  {
    Transformer::PreconditionList preconditions = (*tpos)->get_preconditions(msc);
    // walk through all installed checkers
    for(Transformer::PreconditionList::const_iterator ppos = preconditions.begin();
      ppos != preconditions.end(); ppos++)
    {
      CheckerPtrList::const_iterator checker = find_checker(ppos->property_name);
      if(checker != m_checkers.end())
      {
        size_t icheck = checker - m_checkers.begin();

        if(ppos->priority < priorities[icheck])
          priorities[icheck] = ppos->priority;
      }
    }
  }

  // execute the check
  int status = run_checks(msc, priorities);
  // check the status
  if(status < 0)
  {
    m_reportView->Print(RS_ERROR,
      stringize() << "Transformation failed. Preconditions violated.");
    return NULL;
  }

  MscPtr result = msc;
  // execute all required transformations
  for(TransformerPtrList::const_iterator tpos = transformers.begin();
    tpos != transformers.end(); tpos++)
  {
    BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(result);
    if(bmsc != NULL)
    {
      try
      {
        result = (*tpos)->transform(bmsc);
      }
      catch(std::exception &exc)
      {
        m_reportView->Print(RS_ERROR, stringize() << (*tpos)->get_name()
          << " internal error: " << exc.what());
      }
    }

    HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(result);
    if(hmsc != NULL)
    {
      try
      {
        result = (*tpos)->transform(hmsc);
      }
      catch(std::exception &exc)
      {
        m_reportView->Print(RS_ERROR, stringize() << (*tpos)->get_name()
          << " internal error: " << exc.what());
      }
    }

    if(result == NULL)
    {
      m_reportView->Print(RS_ERROR, stringize() << (*tpos)->get_name()
        << " failed.");
      return NULL;
    }
  }

  return result;
}

VAORC CDocumentMonitor::OnMenuImport(Visio::IVApplicationPtr vsoApp)
{
  // clear the verification report
  m_reportView->Reset();

  char fileName[_MAX_PATH];
  // ask user for the filename
  if (GetInputFile(fileName) < 0)
    return VAORC_FAILURE;

  ImportFormatterPtr formatter = GetImportFormatter(fileName);
  if(formatter == NULL)
    return VAORC_FAILURE;

  // import the drawing
  ImportDocument(formatter, fileName);
  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuImportSettings(Visio::IVApplicationPtr vsoApp)
{
  CImportSettingsDlg sheet(vsoApp);
  sheet.DoModal();

  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuExport(Visio::IVApplicationPtr vsoApp)
{
  // clear the verification report
  m_reportView->Reset();

  char fileName[_MAX_PATH];
  // ask user for the filename
  if (GetOutputFile(fileName) < 0)
    return VAORC_FAILURE;

  ExportFormatterPtr formatter = GetExportFormatter(fileName);
  if(formatter == NULL)
    return VAORC_FAILURE;

  std::ofstream stream;
  stream.open(fileName, std::ios::out | std::ios::trunc);
  if(!stream.good())
  {
    MessageBox(GetActiveWindow(),
      _T("Cannot export given file."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
    return VAORC_FAILURE;
  }

  // export the drawing
  ExportActiveDocument(formatter, stream);
  stream.close();

  return VAORC_SUCCESS;
}

int CDocumentMonitor::check_membership_preconditions(MscPtr& msc)
{
  // initialize a list of user defined priorities
  // items listed in the same order as m_checkers
  RequiredCheckersList priorities;
  priorities.insert(priorities.begin(), m_checkers.size(), PrerequisiteCheck::PP_DISREGARDED);

  // examine membership preconditions
  Searcher::PreconditionList preconditions = m_membership->get_preconditions(msc);
  // walk through all installed checkers
  for(Searcher::PreconditionList::const_iterator ppos = preconditions.begin();
    ppos != preconditions.end(); ppos++)
  {
    CheckerPtrList::const_iterator checker = find_checker(ppos->property_name);
    if(checker != m_checkers.end())
    {
      size_t icheck = checker - m_checkers.begin();

      if(ppos->priority < priorities[icheck])
        priorities[icheck] = ppos->priority;
    }
  }

  // execute the check
  return run_checks(msc, priorities);
}

VAORC CDocumentMonitor::OnMenuFindFlow(Visio::IVApplicationPtr vsoApp)
{
  CFindDlg dlg(vsoApp);
  // ask user for the drawings to be compared
  if (dlg.DoModal() != IDOK)
    return VAORC_FAILURE;

  // clear the verification report
  m_reportView->Reset();

  CDrawingExtractor needle_extractor(m_reportView);
  std::vector<MscPtr> msc_needles;
  // obtain MSC to search for (needle)
  for(u_int i = 0; i < dlg.m_pages1.size(); i++)
  {
    MscPtr msc_needle = needle_extractor.extract_msc(dlg.m_pages1.at(i));

    // execute the check
    int needle_status = check_membership_preconditions(msc_needle);
    // check the status
    if(needle_status < 0)
    {
      m_reportView->Print(RS_ERROR, stringize()
        << "Search failed. Preconditions of " << msc_needle->get_label() << " violated.");
      return VAORC_FAILURE;
    }
    if(msc_needle)
      msc_needles.push_back(msc_needle);
  }

  MscPtr msc_haystack;
  // obtain MSC to search in (haystack)
  CDrawingExtractor haystack_extractor(m_reportView);
  msc_haystack = haystack_extractor.extract_msc(dlg.m_page2);

  // execute the check
  int haystack_status = check_membership_preconditions(msc_haystack);
  // check the status
  if(haystack_status < 0)
  {
    m_reportView->Print(RS_ERROR, stringize()
      << "Search failed. Preconditions of " << msc_haystack->get_label() << " violated.");
    return VAORC_FAILURE;
  }

  if (msc_haystack == NULL || msc_needles.empty())
    return VAORC_FAILURE;

  MscPtr result;

  try
  {
    if(dlg.m_diffEnabled)
      result = m_membership->diff(msc_haystack, msc_needles);
    else
      result = m_membership->find(msc_haystack, msc_needles);
  }
  catch(std::exception &exc)
  {
    m_reportView->Print(RS_ERROR, stringize()
      << "Search failed. Internal error: " << exc.what());
    return VAORC_FAILURE;
  }

  std::wstring flows;
  for(u_int i = 0; i < msc_needles.size(); i++)
  {
    if(i) flows += _T(", ");
    flows += msc_needles.at(i)->get_label();
  }

  if (result != NULL)
  {
    if(dlg.m_diffEnabled)
      m_reportView->Print(RS_NOTICE, stringize()
        << "Flow \"" << flows << "\" differs from the speification \"" << msc_haystack->get_label()
        << "\".", result);
    else
    {
      if(msc_needles.size() > 1)
        m_reportView->Print(RS_NOTICE, stringize() 
          << "Flows coverege of the specification \"" << msc_haystack->get_label() << "\".", result);
      else
        m_reportView->Print(RS_NOTICE, stringize()
          << "Flow coverege of the specification \"" << msc_haystack->get_label() << "\".", result);
    }
  }
  else
  {
    if(dlg.m_diffEnabled)
      m_reportView->Print(RS_NOTICE, stringize()
        << "Flow \"" << flows << "\" and specification " << msc_haystack->get_label()
        << " are same.");
  }

  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuRepaint(Visio::IVApplicationPtr vsoApp)
{	
  // clear the verification report
  m_reportView->Reset();

  long scope_id = vsoApp->BeginUndoScope("Repaint");

  //Disable message numbering, auto snapping
  selectAll(vsoApp, false, SELECTION_REPLACE);
  OnMenuDisableMessageEnumeration(vsoApp);
  bool bSnappingEnabled = CMessageSnapping::isEnabled();
  if(bSnappingEnabled)
    CMessageSnapping::setEnabled(false);

  CDrawingExtractor extractor(m_reportView);
  // delete all MSC symbols, preserve ignored shapes
  extractor.m_remove_extracted = true;

  std::vector<MscPtr> drawing;
  // this should repaint all pages, but we don't know the HMSC root
  // walk through all pages are repaint each drawing separately
  for(int i = 1; i <= vsoApp->ActiveDocument->Pages->Count; i++)
  {
    Visio::IVPagePtr page = vsoApp->ActiveDocument->Pages->Item[i];

    MscPtr msc = extractor.extract_msc(page);
    if(msc == NULL)
    {
      vsoApp->EndUndoScope(scope_id, false);

      m_reportView->Print(RS_ERROR,
        stringize() << "Repaint failed.");

      return VAORC_FAILURE;
    }

    drawing.push_back(msc);
  }

  CDrawingVisualizer visualizer(vsoApp);
  visualizer.m_ask_overwrite = false;
  visualizer.visualize_msc(vsoApp->ActiveDocument, drawing);

  //Enable auto snapping back (if was enabled)
  CMessageSnapping::setEnabled(bSnappingEnabled);

  vsoApp->EndUndoScope(scope_id, true);

  m_reportView->Print(RS_NOTICE,
    stringize() << "Repaint completed.");

  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuTransform(Visio::IVApplicationPtr vsoApp, int index)
{
  // clear the verification report
  m_reportView->Reset();

  TransformerPtr transformer = m_transformers[index];
  if(transformer == NULL)
    return VAORC_FAILURE;

  long scope_id = vsoApp->BeginUndoScope(transformer->get_name().c_str());

  //Disable message numbering, auto snapping
  selectAll(vsoApp, false, SELECTION_REPLACE);
  OnMenuDisableMessageEnumeration(vsoApp);
  bool bSnappingEnabled = CMessageSnapping::isEnabled();
  if(bSnappingEnabled)
    CMessageSnapping::setEnabled(false);

  Visio::IVPagePtr vsoPage = m_vsoApp->GetActivePage();

  CDrawingExtractor extractor(m_reportView);
  // delete all MSC symbols, preserve ignored shapes
  extractor.m_remove_extracted = true;
  MscPtr msc = extractor.extract_msc(vsoPage);
  if(msc == NULL)
  {
    m_vsoApp->EndUndoScope(scope_id, false);

    m_reportView->Print(RS_ERROR,
      stringize() << transformer->get_name() << " failed.");
	
    return VAORC_FAILURE;
  }

  TransformerPtrList transformer_list;
  transformer_list.push_back(transformer);
  // execute the transformation
  MscPtr result = run_transformers(msc, transformer_list);
  if(result == NULL)
  {
    m_vsoApp->EndUndoScope(scope_id, false);

    m_reportView->Print(RS_ERROR,
      stringize() << transformer->get_name() << " failed.");	
    return VAORC_FAILURE;
  }

  std::vector<MscPtr> drawing;
  drawing.push_back(result);

  CDrawingVisualizer visualizer(vsoApp);
  visualizer.m_ask_overwrite = false;
  visualizer.visualize_msc(vsoApp->ActiveDocument, drawing);

  //Enable auto snapping if it was enabled
  CMessageSnapping::setEnabled(bSnappingEnabled);

  vsoApp->EndUndoScope(scope_id, true);

  m_reportView->Print(RS_NOTICE,
    stringize() << transformer->get_name() << " completed.");

  return VAORC_SUCCESS;
}

void CDocumentMonitor::selectAll(Visio::IVApplicationPtr vsoApp, bool select_instances, SelectionType selType)
{
  Visio::IVPagePtr page = vsoApp->ActivePage;
  Visio::IVSelectionPtr selection;
  if (selType == SELECTION_ADD)
    selection = vsoApp->ActiveWindow->Selection;
  else
    selection = page->CreateSelection(Visio::visSelTypeEmpty, Visio::visSelModeSkipSuper);

  for(int i=1; i<=page->Shapes->Count; i++)
  {
    Visio::IVShapePtr shape = page->Shapes->Item[i];
    switch (get_shape_type(shape))
    {
    case ST_BMSC_INSTANCE:
      if (select_instances)
        selection->Select(shape, Visio::visSelect);
      break;
    case ST_BMSC_MESSAGE:
    case ST_BMSC_MESSAGE_LOST:
    case ST_BMSC_MESSAGE_FOUND:
      if (!select_instances)
        selection->Select(shape, Visio::visSelect);
      break;
    }
  }

  vsoApp->ActiveWindow->Selection = selection;
}

VAORC CDocumentMonitor::OnMenuSelectAllInstances(Visio::IVApplicationPtr vsoApp, SelectionType selType)
{
  selectAll(vsoApp, true, selType);
  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuSelectAllMessages(Visio::IVApplicationPtr vsoApp, SelectionType selType)
{
  selectAll(vsoApp, false, selType);
  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuMessageJump(Visio::IVApplicationPtr vsoApp, bool left, bool asCopy)
{
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;

  if(selection->Count > 1 || !selection->Count)
    return VAORC_FAILURE;

  Visio::IVShapePtr shape = selection->Item[1];
  if(!isMessageShape(shape))
    return VAORC_FAILURE;

  long scopeId = vsoApp->BeginUndoScope("Jump");
  std::vector<Visio::IVShapePtr> instances  = CMessageSnapping::getConnectedInstances(shape);
  MsgConnectedEndpoints conn                = CShapeUtils::getConnectedEndpoints(shape);
  if(!instances.size())
  {
    CMessageSnapping::snap(shape, CShapeUtils::getShapeCell(shape, "PinX"), CShapeUtils::getShapeCell(shape, "PinY"), MSSNAP_PRESERVE_VERTICAL);
    instances = CMessageSnapping::getConnectedInstances(shape);
  }
  //else if(conn != MSCE_BOTH) //If message isn't connected to both endpoints(not for LOST/FOUND), snap unplugged end to nearest instance
  //{
  //  CMessageSnapping::snapEndPointToClosestInstance(shape, (conn == MSCE_BEGIN) ? "EndX" : "BeginX", CShapeUtils::getMsgDirection(shape));
  //  instances = CMessageSnapping::getConnectedInstances(shape);
  //}
  if(!CMessageJump::jump(shape, instances, (MsgJumpDirection)left, asCopy) && CShapeUtils::getConnectedEndpoints(shape) == MSCE_NONE)
    CMessageSnapping::resnap(shape, vsoApp->ActivePage->Shapes, 0.001);

  if(CMessageJump::getMsgNeedsResnap(shape))
    CMessageSnapping::resnap(shape, vsoApp->ActivePage->Shapes, 0.0001);
  vsoApp->EndUndoScope(scopeId, true);

  return VAORC_SUCCESS;
}

std::pair<Visio::IVShapePtr, Visio::IVShapePtr> CDocumentMonitor::getInstancesPair(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVSelectionPtr selection = CPageUtils::FilterSelection(vsoApp->ActiveWindow->Selection, ST_BMSC_INSTANCE);
  if (selection->Count == 0)
  {
    selection->SelectAll();
    selection = CPageUtils::FilterSelection(selection, ST_BMSC_INSTANCE);
  }

  switch (selection->Count)
  {
  case 0:
    return std::make_pair<Visio::IVShapePtr, Visio::IVShapePtr>(NULL, NULL);
  case 1:
    return std::make_pair<Visio::IVShapePtr, Visio::IVShapePtr>(selection->Item[1], NULL);
  case 2:
    // NOTE: respects the order in which the items were selected
    return std::make_pair<Visio::IVShapePtr, Visio::IVShapePtr>(selection->Item[1], selection->Item[2]);
  default:
    Visio::IVShapePtr leftmost = selection->Item[1];
    Visio::IVShapePtr rightmost = selection->Item[1];
    CShapeComparator instanceCmp;
    for (int i=2; i<=selection->Count; i++)
    {
      Visio::IVShapePtr item = selection->Item[i];
      if (get_shape_type(item) != ST_BMSC_INSTANCE) continue;

      if (instanceCmp(item, leftmost)) leftmost = item;
      if (!instanceCmp(item, rightmost)) rightmost = item;
    }
    return std::make_pair<Visio::IVShapePtr, Visio::IVShapePtr>(leftmost, rightmost);
  }
}

VAORC CDocumentMonitor::OnMenuFlipMessageDirection(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;
  std::vector<_bstr_t> formulas;

  long scopeId = vsoApp->BeginUndoScope("Flip direction");

  vsoApp->EventsEnabled = false;
  for(int i = 1; i <= selection->Count; i++)
  {
    Visio::IVShapePtr shape = selection->Item[i];
    if(!isMessageShape(shape))
      continue;
    for(int i = 0; i < 4; i++)
      formulas.push_back(shape->GetCellsSRC(visSectionObject, visRowXForm1D, i)->FormulaU);
    try {
      for(int i = 0; i < 2; i++)
      {
        shape->GetCellsSRC(visSectionObject, visRowXForm1D, i)->FormulaU = formulas.at(i+2);
        shape->GetCellsSRC(visSectionObject, visRowXForm1D, i+2)->FormulaU = formulas.at(i);
      }
    }
    catch(_com_error err){
      MessageBox(GetActiveWindow(), L"Exception occurred", L"", MB_OK);
    }
    formulas.clear();
    //Flip lost and found messages
    TShapeType shapeType = get_shape_type(shape);
    if(shapeType == ST_BMSC_MESSAGE_FOUND || shapeType == ST_BMSC_MESSAGE_LOST)
    {
      CMessageJump::changeMsgType(shape, (TShapeType)((shapeType == 3) ? 2 : 3));
      CMessageSnapping::resnap(shape, vsoApp->ActivePage->Shapes, 0.0001);
    }
  }
  vsoApp->EventsEnabled = true;

  vsoApp->EndUndoScope(scopeId, true);

  return VAORC_SUCCESS;
}

std::vector<Visio::IVShapePtr> CDocumentMonitor::getInstancesInBetween(Visio::IVShapePtr boundaryOne, Visio::IVShapePtr boundaryTwo, double yCoord)
{
  std::vector<Visio::IVShapePtr> qualified;
  double fromX = CShapeUtils::GetShapeBeginX(boundaryOne);
  double toX   = CShapeUtils::GetShapeBeginX(boundaryTwo);

  if (fabs(fromX - toX) < EPSILON)
    return qualified;
  if (fabs(fromX - CShapeUtils::GetShapeEndX(boundaryOne)) > EPSILON)
    return qualified;
  if (fabs(toX   - CShapeUtils::GetShapeEndX(boundaryTwo)) > EPSILON)
    return qualified;
  if (CShapeUtils::GetShapeBeginY(boundaryOne) < yCoord ||
      CShapeUtils::GetShapeEndY(boundaryOne)   > yCoord ||
      CShapeUtils::GetShapeBeginY(boundaryTwo) < yCoord ||
      CShapeUtils::GetShapeEndY(boundaryTwo)   > yCoord)
  {
    // some boundary out of yCoord
    return qualified;
  }

  const Visio::IVSelectionPtr selection = CPageUtils::FilterSelection(boundaryOne->Application->ActiveWindow->Selection, ST_BMSC_INSTANCE);
  if (selection->Count > 2)
  {
    // special care when there are more than 2 instances selected
    // exactly the selected instances should be the subject of message sequence
    // if any of them does not meet the criteria (vertical, intersects yCoord), report an error
    for (int i=1; i<=selection->Count; i++) {
      Visio::IVShapePtr shape = selection->Item[i];

      double bX = CShapeUtils::GetShapeBeginX(shape);
      double eX = CShapeUtils::GetShapeEndX(shape);
      double bY = CShapeUtils::GetShapeBeginY(shape);
      double eY = CShapeUtils::GetShapeEndY(shape);

      // must be vertical and at right yCoord
      if (fabs(bX - eX) > EPSILON || bY < yCoord || eY > yCoord)
      {
        return std::vector<Visio::IVShapePtr>(); // error - a member of selection should have qualified
      }
      qualified.push_back(shape);
    }
  }
  else
  {
    // there are no more than 2 instances selected - scan the whole page
    Visio::IVPagePtr page = boundaryOne->ContainingPage;
    // push back all qualifying shapes based on X-coord and Y-coord
    for (int i=1; i <= page->Shapes->Count; i++)
    {
      Visio::IVShapePtr shape = page->Shapes->Item[i];

      if (get_shape_type(shape) != ST_BMSC_INSTANCE) continue;

      double bX = CShapeUtils::GetShapeBeginX(shape);
      double eX = CShapeUtils::GetShapeEndX(shape);
      double bY = CShapeUtils::GetShapeBeginY(shape);
      double eY = CShapeUtils::GetShapeEndY(shape);

      if (fabs(bX - eX) > EPSILON) continue; // not vertical
      if (bY < yCoord || eY > yCoord) continue; // wrong yCoord

      if ((fromX <= bX && bX <= toX) || (fromX >= bX && bX >= toX))
      {
        qualified.push_back(shape);
      }
    }
  }

  CShapeComparator comp((fromX < toX ? ORDER_ASC : ORDER_DESC));
  std::sort(qualified.begin(), qualified.end(), comp);
  return qualified;
 }

VAORC CDocumentMonitor::OnMenuMessageSequence(Visio::IVApplicationPtr vsoApp, Visio::IVSelectionPtr oldSelection)
{
  // step 0: turn off message snapping
  bool snapStatus = CMessageSnapping::isEnabled();
  CMessageSnapping::setEnabled(false);
  // step 1: determine the boundary instances between which to draw message sequence
  std::pair<Visio::IVShapePtr, Visio::IVShapePtr> boundaries = getInstancesPair(vsoApp);
  if (boundaries.first == NULL)
  {
    MessageBox(GetActiveWindow(),
      _T("There are no instances in the current selection or drawing."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
    return VAORC_FAILURE;
  }

  double initYPos = m_addon->GetMousePosY();

  if (m_addon->GetState(vsoApp) == CStudioAddon::STATE_MESSAGE_SEQUENCE_WAITING_FOR_SEL_CHANGE)
  {
    // was waiting for selecting another instance
    // continue only if the user selected exactly one instance both before and now
    m_addon->ResetState(vsoApp);
    if (boundaries.second != NULL || oldSelection->Count != 1)
    {
      return VAORC_FAILURE;
    }
    boundaries.second = boundaries.first;
    boundaries.first = oldSelection->Item[1];
    initYPos = m_addon->GetPrevMousePosY(vsoApp);
  }
  else
  {
    if (boundaries.second == NULL)
    {
      // selected just one instance, prompt to select another
      this->m_addon->SetState(vsoApp, CStudioAddon::STATE_MESSAGE_SEQUENCE_WAITING_FOR_SEL_CHANGE,
        _T("Select another instance to draw a message sequence to."));
      this->m_addon->SaveMousePos(vsoApp);
      return VAORC_SUCCESS;
    }
  }

  // step 2: basic validation - do the two instances intersect in their y-coords?
  if (CShapeUtils::GetShapeBeginY(boundaries.first) < CShapeUtils::GetShapeEndY(boundaries.second) ||
      CShapeUtils::GetShapeBeginY(boundaries.second) < CShapeUtils::GetShapeEndY(boundaries.first))
  {
    MessageBox(GetActiveWindow(),
      _T("The instances are in wrong positions to make\na message sequence between them."),
      _T("Error"), MB_OK | MB_ICONEXCLAMATION);
    return VAORC_FAILURE;
  }
 
  // step 3: dialog
  CShapeComparator instanceCmp;
  MsgSeqDirection origDir = (instanceCmp(boundaries.first, boundaries.second) ? MSDIR_RIGHT : MSDIR_LEFT);
  CShapeUtils::MarkShape(boundaries.first);
  CShapeUtils::MarkShape(boundaries.second);
  CMessageSequenceDlg dlg(vsoApp, initYPos, origDir);
  LRESULT dlgResult = dlg.DoModal(); // update the instances marking on update of yCoord
  CShapeUtils::UnmarkShape(boundaries.first);
  CShapeUtils::UnmarkShape(boundaries.second);
  if (!dlgResult) return VAORC_FAILURE;

  MsgSeqDirection direction = dlg.m_direction;
  double startPosY = CPageUtils::ConvertUnits(vsoApp->ActivePage, dlg.m_start_pos_y);
  double verticalSpace = CPageUtils::ConvertUnits(vsoApp->ActivePage, dlg.m_vertical_space);
  double verticalSpaceLeftRight = CPageUtils::ConvertUnits(vsoApp->ActivePage, dlg.m_vertical_space_left_right);
  CString leftMessage = dlg.m_left_msg_captions;
  CString rightMessage = dlg.m_right_msg_captions;
  MsgSeqCoregionTreatment coregionTreatment = dlg.m_coregionTreatment;

  // step 4: make an ordered list of instances being subject of message sequence drawing
  std::pair<Visio::IVShapePtr, Visio::IVShapePtr> orderedBoundaries = std::make_pair<Visio::IVShapePtr, Visio::IVShapePtr>(boundaries.first, boundaries.second);
  if (!instanceCmp(orderedBoundaries.first, orderedBoundaries.second))
  {
    swap(orderedBoundaries);
  }
  // order the pair so that we get the right direction by iterating the resulting list
  if (direction == MSDIR_LEFT || direction == MSDIR_LEFT_RIGHT)
  {
    swap(orderedBoundaries);
  }
  // get the ordered list
  std::vector<Visio::IVShapePtr> instList = getInstancesInBetween(orderedBoundaries.first, orderedBoundaries.second, startPosY);
  if (instList.size() == 0)
  {
    MessageBox(GetActiveWindow(),
      _T("No instances matching the criteria.\nSome of the selected instances are out of Y-start (mouse position)."),
      _T("Error"), MB_OK | MB_ICONEXCLAMATION);
    return VAORC_FAILURE;
  }
  
  // step 5: draw the message sequence along the list
  long undoScope = vsoApp->BeginUndoScope("Message Sequence"); // FIXME: makes "New Row" undo action, would like to have "Message Sequence"
  Visio::IVPagePtr vsoPage = vsoApp->ActivePage;
  CDrawingVisualizer visualizer(vsoApp);
  Visio::IVMasterPtr msgMaster = visualizer.find_master(ST_BMSC_MESSAGE);
  Visio::IVMasterPtr orderingMaster = visualizer.find_master(ST_BMSC_ORDER_LINE);
  Visio::IVSelectionPtr selection = vsoPage->CreateSelection(Visio::visSelTypeEmpty, Visio::visSelModeSkipSuper);
  
  double yPos = startPosY;
  std::vector<Visio::IVShapePtr>::iterator from = instList.begin();
  std::vector<Visio::IVShapePtr>::iterator to = instList.begin();
  ++to;
  const TCHAR* caption = (instanceCmp(*from, *to) ? rightMessage : leftMessage);
  Visio::IVShapePtr msg = NULL;
  for ( ; to != instList.end(); ++from, ++to, yPos-=verticalSpace)
  {
    TRACE("Drawing from " << (*from)->Name << " (" << (*from)->Text << ") to " << (*to)->Name << " (" << (*to)->Text << ") at y=" << yPos);
    msg = DrawMessage(msgMaster, orderingMaster, *from, *to, yPos, caption, coregionTreatment, msg);
    if (msg == NULL)
    {
      vsoApp->EndUndoScope(undoScope, false);
      return VAORC_FAILURE;
    }
    selection->Select(msg, Visio::visSelect);
  }
  
  if (direction == MSDIR_LEFT_RIGHT || direction == MSDIR_RIGHT_LEFT)
  {
    // change yPos by verticalSpaceLeftRight instead of last verticalSpace
    yPos += verticalSpace - verticalSpaceLeftRight;

    std::vector<Visio::IVShapePtr>::reverse_iterator rfrom = instList.rbegin();
    std::vector<Visio::IVShapePtr>::reverse_iterator rto = instList.rbegin();
    ++rto;
    caption = (caption == leftMessage ? rightMessage : leftMessage);
    for ( ; rto != instList.rend(); ++rfrom, ++rto, yPos-=verticalSpace)
    {
      TRACE("Drawing from " << (*rfrom)->Name << " (" << (*rfrom)->Text << ") to " << (*rto)->Name << " (" << (*rto)->Text << ") at y=" << yPos);
      msg = DrawMessage(msgMaster, orderingMaster, *rfrom, *rto, yPos, caption, coregionTreatment, msg);
      if (msg == NULL)
      {
        vsoApp->EndUndoScope(undoScope, false);
        return VAORC_FAILURE;
      }
      selection->Select(msg, Visio::visSelect);
    }
  }

  //Turn message snapping back on
  CMessageSnapping::setEnabled(snapStatus);

  vsoApp->EndUndoScope(undoScope, true);
  vsoApp->ActiveWindow->Selection = selection;

  return VAORC_SUCCESS;
}

Visio::IVShapePtr CDocumentMonitor::DrawMessage(Visio::IVMasterPtr msgMaster, Visio::IVMasterPtr orderingMaster, Visio::IVShapePtr from, Visio::IVShapePtr to, double yCoord, const TCHAR* caption, MsgSeqCoregionTreatment coregionTreatment, Visio::IVShapePtr prevMsg)
{
  Visio::IVShapePtr fromCoregion = CShapeUtils::GetCoregionAt(from, yCoord);
  Visio::IVShapePtr toCoregion = CShapeUtils::GetCoregionAt(to, yCoord);

  Visio::IVPagePtr page = from->Application->ActivePage;
  Visio::IVShapePtr vsoMsg = page->Drop(msgMaster, 0, 0);
  vsoMsg->Text = caption;
  vsoMsg->Data2 = caption; //NOTE: fixes bad captions when using auto message numbering

  Visio::IVShapePtr fromShape = (fromCoregion == NULL ? from : fromCoregion);
  Visio::IVShapePtr toShape = (toCoregion == NULL ? to : toCoregion);

  double vsoMsgBeginX = from->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX]->Result[0];
  double vsoMsgEndX = to->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX]->Result[0];
  double fromX = (vsoMsgBeginX <= vsoMsgEndX ? 1.0 : 0.0);
  double toX = (vsoMsgBeginX < vsoMsgEndX ? 0.0 : 1.0);

  Visio::IVCellPtr fromCell = vsoMsg->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
  double fromLen     = CShapeUtils::GetShapeBeginY(fromShape) - CShapeUtils::GetShapeEndY(fromShape);
  double fromYOffset = CShapeUtils::GetShapeBeginY(fromShape) - yCoord;
  fromCell->GlueToPos(fromShape, (fromYOffset/fromLen), fromX); // NOTE: Visio's x-axis is here in fact y, and vice versa

  Visio::IVCellPtr toCell = vsoMsg->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX];
  double toLen = CShapeUtils::GetShapeBeginY(toShape) - CShapeUtils::GetShapeEndY(toShape);
  double toYOffset = CShapeUtils::GetShapeBeginY(toShape) - yCoord;
  toCell->GlueToPos(toShape, (toYOffset/toLen), toX); // NOTE: Visio's x-axis is here in fact y, and vice versa

  if (coregionTreatment == MSCOR_ERROR && (fromCoregion != NULL || toCoregion != NULL))
  {
    TRACE("A forbidden coregion found at instance " << to->Name << " (" << to->Text << ") at y=" << yCoord);
    MessageBox(GetActiveWindow(),
      _T("A coregion was reached by the message sequence"),
      _T("Error"), MB_OK | MB_ICONEXCLAMATION);
    return NULL;
  }

  if (prevMsg != NULL && fromCoregion != NULL &&
      (coregionTreatment == MSCOR_CONNECT_LINE || coregionTreatment == MSCOR_CONNECT_SIDE_SIDE))
  {
    Visio::IVShapePtr vsoOrdering = page->Drop(orderingMaster, 0, 0);
    if (coregionTreatment == MSCOR_CONNECT_LINE)
    {
      vsoOrdering->DeleteRow(visSectionFirstComponent, visRowComponent+2);
      vsoOrdering->DeleteRow(visSectionFirstComponent, visRowComponent+2);
    }
    Visio::IVCellPtr ordFromCell = vsoOrdering->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
    Visio::IVCellPtr ordToCell = vsoOrdering->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX];
    ordFromCell->GlueTo(prevMsg->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX]);
    ordToCell->GlueTo(fromCell);
  }

  return vsoMsg;
}

VAORC CDocumentMonitor::OnMenuWindowsReporter(Visio::IVApplicationPtr vsoApp)
{
  if(m_reportVisible)
    // Close() --> CReportView::OnFinalMessage() --> CDocumentMonitor::OnHideReportView()
    m_reportWindow->Close();
  else
    ShowReportView();

  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuEnableMessageEnumeration(Visio::IVApplicationPtr vsoApp)
{
  //Static variables so dialog can use them to get previous choice
  static int	startingIndex = 1;
  static int	numberingType = 0;
  static BSTR add	  = _T(".");

  //Check if there are any messages on the page to be numbered
  Visio::IVShapesPtr shapes = vsoApp->ActivePage->PageSheet->Shapes;
  bool bMsgExists = false;
  for(int i=1; i<=shapes->Count; i++)
  {
    Visio::IVShapePtr shape = shapes->Item[i];
    if(isMessageShape(shape))
    {
      bMsgExists = true;
      break;
    }
  }
  if(!bMsgExists)
  {
    MessageBox(GetActiveWindow(),_T("There are no messages in the current drawing!"),
      _T("Error"), MB_ICONWARNING | MB_OK);
    return VAORC_FAILURE;
  }

  CEnumerationDlg options(startingIndex,numberingType, add);
  if(options.DoModal() != IDOK)
    return VAORC_FAILURE;

  int ID = CEnumerateUtils::getGroupCount(vsoApp);

  _bstr_t groupID = stringize() << _T("Enum") << ++ID;

  startingIndex = options.getStartingIndex();
  numberingType = options.getNumberingType();
  add = options.getAddition();

  CEnumerateUtils::saveGroupSettings(vsoApp,groupID,startingIndex,numberingType,add);

  if (vsoApp->ActiveWindow->Selection->Count)
    enumerate(vsoApp, groupID, true,true);
  else
    enumerate(vsoApp, groupID, true,false);

  CEnumerateUtils::setGroupCount(vsoApp,ID);

  return VAORC_SUCCESS;
}
VAORC CDocumentMonitor::OnMenuDisableMessageEnumeration(Visio::IVApplicationPtr vsoApp)
{	
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;
  Visio::IVShapePtr page = vsoApp->ActivePage->PageSheet;
  std::set<_bstr_t> groups;

  long undo_id = vsoApp->BeginUndoScope("DeleteNumbering");

  //get all NUMBERED messages on the page
  int numberedMessages = 0;
  for(int i=1; i<=page->Shapes->Count; i++)
  {
    Visio::IVShapePtr shape = page->Shapes->Item[i];
    if(isMessageShape(shape) && _tcsicmp(shape->Data1,_T("1")) == 0)
      numberedMessages++;
  }

  int messagesOnSelection = 0;          //number of messages in current selection
  int selectedNumberedMessages = 0;     //number of numbered messages in current selection
  for(int i=1; i<=selection->Count; i++)
  {
    Visio::IVShapePtr shape = selection->Item[i];
    if(isMessageShape(shape))
    {
      messagesOnSelection++;
      if(_tcsicmp(shape->Data1,_T("1")) == 0)
        selectedNumberedMessages++;
    }
  }

  //if this is true, we disable numbering only on selected messages
  if (selection->Count && (selectedNumberedMessages != numberedMessages) && messagesOnSelection)
  {		
    //get which groups are selected
    for(int i=1; i<= selection->Count; i++)
      if(_tcsicmp(selection->Item[i]->Data1,_T("1")) == 0)
        groups.insert(selection->Item[i]->Data3);		

    //remove specific messages from groups
    for(std::set<_bstr_t>::iterator it = groups.begin(); it != groups.end(); it++)
      enumerate(vsoApp, *it, false,true);
  }
  else
  {
    enumerate(vsoApp,_T(""), false,false);
    //erase all enum row information
    CEnumerateUtils::eraseEnumInfo(vsoApp);
  }

  vsoApp->EndUndoScope(undo_id,true);

  return VAORC_SUCCESS;
}

void CDocumentMonitor::autoEnumerate(Visio::IVShapePtr msgShape)
{
  const TCHAR* regFolder = _T("Software\\Sequence Chart Studio\\MessageNumbering");
  Visio::IVSelectionPtr selection = m_vsoApp->ActiveWindow->Selection;

  if(selection->Count == 1 && _tcsicmp(msgShape->Data1,_T("")) == 0)
  {
    //Return if auto enumeration is off
    if(!GetRegistry<bool>(regFolder,  NULL, _T("AutoEnum"), 0))
      return;

    Visio::IVShapePtr pageSheet = msgShape->Application->ActivePage->PageSheet;
    Visio::IVShapePtr closestMsg;

    int bAutoEnumType = GetRegistry<bool>(regFolder,  NULL, _T("AutoEnumType"), 0);
    closestMsg = CEnumerateUtils::getClosestMessage(msgShape->Application,msgShape,bAutoEnumType ? true : false);
    //enumerate according to closest message 
    if(closestMsg && _tcsicmp(closestMsg->Data3,_T("")) != 0)
    {
      enumerate(msgShape->Application,closestMsg->Data3,true,true);
    }
    //or use numbering style
    else if(GetRegistry<bool>(regFolder, NULL, _T("AutoEnumStyleEnabled"), 0) && bAutoEnumType)
    {
      int autoEnumGroupID = CEnumerateUtils::getAutoEnumGroup(m_vsoApp);
      if(autoEnumGroupID == -1)
      {
        int ID = CEnumerateUtils::getGroupCount(m_vsoApp);
        _bstr_t groupID = stringize() << _T("Enum") << ++ID;

        //addition string
        HKEY hKey;
        if(RegCreateKey(HKEY_CURRENT_USER, regFolder,&hKey) != ERROR_SUCCESS)
        {
          MessageBox(GetActiveWindow(), _T("Failed load key for addition string!"), _T("Error"), MB_OK);
          return;
        }

        DWORD dwType = 0;
        wchar_t buffer[5] = {0};
        DWORD dwSize = sizeof(buffer);
        if(!RegQueryValueEx(hKey,_T("AutoEnumStyleAddition"),0,&dwType, (LPBYTE)buffer, &dwSize) == ERROR_SUCCESS)
        {
          RegCloseKey(hKey);
          return;
        }
        RegCloseKey(hKey);

        //save group setting into page sheet
        CEnumerateUtils::saveGroupSettings(m_vsoApp, groupID,
          GetRegistry<int>(regFolder, NULL, _T("AutoEnumStyleIndex"), 1),
          GetRegistry<int>(regFolder, NULL, _T("AutoEnumStyleNumType"), 0),
          buffer);

        enumerate(m_vsoApp, groupID, true,true);

        CEnumerateUtils::setGroupCount(m_vsoApp,ID);
        pageSheet->AddNamedRow(visSectionUser,_T("AutoEnumGroupID"),visTagDefault);
        pageSheet->Cells["User.AutoEnumGroupID"]->FormulaU = stringize() << ID;
      }
    }
  }

  //Check which groups have changed
  std::set<_bstr_t> groups;
  //Collect all groups in current selection
  for(int i=1; i<=selection->Count; i++)
  {
    Visio::IVShapePtr shape = selection->Item[i];
    if(isMessageShape(shape) && (_tcsicmp(shape->Data3,_T("")) != 0))
      groups.insert(shape->Data3);
  }
  //Re-number groups
  for(std::set<_bstr_t>::iterator it = groups.begin(); it != groups.end(); it++)
    drawNumbers(m_vsoApp->ActivePage->Shapes,*it);
}

VAORC CDocumentMonitor::OnMenuSelectNumberedGroup(Visio::IVApplicationPtr vsoApp)
{
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;
  if(selection->Count == 0 || !strcmp(selection->Item[1]->Data3,""))
    return VAORC_FAILURE;

  CEnumerateUtils::selectGroup(vsoApp,selection->Item[1]->Data3);

  return VAORC_SUCCESS;
}

void CDocumentMonitor::enumerate(Visio::IVApplicationPtr vsoApp, _bstr_t groupID, bool enable, bool onlySelected)
{
  Visio::IVShapesPtr shapesOnPage = vsoApp->ActivePage->Shapes;
  Visio::IVSelectionPtr selection = vsoApp->ActiveWindow->Selection;
  std::set<_bstr_t> formerGroups;

  if(onlySelected)
  {
    onlySelected = false;
    for(int i=1; i<=selection->Count; i++)
      if(isMessageShape(selection->Item[i]))
      {
        onlySelected = true; break;
      }
  }

  int maxCount = (onlySelected ? (selection->Count) : (shapesOnPage->Count));
  for(int i=1; i<=maxCount; i++)
  {
    Visio::IVShapePtr shape;

    if(onlySelected)
      shape = selection->Item[i];
    else
      shape = shapesOnPage->Item[i];

    if(isMessageShape(shape))
    {
      if(enable)
        CEnumerateUtils::enableEnumeration(shape,groupID,formerGroups);
      else
        CEnumerateUtils::disableEnumeration(shape);

    }
  }

  if(enable || onlySelected)
    drawNumbers(shapesOnPage, groupID);

  if(formerGroups.size())
    for(std::set<_bstr_t>::iterator it = formerGroups.begin(); it != formerGroups.end(); it++)
      drawNumbers(shapesOnPage,*it);
}

void CDocumentMonitor::drawNumbers(Visio::IVShapesPtr shapesOnPage, _bstr_t groupID)
{	
  std::vector<Visio::IVShapePtr> messageShapes;
  Visio::IVShapePtr page = shapesOnPage->Application->ActivePage->PageSheet;

  //Default values
  int index				= 1;
  int enumerationType		= 0;
  std::wstring addition	= _T("");
  CEnumerateUtils::loadGroupSettings(shapesOnPage->Application,groupID,index,enumerationType,addition);

  //for all message shapes check whether they are checked as numbered (Data1 == 1)
  for(int i=1; i<=shapesOnPage->Count; i++)
  {
    Visio::IVShapePtr shape = shapesOnPage->Item[i];
    //It must be message shape, marked as numbered and in the right group
    if(isMessageShape(shape) && _tcsicmp(shape->Data1,_T("1")) == 0 && _tcsicmp(shape->Data3,groupID) == 0)
      messageShapes.push_back(shape);								 //Then put into vector
  }

  //heap sort (std::sort failed on bigger numbers, deletes pointers..)
  std::make_heap(messageShapes.begin(), messageShapes.end(),&CEnumerateUtils::messageCompare);
  std::sort_heap(messageShapes.begin(),messageShapes.end(),&CEnumerateUtils::messageCompare);

  std::wstring textIndex;
  for(std::vector<Visio::IVShapePtr>::iterator it = messageShapes.begin(); it != messageShapes.end(); ++it)
  {
    switch(enumerationType)
    {
    case 0: textIndex = stringize() << index; break;											//numbers
    case 1: textIndex = (index <= 4000) ? CEnumerateUtils::int2Romans(index) : _T("?"); break;	//romans
    case 2: textIndex = CEnumerateUtils::int2Char(index,false); break;							//small letters
    case 3: textIndex = CEnumerateUtils::int2Char(index,true); break;							//capital letters
    }
    (*it)->Text = stringize() << textIndex << addition << (*it)->Data2;
    index++;
  }
}

VAORC CDocumentMonitor::OnMenuGlobalSettings(Visio::IVApplicationPtr vsoApp)
{
  CGlobalSettingsDlg sheet(vsoApp, _T("Settings"));
  sheet.DoModal();

  //Message Numbering processing
  const TCHAR* regFolder = _T("Software\\Sequence Chart Studio\\MessageNumbering");
  if(GetRegistry<bool>(regFolder,  NULL, _T("AutoEnumStyleIndexChanged"), 0))
  {
    CEnumerateUtils::eraseAutoEnumGroup(vsoApp);
    SetRegistry<bool>(HKEY_CURRENT_USER, regFolder, _T("AutoEnumStyleIndexChanged"), false);
  }

  return VAORC_SUCCESS;
}

VAORC CDocumentMonitor::OnMenuBeautifySettings(Visio::IVApplicationPtr vsoApp)
{
  CBeautifySettingsDlg sheet(vsoApp);
  sheet.DoModal();

  return VAORC_SUCCESS;
}

void CDocumentMonitor::ShowReportView()
{
  if(m_reportVisible)
    return; // already shown

  Visio::IVWindowPtr vsoWindow = m_vsoApp->GetActiveWindow();

  const int defWidth = 500;
  const int defHeight = 150;
  m_reportWindow = vsoWindow->Windows->Add(LoadStringResource(IDS_REPORT_VIEW).c_str(),
    Visio::visWSVisible | Visio::visWSAnchorLeft, Visio::visAnchorBarAddon, 1, 1, defWidth, defHeight);

  LoadLibrary(CRichEditCtrl::GetLibraryName());

  RECT rc = {0, 0, defWidth, defHeight};
  HWND hwndTV = m_reportView->Create((HWND)m_reportWindow->WindowHandle,
    rc, NULL,
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

  m_reportMenuItem->State = Visio::visButtonDown;
  m_vsoDocument->CustomMenus->UpdateUI();
  m_reportVisible = true;
}

void CDocumentMonitor::OnHideReportView()
{
  m_reportMenuItem->State = Visio::visButtonUp;
  m_vsoDocument->CustomMenus->UpdateUI();
  m_reportVisible = false;
}

std::wstring filter_to_wstring(const std::string& str)
{
  std::wstring res;

  const char *spos = str.data();
  size_t ssize = str.length();
  while(ssize > 0)
  {
    int inc;
    // this converts '@' to '\0'
    // note: this function is intended only for CFileDialog filter strings
    if(*spos == '\0' || *spos == '@')
    {
      res.push_back(wchar_t('\0'));
      inc = 1;
    }
    else
    {
      wchar_t wc;
      inc = mbtowc(&wc, spos, ssize);
      res.push_back(wc);
    }

    spos += inc;
    ssize -= inc;
  }
  return res;
}

int CDocumentMonitor::GetInputFile(char* fileName)
{
  std::string extension;
  std::stringstream filter;

  // construct the filter string
  // _T("Text Documents (*.txt)\0*.txt\0All Files (*.*)\0*.*\0")
  for(FormatterPtrList::const_iterator fpos = m_formatters.begin();
    fpos != m_formatters.end(); fpos++)
  {
    ImportFormatterPtr formatter = boost::dynamic_pointer_cast<ImportFormatter>(*fpos);
    if(formatter == NULL)
      continue;

    filter << (*fpos)->get_description()
      << " (*." << (*fpos)->get_extension() << ")@*." << (*fpos)->get_extension() << "@";

    if(extension.empty())
      extension = (*fpos)->get_extension();
  }
  // append a filter to display all files
  filter << "All Files (*.*)@*.*@";
  // translate char --> TCHAR and '@' --> '\0'
  // note: Windows stringstream cannot handle '\0' inside a string
  std::wstring extension_w = filter_to_wstring(extension);
  std::wstring filter_w = filter_to_wstring(filter.str());

  CFileDialog dialog(TRUE /* open */,
    extension_w.c_str(), NULL,
    OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_FILEMUSTEXIST,
    filter_w.c_str());
  INT_PTR res = dialog.DoModal();

  if(res != IDOK)
    return -1; // failure

  TRACE("GetInputFile() loading " << dialog.m_szFileTitle << " from " << dialog.m_szFileName);

  wcstombs(fileName, dialog.m_szFileName, _MAX_PATH);
  return 0; // success
}

ImportFormatterPtr CDocumentMonitor::GetImportFormatter(const char* fileName)
{
  const char *fileExtension = strrchr(fileName, '.');
  if(fileExtension == NULL || *fileExtension == '\0')
  {
    MessageBox(GetActiveWindow(),
      _T("No extension given. Cannot determine file type."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
    return ImportFormatterPtr(); // NULL
  }

  // look for the appropriate export formatter
  for(FormatterPtrList::const_iterator fpos = m_formatters.begin();
    fpos != m_formatters.end(); fpos++)
  {
    if(_stricmp(fileExtension+1, (*fpos)->get_extension().c_str()) != 0)
      continue;

    ImportFormatterPtr formatter = boost::dynamic_pointer_cast<ImportFormatter>(*fpos);
    if(formatter == NULL)
      continue;

    return formatter;
  }

  MessageBox(GetActiveWindow(),
    _T("No suitable import filter found."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
  return ImportFormatterPtr(); // NULL
}

void CDocumentMonitor::ImportDocument(const ImportFormatterPtr& formatter, const std::string& filename)
{
  // load the document
  std::vector<MscPtr> drawing = formatter->load_msc(filename);
  if(drawing.empty())
  {
    m_reportView->Print(RS_ERROR,
      stringize() << "Import failed. Cannot load '" << TOWSTRING(filename) << "'.");
    return;
  }

  // perform the post-import transformations
  for(std::vector<MscPtr>::iterator dpos = drawing.begin();
    dpos != drawing.end(); dpos++)
  {
    TransformerPtrList transformer_list;

    ImportFormatter::TransformationList transformations =
      formatter->get_transformations(*dpos);
    // inspect all required transformations
    for(ImportFormatter::TransformationList::const_iterator tpos = transformations.begin();
      tpos != transformations.end(); tpos++)
    {
      TransformerPtrList::const_iterator transformer = find_transformer(*tpos);
      // schedule the transformation
      if(transformer != m_transformers.end())
			{
        transformer_list.push_back(*transformer);

				Beautify* beautify = dynamic_cast<Beautify*>((transformer)->get());
				if (beautify != NULL) 
					beautify->set_is_imported(1);
			}
    }

    // execute the transformations
    MscPtr result = run_transformers(*dpos, transformer_list);
    if(result == NULL)
    {
      m_reportView->Print(RS_ERROR,
        stringize() << "Import failed. Cannot perform post-import transformations.");
      return;
    }
  }

  long scope_id = m_vsoApp->BeginUndoScope("Import");

  CDrawingVisualizer visualizer(m_vsoApp);
  // visualize the drawing
  visualizer.visualize_msc(m_vsoApp->ActiveDocument, drawing);

  m_reportView->Print(RS_NOTICE,
    stringize() << "Document imported successfully.");
  m_vsoApp->EndUndoScope(scope_id, true);
}

int CDocumentMonitor::GetOutputFile(char* fileName)
{
  std::string extension;
  std::stringstream filter;

  // construct the filter string
  // _T("Text Documents (*.txt)\0*.txt\0All Files (*.*)\0*.*\0")
  for(FormatterPtrList::const_iterator fpos = m_formatters.begin();
    fpos != m_formatters.end(); fpos++)
  {
    ExportFormatterPtr formatter = boost::dynamic_pointer_cast<ExportFormatter>(*fpos);
    if(formatter == NULL)
      continue;

    filter << (*fpos)->get_description()
      << " (*." << (*fpos)->get_extension() << ")@*." << (*fpos)->get_extension() << "@";

    if(extension.empty())
      extension = (*fpos)->get_extension();
  }
  // append a filter to display all files
  filter << "All Files (*.*)@*.*@";
  // translate char --> TCHAR and '@' --> '\0'
  // note: Windows stringstream cannot handle '\0' inside a string
  std::wstring extension_w = filter_to_wstring(extension);
  std::wstring filter_w = filter_to_wstring(filter.str());

  CFileDialog dialog(FALSE /* save as */,
    extension_w.c_str(), NULL,
    OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT,
    filter_w.c_str());
  INT_PTR res = dialog.DoModal();

  if(res != IDOK)
    return -1; // failure

  TRACE("GetOutputFile() saving " << dialog.m_szFileTitle << " as " << dialog.m_szFileName);

  wcstombs(fileName, dialog.m_szFileName, _MAX_PATH);
  return 0; // success
}

ExportFormatterPtr CDocumentMonitor::GetExportFormatter(const char* fileName)
{
  const char *fileExtension = strrchr(fileName, '.');
  if(fileExtension == NULL || *fileExtension == '\0')
  {
    MessageBox(GetActiveWindow(),
      _T("No extension given. Cannot determine file type."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
    return ExportFormatterPtr(); // NULL
  }

  // look for the appropriate export formatter
  for(FormatterPtrList::const_iterator fpos = m_formatters.begin();
    fpos != m_formatters.end(); fpos++)
  {
    if(_stricmp(fileExtension+1, (*fpos)->get_extension().c_str()) != 0)
      continue;

    ExportFormatterPtr formatter = boost::dynamic_pointer_cast<ExportFormatter>(*fpos);
    if(formatter == NULL)
      continue;

    return formatter;
  }

  MessageBox(GetActiveWindow(),
    _T("No suitable export filter found."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
  return ExportFormatterPtr(); // NULL
}

void CDocumentMonitor::ExportActiveDocument(const ExportFormatterPtr& formatter, std::ostream& stream)
{
  CDrawingExtractor extractor(m_reportView);

  MscPtr active_msc;
  std::vector<MscPtr> msc;

  // export all pages of the active document
  for(int i = 1; i <= m_vsoDocument->Pages->Count; i++)
  {
    Visio::IVPagePtr vsoPage = m_vsoDocument->Pages->Item[i];
    // extract the drawing
    MscPtr drawing = extractor.extract_msc(vsoPage);
    if(drawing != NULL)
    {
      if(vsoPage == m_vsoApp->ActivePage)
        active_msc = drawing;
      else
        msc.push_back(drawing);
    }
    else
    {
      m_reportView->Print(RS_ERROR,
        stringize() << "Export failed. Graphical errors in the drawing.");
      return;
    }
  }

  // initialize a list of user defined priorities
  // items listed in the same order as m_checkers
  RequiredCheckersList priorities;
  priorities.insert(priorities.begin(), m_checkers.size(), PrerequisiteCheck::PP_DISREGARDED);

  ExportFormatter::PreconditionList preconditions = formatter->get_preconditions(active_msc);
  // walk through all installed checkers
  for(ExportFormatter::PreconditionList::const_iterator ppos = preconditions.begin();
    ppos != preconditions.end(); ppos++)
  {
    CheckerPtrList::const_iterator checker = find_checker(ppos->property_name);
    if(checker != m_checkers.end())
    {
      size_t icheck = checker - m_checkers.begin();

      priorities[icheck] = ppos->priority;
    }
  }

  // execute the check
  int status = run_checks(active_msc, priorities);
  // check the status
  if(status < 0)
  {
    m_reportView->Print(RS_ERROR,
      stringize() << "Export failed. Preconditions violated.");
    return;
  }

  formatter->save_msc(stream, (const wchar_t*)m_vsoDocument->Name, active_msc, msc);

  m_reportView->Print(RS_NOTICE,
    stringize() << "Document exported successfully.");
}

int CDocumentMonitor::DisplayDocument(const MscPtr& msc)
{
  long scope_id = m_vsoApp->BeginUndoScope("Show MSC");
  Visio::IVDocumentPtr vsoDocument;

  try
  {
    vsoDocument = m_vsoApp->Documents->Add(VST_FILE_NAME);
  }
  catch (_com_error&)
  {
    m_vsoApp->EndUndoScope(scope_id, false);

    m_reportView->Print(RS_ERROR,
      _T("Cannot display MSC due to internal error: Bad document template."));
    return 0;
  }

  try
  {
    std::vector<MscPtr> drawing;
    drawing.push_back(msc);

    CDrawingVisualizer visualizer(m_vsoApp);
    visualizer.visualize_msc(vsoDocument, drawing);
  }
  catch(std::exception &exc)
  {
    m_vsoApp->EndUndoScope(scope_id, false);

    short oldresp = m_vsoApp->AlertResponse;
    m_vsoApp->AlertResponse = IDNO; // avoid the "Save? Yes-No" dialog
    // close the document
    vsoDocument->Close();
    m_vsoApp->AlertResponse = oldresp;

    m_reportView->Print(RS_ERROR, stringize()
      << _T("Cannot display MSC due to internal error: ") << exc.what());
    return 0;
  }

  m_vsoApp->EndUndoScope(scope_id, true);
  return 0;
}

int CDocumentMonitor::InvokeHelp(const std::wstring& filename)
{
  std::basic_string<TCHAR> path = GetVisioModulePath();
  path += L"\\help\\" + filename;
  ShellExecute(NULL, L"open", path.c_str() , NULL, NULL, SW_SHOWNORMAL);
  return 0;
  try
  {
    m_vsoApp->InvokeHelp(path.c_str(), HH_DISPLAY_TOPIC, 1);
  }
  catch (_com_error &err)
  {
    m_reportView->Print(RS_ERROR, stringize()
      << _T("Cannot display help due to internal error: ") << err.Description());
  }

  return 0;
}

Visio::IVShapePtr CDocumentMonitor::FindShape(const _bstr_t& id)
{
  for(int i = 1; i <= m_vsoDocument->Pages->Count; i++)
  {
    Visio::IVPagePtr page = m_vsoDocument->Pages->Item[i];

    // check if the shape "id" is on this page
    // the page->Shapes->Item[id] cannot be used as it throws an exception
    for(int j = 1; j <= page->Shapes->Count; j++)
    {
      Visio::IVShapePtr shape = page->Shapes->Item[j];

      if(shape->UniqueID[visGetGUID] == id)
        return shape;
    }
  }

  // the shape was probably deleted by user
  TRACE("CDocumentMonitor::FindShape() shape " << id << " not found");
  return NULL;
}

int CDocumentMonitor::SelectShapes(const std::vector<_bstr_t>& ids)
{
  std::vector<Visio::IVShapePtr> shapes;
  // find referenced shaped
  // note: some of the shapes may have been deleted by user
  for(std::vector<_bstr_t>::const_iterator ipos = ids.begin();
    ipos != ids.end(); ipos++)
  {
    Visio::IVShapePtr shape = FindShape(*ipos);
    if(shape != NULL)
      shapes.push_back(shape);
  }

  // is there something to select?
  if(shapes.size() > 0)
  {
    // the selected shapes must be on a single page
    // the page is obtained from the first available shape
    Visio::IVShapePtr firstshape = shapes[0];
    Visio::IVPagePtr page = firstshape->ContainingPage;

    Visio::IVCellPtr locPinX = firstshape->CellsSRC[visSectionObject][visRowXFormOut][visXFormLocPinX];
    Visio::IVCellPtr locPinY = firstshape->CellsSRC[visSectionObject][visRowXFormOut][visXFormLocPinY];
    double posx, posy;
    firstshape->XYToPage(locPinX->ResultIU, locPinY->ResultIU, &posx, &posy);

    Visio::IVSelectionPtr selection =
      page->CreateSelection(Visio::visSelTypeSingle, Visio::visSelModeSkipSuper, (IDispatch *)firstshape);

    // select the remaining shapes
    for(std::vector<Visio::IVShapePtr>::const_iterator spos = shapes.begin()+1;
      spos != shapes.end(); spos++)
    {
      // ignore shapes on different pages
      if((*spos)->ContainingPage == page)
        selection->Select(*spos, visSelect);
    }

    m_vsoApp->ActiveWindow->Page = (IDispatch *)page;
    m_vsoApp->ActiveWindow->ScrollViewTo(posx, posy);
    m_vsoApp->ActiveWindow->Selection = selection;
  }

  return 0;
}

// $Id: document.cpp 1070 2011-03-29 07:27:45Z madzin $
