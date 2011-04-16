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
 * $Id: document.h 1065 2011-03-18 00:43:24Z mbezdeka $
 */

#pragma once
#include "stdafx.h"
#include "reportview.h"
#include "data/formatter.h"
#include "data/checker.h"
#include "data/searcher.h"
#include "data/simulator.h"
#include "data/transformer.h"
#include "enums.h"

#include "pageutils.h"
#include "shapeutils.h"
#include "enumerateUtils.h"

#include "addon.h"

//! template used to create new documents
static const _bstr_t VST_FILE_NAME = _T("MSC.vtx");

typedef std::vector<FormatterPtr> FormatterPtrList;
typedef std::vector<CheckerPtr> CheckerPtrList;
typedef std::vector<TransformerPtr> TransformerPtrList;

template <typename T1, typename T2>
void swap(std::pair<T1,T2> &p)
{
  T1 t = p.first;
  p.first = p.second;
  p.second = t;
}

class CDocumentMonitor : public SimulationListener
{
public:
  CDocumentMonitor(CStudioAddon *addon,
    Visio::IVApplicationPtr vsoApp, Visio::IVDocumentPtr vsoDocument);
  virtual ~CDocumentMonitor();

  VAORC OnDropShape(int iDocumentIndex, int iPageIndex, _bstr_t sShapeU);
  VAORC OnOpenReference(int iDocumentIndex, int iPageIndex, _bstr_t sShapeU);

  void InitMenu();
  void InitToolbar();

  enum MenuItems
  {
    // context menu items
    MENU_ADD_INSTANCES = 102,
    MENU_MESSAGE_SEQUENCE,
    MENU_SELECT_NUMBERED_GROUP,
    MENU_FLIP_MESSAGE_DIRECTION,
    // standard Check menu items
    MENU_WINDOWS_REPORTER = 200,
    MENU_IMPORT,
    MENU_EXPORT,
    MENU_REPAINT,
    MENU_FIND_FLOW,
    MENU_SIMULATION_START,
    MENU_SIMULATION_STOP,
    MENU_SIMULATOR_OPTIONS,
    MENU_VERIFY,
    MENU_CHECK_OPTIONS,
    MENU_SELECT_ALL_INSTANCES,
    MENU_SELECT_ALL_MESSAGES,
    MENU_SELECT_ADD_ALL_INSTANCES,
    MENU_SELECT_ADD_ALL_MESSAGES,
    MENU_MESSAGE_JUMP_LEFT,
    MENU_MESSAGE_JUMP_RIGHT,
    MENU_MESSAGE_JUMP_LEFT_COPY,
    MENU_MESSAGE_JUMP_RIGHT_COPY,
    MENU_ENABLE_MESSAGE_ENUMERATION,
    MENU_DISABLE_MESSAGE_ENUMERATION,
    MENU_GLOBAL_SETTINGS,
		MENU_BEAUTIFY_SETTINGS,
		MENU_IMPORT_SETTINGS,
  };

