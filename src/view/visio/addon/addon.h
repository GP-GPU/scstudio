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
 * $Id: addon.h 884 2010-09-07 22:14:50Z mbezdeka $
 */

#pragma once

#include <map>
#include <set>
#include <vector>

class CReportView;
class CDocumentMonitor;

// internal add-on name
#define ADDON_NAME "Sequence Chart Studio"

class CStudioAddon : public VAddon, public VEventHandler
{
public:
  CStudioAddon(LPCTSTR pNameU, UINT uIDLocalName);
  virtual VAORC About(LPVAOV2LSTRUCT pV2L);
  virtual VAORC Run(LPVAOV2LSTRUCT pV2L);
  virtual VAORC Unload(WORD wParam, LPVOID p);

  VAORC DisplaySimulatorOptions();
  VAORC DisplayCheckOptions();
  VAORC DoInstancesField(CDocumentMonitor* documentMonitor, Visio::IVApplicationPtr vsoApp);

  virtual HRESULT HandleVisioEvent(
    IUnknown *ipSink, // [in] ipSink [assert]
    short nEventCode, // [in] code of event that's firing
    IDispatch *pSourceObj, //! [in] object that is firing event
    long nEventID, // [in] id of event that is firing
    long nEventSeqNum, // [in] sequence number of event
    IDispatch *pSubjectObj, // [in] subject of this event
    VARIANT vMoreInfo, // [in] other info
    VARIANT *pvResult); // [retval][out] return a value to Visio for query events

  void HandleCellChanged(Visio::IVCellPtr vsoCell);
  void HandleConnectionsAdded(Visio::IVConnectsPtr vsoConnects);
  void HandleMarker(Visio::IVApplicationPtr vsoApp);
  void HandleKeyDown(Visio::IVKeyboardEventPtr vsoKeyboardEvent, Visio::IVApplicationPtr vsoApp);
  void HandleKeyUp(Visio::IVKeyboardEventPtr vsoKeyboardEvent, Visio::IVApplicationPtr vsoApp);
  void HandleMouseMove(Visio::IVMouseEventPtr vsoMouseEvent, Visio::IVApplicationPtr vsoApp);
  void HandleMouseDown(Visio::IVMouseEventPtr vsoMouseEvent, Visio::IVApplicationPtr vsoApp);
  void HandleWinSelChange(Visio::IVApplicationPtr vsoApp);
  void HandleBeforeTextEdit(Visio::IVShapePtr shapePtr, Visio::IVApplicationPtr vsoApp);
  void HandleAfterTextEdit(Visio::IVShapePtr shapePtr, Visio::IVApplicationPtr vsoApp);
  void HandleBeforeShapeDeleted(Visio::IVShapePtr shapePtr, Visio::IVApplicationPtr vsoApp);
  void HandleTextChanged(Visio::IVShapePtr shapePtr,Visio::IVDocumentPtr vsoDocument);
  void HandleNonePending(Visio::IVApplicationPtr vsoApp);
  void HandleVisioIsIdle(Visio::IVApplicationPtr vsoApp);

  void RegisterPersistentEvents(Visio::IVDocumentPtr vsoDocument);
  CDocumentMonitor *GetDocumentMonitor(Visio::IVApplicationPtr vsoApp, Visio::IVDocumentPtr vsoDocument);
  void StopDocumentMonitor(Visio::IVApplicationPtr vsoApp, Visio::IVDocumentPtr vsoDocument);

  enum TAddonState
  {
    STATE_INIT,
    STATE_MESSAGE_SEQUENCE_WAITING_FOR_SEL_CHANGE,
  };

  TAddonState GetState(Visio::IVApplicationPtr vsoApp) { return m_states[vsoApp]; }
  void SetState(Visio::IVApplicationPtr vsoApp, TAddonState state, const TCHAR* message=NULL);
  void ResetState(Visio::IVApplicationPtr vsoApp);

  /**
   * Gets last mouse X-coordinate.
   * Watch out! It is in units of current page.
   * @return last X-coordinate of mouse pointer in units of current page
   */
  double GetMousePosX() { return m_mousePosX; }

  /**
   * Gets last mouse Y-coordinate.
   * Watch out! It is in units of current page.
   * @return last Y-coordinate of mouse pointer in units of current page
   */
  double GetMousePosY() { return m_mousePosY; }

  /**
   * Saves current mouse position for a later retrieval.
   * @param vsoApp the app to save the position for
   */
  void SaveMousePos(Visio::IVApplicationPtr vsoApp) {
    m_prevMousePosX[vsoApp] = m_mousePosX;
    m_prevMousePosY[vsoApp] = m_mousePosY;
  }

  /**
   * Retrieves a previously stored mouse position.
   * It is in units of current page.
   * @param vsoApp the app it was stored in
   * @return previously stored X-coordinate of mouse pointer in units of current page
   */
  double GetPrevMousePosX(Visio::IVApplicationPtr vsoApp) { return m_prevMousePosX[vsoApp]; }

  /**
   * Retrieves a previously stored mouse position.
   * It is in units of current page.
   * @param vsoApp the app it was stored in
   * @return previously stored Y-coordinate of mouse pointer in units of current page
   */
  double GetPrevMousePosY(Visio::IVApplicationPtr vsoApp) { return m_prevMousePosY[vsoApp]; }

  double GetRelMousePosX() { return m_mouseRelPosX; }
  double GetRelMousePosY() { return m_mouseRelPosY; }

  void  SetOnDropShapeState(bool state) { m_bOnDropShape = state;}
  bool  GetOnDropShapeState() { return m_bOnDropShape; }

  bool  GetCtrlKeyDown() { return m_bCtrlDown; } 

private:
  IUnknown *m_pIAddonSink;
  Visio::IVEventPtr m_vsoMarkerEvent;

  typedef std::map<long, CDocumentMonitor*> DocumentMonitorsMap;
  DocumentMonitorsMap m_DocumentMonitors;

  long m_keyButtonState;
  double m_mousePosX;
  double m_mousePosY;
  bool m_bKeyDown;
  

  std::map<long, TAddonState> m_states;
  std::map<long, Visio::IVSelectionPtr> m_oldSelections;
  std::map<long, Visio::IVSelectionPtr> m_curSelections;
  std::map<long, double> m_prevMousePosX;
  std::map<long, double> m_prevMousePosY;
  
  //Message numbering
  bool				m_bMessageTextEdited;
  bool				m_bCellChanged;
  bool				m_bShapeDeleted;
  std::set<_bstr_t>	m_enumerationGroups;

  //Message snapping
  bool        m_bSnap;
  bool        m_bBeginChanged;
  bool        m_bEndChanged;
  bool        m_bMoving;
  bool        m_bCtrlDown;
  bool        m_bArrowKeyDown;
  bool        m_bOnDropShape;
  bool        m_bShapeChanged;
  double      m_mouseRelPosX;
  double      m_mouseRelPosY;
  std::vector<Visio::IVShapePtr> instances;

  //Message jumping
  int         m_iJumpType;
};

// $Id: addon.h 884 2010-09-07 22:14:50Z mbezdeka $
