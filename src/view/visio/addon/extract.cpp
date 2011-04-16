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
 * $Id: extract.cpp 860 2010-08-25 10:46:51Z mbezdeka $
 */

#include "stdafx.h"
#include "extract.h"
#include "errors.h"

void RemoveKnownSymbols(Visio::IVPagePtr page)
{
  Visio::IVSelectionPtr selection =
    page->CreateSelection(Visio::visSelTypeEmpty, Visio::visSelModeSkipSuper);

  // walk through all shapes
  for(int i = 1; i <= page->Shapes->Count; i++)
  {
    Visio::IVShapePtr shape = page->Shapes->Item[i];

    TShapeType type = get_shape_type(shape);
    switch(type)
    {
      case ST_BMSC_INSTANCE:
      case ST_BMSC_MESSAGE:
      case ST_BMSC_MESSAGE_LOST:
      case ST_BMSC_MESSAGE_FOUND:
      case ST_BMSC_COREGION:
      case ST_BMSC_ORDER_LINE:
      case ST_BMSC_ORDER_ARROW:
      case ST_HMSC_CONNECTION:
      case ST_HMSC_START:
      case ST_HMSC_END:
      case ST_HMSC_REFERENCE:
      case ST_HMSC_CONDITION:
      case ST_HMSC_LINE:
      case ST_HMSC_ARROW:
      case ST_COMMENT:
      case ST_TEXT:
      case ST_TIME_INTERVAL:
      case ST_TIME_DIRECTED:
        // known symbols will be repainted
        selection->Select(shape, Visio::visSelect);
        break;

      case ST_TIME_ABSOLUTE:
      case ST_BMSC_ACTION:
      case ST_BMSC_CONDITION:
      case ST_PICTURE:
      case ST_MARKER_EVENT:
      case ST_UNKNOWN:
        // keep unknown and shapes not yet supported in msc.h
        break;
    }
  }

  // delete all selected shapes
  selection->Delete();
}

MscPtr CDrawingExtractor::extract_msc(Visio::IVPagePtr vsoPage)
{
  // execute the extraction
  MscPtr result = __extract_msc(vsoPage);

  if(result && m_remove_extracted)
  {
    // remove known symbols from all extracted pages
    // note: all extracted pages are stored in the cache
    for(int i = 1; i <= vsoPage->Document->Pages->Count; i++)
    {
      Visio::IVPagePtr page = vsoPage->Document->Pages->Item[i];

      MscCacheMap::iterator mpos = m_msc_cache.find((const wchar_t*)(page->Name));
      // if the drawing is in the cache
      if(mpos != m_msc_cache.end())
        RemoveKnownSymbols(page);
    }
  }

  return result;
}

MscPtr CDrawingExtractor::__extract_msc(Visio::IVDocumentPtr vsoDocument, const wchar_t* name)
{
  // random walk though all pages in the given document
  for(int i = 1; i <= vsoDocument->Pages->Count; i++)
  {
    Visio::IVPagePtr page = vsoDocument->Pages->Item[i];

    if(_wcsicmp(page->Name, name) == 0)
      return __extract_msc(page);
  }

  return NULL;
}

MscPtr CDrawingExtractor::__extract_msc(Visio::IVPagePtr vsoPage)
{
  MscCacheMap::iterator mpos = m_msc_cache.find((const wchar_t*)(vsoPage->Name));
  if(mpos != m_msc_cache.end())
  {
    TRACE("__extract_msc() drawing found in cache");
    return mpos->second;
  }

  TDrawingType type = get_drawing_type(vsoPage);
  if(type == DT_BMSC)
  {
    return extract_bmsc(vsoPage);
  }
  else if(type == DT_HMSC)
  {
    return extract_hmsc(vsoPage);
  }
  else
  {
    PrintError(stringize() << vsoPage->Name << ": "
      << "Drawing is neither basic MSC nor HMSC.");
    return NULL;
  }
}

TShapeType get_shape_type(Visio::IVShapePtr shape)
{
  // walk though all user-defined cells
  // note: rows are numbered starting with 0
  for(short i = 0; i < shape->RowCount[visSectionUser]; i++)
  {
    Visio::IVCellPtr cell = shape->CellsSRC[visSectionUser][visRowUser+i][visUserValue];
    if(cell == NULL)
      continue;

    // note: to keep stencil compatibility, the names must not be changed
    struct SymbolsStruct {
      TCHAR *name;
      TShapeType type;
    } symbols[] = {
      {_T("bmsc.instance.line"), ST_BMSC_INSTANCE},
      {_T("bmsc.message"),       ST_BMSC_MESSAGE},
      {_T("bmsc.message.lost"),  ST_BMSC_MESSAGE_LOST},
      {_T("bmsc.message.found"), ST_BMSC_MESSAGE_FOUND},
      {_T("bmsc.action"),        ST_BMSC_ACTION},
      {_T("bmsc.condition"),     ST_BMSC_CONDITION},
      {_T("bmsc.coregion.box"),  ST_BMSC_COREGION},
      {_T("bmsc.order.line"),    ST_BMSC_ORDER_LINE},
      {_T("bmsc.order.arrow"),   ST_BMSC_ORDER_ARROW},
      {_T("hmsc.connection"),    ST_HMSC_CONNECTION},
      {_T("hmsc.start"),         ST_HMSC_START},
      {_T("hmsc.end"),           ST_HMSC_END},
      {_T("hmsc.reference"),     ST_HMSC_REFERENCE},
      {_T("hmsc.condition"),     ST_HMSC_CONDITION},
      {_T("hmsc.line"),          ST_HMSC_LINE},
      {_T("hmsc.arrow"),         ST_HMSC_ARROW},
      {_T("comment"),            ST_COMMENT},
      {_T("text"),               ST_TEXT},
      {_T("time.interval"),      ST_TIME_INTERVAL},
      {_T("time.directed"),      ST_TIME_DIRECTED},
      {_T("time.absolute"),      ST_TIME_ABSOLUTE},
      {_T("picture"),            ST_PICTURE},
      {_T("marker.event"),       ST_MARKER_EVENT},
      {NULL,                     ST_UNKNOWN}
    };

    if(_tcsicmp(cell->RowName, _T("mscSymbol")) == 0)
    {
      _bstr_t symbol_name = cell->ResultStr[visNoCast];

      for(SymbolsStruct *pos = symbols; pos->name != NULL; pos++)
      {
        if(_tcsicmp(symbol_name, pos->name) == 0)
          return pos->type;
      }

      // shape with unknown mscSymbol
      TRACE("get_shape_type() found unknown mscSymbol '" << symbol_name << "'");
      return ST_UNKNOWN;
    }
  }

  // shape without mscSymbol
  return ST_UNKNOWN;
}