  VAORC OnMenuWindowsReporter(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuImport(Visio::IVApplicationPtr vsoApp);
	VAORC	OnMenuImportSettings(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuExport(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuRepaint(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuTransform(Visio::IVApplicationPtr vsoApp, int index);
  VAORC OnMenuSelectAllInstances(Visio::IVApplicationPtr vsoApp, SelectionType selType);
  VAORC OnMenuSelectAllMessages(Visio::IVApplicationPtr vsoApp, SelectionType selType);
  VAORC OnMenuMessageJump(Visio::IVApplicationPtr vsoApp, bool left, bool asCopy);
  VAORC OnMenuMessageSequence(Visio::IVApplicationPtr vsoApp, Visio::IVSelectionPtr oldSelection=NULL);
  VAORC OnMenuFindFlow(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuSimulationStart(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuSimulationStop(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuVerify(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuEnableMessageEnumeration(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuDisableMessageEnumeration(Visio::IVApplicationPtr vsoApp);
  VAORC	OnMenuSelectNumberedGroup(Visio::IVApplicationPtr vsoApp);
  VAORC	OnMenuGlobalSettings(Visio::IVApplicationPtr vsoApp);
	VAORC	OnMenuBeautifySettings(Visio::IVApplicationPtr vsoApp);
  VAORC OnMenuFlipMessageDirection(Visio::IVApplicationPtr vsoApp);

  void ShowReportView();
  void OnHideReportView();

  int GetInputFile(char* fileName);
  ImportFormatterPtr GetImportFormatter(const char* fileName);
  void ImportDocument(const ImportFormatterPtr& formatter, const std::string& filename);
  int GetOutputFile(char* fileName);
  ExportFormatterPtr GetExportFormatter(const char* fileName);
  void ExportActiveDocument(const ExportFormatterPtr& formatter, std::ostream& stream);

  int DisplayDocument(const MscPtr& msc);
  int InvokeHelp(const std::wstring& filename);
  Visio::IVShapePtr FindShape(const _bstr_t& id);
  int SelectShapes(const std::vector<_bstr_t>& ids);

  void OnSimulationResult();
  void OnSimulationError();

  /**
   * Retrieves the document being monitored.
   * @return document being monitored
   */
  Visio::IVDocumentPtr GetMonitoredDocument() { return m_vsoDocument; }

  /**
   * Tells whether the active document page is empty.
   * @param vsoApp the application for which to tell the result
   * @return true when the active document page is empty, false if not
   */
  bool IsEmpty(Visio::IVApplicationPtr vsoApp) { return (vsoApp->ActivePage->Shapes->Count == 0); }

  /**
   * Toggles the toolbar items according to whether modifiers (Ctrl or Shift) are pressed.
   * @param modifiersPressed are modifiers (Ctrl or Shift) pressed?
   */
  void ToggleToolbarItems(bool modifiersPressed);

  /**
   * Draws a field of instances on the current page.
   * 
   * All coordinates are expected to be in the millimeters (as it seems to be the units visualizer uses), and the upper-left corner
   * is expected to be at [0,0].
   *
   * @param vsoApp          the monitored application where to draw
   * @param instancesCnt    number of instances to draw
   * @param instanceLength  requested length of the instances
   * @param instanceWidth   requested width of the instances
   * @param startX          x-coordinate where to start drawing the upper left corner of the first instance
   * @param startY          y-coordinate where to start drawing the upper left corner of the first instance
   * @param useConstSpacing use constant (true) or dynamic (false) spacing?
   * @param spacingOrWidth  constant spacing between instances for constant spacing, or total width for dynamic spacing
   * @return result: VAORC_SUCCESS or VAORC_FAILURE
   */
  VAORC DrawInstancesField(Visio::IVApplicationPtr vsoApp, int instancesCnt, Coordinate instanceLength, Coordinate instanceWidth, Coordinate startX, Coordinate startY, bool useConstSpacing, Coordinate spacingOrWidth);

  // event objects for this document
  Visio::IVEventPtr m_vsoPageAddedEvent;
  Visio::IVEventPtr m_vsoBeforeDocumentClosedEvent;

private:
  CDocumentMonitor(const CDocumentMonitor &other); // not implemented

  int LoadModulesFromRegistry(HKEY hKey);

  std::vector<HINSTANCE> m_open_modules;
  FormatterPtrList m_formatters;

  CheckerPtrList m_checkers;
  CheckerPtrList::const_iterator find_checker(const std::wstring& property_name) const;

  typedef std::vector<int> RequiredCheckersList;
  int run_checks(MscPtr& msc, const RequiredCheckersList& priorities);

  TransformerPtrList m_transformers;
  TransformerPtrList::const_iterator find_transformer(const std::wstring& name) const;

  short beautifyIndex;
  short beautifyCmdNum;
  short tightenTimeIndex; 

  MscPtr run_transformers(MscPtr& msc, const TransformerPtrList& transformer_list);

  MeasurementMap m_marker_measurements;
  virtual void on_result(const MeasurementMap& measurements)
  {
    m_marker_measurements = measurements;
    m_vsoApp->QueueMarkerEvent("SimulationResult");
  }

  std::string m_marker_error;
  virtual void on_error(const std::string& what)
  {
    m_marker_error = what;
    m_vsoApp->QueueMarkerEvent("SimulationError");
  }

#if(EXCEL_FOUND)
  Excel::_ApplicationPtr m_excelApp;
  Excel::_WorksheetPtr m_outputSheet;

  typedef std::map<std::string, int> AnnotatedColumns;
  AnnotatedColumns m_annotatedColumns;
  size_t m_annotatedRows;
#endif // EXCEL_FOUND

  int check_membership_preconditions(MscPtr& msc);
  SearcherPtr m_membership;
  SimulatorPtr m_simulator;

  CStudioAddon *m_addon;
  Visio::IVApplicationPtr m_vsoApp;
  Visio::IVDocumentPtr m_vsoDocument;

  bool m_reportVisible;
  Visio::IVWindowPtr m_reportWindow;
  Visio::IVMenuItemPtr m_reportMenuItem;
  CReportView *m_reportView;
  ConfigProvider *m_configProvider;

  Visio::IVToolbarPtr m_toolbar;

  Visio::IVMenuItemPtr m_simulationStartMenuItem;
  Visio::IVMenuItemPtr m_simulationStopMenuItem;
  Visio::IVToolbarItemPtr m_simulationStartToolbarItem;
  Visio::IVToolbarItemPtr m_simulationStopToolbarItem;

  Visio::IVToolbarItemPtr m_selInstancesToolbarItem;
  Visio::IVToolbarItemPtr m_selMessagesToolbarItem;
  Visio::IVToolbarItemPtr m_selAddInstancesToolbarItem;
  Visio::IVToolbarItemPtr m_selAddMessagesToolbarItem;
  //Message enumeration
  Visio::IVToolbarItemPtr m_enableMessageEnumerationToolbarItem;
  Visio::IVToolbarItemPtr m_disableMessageEnumerationToolbarItem;
  //Message jumping
  Visio::IVToolbarItemPtr m_msgJumpLeftToolbarItem;
  Visio::IVToolbarItemPtr m_msgJumpRightToolbarItem;
  Visio::IVToolbarItemPtr m_msgJumpLeftCopyToolbarItem;
  Visio::IVToolbarItemPtr m_msgJumpRightCopyToolbarItem;

  /**
   * Selects all instances or all messages on the active page.
   * @param vsoApp           the application to make selection in
   * @param select_instances when true, selects all instances,
   *                         when false, selects all messages (including lost and found messages)
   * @param selType          selection type - either add all objects to an existing selection, or
   *                         replace the old selection with the new one
   */
  void selectAll(Visio::IVApplicationPtr vsoApp, bool select_instances, SelectionType selType); // FIXME: refactor: pass TShapeType instead of select_instances and use CPageUtils::filterSelection() (but watch out when adding to an existing selection)

  /**
   * Gets a pair of instances based on the current selection.
   *
   * If there are no instances selected in the application, the instances returned
   * are the leftmost and the rightmost of all instances on the active page.
   * If there is exactly 1 instance selected, it puts it into the returned pair both at first
   * and NULL at second position.
   * If there are exactly 2 instances selected, these two are returned in the order of
   * their selection.
   * If more than 2 instances are selected, returned are the leftmost and the rightmost
   * of the selected instances.
   * Anyway, if there are no instances available, a pair of two NULL pointers is returned,
   * or if there is just one instance, a pair with a pointer to it and NULL is returned.
   *
   * @param vsoApp           the application to get the instances from
   * @return a pair of instances chosen using rules mentioned above
   */
  std::pair<Visio::IVShapePtr, Visio::IVShapePtr> getInstancesPair(Visio::IVApplicationPtr vsoApp);

  /**
   * Gets all instances between some two instances at a given height.
   * 
   * Taken are all instances with x-coords between x-coords of the two instances given and
   * y-coords beginning above yCoord and ending below yCoords (imagine a horizontal line at
   * height yCoord - then the instances taken into account are only these intersecting this line).
   * 
   * Furthermore, if there is a selection of at least 3 instances on the page, only those instances
   * present in this selection are chosen.
   *
   * The function respects the order of boundaries: if boundaryOne is the left of the two,
   * all instances returned are ordered by the x-coords in the ascending direction; otherwise,
   * they are returned in a descending order.
   * In the special case both boundaries are of same x-coords, an empty list is returned.
   * An empty list is also returned if any of the boundaries is not strictly vertical.
   * Those other instances which would qualify to the resulting list, but are not vertical,
   * are filtered out.
   *
   * The yCoord is an absolute coordinate (i.e. relative to the page); if any of the boundary
   * instances do not intersect a line at yCoord, an empty list is returned.
   *
   * @param boundaryOne     an instances serving as one of boundaries
   * @param boundaryTwo     another instance serving as a boundary
   * @param yCoord          height of an imaginary line which all returned instances must intersect
   * @return a list of instances meeting above criteria according to boundaries and yCoord given
   */
  std::vector<Visio::IVShapePtr> getInstancesInBetween(Visio::IVShapePtr boundaryOne, Visio::IVShapePtr boundaryTwo, double yCoord);

  /**
   * Draws a message between the given instances at yCoord.
   *
   * On an error (possibly because of a forbidden coregion), an error message box is invoked.
   * 
   * @param msgMaster         message master shape
   * @param orderingMaster    ordering master shape
   * @param from              from which instance
   * @param to                to which instance
   * @param yCoord            the y-coordinate at which to draw the message; relative to the page
   * @param caption           caption of the message
   * @param coregionTreatment the way coregions should be treated when crossed by the message
   * @param prevMsg           previous message in the sequence which to connect this message with if the
   *                          coregion treatment is some kind of connection;
   *                          might be NULL in which case no connecting will take place
   * @return pointer to the message shape drawn or NULL if the message could not be drawn (probably because of a forbidden coregion)
   */
  Visio::IVShapePtr DrawMessage(Visio::IVMasterPtr msgMaster, Visio::IVMasterPtr orderingMaster, Visio::IVShapePtr from, Visio::IVShapePtr to, double yCoord, const TCHAR* caption, MsgSeqCoregionTreatment coregionTreatment, Visio::IVShapePtr prevMsg);

public:
  /**
   * Recompute positions in order left - right, top - down a draws the numbers into text field
   * @param shapesOnPage	pointer to all shapes on a page
   */
  void	drawNumbers(Visio::IVShapesPtr shapesOnPage, _bstr_t groupID);
  /**
   * Disable/enable enumeration toolbar buttons, menu buttons and hot keys during text editing
   * @param enabled		Enable or disable controls
   */
  void	ToogleNumeration(bool enabled);

private:
  void autoEnumerate(Visio::IVShapePtr msgShape);
  void enumerate(Visio::IVApplicationPtr vsoApp, _bstr_t groupID, bool enable, bool onlySelected);

  Visio::IVMenuItemsPtr	visMenuItems;
  Visio::IVMenuItemPtr itemMessageEnumerating;
  Visio::IVAccelItemPtr accelItemMessageEnumerationDisable;
  Visio::IVAccelItemPtr accelItemMessageEnumeration;

public:
  Visio::IVUIObjectPtr GetMostCustomMenus();
  Visio::IVUIObjectPtr GetMostCustomToolbars();
};

// $Id: document.h 1065 2011-03-18 00:43:24Z mbezdeka $
