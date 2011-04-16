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
 * $Id: visualize.h 1029 2011-02-02 22:17:59Z madzin $
 */

#pragma once

#include "data/msc.h"

class MasterId
{
public:
  MasterId(TShapeType type, int style = 0)
    : m_type(type), m_style(style)
  {
  }

  bool operator < (const MasterId& r) const
  {
    return (m_type < r.m_type) ||
      (m_type == r.m_type && m_style < r.m_style);
  }

private:
  TShapeType m_type;
  int m_style;
};

class CDrawingVisualizer
{
public:
  CDrawingVisualizer(Visio::IVApplicationPtr vsoApp);
  void visualize_msc(Visio::IVDocumentPtr vsoDocument, const std::vector<MscPtr>& drawing);

  bool m_ask_overwrite; //! ask if the target page is not empty

  Visio::IVMasterPtr find_master(TShapeType type, int style = 0);

protected:
  Visio::IVApplicationPtr m_vsoApp;

  typedef std::map<MasterId, Visio::IVMasterPtr> MasterIdMap;
  MasterIdMap m_masters;

  typedef std::map<MscMessagePtr,Visio::IVShapePtr> MessagePtrMap;
  typedef std::map<InstancePtr,Visio::IVShapePtr> InstancePtrMap;
  typedef std::map<TimeRelationEventPtr,Visio::IVShapePtr> TimeRelationEventPtrMap;

  Visio::IVShapePtr DropMaster(Visio::IVPagePtr vsoPage, Visio::IVMasterPtr master, const MscPoint& pos);
  void SetLineBegin(Visio::IVShapePtr what, const MscPoint& pos);
  void SetLineEnd(Visio::IVShapePtr what, const MscPoint& pos);
  void SetControlPos(Visio::IVShapePtr shape, const wchar_t* row, Coordinate pos);

  Visio::IVShapePtr drop_message(Visio::IVPagePtr vsoPage, MessagePtrMap& messages,
    MscMessagePtr message, Visio::IVMasterPtr master);
  void show_event(Visio::IVPagePtr vsoPage, MessagePtrMap& messages,
    Visio::IVShapePtr parent, EventPtr event);
  Visio::IVShapePtr connect_events(Visio::IVPagePtr vsoPage,
    Visio::IVShapePtr parent, EventPtr pred_event, EventPtr succ_event);

  Visio::IVShapePtr drop_time_relation(Visio::IVPagePtr vsoPage, TimeRelationPtr time_relation);
  void show_time_relations(Visio::IVPagePtr vsoPage, TimeRelationEventPtrMap& time_relations,
    Visio::IVShapePtr parent, EventPtr event);

  template<class C>
  Visio::IVShapePtr DropComment(Visio::IVPagePtr vsoPage, boost::intrusive_ptr<C> comment)
  {
    Visio::IVShapePtr result = vsoPage->Drop(find_master(ST_COMMENT), 0, 0);
    result->Text = comment->get_text().c_str();
    result->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters] = comment->get_width();

    switch(comment->get_marked())
    {
      case NONE: CShapeUtils::MarkShape(result, SC_BLACK); break;
      case MARKED: CShapeUtils::MarkShape(result, SC_RED); break;
      case ADDED: CShapeUtils::MarkShape(result, SC_GREEN); break;
      case REMOVED: CShapeUtils::MarkShape(result, SC_RED); break;
      default: throw std::runtime_error("Error: unexpected behaviour");
    }

    return result;
  }

  void visualize_msc(Visio::IVPagePtr vsoPage, const MscPtr& msc);
  // note: insertion to m_printing must not invalidate iterators
  std::list<MscPtr> m_printing;

// FIXME: changed to public
public:
  void visualize_bmsc(Visio::IVPagePtr vsoPage, const BMscPtr& bmsc);
protected:

  typedef std::map<HMscNodePtr,Visio::IVShapePtr> NodePtrMap;
  Visio::IVShapePtr drop_hmsc_node(Visio::IVPagePtr vsoPage, NodePtrMap& nodes, HMscNodePtr node);
  void show_time_relations1(Visio::IVPagePtr vsoPage, Visio::IVShapePtr shape,
    TimeRelationRefNodePtrSet relations);
  void show_time_relations2(Visio::IVPagePtr vsoPage, NodePtrMap& nodes,
    const ReferenceNodePtr& reference_node, Visio::IVShapePtr shape,
    TimeRelationRefNodePtrSet relations);

  void visualize_hmsc(Visio::IVPagePtr vsoPage, const HMscPtr& hmsc);
};

// $Id: visualize.h 1029 2011-02-02 22:17:59Z madzin $
