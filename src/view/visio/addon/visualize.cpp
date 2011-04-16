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
 * $Id: visualize.cpp 1029 2011-02-02 22:17:59Z madzin $
 */

#include "stdafx.h"
#include "dllmodule.h"
#include "document.h"
#include "extract.h"
#include "visualize.h"
#include "errors.h"

#include <atlmisc.h>

#include <math.h>
#include <iterator>

CDrawingVisualizer::CDrawingVisualizer(Visio::IVApplicationPtr vsoApp)
{
  m_vsoApp = vsoApp;
  m_ask_overwrite = true;

  std::list<std::wstring> stencil_names =
    GetRegistryStrings(SCSTUDIO_REGISTRY_ROOT _T("\\Stencils"));

  for(std::list<std::wstring>::const_iterator npos = stencil_names.begin();
    npos != stencil_names.end(); npos++)
  {
    if(npos->empty())
      continue;

    Visio::IVDocumentPtr stencil;
    try
    {
      stencil = vsoApp->Documents->Item[npos->c_str()];
    }
    catch (_com_error&)
    {
      // silently skip this stencil
      // the stencil may be installed, but not currently opened
      continue;
    }

    for(int mpos = 1; mpos <= stencil->Masters->Count; mpos++)
    {
      Visio::IVMasterPtr master = stencil->Masters->Item[mpos];
      for(int spos = 1; spos <= master->Shapes->Count; spos++)
      {
        //MessageBox(GetActiveWindow(), master->Shapes->Item[spos]->Name, _T("Debug"), MB_OK);
        TShapeType type = get_shape_type(master->Shapes->Item[spos]);
        if(type == ST_UNKNOWN)
          continue;

        int style = get_shape_attribute(master->Shapes->Item[spos], _T("mscStyle"));

        MasterIdMap::const_iterator imaster = m_masters.find(MasterId(type,style));
        // if the master is already known
        if(imaster != m_masters.end())
          break;

        m_masters.insert(MasterIdMap::value_type(MasterId(type,style), master));
      }
    }
  }

  enum TShapeType mandatory_shapes[] =
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
    ST_UNKNOWN
  };

  // verify that the stencils contain all shapes required for any MSC
  for(TShapeType* tpos = mandatory_shapes; *tpos != ST_UNKNOWN; tpos++)
  {
    if(m_masters[MasterId(*tpos)] == NULL)
    {
      DisplayException(vsoApp, _T("Failed to open MSC stencils in the document."), MB_OK);
      break;
    }
  }
}

void CDrawingVisualizer::visualize_msc(Visio::IVDocumentPtr vsoDocument, const std::vector<MscPtr>& drawing)
{
  std::set<std::wstring> printed;
  Visio::IVPagePtr activePage = m_vsoApp->ActivePage;

  // list of MSC to be printed
  // new references may be added to m_printing by visualize_hmsc()
  std::copy(drawing.begin(), drawing.end(), std::back_inserter(m_printing));

  for(std::list<MscPtr>::const_iterator pos = m_printing.begin();
    pos != m_printing.end(); pos++)
  {
    if(*pos == NULL)
      continue;

    // if not already visualized
    if(printed.find((*pos)->get_label()) == printed.end())
    {
      Visio::IVPagePtr newPage;
      for(int i = 1; i <= vsoDocument->Pages->Count; i++)
      {
        Visio::IVPagePtr thisPage = vsoDocument->Pages->Item[i];
        if(_tcsicmp(thisPage->Name, (*pos)->get_label().c_str()) == 0)
        {
          newPage = thisPage;
          break;
        }
      }

      if(newPage == NULL)
      {
        if(pos == m_printing.begin())
          newPage = activePage;
        else
          newPage = vsoDocument->Pages->Add();
      }

      // check if the active page is empty
      if(m_ask_overwrite && newPage->Shapes->Count > 0)
      {
        // siwtch to the page
        m_vsoApp->ActiveWindow->Page = (IDispatch *)newPage;

        int answer = MessageBox(GetActiveWindow(),
          _T("This page is not empty. Do you want to overwrite this drawing?\n")
          _T("Say 'yes' to overwrite the drawing on this page. Say 'no' to append a new page."),
          _T("Overwrite this drawing?"), MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION);

        if(answer == IDYES)
        {
          // clear existing page
          for(int i = newPage->Shapes->Count; i > 0; i--)
            newPage->Shapes->Item[i]->Delete();
        }
        else if(answer == IDNO)
        {
          // append new page
          newPage = vsoDocument->Pages->Add();
        }
        else if(answer == IDCANCEL)
        {
          return;
        }
      }

      visualize_msc(newPage, *pos);
      printed.insert((*pos)->get_label());
    }
  }

  m_vsoApp->ActiveWindow->Page = (IDispatch *)activePage;
  double left, bottom, right, top;
  activePage->BoundingBox(visTypePage, &left, &bottom, &right, &top);
  // scroll to view the drawing
  m_vsoApp->ActiveWindow->Zoom = 1;
  m_vsoApp->ActiveWindow->ScrollViewTo((left+right)/2.0, (top+bottom)/2.0);

  // the last dropped shape would be selected
  m_vsoApp->ActiveWindow->DeselectAll();

  m_printing.clear();
}