int get_shape_attribute(Visio::IVShapePtr shape, TCHAR* attribute_name)
{
  // walk though all user-defined cells
  for(short i = 0; i < shape->RowCount[visSectionUser]; i++)
  {
    Visio::IVCellPtr cell = shape->CellsSRC[visSectionUser][visRowUser+i][visUserValue];
    if(cell != NULL && _tcsicmp(cell->RowName, attribute_name) == 0)
      return (int)cell->Result[visNoCast];
  }

  return 0; // default value
}

bool isMessageShape(Visio::IVShapePtr shape)
{
  switch(get_shape_type(shape))
  {
  case ST_BMSC_MESSAGE:
  case ST_BMSC_MESSAGE_FOUND:
  case ST_BMSC_MESSAGE_LOST:
    return true;
  }
  return false;
}

TDrawingType get_drawing_type(Visio::IVPagePtr vsoPage)
{
  Visio::IVShapesPtr vsoShapes = vsoPage->Shapes;

  // walk through all shapes in the drawing
  for(int i = 1; i <= vsoShapes->Count; i++)
  {
    // check for significant shapes in the drawing
    TShapeType type = get_shape_type(vsoShapes->Item[i]);
    switch(type)
    {
      case ST_BMSC_INSTANCE:
        TRACE("get_drawing_type() detected bMSC");
        return DT_BMSC;

      case ST_HMSC_START:
      case ST_HMSC_END:
      case ST_HMSC_REFERENCE:
        TRACE("get_drawing_type() detected HMSC");
        return DT_HMSC;

      default:
        break;
    }
  }

  TRACE("get_drawing_type() unknown drawing type");
  return DT_UNKNOWN;
}

inline double align5(double mm)
{
  return floor(mm/5)*5;
}

MscPoint GetLineBegin(Visio::IVShapePtr shape)
{
  double page_height = align5(shape->ContainingPage->PageSheet->CellsSRC[visSectionObject][visRowPage][visPageHeight]->Result[visMillimeters]);

  return MscPoint(
    shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX]->Result[visMillimeters],
    page_height - shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginY]->Result[visMillimeters]);
}

MscPoint GetLineEnd(Visio::IVShapePtr shape)
{
  double page_height = align5(shape->ContainingPage->PageSheet->CellsSRC[visSectionObject][visRowPage][visPageHeight]->Result[visMillimeters]);

  return MscPoint(
    shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX]->Result[visMillimeters],
    page_height - shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndY]->Result[visMillimeters]);
}

MscPoint GetPinPos(Visio::IVShapePtr shape)
{
  double page_height = align5(shape->ContainingPage->PageSheet->CellsSRC[visSectionObject][visRowPage][visPageHeight]->Result[visMillimeters]);

  return MscPoint(
    shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormPinX]->Result[visMillimeters],
    page_height - shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormPinY]->Result[visMillimeters]);
}

CDrawingExtractor::SPoint CDrawingExtractor::get_connect_point(Visio::IVConnectPtr connect)
{
  SPoint result; // [visio internal units]
  Visio::IVShapePtr shape = connect->ToSheet;

  short cellrow = connect->ToCell->Row;
  result.m_x = shape->CellsSRC[visSectionConnectionPts][cellrow][visX]->Result[0];
  result.m_y = shape->CellsSRC[visSectionConnectionPts][cellrow][visY]->Result[0];

  result.m_from_id = connect->FromSheet->ID;
  result.m_to_id = connect->ToSheet->ID;

  return result;
}

CDrawingExtractor::SPoint
CDrawingExtractor::point_to_page(Visio::IVShapePtr shape, const CDrawingExtractor::SPoint& point)
{
  SPoint result; // [visio internal units]
  shape->XYToPage(point.m_x, point.m_y, &result.m_x, &result.m_y);

  return result;
}

Coordinate CDrawingExtractor::GetControlPos(Visio::IVShapePtr shape, const wchar_t* row)
{
  // walk though all controls
  // note: rows are numbered starting with 0
  for(short i = 0; i < shape->RowCount[visSectionControls]; i++)
  {
    Visio::IVCellPtr cell = shape->CellsSRC[visSectionControls][visRowControl+i][visCtlY];
    if(cell == NULL)
      continue;

    if(_tcsicmp(cell->RowName, row) == 0)
      return cell->Result[visMillimeters];
  }

  return 0;
}

void CDrawingExtractor::assert_no_nested_FromConnects(Visio::IVShapePtr shape)
{
  for(int i = 1; i <= shape->Shapes->Count; i++)
    assert_no_FromConnects(shape->Shapes->Item[i]);
}

void CDrawingExtractor::assert_no_FromConnects(Visio::IVShapePtr shape)
{
  std::wstring page_name = (const wchar_t*)shape->ContainingPage->Name;

  // assert this shape has no FromConnect entries
  if(shape->FromConnects->Count > 0)
  {
    PrintWarning(stringize() << page_name << ": "
      << "Unexpected connections to '" << shape->Name << "'",
      shapelist() << shape);
  }

  for(int i = 1; i <= shape->Shapes->Count; i++)
    assert_no_FromConnects(shape->Shapes->Item[i]);
}

