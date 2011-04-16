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
 * $Id: pageutils.h 949 2010-09-16 21:15:00Z mbezdeka $
 */

#pragma once
#include "stdafx.h"
#include "extract.h"

/**
 * A utility class for functionality on a Visio Page.
 */
class CPageUtils
{
public:

  /**
   * Returns measurement units of a Visio page.
   * @return VisUnitCodes unit code of units of the given page
   */
  static short GetPageUnits(Visio::IVPagePtr vsoPage);

  /**
   * Converts a value between specified units.
   * @param vsoPage   the document page in which to convert the units
   * @param value     the value to be converted
   * @param unitsFrom from which units to convert; use VisUnitCodes constants or 0 for Visio's internal units
   * @param unitsTo   to which units to convert; use VisUnitCodes constants or 0 for Visio's internal units
   * @return \a value converted from \a unitsFrom to \a unitsTo
   */
  static double ConvertUnits(Visio::IVPagePtr vsoPage, double value, int unitsFrom=visPageUnits, int unitsTo=0);

  /**
   * Converts a value between specified units.
   * @param vsoApp    the application in which to convert the units - it uses its current page
   * @param value     the value to be converted
   * @param unitsFrom from which units to convert; use VisUnitCodes constants or 0 for Visio's internal units
   * @param unitsTo   to which units to convert; use VisUnitCodes constants or 0 for Visio's internal units
   * @return \a value converted from \a unitsFrom to \a unitsTo
   */
  static double ConvertUnits(Visio::IVApplicationPtr vsoApp, double value, int unitsFrom=visPageUnits, int unitsTo=0) {
    return ConvertUnits(vsoApp->ActivePage, value, unitsFrom, unitsTo);
  }

  /**
   * Retrieves width of a given page.
   * @param vsoPage the page which to get width of
   * @param units   units in which to retrieve the result; use VisUnitCodes constants or 0 for Visio's internal units;
   *                the default are units of the given page
   * @return width of the page in units \a units
   */
  static double GetPageWidth(Visio::IVPagePtr vsoPage, int units=visPageUnits);

  /**
   * Retrieves height of a given page.
   * @param vsoPage the page which to get height of
   * @param units   units in which to retrieve the result; use VisUnitCodes constants or 0 for Visio's internal units;
   *                the default are units of the given page
   * @return height of the page in units
   */
  static double GetPageHeight(Visio::IVPagePtr vsoPage, int units = visPageUnits);

  static const LPCTSTR VisioUnitToString(short unitCode);

  /**
   * Filters out all shapes from a given selection which are not of a given type.
   * 
   * @param selection the selection to be filtered
   * @param shapeType 
   * @return a new selection consisting only of shapes of the given type
   */
  static Visio::IVSelectionPtr FilterSelection(Visio::IVSelectionPtr selection, TShapeType shapeType);

  /**
   *  Returns intersection point between two vectors represented by 4 points
   *  @param p1 endpoint of the first line
   *  @param p2 beginpoint of the first line
   *  @param p3 endpoint of the second line
   *  @param p4 beginpoint of the second line
   */
  static MscPoint* getIntersectionPoint(const MscPoint& p1, const MscPoint& p2, const MscPoint& p3, const MscPoint& p4);

  static MscPoint getNormalVec(const MscPoint& origVector, double newVectorSize);
};

// $Id: pageutils.h 949 2010-09-16 21:15:00Z mbezdeka $