Visio::IVMasterPtr CDrawingVisualizer::find_master(TShapeType type, int style)
{
  MasterIdMap::const_iterator imaster;

  // search for the requested style
  imaster = m_masters.find(MasterId(type, style));
  if(imaster != m_masters.end())
    return imaster->second;

  // search for a default style
  imaster = m_masters.find(MasterId(type));
  if(imaster != m_masters.end())
    return imaster->second;

  return NULL;
}

double get_page_height(Visio::IVPagePtr vsoPage)
{
  double mm = vsoPage->PageSheet->CellsSRC[visSectionObject][visRowPage][visPageHeight]->Result[visMillimeters];
  // keep vertical alignment
  return floor(mm/5)*5;
}

Visio::IVShapePtr CDrawingVisualizer::DropMaster(Visio::IVPagePtr vsoPage, Visio::IVMasterPtr master, const MscPoint& pos)
{
  double page_height = get_page_height(vsoPage);
  // drop the master to bottom-left corner
  // note: drop coordinates are in Visio internal units
  Visio::IVShapePtr shape = vsoPage->Drop(master, 0, 0);
  // move shape to the right position
  shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormPinX]->Result[visMillimeters] = pos.get_x(),
  shape->CellsSRC[visSectionObject][visRowXFormOut][visXFormPinY]->Result[visMillimeters] = page_height - pos.get_y();
  return shape;
}

void CDrawingVisualizer::SetLineBegin(Visio::IVShapePtr what, const MscPoint& pos)
{
  double page_height = get_page_height(what->ContainingPage);

  what->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX]->Result[visMillimeters] = pos.get_x();
  what->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginY]->Result[visMillimeters] = page_height-pos.get_y();
}

void CDrawingVisualizer::SetLineEnd(Visio::IVShapePtr what, const MscPoint& pos)
{
  double page_height = get_page_height(what->ContainingPage);

  what->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX]->Result[visMillimeters] = pos.get_x();
  what->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndY]->Result[visMillimeters] = page_height-pos.get_y();
}

void CDrawingVisualizer::SetControlPos(Visio::IVShapePtr shape, const wchar_t* row, Coordinate pos)
{
  // walk though all controls
  // note: rows are numbered starting with 0
  for(short i = 0; i < shape->RowCount[visSectionControls]; i++)
  {
    Visio::IVCellPtr cell = shape->CellsSRC[visSectionControls][visRowControl+i][visCtlY];
    if(cell == NULL)
      continue;

    if(_tcsicmp(cell->RowName, row) == 0)
    {
      cell->Result[visMillimeters] = pos;
      break;
    }
  }
}

