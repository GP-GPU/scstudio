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
 * $Id: shapeutils.h 1023 2011-01-08 11:44:57Z xpekarc $
 */

#pragma once
#include "stdafx.h"
#include "extract.h"
#include "enums.h"

const double EPSILON = 1e-10;

/**
 * A utility class for functionality on a Visio Shape.
 */
class CShapeUtils
{
public:

  static double GetShapeBeginX(Visio::IVShapePtr shape) { return shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginX]->Result[""]; }
  static double GetShapeEndX(Visio::IVShapePtr shape)   { return shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndX]->Result[""]; }
  static double GetShapeBeginY(Visio::IVShapePtr shape) { return shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DBeginY]->Result[""]; }
  static double GetShapeEndY(Visio::IVShapePtr shape)   { return shape->CellsSRC[visSectionObject][visRowXForm1D][vis1DEndY]->Result[""]; }
  
  static double getShapeCell(Visio::IVShapePtr shape, const _bstr_t & propertyName, VisUnitCodes units = visPageUnits)
                                                        { return shape->Cells[propertyName]->Result[units]; }

  static void GlueBeginToPos(Visio::IVShapePtr what, Visio::IVShapePtr where, const MscPoint& pos);
  static void GlueBeginToShape(Visio::IVShapePtr what, Visio::IVShapePtr where);
  static void GlueEndToPos(Visio::IVShapePtr what, Visio::IVShapePtr where, const MscPoint& pos);

  /**
   *  Marking, unmarking shapes
   *  
   *  @param shape the shape to be marked
   *  @param color the color the shape will be marked with (color enums can be found in enums.h)
   */
  static void MarkShape(Visio::IVShapePtr shape, ShapeColor color = SC_RED);
  static void UnmarkShape(Visio::IVShapePtr shape);

  /**
   * Get a coregion connected to shape, crossing yPos.
   * @param shape the shape on which to find a coregion
   * @param yPos  the Y-position relative to the page in internal units
   * @return pointer to the coregion found or NULL if there is no such coregion
   */
  static Visio::IVShapePtr GetCoregionAt(Visio::IVShapePtr shape, double yPos);

  static void swapShape(Visio::IVShapePtr& shape1, Visio::IVShapePtr& shape2);
  
  static MsgConnectedEndpoints getConnectedEndpoints(Visio::IVShapePtr shape);

  static Visio::IVShapePtr duplicateShape(Visio::IVShapePtr shape, TShapeType newShapeType);

  static double getCoregionHeight(Visio::IVShapePtr coregion, VisUnitCodes units = visPageUnits);

	static void setCoregionWidth(Visio::IVShapePtr coregion, double newWidth, VisUnitCodes units = visPageUnits);
  /**
   * Get shapes connected to another shape
   * @param shape               shape the other shapes are connected to
   * @param connectedShapeType  filter for connected shapes (e.g. coregions, messages, etc.)
   */
  static std::vector<Visio::IVShapePtr> getConnectedShapes(Visio::IVShapePtr shape, TShapeType connectedShapeType);
  /**
   *  Returns the direction of given message
   */
  static MsgDirection getMsgDirection(Visio::IVShapePtr msgShape);
};

/**
 * Shape comparator suitable for sorting. Allows for ascending or descending order.
 */
class CShapeComparator
{
  TOrderingDirection direction;
public:
  CShapeComparator(TOrderingDirection direction=ORDER_ASC) : direction(direction) {}

  bool operator()(const Visio::IVShapePtr shapeOne, const Visio::IVShapePtr shapeTwo);
};

// $Id: shapeutils.h 1023 2011-01-08 11:44:57Z xpekarc $