CoregionAreaPtr CDrawingExtractor::create_coregion_area(EventPointMap& events,
  const MscMessageMap& messages, Visio::IVShapePtr coregion)
{
  Visio::IVPagePtr vsoPage = coregion->ContainingPage;
  std::wstring page_name = (const wchar_t*)vsoPage->Name;

  typedef std::multimap<SPoint,CoregionEventPtr> TCoregionEvents;
  TCoregionEvents coregion_events; // events in this coregion
  std::set<long> relations;

  CoregionAreaPtr area = new CoregionArea();

  // assert there is no connection to inner shapes of the coregion group
  assert_no_nested_FromConnects(coregion);

  // step 1: create events
  // random walk through all connectors from other shapes
  for(int i = 1; i <= coregion->FromConnects->Count; i++)
  {
    Visio::IVConnectPtr connect = coregion->FromConnects->Item[i];
    SPoint pos = get_connect_point(connect);
    // shape connected to this point
    Visio::IVShapePtr shape = connect->FromSheet;

    CoregionEventPtr event;

    // connect the message
    TShapeType type = get_shape_type(shape);
    switch(type)
    {
      case ST_BMSC_MESSAGE:
      {
        CompleteMessagePtr message = find_message<CompleteMessage>(messages, shape->ID);
        if(message == NULL)
          continue;
        // create coregion event
        event = area->add_event(new CoregionEvent());
        event->set_position(ConvertEventPoint(coregion, pos));
        // connect the message
        switch(connect->FromPart)
        {
          case visBegin:
            message->glue_send_event(event);
            break;
          case visEnd:
            message->glue_receive_event(event);
            break;

          default:
            PrintError(stringize() << page_name << ": "
              << "Wrongly connected message '" << shape->Name << "'",
              shapelist() << shape << coregion);
            continue;
        }
        break;
      }

      case ST_BMSC_MESSAGE_LOST:
      {
        IncompleteMessagePtr message = find_message<IncompleteMessage>(messages, shape->ID);
        if(message == NULL)
          continue;
        // create coregion event
        event = area->add_event(new CoregionEvent());
        event->set_position(ConvertEventPoint(coregion, pos));
        // connect the message
        if(connect->FromPart == visBegin)
          message->glue_event(event);
        else
        {
          PrintError(stringize() << page_name << ": "
            << "Wrongly connected message '" << shape->Name << "'",
            shapelist() << shape << coregion);
          continue;
        }
        break;
      }
      case ST_BMSC_MESSAGE_FOUND:
      {
        IncompleteMessagePtr message = find_message<IncompleteMessage>(messages, shape->ID);
        if(message == NULL)
          continue;
        // create coregion event
        event = area->add_event(new CoregionEvent());
        event->set_position(ConvertEventPoint(coregion, pos));
        // connect the message
        if(connect->FromPart == visEnd)
          message->glue_event(event);
        else
        {
          PrintError(stringize() << page_name << ": "
            << "Wrongly connected message '" << shape->Name << "'",
            shapelist() << shape << coregion);
          continue;
        }
        break;
      }

      case ST_BMSC_ORDER_LINE:
      case ST_BMSC_ORDER_ARROW:
        relations.insert(shape->ID);
        continue;

      // ignore other shapes
      default:
        continue;
    }

    // store the CoregionEventPtr for each point
    coregion_events.insert(std::make_pair<SPoint,CoregionEventPtr>(pos, event));
    events.insert(std::make_pair<SPoint,EventPtr>(point_to_page(coregion, pos), event));
  }

  // check for collisional shapes
  for(TCoregionEvents::iterator epos = coregion_events.begin();
    epos != coregion_events.end(); epos = coregion_events.upper_bound(epos->first))
  {
    if(coregion_events.count(epos->first) > 1)
    {
      shapelist this_shapelist;

      std::pair<TCoregionEvents::iterator, TCoregionEvents::iterator> collision =
        coregion_events.equal_range(epos->first);
      for(TCoregionEvents::iterator cpos = collision.first;
        cpos != collision.second; cpos++)
      {
        this_shapelist << vsoPage->Shapes->ItemFromID[cpos->first.m_from_id];
      }

      PrintError(stringize() << page_name << ": "
        << "Multiple events cannot be attached to one point.",
        this_shapelist << coregion);
    }
  }

  // step 2: connect events
  for(std::set<long>::const_iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    Visio::IVShapePtr line = vsoPage->Shapes->ItemFromID[*rpos];

    // check the connector is properly connected
    if(line->Connects->Count != 2)
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached general ordering.",
        shapelist() << line);
      continue;
    }

    Visio::IVConnectPtr connect1 = line->Connects->Item[1];
    SPoint pos1 = get_connect_point(connect1);
    Visio::IVConnectPtr connect2 = line->Connects->Item[2];
    SPoint pos2 = get_connect_point(connect2);

    TCoregionEvents::const_iterator event1 = coregion_events.find(pos1);
    TCoregionEvents::const_iterator event2 = coregion_events.find(pos2);
    if(event1 == coregion_events.end() || event2 == coregion_events.end()
      || event1 == event2)
    {
      PrintError(stringize() << page_name << ": "
        << "General ordering must connect two different events.",
        shapelist() << line);
      continue;
    }

    TShapeType line_type = get_shape_type(line);
    if(line_type == ST_BMSC_ORDER_LINE)
    {
      // the position is determined from the X coordinate
      if(pos1.m_x < pos2.m_x)
        event1->second->add_successor(event2->second.get());
      else if(pos2.m_x < pos1.m_x)
        event2->second->add_successor(event1->second.get());
      else
      {
        PrintError(stringize() << page_name << ": "
          << "Cannot determine event order.",
          shapelist() << line);
      }
    }
    else if(line_type == ST_BMSC_ORDER_ARROW)
    {
      // the position is determined from the arrow
      if(connect1->FromPart == visBegin && connect2->FromPart == visEnd)
        event1->second->add_successor(event2->second.get());
      else if(connect2->FromPart == visBegin && connect1->FromPart == visEnd)
        event2->second->add_successor(event1->second.get());
      else
      {
        PrintError(stringize() << page_name << ": "
          << "Cannot determine event order.",
          shapelist() << line);
      }
    }
    else
    {
      // this should never happen
      TRACE("process_coregion() unknown connector in the list of relations");
    }
  }

  return area;
}

InstancePtr CDrawingExtractor::new_instance_ptr(Visio::IVShapePtr shape)
{
  InstancePtr result = new Instance((const wchar_t*)shape->Text);

  result->set_visual_style(get_shape_attribute(shape, _T("mscStyle")));
  result->set_line_begin(GetLineBegin(shape));
  result->set_line_end(GetLineEnd(shape));
  result->set_width(2.0*GetControlPos(shape, _T("mscHeadWidth")));

  return result;
}