// duplicate of CShapeUtils::MarkShape - removed from here
//void MarkShape(Visio::IVShapePtr what)
//{
//  if (what->Type == Visio::visTypeGroup) 
//  {
//    for(long i = 1; i <= what->Shapes->Count; i++)
//    {
//      MarkShape(what->Shapes->Item[i]);
//    }
//  } else {
//   // line color = red
//   what->CellsSRC[visSectionObject][visRowLine][visLineColor]->ResultIU = 2;
//   // text color = red
//   what->CellsSRC[visSectionCharacter][visRowCharacter][visCharacterColor]->ResultIU = 2;
//  }
//}

// duplicate of CShapeUtils::GlueBeginToPos - removed from here
//void GlueBeginToPos(Visio::IVShapePtr what, Visio::IVShapePtr where, const MscPoint& pos)
//{
//  double height = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters];
//  double width = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters];
//
//  Visio::IVCellPtr cell = what->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
//  // glue coordinates represent decimal fractions of the shape's width and height
//  // note: instances in Visio are rotated by 90', height is the 'x' coordinate
//  cell->GlueToPos(where, pos.get_y()/max(1.0,width), pos.get_x()/max(1.0,height));
//}

// duplicate of CShapeUtils::GlueBeginToShape - removed from here
//void GlueBeginToShape(Visio::IVShapePtr what, Visio::IVShapePtr where)
//{
//  double height = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters];
//  double width = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters];
//
//  Visio::IVCellPtr from_cell = what->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
//  Visio::IVCellPtr to_cell = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormPinX];
//
//  from_cell->GlueTo(to_cell);
//}

// duplicate of CShapeUtils::GlueEndToPos - removed from here
//void GlueEndToPos(Visio::IVShapePtr what, Visio::IVShapePtr where, const MscPoint& pos)
//{
//  double height = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters];
//  double width = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters];
//
//  Visio::IVCellPtr cell = what->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX];
//  // note: instances in Visio are rotated by 90', height is the 'x' coordinate
//  cell->GlueToPos(where, pos.get_y()/max(1.0,width), pos.get_x()/max(1.0,height));
//}

Visio::IVShapePtr CDrawingVisualizer::drop_message(Visio::IVPagePtr vsoPage, MessagePtrMap& messages,
  MscMessagePtr message, Visio::IVMasterPtr master)
{
  Visio::IVShapePtr msg = messages[message];
  if(msg != NULL)
    return msg;

  msg = vsoPage->Drop(master, 0, 0);

  switch(message->get_marked())
  {
    case NONE: CShapeUtils::MarkShape(msg, SC_BLACK); break;
    case MARKED: CShapeUtils::MarkShape(msg, SC_RED); break;
    case ADDED: CShapeUtils::MarkShape(msg, SC_GREEN); break;
    case REMOVED: CShapeUtils::MarkShape(msg, SC_RED); break;
    default: throw std::runtime_error("Error: unexpected behaviour");
  }

  msg->Text = message->get_label().c_str();
  messages[message] = msg;

  return msg;
}

