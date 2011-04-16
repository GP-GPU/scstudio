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
 * $Id: extract.h 860 2010-08-25 10:46:51Z mbezdeka $
 */

#pragma once
#include "reportview.h"

#include "data/msc.h"

enum TShapeType
{
  ST_BMSC_INSTANCE,
  ST_BMSC_MESSAGE,
  ST_BMSC_MESSAGE_LOST,
  ST_BMSC_MESSAGE_FOUND,
  ST_BMSC_ACTION,
  ST_BMSC_CONDITION,
  ST_BMSC_COREGION,
  ST_BMSC_ORDER_LINE,
  ST_BMSC_ORDER_ARROW,
  ST_HMSC_CONNECTION,
  ST_HMSC_START,
  ST_HMSC_END,
  ST_HMSC_REFERENCE,
  ST_HMSC_CONDITION,
  ST_HMSC_LINE,
  ST_HMSC_ARROW,
  ST_COMMENT,
  ST_TEXT,
  ST_TIME_INTERVAL,
  ST_TIME_DIRECTED,
  ST_TIME_ABSOLUTE,
  ST_PICTURE,
  ST_MARKER_EVENT,
  ST_UNKNOWN
};

//! determine MSC symbol represented by the given shape
TShapeType get_shape_type(Visio::IVShapePtr shape);

enum TDrawingType
{
  DT_UNKNOWN = 0,
  DT_BMSC = 1,
  DT_HMSC = 2
};

//! read a given attribute of the shape
int get_shape_attribute(Visio::IVShapePtr shape, TCHAR* attribute_name);

//! determine type of the drawing on the given page
TDrawingType get_drawing_type(Visio::IVPagePtr vsoPage);

//! determine if shape is message(ordinary, lost, found)
bool isMessageShape(Visio::IVShapePtr shape);

MscPoint GetLineBegin(Visio::IVShapePtr shape);
MscPoint GetLineEnd(Visio::IVShapePtr shape);
MscPoint GetPinPos(Visio::IVShapePtr shape);

class CDrawingExtractor
{
public:
  CDrawingExtractor(CReportView *reporter)
  {
    m_remove_extracted = false;

    m_reporter = reporter;
    m_was_error = false;
  }

  //! extracts drawing at a given page
  MscPtr extract_msc(Visio::IVPagePtr vsoPage);

  bool m_remove_extracted; //! remove the extracted objects; useful for transformers

protected:
  CReportView *m_reporter;
  bool m_was_error;

  //! extracts drawing with given name in a given document
  MscPtr __extract_msc(Visio::IVDocumentPtr vsoDocument, const wchar_t* name);
  //! extracts drawing at a given page
  MscPtr __extract_msc(Visio::IVPagePtr vsoPage);

  void PrintError(const std::wstring& message,
    const std::vector<_bstr_t>& references = std::vector<_bstr_t>())
  {
    m_reporter->Print(RS_ERROR, message, references);
    m_was_error = true;
  }

  void PrintWarning(const std::wstring& message,
    const std::vector<_bstr_t>& references = std::vector<_bstr_t>())
  {
    m_reporter->Print(RS_WARNING, message, references);
  }

  // note: names must be case insensitive as Visio changes page NAME-->Name
  typedef std::map<std::wstring,MscPtr,nocase_comparator<wchar_t> > MscCacheMap;
  MscCacheMap m_msc_cache;

  //! 2D coordinates in visio internal units, having [0,0] bottom left
  struct SPoint
  {
    double m_x;
    double m_y;

    long m_from_id;
    long m_to_id;

    bool operator < (const SPoint& p2) const
    {
      int resy = fcmp(m_y, p2.m_y);
      return resy < 0 || (resy == 0 && fcmp(m_x, p2.m_x) < 0);
    }
  };

  //! get coordinates of the given Connect element
  SPoint get_connect_point(Visio::IVConnectPtr connect);
  SPoint point_to_page(Visio::IVShapePtr shape, const SPoint& point);

  Coordinate ConvertCoordinate(Visio::IVShapePtr shape, double val) const
  {
    return shape->Application->ConvertResult(val, 0, visMillimeters);
  }