BMscPtr CDrawingExtractor::extract_bmsc(Visio::IVPagePtr vsoPage)
{
  // keep previous error indicator; this function may be called recursively
  bool was_error = m_was_error;
  m_was_error = false;

  std::wstring page_name = (const wchar_t*)vsoPage->Name;
  BMscPtr bmsc = new BMsc(page_name);
  m_msc_cache[page_name] = bmsc;

  // temporary mappers Visio shape-id --> msc.h
  EventPointMap events; // all events in the drawing
  std::map<long,InstancePtr> instances;
  MscMessageMap messages;
  TimeRelationEventMap time_relations;
  std::map<long,CommentPtr> comments;

  // first walk through all shapes: create objects
  for(int i = 1; i <= vsoPage->Shapes->Count; i++)
  {
    Visio::IVShapePtr shape = vsoPage->Shapes->Item[i];

    try
    {
      TShapeType type = get_shape_type(shape);
      switch(type)
      {
        case ST_BMSC_INSTANCE:
          instances[shape->ID] = new_instance_ptr(shape);
          break;

        case ST_BMSC_MESSAGE:
          messages[shape->ID] = new CompleteMessage((const wchar_t*)shape->Text);
          break;
        case ST_BMSC_MESSAGE_LOST:
        {
          IncompleteMessage *new_message = new IncompleteMessage(LOST, (const wchar_t*)shape->Text);
          // lost message: glued on begin, dot on end
          new_message->set_dot_position(GetLineEnd(shape)-GetLineBegin(shape));
          messages[shape->ID] = new_message;
          break;
        }
        case ST_BMSC_MESSAGE_FOUND:
        {
          IncompleteMessage *new_message = new IncompleteMessage(FOUND, (const wchar_t*)shape->Text);
          // found message: glued on end, dot on begin
          new_message->set_dot_position(GetLineBegin(shape)-GetLineEnd(shape));
          messages[shape->ID] = new_message;
          break;
        }

        case ST_TIME_INTERVAL:
        case ST_TIME_DIRECTED:
        {
          TimeRelationEvent *new_relation = new TimeRelationEvent((const char*)shape->Text);
          new_relation->set_directed(type == ST_TIME_DIRECTED);
          new_relation->set_width(GetControlPos(shape, _T("Width")));
          time_relations[shape->ID] = new_relation;
          break;
        }

        case ST_BMSC_ACTION:
        case ST_BMSC_CONDITION:
        case ST_BMSC_COREGION:
        case ST_BMSC_ORDER_LINE:
        case ST_BMSC_ORDER_ARROW:
          // ignore other basic MSC symbols
          break;

        case ST_COMMENT:
        {
          Comment* new_comment = new Comment((const wchar_t*)shape->Text);
          new_comment->set_position(GetLineEnd(shape));
          // note: this shape is rotated by 90 degrees
          new_comment->set_width(GetHeight(shape));
          comments[shape->ID] = new_comment;
          break;
        }
        case ST_TEXT:
        {
          Comment* new_comment = new Comment((const wchar_t*)shape->Text);
          new_comment->set_position(GetPinPos(shape));
          new_comment->set_width(GetWidth(shape));
          bmsc->add_comment(new_comment);
          break;
        }

        case ST_HMSC_CONNECTION:
        case ST_HMSC_START:
        case ST_HMSC_END:
        case ST_HMSC_REFERENCE:
        case ST_HMSC_LINE:
        case ST_HMSC_ARROW:
          PrintError(stringize() << page_name << ": "
            << "HMSC symbol '" << shape->Name << "' not allowed in basic MSC drawing.",
            shapelist() << shape);
          break;

        case ST_PICTURE:
          // ignore clip art
          break;

        case ST_MARKER_EVENT:
          PrintWarning(stringize() << page_name << ": "
            << "Event marker detected. This drawing has already been checked.",
            shapelist() << shape);
          break;

        case ST_UNKNOWN:
        default:
          PrintWarning(stringize() << page_name << ": "
            << "Ignored a strange symbol '" << shape->Name << "'",
            shapelist() << shape);
          break;
      }
    }
    catch(std::exception& exc)
    {
      PrintError(stringize() << page_name << ": " << exc.what(),
        shapelist() << shape);
    }
  }

  // walk though all detected instances: create connections
  for(std::map<long,InstancePtr>::iterator ipos = instances.begin();
    ipos != instances.end(); ipos++)
  {
    Visio::IVShapePtr instance = vsoPage->Shapes->ItemFromID[ipos->first];
    // connect instance to the bMSC
    bmsc->add_instance(ipos->second);

    // assert there is no connection to inner shapes of the instance group
    assert_no_nested_FromConnects(instance);

    typedef std::multiset<SStrictOrder> TEventSet;
    TEventSet strict_events; // events in this strict area
    // random walk through all connectors from others
    for(int i = 1; i <= instance->FromConnects->Count; i++)
    {
      Visio::IVConnectPtr connect = instance->FromConnects->Item[i];
      SPoint pos = get_connect_point(connect);

      // shape connected to this instance
      Visio::IVShapePtr shape = connect->FromSheet;

      SStrictOrder event;
      event.shape_id = shape->ID;
      event.shape_type = get_shape_type(shape);
      event.event_pos = pos;
      // note: instances in Visio are rotated by 90', height is the 'x' coordinate
      event.event_height = pos.m_x;

      // which shape is connected?
      switch(event.shape_type)
      {
        case ST_BMSC_MESSAGE:
          // which side of the message is connected?
          if(connect->FromPart == visBegin)
            event.event_type = SStrictOrder::ET_OUTGOING;
          else if(connect->FromPart == visEnd)
            event.event_type = SStrictOrder::ET_INCOMING;
          else
          {
            PrintError(stringize() << page_name << ": "
              << "Wrongly connected message '" << shape->Name << "'",
              shapelist() << shape << instance);
            continue;
          }
          break;

        case ST_BMSC_MESSAGE_LOST:
          if(connect->FromPart == visBegin)
            event.event_type = SStrictOrder::ET_OUTGOING;
          else
          {
            PrintError(stringize() << page_name << ": "
              << "Wrongly connected message '" << shape->Name << "'",
              shapelist() << shape << instance);
            continue;
          }
          break;

        case ST_BMSC_MESSAGE_FOUND:
          if(connect->FromPart == visEnd)
            event.event_type = SStrictOrder::ET_INCOMING;
          else
          {
            PrintError(stringize() << page_name << ": "
              << "Wrongly connected message '" << shape->Name << "'",
              shapelist() << shape << instance);
            continue;
          }
          break;

        case ST_TIME_INTERVAL:
        case ST_TIME_DIRECTED:

        case ST_BMSC_ACTION:
        case ST_BMSC_CONDITION:

        case ST_COMMENT:
          // ignore comments
          continue;

        case ST_BMSC_COREGION:
          // which side of the corregion is connected?
          // note: proper coregion placement is required to assure ORDER_LINE ordering
          if(connect->FromPart == visBegin)
            event.event_type = SStrictOrder::ET_OUTGOING;
          else if(connect->FromPart == visEnd)
            event.event_type = SStrictOrder::ET_INCOMING;
          else
          {
            PrintError(stringize() << page_name << ": "
              << "Wrongly connected coregion '" << shape->Name << "'",
              shapelist() << shape << instance);
            continue;
          }
          break;

        default:
          PrintWarning(stringize() << page_name << ": "
            << "Ignored a connection to unknown symbol '" << shape->Name << "'",
            shapelist() << shape << instance);
          continue;
      }

      strict_events.insert(event);
    }

    // check for collisional shapes
    for(TEventSet::iterator epos = strict_events.begin();
      epos != strict_events.end(); epos = strict_events.upper_bound(*epos))
    {
      if(strict_events.count(*epos) > 1)
      {
        shapelist this_shapelist;

        std::pair<TEventSet::iterator, TEventSet::iterator> collision =
          strict_events.equal_range(*epos);
        for(TEventSet::iterator cpos = collision.first;
          cpos != collision.second; cpos++)
        {
          this_shapelist << vsoPage->Shapes->ItemFromID[cpos->shape_id];
        }

        PrintError(stringize() << page_name << ": "
          << "Multiple events cannot be attached to one point.",
          this_shapelist << instance);
      }
    }

    enum
    {
      ST_EXPECTING_AREA, // blank space between areas
      ST_STRICT,         // building a strict order area
      ST_COREGION        // building a coregion area
    } state = ST_EXPECTING_AREA;

    StrictOrderAreaPtr strict_area;
    CoregionAreaPtr coregion_area;
    long coregion_id;

    // walk though the events in a time-order
    for(TEventSet::iterator epos = strict_events.begin();
      epos != strict_events.end(); epos++)
    {
      Visio::IVShapePtr shape = vsoPage->Shapes->ItemFromID[epos->shape_id];

      switch(epos->shape_type)
      {
        case ST_BMSC_MESSAGE:
          switch(state)
          {
            case ST_EXPECTING_AREA:
              ipos->second->add_area(strict_area = new StrictOrderArea());
              state = ST_STRICT;
              /* no break */
            case ST_STRICT:
            {
              CompleteMessagePtr message = find_message<CompleteMessage>(messages, epos->shape_id);
              if(message == NULL)
                continue;

              StrictEventPtr event = strict_area->add_event(new StrictEvent());
              event->set_position(ConvertEventPoint(instance, epos->event_pos));
              if(epos->event_type == SStrictOrder::ET_OUTGOING)
                message->glue_send_event(event);
              else if(epos->event_type == SStrictOrder::ET_INCOMING)
                message->glue_receive_event(event);

              events.insert(std::make_pair<SPoint,EventPtr>(point_to_page(instance, epos->event_pos), event));
              break;
            }

            case ST_COREGION:
              PrintError(stringize() << page_name << ": "
                << "Instance events cannot occur behind coregion.",
                shapelist() << shape << instance);
              break;
          }
          break;

        case ST_BMSC_MESSAGE_LOST:
        case ST_BMSC_MESSAGE_FOUND:
          switch(state)
          {
            case ST_EXPECTING_AREA:
              ipos->second->add_area(strict_area = new StrictOrderArea());
              state = ST_STRICT;
              /* no break */
            case ST_STRICT:
            {
              IncompleteMessagePtr message = find_message<IncompleteMessage>(messages, epos->shape_id);
              if(message == NULL)
                continue;

              StrictEventPtr event = strict_area->add_event(new StrictEvent());
              event->set_position(ConvertEventPoint(instance, epos->event_pos));
              message->glue_event(event);

              events.insert(std::make_pair<SPoint,EventPtr>(point_to_page(instance, epos->event_pos), event));
              break;
            }

            case ST_COREGION:
              PrintError(stringize() << page_name << ": "
                << "Instance events cannot occur behind coregion.",
                shapelist() << shape << instance);
              break;
          }
          break;

        case ST_BMSC_COREGION:
        {
          switch(state)
          {
            case ST_EXPECTING_AREA:
            case ST_STRICT:
              if(epos->event_type == SStrictOrder::ET_OUTGOING)
              {
                ipos->second->add_area(coregion_area = create_coregion_area(events, messages, shape));
                coregion_area->set_begin_height(ConvertCoordinate(instance, epos->event_height));
                coregion_area->set_width(shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters]);
              }
              else
              {
                PrintError(stringize() << page_name << ": "
                  << "Misdirected coregion.",
                  shapelist() << shape);
              }

              state = ST_COREGION;
              coregion_id = epos->shape_id;
              break;

            case ST_COREGION:
              if(coregion_id != epos->shape_id)
              {
                PrintError(stringize() << page_name << ": "
                  << "Overlapping coregion.",
                  shapelist() << shape);
              }
              else if(epos->event_type == SStrictOrder::ET_INCOMING)
              {
                coregion_area->set_end_height(ConvertCoordinate(instance, epos->event_height));
                state = ST_EXPECTING_AREA;
              }
              else
              {
                // the error report should already have been printed
                TRACE("extract_bmsc() bad coregion connection");
              }
              break;
          }
          break;
        }
      }
    }

    if(state == ST_COREGION)
    {
      Visio::IVShapePtr shape = vsoPage->Shapes->ItemFromID[coregion_id];

      PrintError(stringize() << page_name << ": "
        << "Disconnected coregion.",
        shapelist() << shape);
    }
  }

  // walk through detected messages: check all messages are connected
  for(MscMessageMap::const_iterator mpos = messages.begin();
    mpos != messages.end(); mpos++)
  {
    Visio::IVShapePtr shape = vsoPage->Shapes->ItemFromID[mpos->first];

    if(!mpos->second->is_glued())
    {
      PrintError(stringize() << page_name << ": "
        << "Disconnected message '" << shape->Name << "'",
        shapelist() << shape);
    }
  }

  // walk though all detected time relations: create measurements
  for(TimeRelationEventMap::iterator rpos = time_relations.begin();
    rpos != time_relations.end(); rpos++)
  {
    Visio::IVShapePtr shape = vsoPage->Shapes->ItemFromID[rpos->first];

    SPoint point_a;
    point_a.m_x = shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX]->Result[0];
    point_a.m_y = shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginY]->Result[0];
    // find event at point a
    EventPointMap::iterator event_a = events.find(point_a);

    SPoint point_b;
    point_b.m_x = shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX]->Result[0];
    point_b.m_y = shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndY]->Result[0];
    // find event at point b
    EventPointMap::iterator event_b = events.find(point_b);

    if(event_a == events.end() || event_b == events.end())
    {
      PrintError(stringize() << page_name << ": "
        << "Disconnected time constraint '" << shape->Name << "'",
        shapelist() << shape);
      continue;
    }

    if(event_a == event_b)
    {
      PrintError(stringize() << page_name << ": "
        << "Time constraint cannot be attached to a single point.",
        shapelist() << shape);
      continue;
    }

    // connect the measurement to the events
    rpos->second->glue_events(event_a->second.get(), event_b->second.get());
  }

  // walk though all detected comments
  for(std::map<long,CommentPtr>::iterator cpos = comments.begin();
    cpos != comments.end(); cpos++)
  {
    Visio::IVShapePtr shape = vsoPage->Shapes->ItemFromID[cpos->first];

    // (1) check if the comment is connected to an event
    SPoint point;
    point.m_x = shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX]->Result[0];
    point.m_y = shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginY]->Result[0];
    // find event at the point
    EventPointMap::iterator event = events.find(point);

    if(event != events.end())
    {
      // connect comment to the events
      event->second->add_comment(cpos->second);
      continue;
    }

    // TODO: Comments may be connected to other elements (e.g. instances)

    PrintError(stringize() << page_name << ": "
      << "Disconnected comment '" << shape->Name << "'",
      shapelist() << shape);
  }

  // return NULL on error
  if(m_was_error)
  {
    m_was_error = was_error;
    return NULL;
  }

  m_was_error = was_error;
  return bmsc;
}