void CDrawingVisualizer::show_event(Visio::IVPagePtr vsoPage, MessagePtrMap& messages,
  Visio::IVShapePtr parent, EventPtr event)
{
  CompleteMessagePtr complete_message = event->get_complete_message();
  if(complete_message != NULL)
  {
    Visio::IVShapePtr msg = drop_message(vsoPage, messages, complete_message, find_master(ST_BMSC_MESSAGE));

    if(complete_message->get_send_event() == event)
      CShapeUtils::GlueBeginToPos(msg, parent, event->get_position());
    else if(complete_message->get_receive_event() == event)
      CShapeUtils::GlueEndToPos(msg, parent, event->get_position());
  }

  IncompleteMessagePtr incomplete_message = event->get_incomplete_message();
  if(incomplete_message != NULL)
  {
    if(incomplete_message->is_lost())
    {
      Visio::IVShapePtr msg = drop_message(vsoPage, messages, incomplete_message, find_master(ST_BMSC_MESSAGE_LOST));
      CShapeUtils::GlueBeginToPos(msg, parent, event->get_position());
      SetLineEnd(msg, GetLineBegin(msg)+incomplete_message->get_dot_position());
    }
    else if(incomplete_message->is_found())
    {
      Visio::IVShapePtr msg = drop_message(vsoPage, messages, incomplete_message, find_master(ST_BMSC_MESSAGE_FOUND));
      CShapeUtils::GlueEndToPos(msg, parent, event->get_position());
      SetLineBegin(msg, GetLineEnd(msg)+incomplete_message->get_dot_position());
    }
  }

  if(event->get_marked() == MARKED)
  {
    Visio::IVMasterPtr marker_master = vsoPage->Document->Masters->Item["Event Marker"];

    double posX, posY;
    // convert drop coordinates to Visio internal units
    // note: instances in Visio are rotated by 90', height is the 'x' coordinate
    parent->XYToPage(
      vsoPage->Application->ConvertResult(event->get_position().get_y(), visMillimeters, visInches),
      vsoPage->Application->ConvertResult(event->get_position().get_x(), visMillimeters, visInches),
      &posX, &posY);

    Visio::IVShapePtr marker = vsoPage->Drop(marker_master, posX, posY);
  }

  for(CommentPtrSet::const_iterator cpos = event->get_comments().begin();
    cpos != event->get_comments().end(); cpos++)
  {
    Visio::IVShapePtr comment = DropComment(vsoPage, *cpos);

    CShapeUtils::GlueBeginToPos(comment, parent, event->get_position());
    SetLineEnd(comment, (*cpos)->get_position());
  }
}

Visio::IVShapePtr CDrawingVisualizer::connect_events(Visio::IVPagePtr vsoPage,
  Visio::IVShapePtr parent, EventPtr pred_event, EventPtr succ_event)
{
  Visio::IVShapePtr connector;

  // events are on the same height
  if(fcmp(pred_event->get_position().get_y(), succ_event->get_position().get_y()) == 0)
    connector = vsoPage->Drop(find_master(ST_BMSC_ORDER_ARROW), 0, 0);
  else
    connector = vsoPage->Drop(find_master(ST_BMSC_ORDER_LINE), 0, 0);

  CShapeUtils::GlueBeginToPos(connector, parent, pred_event->get_position());
  CShapeUtils::GlueEndToPos(connector, parent, succ_event->get_position());

  // events are on the same side
  if(fcmp(pred_event->get_position().get_x(), succ_event->get_position().get_x()) == 0)
  {
    // set the vertical line offset
    if(fcmp(pred_event->get_position().get_x(), 0.0) <= 0)
      connector->CellsSRC[visSectionControls][visRowControl][visCtlX]->Result[visMillimeters] = 5.0;
    else
      connector->CellsSRC[visSectionControls][visRowControl][visCtlX]->Result[visMillimeters] = -5.0;
  }

  return connector;
}

Visio::IVShapePtr
CDrawingVisualizer::drop_time_relation(Visio::IVPagePtr vsoPage, TimeRelationPtr time_relation)
{
  Visio::IVShapePtr result;

  if(time_relation->is_directed())
    result = vsoPage->Drop(find_master(ST_TIME_DIRECTED), 0, 0);
  else
    result = vsoPage->Drop(find_master(ST_TIME_INTERVAL), 0, 0);

  SetControlPos(result, _T("Width"), time_relation->get_width());

  switch(time_relation->get_marked())
  {
    case NONE: CShapeUtils::MarkShape(result, SC_BLACK); break;
    case MARKED: CShapeUtils::MarkShape(result, SC_RED); break;
    case ADDED: CShapeUtils::MarkShape(result, SC_GREEN); break;
    case REMOVED: CShapeUtils::MarkShape(result, SC_RED); break;
    default: throw std::runtime_error("Error: unexpected behaviour");
  }

  result->Text = time_relation->get_label().c_str();

  return result;
}

