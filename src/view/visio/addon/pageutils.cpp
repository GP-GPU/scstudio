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
 * $Id: pageutils.cpp 949 2010-09-16 21:15:00Z mbezdeka $
 */

#include "pageutils.h"

short CPageUtils::GetPageUnits(Visio::IVPagePtr vsoPage)
{
  return vsoPage->PageSheet->CellsSRC[visSectionObject][visRowPage][visPageScale]->Units;
}

double CPageUtils::ConvertUnits(Visio::IVPagePtr vsoPage, double value, int unitsFrom, int unitsTo)
{
  // convert the value via a scratch cell
  Visio::IVShapePtr sheet = vsoPage->PageSheet;

  // initialize a scratch cell
  short scratchSectionExisted = sheet->SectionExists[visSectionScratch][false];
  sheet->AddRow(visSectionScratch, visRowScratch, visTagDefault);
  Visio::IVCellPtr scratch = sheet->Cells["Scratch.X1"];

  // convert the value
  if (unitsFrom == 0) {
    scratch->Result[""] = value; // poor Visio API does not have a constant for internal units
  } else {
    scratch->Result[unitsFrom] = value;
  }
  double convertedValue = scratch->Result[unitsTo];

  // clean up
  sheet->DeleteRow(visSectionScratch, visRowScratch);
  if (!scratchSectionExisted) {
    sheet->DeleteSection(visSectionScratch);
  }

  return convertedValue;
}

double CPageUtils::GetPageWidth(Visio::IVPagePtr vsoPage, int units)
{
  Visio::IVCellPtr cell = vsoPage->PageSheet->CellsSRC[visSectionObject][visRowPage][visPageWidth];
  return (units == 0 ? cell->Result[""] : cell->Result[units]);
}

double CPageUtils::GetPageHeight(Visio::IVPagePtr vsoPage, int units)
{
  Visio::IVCellPtr cell = vsoPage->PageSheet->CellsSRC[visSectionObject][visRowPage][visPageHeight];
  return (units == 0 ? cell->Result[""] : cell->Result[units]);
}

const LPCTSTR CPageUtils::VisioUnitToString(short unitCode)
{
  switch (unitCode) {
    case visCentimeters:
      return _T("cm");
    case visCiceros:
      return _T("c");
    case visDate:
      return _T("date");
    case visDegrees:
      return _T("deg");
    case visDidots:
      return _T("d");
    case visElapsedWeek:
      return _T("ew");
    case visElapsedDay:
      return _T("ed");
    case visElapsedHour:
      return _T("eh");
    case visElapsedMin:
      return _T("em");
    case visElapsedSec:
      return _T("es");
    case visFeet:
      return _T("ft");
    case visInches:
      return _T("in");
    case visKilometers:
      return _T("km");
    case visMeters:
      return _T("m");
    case visMiles:
      return _T("mi");
    case visMillimeters:
      return _T("mm");
    case visMin:
      return _T("'");
    case visNautMiles:
      return _T("nm");
    case visPercent:
      return _T("%");
    case visPicas:
      return _T("p");
    case visPoints:
      return _T("pt");
    case visRadians:
      return _T("rad");
    case visSec:
      return _T("\"");
    case visYards:
      return _T("yd");
    default:
      return _T("???");
  }
}

Visio::IVSelectionPtr CPageUtils::FilterSelection(Visio::IVSelectionPtr selection, TShapeType shapeType)
{
  Visio::IVPagePtr page = selection->Application->ActivePage;
  Visio::IVSelectionPtr newSel = page->CreateSelection(Visio::visSelTypeEmpty, Visio::visSelModeSkipSuper);
  for (int i=1; i<=selection->Count; i++)
  {
    if (get_shape_type(selection->Item[i]) == shapeType)
    {
      newSel->Select(selection->Item[i], Visio::visSelect);
    }
  }
  return newSel;
}

MscPoint* CPageUtils::getIntersectionPoint(const MscPoint& p1, const MscPoint& p2, const MscPoint& p3, const MscPoint& p4)
{
  double x1 = p1.get_x(), x2 = p2.get_x(), x3 = p3.get_x(), x4 = p4.get_x();
  double y1 = p1.get_y(), y2 = p2.get_y(), y3 = p3.get_y(), y4 = p4.get_y();

  double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  if(d == 0)
    return NULL;

  double pre = (x1*y2 - y1*x2);
  double post = (x3*y4 - y3*x4);
  double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
  double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

  return new MscPoint(x,y);
}

MscPoint CPageUtils::getNormalVec(const MscPoint &origVector, double newVectorSize)
{
  //Create normal vector
  MscPoint normalVec(origVector.get_y(), -origVector.get_x());
  //Normalize
  double normalize  = sqrt(pow(normalVec.get_x(), 2) + pow(normalVec.get_y(), 2));
  normalVec = MscPoint(normalVec.get_x() / normalize * newVectorSize, normalVec.get_y() / normalize * newVectorSize);

  return MscPoint(normalVec);
}

// $Id: pageutils.cpp 949 2010-09-16 21:15:00Z mbezdeka $