CDrawingExtractor::TConnectedSide
CDrawingExtractor::get_connected_side(Visio::IVConnectPtr connect)
{
  Visio::IVShapePtr line = connect->FromSheet;
  Visio::IVShapePtr shape = connect->ToSheet;

  std::wstring page_name = line->ContainingPage->Name;

  if(connect->ToPart >= visConnectionPoint)
  {
    // position of respective connection point
    SPoint pos = get_connect_point(connect);
    // dimensions of the shape
    Visio::IVCellPtr shape_height = shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight];
    Visio::IVCellPtr shape_width = shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth];

    // if attached to the bottom edge of the symbol
    // note: coordinate origin [0,0] is at bottom left
    if(fcmp(pos.m_y, 0.0) == 0)
      return CS_SHAPE_BOTTOM;
    // if attached to the top edge of the symbol
    else if(fcmp(pos.m_y, shape_height->Result[0]) == 0)
      return CS_SHAPE_TOP;

    if(fcmp(pos.m_x, 0.0) == 0)
      return CS_SHAPE_LEFT;
    else if(fcmp(pos.m_x, shape_width->Result[0]) == 0)
      return CS_SHAPE_RIGHT;
  }
  else if(connect->ToPart == visWholeShape)
    return CS_WHOLE_SHAPE;

  PrintError(stringize() << page_name << ": "
    << "Wrongly connected shape '" << shape->Name << "'.",
    shapelist() << shape << line);
  return CS_ERROR;
}