void CDrawingVisualizer::show_time_relations(Visio::IVPagePtr vsoPage, TimeRelationEventPtrMap& time_relations,
  Visio::IVShapePtr parent, EventPtr event)
{
  for(TimeRelationEventPtrList::const_iterator rpos = event->get_time_relations().begin();
    rpos != event->get_time_relations().end(); rpos++)
  {
    Visio::IVShapePtr dimension;

    TimeRelationEventPtrMap::const_iterator irel = time_relations.find(*rpos);
    if(irel == time_relations.end())
    {
      dimension = drop_time_relation(vsoPage, *rpos);
      time_relations[*rpos] = dimension;
    }
    else
    {
      dimension = irel->second;
      // some coregions might have been drawn over this shape
      dimension->BringToFront();
    }

    if((*rpos)->get_event_a() == event)
      CShapeUtils::GlueBeginToPos(dimension, parent, event->get_position());
    else if((*rpos)->get_event_b() == event)
      CShapeUtils::GlueEndToPos(dimension, parent, event->get_position());
  }
}

void CDrawingVisualizer::visualize_msc(Visio::IVPagePtr vsoPage, const MscPtr& msc)
{
  try
  {
    vsoPage->Name = msc->get_label().c_str();
  }
  catch (_com_error &)
  { }

  // visualize global comments
  for(CommentPtrSet::const_iterator cpos = msc->get_comments().begin();
    cpos != msc->get_comments().end(); cpos++)
  {
    Visio::IVShapePtr text = DropMaster(vsoPage, find_master(ST_TEXT), (*cpos)->get_position());
    text->Text = (*cpos)->get_text().c_str();
    text->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters] = (*cpos)->get_width();

    switch((*cpos)->get_marked())
    {
      case NONE: CShapeUtils::MarkShape(text, SC_BLACK); break;
      case MARKED: CShapeUtils::MarkShape(text, SC_RED); break;
      case ADDED: CShapeUtils::MarkShape(text, SC_GREEN); break;
      case REMOVED: CShapeUtils::MarkShape(text, SC_RED); break;
      default: throw std::runtime_error("Error: unexpected behaviour");
    }
  }

  BMscPtr bmsc = boost::dynamic_pointer_cast<BMsc>(msc);
  if(bmsc != NULL)
    visualize_bmsc(vsoPage, bmsc);

  HMscPtr hmsc = boost::dynamic_pointer_cast<HMsc>(msc);
  if(hmsc != NULL)
    visualize_hmsc(vsoPage, hmsc);
}

