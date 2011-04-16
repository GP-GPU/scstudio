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
 * Copyright (c) 2010 Ondrej Bouda <ondrej.bouda@wheee.cz>
 *
 * $Id: shapeutils.cpp 1032 2011-02-05 11:17:22Z xrehak $
 */
#include "shapeutils.h"
#include "messageSnapping.h"
#include "visualize.h"

void CShapeUtils::GlueBeginToPos(Visio::IVShapePtr what, Visio::IVShapePtr where, const MscPoint& pos)
{
  double height = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters];
  double width = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters];

  Visio::IVCellPtr cell = what->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
  // glue coordinates represent decimal fractions of the shape's width and height
  // note: instances in Visio are rotated by 90', height is the 'x' coordinate
  cell->GlueToPos(where, pos.get_y()/max(1.0,width), pos.get_x()/max(1.0,height));
}

void CShapeUtils::GlueBeginToShape(Visio::IVShapePtr what, Visio::IVShapePtr where)
{
  double height = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters];
  double width = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters];

  Visio::IVCellPtr from_cell = what->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX];
  Visio::IVCellPtr to_cell = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormPinX];

  from_cell->GlueTo(to_cell);
}

void CShapeUtils::GlueEndToPos(Visio::IVShapePtr what, Visio::IVShapePtr where, const MscPoint& pos)
{
  double height = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormHeight]->Result[visMillimeters];
  double width = where->CellsSRC[visSectionObject][visRowXFormOut][visXFormWidth]->Result[visMillimeters];

  Visio::IVCellPtr cell = what->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX];
  // note: instances in Visio are rotated by 90', height is the 'x' coordinate
  cell->GlueToPos(where, pos.get_y()/max(1.0,width), pos.get_x()/max(1.0,height));
}

void CShapeUtils::MarkShape(Visio::IVShapePtr shape, ShapeColor color)
{
  if (shape->Type == Visio::visTypeGroup) 
  {
    for(long i = 1; i <= shape->Shapes->Count; i++)
    {
      MarkShape(shape->Shapes->Item[i], color);
    }
  } else {
   // line color from ShapeColor enum 
   shape->CellsSRC[visSectionObject][visRowLine][visLineColor]->ResultIU = color;
   // text color from ShapeColor enum 
   shape->CellsSRC[visSectionCharacter][visRowCharacter][visCharacterColor]->ResultIU = color;
  }
}

void CShapeUtils::UnmarkShape(Visio::IVShapePtr shape)
{
  MarkShape(shape, SC_BLACK);
}

Visio::IVShapePtr CShapeUtils::GetCoregionAt(Visio::IVShapePtr shape, double yPos)
{
  Visio::IVConnectsPtr connects = shape->FromConnects;
  for (int i=1; i<=connects->Count; i++)
  {
    Visio::IVShapePtr connShape = connects->Item[i]->FromSheet;
    if (get_shape_type(connShape) != ST_BMSC_COREGION)
      continue;

    double beginY = connShape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginY]->Result[0];
    double endY = connShape->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndY]->Result[0];

    if (beginY >= yPos - 1000*EPSILON && yPos >= endY - 1000*EPSILON)
      return connShape;
  }
  return NULL;
}

void CShapeUtils::swapShape(Visio::IVShapePtr& shape1, Visio::IVShapePtr& shape2)
{
  Visio::IVShapePtr buffer = shape1;
  shape1 = shape2;
  shape2 = buffer;
}

MsgConnectedEndpoints CShapeUtils::getConnectedEndpoints(Visio::IVShapePtr shape)
{
  if(!shape || !isMessageShape(shape))
    MSCE_UNKNOWN;

  bool begSnapped = shape->GetCells("BegTrigger")->ResultInt[visNone][visTruncate] == 2;
  bool endSnapped = shape->GetCells("EndTrigger")->ResultInt[visNone][visTruncate] == 2;

  if(begSnapped && endSnapped)
    return MSCE_BOTH;
  if(begSnapped)
    return MSCE_BEGIN;
  if(endSnapped)
    return MSCE_END;

  return MSCE_NONE;
}