CDrawingExtractor::TConnectDirection
CDrawingExtractor::get_connect_direction(Visio::IVConnectPtr connect)
{
  Visio::IVShapePtr line = connect->FromSheet;
  TShapeType line_type = get_shape_type(line);

  Visio::IVShapePtr shape = connect->ToSheet;
  TShapeType shape_type = get_shape_type(shape);

  std::wstring page_name = line->ContainingPage->Name;

  TConnectedSide connected_side = get_connected_side(connect);
  if(connected_side == CS_SHAPE_TOP)
  {
    // top edge: should be incoming
    if(line_type == ST_HMSC_LINE ||
      line_type == ST_HMSC_ARROW && connect->FromPart == visEnd)
    {
      return CD_INCOMING;
    }
    else
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly connected shape '" << shape->Name << "'.",
        shapelist() << shape << line);
      return CD_ERROR;
    }
  }
  else if(connected_side == CS_SHAPE_BOTTOM)
  {
    // bottom edge: should be outgoing
    if(line_type == ST_HMSC_LINE ||
      line_type == ST_HMSC_ARROW && connect->FromPart == visBegin)
    {
      return CD_OUTGOING;
    }
    else
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly connected shape '" << shape->Name << "'.",
        shapelist() << shape << line);
      return CD_ERROR;
    }
  }
  else if(connected_side == CS_SHAPE_LEFT || connected_side == CS_SHAPE_RIGHT)
  {
    PrintError(stringize() << page_name << ": "
      << "Flow lines must be attached to a top or a bottom edge.",
      shapelist() << shape << line);
    return CD_ERROR;
  }
  else if(connected_side == CS_WHOLE_SHAPE)
  {
    // only connection nodes can be connected this way
    if(shape_type == ST_HMSC_CONNECTION)
    {
      if(line_type == ST_HMSC_ARROW)
      {
        if(connect->FromPart == visBegin)
          return CD_OUTGOING;
        else if(connect->FromPart == visEnd)
          return CD_INCOMING;
        else
        {
          PrintError(stringize() << page_name << ": "
            << "Wrongly connected connector '" << shape->Name << "'.",
            shapelist() << shape << line);
          return CD_ERROR;
        }
      }
      else if(line_type == ST_HMSC_LINE)
        return CD_UNKNOWN;
    }
  }

  PrintError(stringize() << page_name << ": "
    << "Wrongly connected connector '" << shape->Name << "'.",
    shapelist() << shape << line);
  return CD_ERROR;
}

CDrawingExtractor::TTimeDirection
CDrawingExtractor::get_time_direction(Visio::IVConnectPtr connect)
{
  Visio::IVShapePtr shape = connect->ToSheet;
  TShapeType shape_type = get_shape_type(shape);

  Visio::IVShapePtr line = connect->FromSheet;
  std::wstring page_name = line->ContainingPage->Name;

  TConnectedSide connected_side = get_connected_side(connect);
  if(connected_side == CS_SHAPE_TOP)
    return TD_TOP;
  else if(connected_side == CS_SHAPE_BOTTOM)
    return TD_BOTTOM;
  else if(connected_side == CS_SHAPE_LEFT || connected_side == CS_SHAPE_RIGHT)
  {
    PrintError(stringize() << page_name << ": "
      << "Time relations must be attached to a top or a bottom edge.",
      shapelist() << shape << line);
    return TD_ERROR;
  }

  PrintError(stringize() << page_name << ": "
    << "Wrongly connected time relation '" << shape->Name << "'.",
    shapelist() << shape << line);
  return TD_ERROR;
}