void CDrawingVisualizer::visualize_bmsc(Visio::IVPagePtr vsoPage, const BMscPtr& bmsc)
{
  std::map<InstancePtr,Visio::IVShapePtr> instances;

  // visualize all instances
  for(InstancePtrList::const_iterator ipos = bmsc->get_instances().begin();
    ipos != bmsc->get_instances().end(); ipos++)
  {
    Visio::IVShapePtr inst =
      vsoPage->Drop(find_master(ST_BMSC_INSTANCE, (*ipos)->get_visual_style()), 0, 0);

    switch((*ipos)->get_marked())
    {
      case NONE: CShapeUtils::MarkShape(inst, SC_BLACK); break;
      case MARKED: CShapeUtils::MarkShape(inst, SC_RED); break;
      case ADDED: CShapeUtils::MarkShape(inst, SC_GREEN); break;
      case REMOVED: CShapeUtils::MarkShape(inst, SC_RED); break;
      default: throw std::runtime_error("Error: unexpected behaviour");
    }

    inst->Text = (*ipos)->get_label().c_str();
    SetControlPos(inst, _T("mscHeadWidth"), (*ipos)->get_width()/2.0);
    SetLineBegin(inst, (*ipos)->get_line_begin());
    SetLineEnd(inst, (*ipos)->get_line_end());

    TRACE("CDrawingVisualizer::visualize_bmsc() instance " << (*ipos)->get_label().c_str());
    instances[*ipos] = inst;
  }

  MessagePtrMap messages;
  TimeRelationEventPtrMap time_relations;

  // walk through all instance events and visualize the messages
  for(InstancePtrList::const_iterator ipos = bmsc->get_instances().begin();
    ipos != bmsc->get_instances().end(); ipos++)
  {
    Visio::IVShapePtr inst = instances[*ipos];
    if(inst == NULL)
      continue;

    for(EventAreaPtr area = (*ipos)->get_first();
      area != NULL; area = area->get_next())
    {
      StrictOrderAreaPtr strict_area = boost::dynamic_pointer_cast<StrictOrderArea>(area);
      if(strict_area != NULL)
      {
        for(StrictEventPtr event = strict_area->get_first();
          event != NULL; event = event->get_successor())
        {
          show_event(vsoPage, messages, inst, event);
          show_time_relations(vsoPage, time_relations, inst, event);
        }
      }

      CoregionAreaPtr coregion_area = boost::dynamic_pointer_cast<CoregionArea>(area);
      if(coregion_area != NULL)
      {
        Visio::IVShapePtr coregion = vsoPage->Drop(find_master(ST_BMSC_COREGION), 0, 0);

        switch(coregion_area->get_marked())
        {
          case NONE: CShapeUtils::MarkShape(coregion, SC_BLACK); break;
          case MARKED: CShapeUtils::MarkShape(coregion, SC_RED); break;
          case ADDED: CShapeUtils::MarkShape(coregion, SC_GREEN); break;
          case REMOVED: CShapeUtils::MarkShape(coregion, SC_RED); break;
          default: throw std::runtime_error("Error: unexpected behaviour");
        }

        CShapeUtils::GlueBeginToPos(coregion, inst, MscPoint(0,coregion_area->get_begin_height()));
        CShapeUtils::GlueEndToPos(coregion, inst, MscPoint(0,coregion_area->get_end_height()));

				CShapeUtils::setCoregionWidth(coregion, coregion_area->get_width() );

        // events to be processed; this is to avoid recursion
        std::list<CoregionEventPtr> event_stack;

        for(CoregionEventPVector::const_iterator mpos = coregion_area->get_minimal_events().begin();
          mpos != coregion_area->get_minimal_events().end(); mpos++)
        {
          // initialize the stack with events with no predecessors
          push_back_if_unique<CoregionEventPtr>(event_stack, *mpos);
        }

        // process all events in the stack
        for(std::list<CoregionEventPtr>::const_iterator epos = event_stack.begin();
          epos != event_stack.end(); epos++)
        {
          show_event(vsoPage, messages, coregion, *epos);
          show_time_relations(vsoPage, time_relations, coregion, *epos);

          for(CoregEventRelPtrVector::const_iterator spos = (*epos)->get_successors().begin();
            spos != (*epos)->get_successors().end(); spos++)
          {
            CoregionEventPtr successor = (*spos)->get_successor();

            Visio::IVShapePtr connector = connect_events(vsoPage, coregion, *epos, successor);

            switch((*spos)->get_marked())
            {
              case NONE: CShapeUtils::MarkShape(connector, SC_BLACK); break;
              case MARKED: CShapeUtils::MarkShape(connector, SC_RED); break;
              case ADDED: CShapeUtils::MarkShape(connector, SC_GREEN); break;
              case REMOVED: CShapeUtils::MarkShape(connector, SC_RED); break;
              default: throw std::runtime_error("Error: unexpected behaviour");
            }

            // add successors of this event to the stack
            // note: std::list<>::push_back doesn't invalidate iterators
            push_back_if_unique<CoregionEventPtr>(event_stack, successor);
          }
        }
      }
    }
  }
}