Visio::IVShapePtr CShapeUtils::duplicateShape(Visio::IVShapePtr origShape, TShapeType newShapeType)
{
  if(!origShape)
    return NULL;

  Visio::IVPagePtr page = origShape->Application->ActivePage;

  bool snapStatus = CMessageSnapping::isEnabled();
  CMessageSnapping::setEnabled(false);
  //Create new shapge
  CDrawingVisualizer visualizer(origShape->Application);
  Visio::IVShapePtr newShape = page->Drop(visualizer.find_master(newShapeType),0,0);
  //Copy shape internal data (for message numbering)
  newShape->Text = origShape->Text;
  newShape->Data1 = origShape->Data1;
  newShape->Data2 = origShape->Data2;
  newShape->Data3 = origShape->Data3;
  try {
  //Copy shape section 
  if(isMessageShape(newShape))
  {
    //Message flip
    if(!newShape->GetCellExists(_T("Actions.flipDirection"),0))
    {
      newShape->AddNamedRow(visSectionAction,_T("flipDirection"),visTagDefault);
      newShape->Cells["Actions.flipDirection.Action"]->FormulaU = _T("=RUNADDONWARGS(\"Sequence Chart Studio\",\"/event=105\")");
      newShape->Cells["Actions.flipDirection.Menu"]->FormulaU = _T("=\"Flip message direction\"");
    }
    //Message numbering
    if(!newShape->GetCellExists(_T("Actions.enumeration"),0) && _tcsicmp(newShape->Data1, _T("1")) == 0)
    {
      newShape->AddNamedRow(visSectionAction,_T("enumeration"),visTagDefault);
      newShape->Cells["Actions.enumeration.Action"]->FormulaU = _T("=RUNADDONWARGS(\"Sequence Chart Studio\",\"/event=104\")");
      newShape->Cells["Actions.enumeration.Menu"]->FormulaU = _T("=\"Select numbering group\"");
    }
  }
  //Copy message position
    for(int i=0; i<4; i++)
      newShape->GetCellsSRC(visSectionObject, visRowXForm1D, i)->Formula = 
      origShape->GetCellsSRC(visSectionObject, visRowXForm1D, i)->ResultStr[visMillimeters];
  }
  catch (_com_error err) {
    MessageBox(GetActiveWindow(), _T("changeMsgType: exception occurred."), L"", MB_OK);
  }
  //Enable snapping back
  CMessageSnapping::setEnabled(snapStatus);

  return newShape;
}

double CShapeUtils::getCoregionHeight(Visio::IVShapePtr coregion, VisUnitCodes units)
{
  return CShapeUtils::getShapeCell(coregion, "Height", units) / 2.0;
}

void CShapeUtils::setCoregionWidth(Visio::IVShapePtr coregion, double newWidth, VisUnitCodes units)
{
	coregion->Cells["Height"]->ResultIU = CPageUtils::ConvertUnits(coregion->Application,newWidth, units);
	coregion->CellsSRC[visSectionControls][visRowControl][visCtlY]->ResultIU = CPageUtils::ConvertUnits(coregion->Application, newWidth+2.5,units);
}

std::vector<Visio::IVShapePtr> CShapeUtils::getConnectedShapes(Visio::IVShapePtr shape, TShapeType connectedShapeType)
{
  std::vector<Visio::IVShapePtr> shapes;
  Visio::IVConnectsPtr connects = shape->FromConnects;

  for (int i=1; i<=connects->Count; i++)
  {
    Visio::IVShapePtr connShape = connects->Item[i]->FromSheet;
    if (get_shape_type(connShape) != connectedShapeType)
      continue;
    if(std::find(shapes.begin(), shapes.end(), connShape) == shapes.end())
      shapes.push_back(connShape);
  }
  return shapes;
}

MsgDirection CShapeUtils::getMsgDirection(Visio::IVShapePtr msgShape)
{
  double BeginX = CShapeUtils::getShapeCell(msgShape,"BeginX");
  double EndX = CShapeUtils::getShapeCell(msgShape,"EndX");

  return (BeginX < EndX) ? MSG_RIGHT : MSG_LEFT;  
}

bool CShapeComparator::operator()(const Visio::IVShapePtr shapeOne, const Visio::IVShapePtr shapeTwo)
{
  if (direction == ORDER_ASC)
    return (CShapeUtils::GetShapeBeginX(shapeOne) < CShapeUtils::GetShapeBeginX(shapeTwo));
  else
    return (CShapeUtils::GetShapeBeginX(shapeOne) > CShapeUtils::GetShapeBeginX(shapeTwo));
}

// $Id: shapeutils.cpp 1032 2011-02-05 11:17:22Z xrehak $