void CDrawingExtractor::set_as_predecessor(const NodeRelationPtr& relation,
  Visio::IVShapePtr shape, const HMscNodePtr& node)
{
  std::wstring page_name = shape->ContainingPage->Name;

  TShapeType type = get_shape_type(shape);
  switch(type)
  {
    case ST_HMSC_CONNECTION:
    case ST_HMSC_START:
    case ST_HMSC_REFERENCE:
    case ST_HMSC_CONDITION:
    {
      PredecessorNode *pred = dynamic_cast<PredecessorNode*>(node.get());
      if(pred != NULL)
        relation->set_predecessor(pred);
      else
        TRACE("set_as_predecessor() unexpected NULL");
      break;
    }

    default:
      PrintError(stringize() << page_name << ": "
        << "Node '" << shape->Name << "' cannot be a predecessor.",
        shapelist() << shape);
      break;
  }
}

void CDrawingExtractor::set_as_successor(const NodeRelationPtr& relation,
  Visio::IVShapePtr shape, const HMscNodePtr& node)
{
  std::wstring page_name = shape->ContainingPage->Name;

  TShapeType type = get_shape_type(shape);
  switch(type)
  {
    case ST_HMSC_CONNECTION:
    case ST_HMSC_END:
    case ST_HMSC_REFERENCE:
    case ST_HMSC_CONDITION:
    {
      SuccessorNode *succ = dynamic_cast<SuccessorNode*>(node.get());
      if(succ != NULL)
        relation->set_successor(succ);
      else
        TRACE("set_as_successor() unexpected NULL");
      break;
    }

    default:
      PrintError(stringize() << page_name << ": "
        << "Node '" << shape->Name << "' cannot be a successor.",
        shapelist() << shape);
      break;
  }
}