Visio::IVShapePtr CDrawingVisualizer::drop_hmsc_node(Visio::IVPagePtr vsoPage, NodePtrMap& nodes, HMscNodePtr node)
{
  Visio::IVShapePtr shape = nodes[node];
  if(shape != NULL)
    return shape;

  StartNodePtr start_node = boost::dynamic_pointer_cast<StartNode>(node);
  if(start_node != NULL)
  {
    shape = DropMaster(vsoPage, find_master(ST_HMSC_START), start_node->get_position());
  }

  ConditionNodePtr condition_node = boost::dynamic_pointer_cast<ConditionNode>(node);
  if(condition_node != NULL)
  {
    shape = DropMaster(vsoPage, find_master(ST_HMSC_CONDITION), condition_node->get_position());
    shape->Text = condition_node->get_label().c_str();
  }

  ConnectionNodePtr connection_node = boost::dynamic_pointer_cast<ConnectionNode>(node);
  if(connection_node != NULL)
  {
    shape = DropMaster(vsoPage, find_master(ST_HMSC_CONNECTION), connection_node->get_position());
  }

  ReferenceNodePtr reference_node = boost::dynamic_pointer_cast<ReferenceNode>(node);
  if(reference_node != NULL)
  {
    shape = DropMaster(vsoPage, find_master(ST_HMSC_REFERENCE), reference_node->get_position());

    MscPtr msc = reference_node->get_msc();
    if(msc != NULL)
    {
      shape->Text = msc->get_label().c_str();
      m_printing.push_back(msc);
    }
    else
      shape->Text = "(void)";
  }

  EndNodePtr end_node = boost::dynamic_pointer_cast<EndNode>(node);
  if(end_node != NULL)
  {
    shape = DropMaster(vsoPage, find_master(ST_HMSC_END), end_node->get_position());
  }

  switch(node->get_marked())
  {
    case NONE: CShapeUtils::MarkShape(shape, SC_BLACK); break;
    case MARKED: CShapeUtils::MarkShape(shape, SC_RED); break;
    case ADDED: CShapeUtils::MarkShape(shape, SC_GREEN); break;
    case REMOVED: CShapeUtils::MarkShape(shape, SC_RED); break;
    default: throw std::runtime_error("Error: unexpected behaviour");
  }

  nodes[node] = shape;

  return shape;
}

void CDrawingVisualizer::show_time_relations1(Visio::IVPagePtr vsoPage,
  Visio::IVShapePtr shape, TimeRelationRefNodePtrSet relations)
{
  // process all time relations
  for(TimeRelationRefNodePtrSet::const_iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    // phase 1: print only constraints applied to this single node
    if((*rpos)->get_ref_node_a() != (*rpos)->get_ref_node_b())
      continue;

    Visio::IVShapePtr dimension = drop_time_relation(vsoPage, *rpos);

    Visio::IVCellPtr from_cell = dimension->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
    from_cell->GlueToPos(shape, 0.5, 0.0);
    Visio::IVCellPtr to_cell = dimension->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX];
    to_cell->GlueToPos(shape, 0.5, 1.0);
  }
}

void CDrawingVisualizer::show_time_relations2(Visio::IVPagePtr vsoPage,
  NodePtrMap& nodes, const ReferenceNodePtr& reference_node,
  Visio::IVShapePtr shape, TimeRelationRefNodePtrSet relations)
{
  // process all time relations
  for(TimeRelationRefNodePtrSet::const_iterator rpos = relations.begin();
    rpos != relations.end(); rpos++)
  {
    // phase 2: constraints applied to this single node have already been printed
    if((*rpos)->get_ref_node_a() == (*rpos)->get_ref_node_b())
      continue;
    // for each "node a" we print the time relation to "node b"
    // we thus skip all "node b"
    if((*rpos)->get_ref_node_a() != reference_node)
      continue;

    if((*rpos)->get_ref_node_a() == NULL || (*rpos)->get_ref_node_b() == NULL)
      throw std::invalid_argument("Disconnected time constraint.");

    Visio::IVShapePtr dimension = drop_time_relation(vsoPage, *rpos);

    Visio::IVCellPtr from_cell = dimension->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
    from_cell->GlueToPos(shape, 0.5, (*rpos)->is_bottom_node_a()? 0.0 : 1.0);

    Visio::IVShapePtr next_shape =
      drop_hmsc_node(vsoPage, nodes, (*rpos)->get_ref_node_b());
    Visio::IVCellPtr to_cell = dimension->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX];
    to_cell->GlueToPos(next_shape, 0.5, (*rpos)->is_bottom_node_b()? 0.0 : 1.0);
  }
}