  MscPoint ConvertEventPoint(Visio::IVShapePtr shape, const SPoint& point) const
  {
    // note: instances in Visio are rotated by 90', height is the 'x' coordinate
    return MscPoint(
      ConvertCoordinate(shape, point.m_y), ConvertCoordinate(shape, point.m_x));
  }

  Coordinate GetWidth(Visio::IVShapePtr shape)
  {
    return shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters];
  }
  Coordinate GetHeight(Visio::IVShapePtr shape)
  {
    return shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters];
  }
  Coordinate GetControlPos(Visio::IVShapePtr shape, const wchar_t* row);

  //! assert the given shape has no connections to its sub-shapes
  void assert_no_nested_FromConnects(Visio::IVShapePtr shape);
  void assert_no_FromConnects(Visio::IVShapePtr shape);

  // event at given page coordinates
  // contains the sematically first event, if multiple events are attached
  typedef std::map<SPoint,EventPtr> EventPointMap;

  typedef std::map<long,MscMessagePtr> MscMessageMap;
  typedef std::map<long,TimeRelationEventPtr> TimeRelationEventMap;
  typedef std::map<long,TimeRelationRefNodePtr> TimeRelationRefNodeMap;

  template<class M>
  boost::intrusive_ptr<M> find_message(const MscMessageMap& messages, long id)
  {
    MscMessageMap::const_iterator mpos = messages.find(id);
    if(mpos == messages.end())
      return NULL;

    return boost::dynamic_pointer_cast<M>(mpos->second);
  }

  //! process the given coregion and build a relevant CoregionArea
  CoregionAreaPtr create_coregion_area(EventPointMap& events, const MscMessageMap& messages,
    Visio::IVShapePtr coregion);

  struct SStrictOrder
  {
    SPoint event_pos;
    double event_height;
    enum
    {
      ET_INCOMING,
      ET_OUTGOING
    } event_type;

    long shape_id;
    TShapeType shape_type;

    bool operator < (const SStrictOrder& s2) const
    {
      int cres = fcmp(event_height, s2.event_height);
      // no other connection allowed with coregion begin/end
      if(shape_type == ST_BMSC_COREGION || s2.shape_type == ST_BMSC_COREGION)
        return cres < 0;
      // old event < new event, incoming event < outgoing event
      return cres < 0 || (cres == 0 && event_type < s2.event_type);
    }
  };

  InstancePtr new_instance_ptr(Visio::IVShapePtr shape);

  //! process the given page and build a relevant BMsc
  BMscPtr extract_bmsc(Visio::IVPagePtr vsoPage);

  enum TConnectedSide
  {
    CS_ERROR = 0,
    CS_SHAPE_TOP,
    CS_SHAPE_BOTTOM,
    CS_SHAPE_LEFT,
    CS_SHAPE_RIGHT,
    CS_WHOLE_SHAPE
  };
  //! Differentiates between top and bottom connections
  TConnectedSide get_connected_side(Visio::IVConnectPtr connect);

  enum TConnectDirection
  {
    CD_ERROR = 0,
    CD_INCOMING,
    CD_OUTGOING,
    CD_UNKNOWN
  };
  //! Determines a type of connection represented by the given Connect
  TConnectDirection get_connect_direction(Visio::IVConnectPtr connect);

  enum TTimeDirection
  {
    TD_ERROR = 0,
    TD_TOP,
    TD_BOTTOM
  };
  TTimeDirection get_time_direction(Visio::IVConnectPtr connect);

  //! sets given shape/node as a predecessor in the given relation
  void set_as_predecessor(const NodeRelationPtr& relation,
    Visio::IVShapePtr shape, const HMscNodePtr& node);
  //! sets given shape/node as a successor in the given relation
  void set_as_successor(const NodeRelationPtr& relation,
    Visio::IVShapePtr shape, const HMscNodePtr& node);

  //! process the given page and build a relevant HMsc
  HMscPtr extract_hmsc(Visio::IVPagePtr vsoPage);
};

// $Id: extract.h 860 2010-08-25 10:46:51Z mbezdeka $