HMscPtr CDrawingExtractor::extract_hmsc(Visio::IVPagePtr vsoPage)
{
  // keep previous error indicator; this function may be called recursively
  bool was_error = m_was_error;
  m_was_error = false;

  std::wstring page_name = (const wchar_t*)vsoPage->Name;
  HMscPtr hmsc = new HMsc(page_name);
  m_msc_cache[page_name] = hmsc;

  // temporary mappers Visio shape-id --> msc.h
  std::map<long,HMscNodePtr> nodes;
  std::map<long,NodeRelationPtr> relations;
  TimeRelationRefNodeMap time_relations;
  std::map<long,CommentPtr> comments;

  // first walk through all shapes: create objects
  for(int i = 1; i <= vsoPage->Shapes->Count; i++)
  {
    Visio::IVShapePtr shape = vsoPage->Shapes->Item[i];

    try
    {
      TShapeType type = get_shape_type(shape);
      switch(type)
      {
        case ST_BMSC_INSTANCE:
        case ST_BMSC_MESSAGE:
        case ST_BMSC_MESSAGE_LOST:
        case ST_BMSC_MESSAGE_FOUND:
          PrintError(stringize() << page_name << ": "
            << "Basic MSC symbol '" << shape->Name << "' not allowed in HMSC drawing.",
            shapelist() << shape);
          break;

        case ST_COMMENT:
        {
          Comment* new_comment = new Comment((const wchar_t*)shape->Text);
          new_comment->set_position(GetLineEnd(shape));
          // note: this shape is rotated by 90 degrees
          new_comment->set_width(GetHeight(shape));
          comments[shape->ID] = new_comment;
          break;
        }
        case ST_TEXT:
        {
          Comment* new_comment = new Comment((const wchar_t*)shape->Text);
          new_comment->set_position(GetPinPos(shape));
          new_comment->set_width(GetWidth(shape));
          hmsc->add_comment(new_comment);
          break;
        }

        case ST_HMSC_CONNECTION:
        {
          ConnectionNode *new_connection = new ConnectionNode();
          new_connection->set_position(GetPinPos(shape));

          hmsc->add_node(new_connection);
          nodes[shape->ID] = new_connection;
          break;
        }
        case ST_HMSC_START:
        {
          if(hmsc->get_start() != NULL)
          {
            // FIXME: how to select the other start symbols?
            PrintError(stringize() << page_name << ": "
              << "Multiple HMSC start symbols.",
              shapelist() << shape);
            continue;
          }
          StartNode *new_start = new StartNode();
          new_start->set_position(GetPinPos(shape));

          hmsc->set_start(new_start);
          nodes[shape->ID] = new_start;
          break;
        }
        case ST_HMSC_END:
        {
          EndNode *new_end = new EndNode();
          new_end->set_position(GetPinPos(shape));

          hmsc->add_node(new_end);
          nodes[shape->ID] = new_end;
          break;
        }
        case ST_HMSC_REFERENCE:
        {
          ReferenceNode *new_node = new ReferenceNode();
          new_node->set_position(GetPinPos(shape));

          hmsc->add_node(new_node);
          nodes[shape->ID] = new_node;

          MscPtr msc = __extract_msc(vsoPage->Document, shape->Text);
          if(msc == NULL)
          {
            PrintError(stringize() << page_name << ": "
              << "Bad reference destination.",
              shapelist() << shape);
          }
          else
            new_node->set_msc(msc);
          break;
        }
        case ST_HMSC_CONDITION:
        {
          ConditionNode *new_condition = new ConditionNode((const char*)shape->Text);
          new_condition->set_position(GetPinPos(shape));

          hmsc->add_node(new_condition);
          nodes[shape->ID] = new_condition;
          break;
        }

        case ST_HMSC_LINE:
        case ST_HMSC_ARROW:
          relations[shape->ID] = new NodeRelation();
          break;

        case ST_TIME_INTERVAL:
        case ST_TIME_DIRECTED:
        {
          TimeRelationRefNode *new_relation = new TimeRelationRefNode((const char*)shape->Text);
          new_relation->set_directed(type == ST_TIME_DIRECTED);
          new_relation->set_width(GetControlPos(shape, _T("Width")));
          time_relations[shape->ID] = new_relation;
          break;
        }

        case ST_PICTURE:
          // ignore clip art
          break;

        case ST_MARKER_EVENT:
          PrintWarning(stringize() << page_name << ": "
            << "Event marker detected. This drawing has already been checked.",
            shapelist() << shape);
          break;

        case ST_UNKNOWN:
        default:
          PrintWarning(stringize() << page_name << ": "
            << "Ignored a strange symbol '" << shape->Name << "'",
            shapelist() << shape);
          break;
      }
    }
    catch(std::exception& exc)
    {
      PrintError(stringize() << page_name << ": " << exc.what(),
        shapelist() << shape);
    }
  }

  if(hmsc->get_start() == NULL)
  {
    PrintError(stringize() << page_name << ": "
      << "Missing HMSC start symbol.");

    m_was_error = was_error;
    return NULL;
  }

  // walk through detected lines and arrows
  for(std::map<long,NodeRelationPtr>::iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    Visio::IVShapePtr line = vsoPage->Shapes->ItemFromID[rpos->first];
    // check the connector is properly connected
    if(line->Connects->Count != 2)
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached connector.",
        shapelist() << line);
      continue;
    }

    Visio::IVConnectPtr connect1 = line->Connects->Item[1];
    Visio::IVShapePtr shape1 = connect1->ToSheet;

    Visio::IVConnectPtr connect2 = line->Connects->Item[2];
    Visio::IVShapePtr shape2 = connect2->ToSheet;

    // determine to what edges is this connector attached
    TConnectDirection dir1 = get_connect_direction(connect1);
    TConnectDirection dir2 = get_connect_direction(connect2);
    if(dir1 == CD_ERROR || dir2 == CD_ERROR)
      continue;

    // got an undirected line between two connection points
    // FIXME: is there a better solution than printing an error report?
    if(dir1 == CD_UNKNOWN && dir2 == CD_UNKNOWN)
    {
      PrintError(stringize() << page_name << ": "
        << "Cannot determine flow direction.",
        shapelist() << line << shape1 << shape2);
      continue;
    }

    std::map<long,HMscNodePtr>::iterator node1 = nodes.find(shape1->ID);
    std::map<long,HMscNodePtr>::iterator node2 = nodes.find(shape2->ID);
    if(node1 == nodes.end())
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached connector.",
        shapelist() << line << shape1);
      continue;
    }
    if(node2 == nodes.end())
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached connector.",
        shapelist() << line << shape2);
      continue;
    }

    if((dir1 == CD_OUTGOING || dir1 == CD_UNKNOWN) &&
      (dir2 == CD_INCOMING || dir2 == CD_UNKNOWN))
    {
      // shape1 --> shape2
      TRACE(shape1->Name << "-->" << shape2->Name);

      set_as_predecessor(rpos->second, shape1, node1->second);
      set_as_successor(rpos->second, shape2, node2->second);
    }
    else if((dir1 == CD_INCOMING || dir1 == CD_UNKNOWN) &&
      (dir2 == CD_OUTGOING || dir2 == CD_UNKNOWN))
    {
      // shape2 --> shape1
      TRACE(shape2->Name << "-->" << shape1->Name);

      set_as_predecessor(rpos->second, shape2, node2->second);
      set_as_successor(rpos->second, shape1, node1->second);
    }
    else
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached connector.",
        shapelist() << line << shape1 << shape2);
      continue;
    }
  }

  // walk though all detected time relations: create measurements
  for(TimeRelationRefNodeMap::iterator rpos = time_relations.begin();
    rpos != time_relations.end(); rpos++)
  {
    Visio::IVShapePtr line = vsoPage->Shapes->ItemFromID[rpos->first];
    // check the connector is properly connected
    if(line->Connects->Count != 2)
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached time relation.",
        shapelist() << line);
      continue;
    }

    Visio::IVConnectPtr connect1 = line->Connects->Item[1];
    Visio::IVShapePtr shape1 = connect1->ToSheet;

    Visio::IVConnectPtr connect2 = line->Connects->Item[2];
    Visio::IVShapePtr shape2 = connect2->ToSheet;

    // determine to what edges is this connector attached
    TTimeDirection dir1 = get_time_direction(connect1);
    TTimeDirection dir2 = get_time_direction(connect2);
    if(dir1 == CD_ERROR || dir2 == CD_ERROR)
      continue;

    ReferenceNodePtr node1;
    std::map<long,HMscNodePtr>::iterator npos1 = nodes.find(shape1->ID);
    if(npos1 != nodes.end())
      node1 = boost::dynamic_pointer_cast<ReferenceNode>(npos1->second);
    if(!node1)
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached time constraint.",
        shapelist() << line << shape1);
      continue;
    }

    ReferenceNodePtr node2;
    std::map<long,HMscNodePtr>::iterator npos2 = nodes.find(shape2->ID);
    if(npos2 != nodes.end())
      node2 = boost::dynamic_pointer_cast<ReferenceNode>(npos2->second);
    if(!node2)
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached time constraint.",
        shapelist() << line << shape2);
      continue;
    }

    if(node1 == node2 && dir1 == dir2)
    {
      PrintError(stringize() << page_name << ": "
        << "Time constraint cannot be attached to a single point.",
        shapelist() << line << shape1);
      continue;
    }

    // connect the measurement to the nodes
    rpos->second->glue_ref_nodes(dir1 == TD_BOTTOM, node1.get(),
      dir2 == TD_BOTTOM, node2.get());
  }

  // walk though all detected comments
  for(std::map<long,CommentPtr>::iterator cpos = comments.begin();
    cpos != comments.end(); cpos++)
  {
    Visio::IVShapePtr comment = vsoPage->Shapes->ItemFromID[cpos->first];
    // check the comment is properly connected
    if(comment->Connects->Count < 1)
    {
      PrintError(stringize() << page_name << ": "
        << "Disconnected comment '" << comment->Name << "'",
        shapelist() << comment);
      continue;
    }
    else if(comment->Connects->Count > 1)
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached comment.",
        shapelist() << comment);
      continue;
    }

    Visio::IVConnectPtr connect = comment->Connects->Item[1];
    Visio::IVShapePtr shape = connect->ToSheet;

    std::map<long,HMscNodePtr>::iterator npos = nodes.find(shape->ID);
    if(npos == nodes.end() || connect->FromPart != visBegin)
    {
      PrintError(stringize() << page_name << ": "
        << "Wrongly attached comment.",
        shapelist() << comment);
      continue;
    }

    (*npos).second->add_comment((*cpos).second);
  }

  // return NULL on error
  if(m_was_error)
  {
    m_was_error = was_error;
    return NULL;
  }

  m_was_error = was_error;
  return hmsc;
}

// $Id: extract.cpp 860 2010-08-25 10:46:51Z mbezdeka $