void CDrawingVisualizer::visualize_hmsc(Visio::IVPagePtr vsoPage, const HMscPtr& hmsc)
{
  NodePtrMap nodes;

  // nodes to be processed; this is to avoid recursion
  std::list<HMscNodePtr> node_stack;

  // initialize the stack with the start node
  push_back_if_unique<HMscNodePtr>(node_stack, hmsc->get_start());

  // process all nodes in the stack
  for(std::list<HMscNodePtr>::const_iterator npos = node_stack.begin();
    npos != node_stack.end(); npos++)
  {
    Visio::IVShapePtr shape = drop_hmsc_node(vsoPage, nodes, *npos);

    PredecessorNode *predecessor_node = dynamic_cast<PredecessorNode*>(npos->get());
    if(predecessor_node != NULL)
    {
      for(NodeRelationPtrVector::const_iterator spos = predecessor_node->get_successors().begin();
        spos != predecessor_node->get_successors().end(); spos++)
      {
        SuccessorNode *successor = (*spos)->get_successor();
        HMscNode *successor_node = dynamic_cast<HMscNode*>(successor);

        Visio::IVShapePtr successor_shape = drop_hmsc_node(vsoPage, nodes, successor_node);

        Visio::IVShapePtr connector = vsoPage->Drop(find_master(ST_HMSC_ARROW), 0, 0);

        switch((*spos)->get_marked())
        {
          case NONE: CShapeUtils::MarkShape(connector, SC_BLACK); break;
          case MARKED: CShapeUtils::MarkShape(connector, SC_RED); break;
          case ADDED: CShapeUtils::MarkShape(connector, SC_GREEN); break;
          case REMOVED: CShapeUtils::MarkShape(connector, SC_RED); break;
          default: throw std::runtime_error("Error: unexpected behaviour");
        }

        Visio::IVCellPtr from_cell = connector->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
        from_cell->GlueToPos(shape, 0.5, 0.0);
        Visio::IVCellPtr to_cell = connector->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX];
        to_cell->GlueToPos(successor_shape, 0.5, 1.0);

        // add successors of this node to the stack
        // note: std::list<>::push_back doesn't invalidate iterators
        push_back_if_unique<HMscNodePtr>(node_stack, successor_node);
      }
    }

    ReferenceNode *reference_node = dynamic_cast<ReferenceNode*>(npos->get());
    if(reference_node != NULL)
    {
      const TimeRelationRefNodePtrSet& tops = reference_node->get_time_relations_top();
      const TimeRelationRefNodePtrSet& bottoms = reference_node->get_time_relations_bottom();

      // phase 1: print "time" constraints
      show_time_relations1(vsoPage, shape, tops);
      // phase 2: print "top" and "bottom" constraints
      show_time_relations2(vsoPage, nodes, reference_node, shape, tops);
      show_time_relations2(vsoPage, nodes, reference_node, shape, bottoms);

      // phase 3: print membership counter as a comment, if present
      if (reference_node->is_attribute_set("membership_counter")) {
        int mcnt = reference_node->get_attribute("membership_counter", 0);
        if (mcnt > 0) {
          std::wstringstream mcnt_stream;
          mcnt_stream << "# of visits: " << mcnt;
          Comment* refNodeComment = new Comment(mcnt_stream.str());
          MscPoint commentPos(GetPinPos(shape));
          refNodeComment->set_position(commentPos);

          Visio::IVShapePtr commentShape = DropComment<Comment>(vsoPage, refNodeComment);
          CShapeUtils::GlueBeginToShape(commentShape, shape);
          SetLineEnd(commentShape, commentPos + MscPoint(20, -10));
          CShapeUtils::MarkShape(commentShape);
        }
      }
    }

    for(CommentPtrSet::const_iterator cpos = (*npos)->get_comments().begin();
      cpos != (*npos)->get_comments().end(); cpos++)
    {
      Visio::IVShapePtr comment = DropComment(vsoPage, *cpos);

      CShapeUtils::GlueBeginToShape(comment, shape);
      SetLineEnd(comment, (*cpos)->get_position());
    }
  }
}

// $Id: visualize.cpp 1029 2011-02-02 22:17:59Z madzin $
