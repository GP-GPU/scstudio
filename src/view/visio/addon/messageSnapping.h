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
 * Copyright (c) 2010 Martin Bezdeka <mbezdeka@seznam.cz>
 *
 * $Id: messageSnapping.h 952 2010-09-16 21:53:03Z mbezdeka $
 */
#pragma once

#include <utility>
#include <vector>
#include "enums.h"
#include "extract.h"
#include "pageutils.h"

typedef boost::shared_ptr<MscPoint> MscPointPtr;

class CMessageSnapping
{
public:

  static bool isEnabled();
  static void setEnabled(bool bEnable = true);
  static bool isArrowKeysEnabled();

  static MsgSnapType getSnapType();
  static void setSnapType(MsgSnapType snapType);
  /**
   *  Get All instances that intersect current msg, 3 types of intersecting - STRAIGHTEN, PRESERVE VERTICAL, PREVERSE SLOPE
   *  returns vector of pointers to instances, if zero, there are no intersecting instances
   */
  static std::vector<Visio::IVShapePtr> getIntersectInstances(Visio::IVShapePtr msgShape, double msgPosY, MsgSnapType snapType);
  /**
   *  returns two closest instances for given message (first of the pair is instance on the left from the message, second is ono the right)
   */
  static std::pair<Visio::IVShapePtr, Visio::IVShapePtr> getClosestInstancePair(Visio::IVShapePtr msgShape, double msgSnapPointX, double msgSnapPointY, 
                                                                                const std::vector<Visio::IVShapePtr>& instances, MsgSnapType snapType);
  /**
   *  Glue given message on the given instances
   *  If leftInstance or rightInstance is NULL, message will be connected only to not null one
   *  designed only for horizontal messages (diagonal messages will be snapped according their PinX point and reformed to horizontal messages)
   *  @param onlyOnInstanceLine if true avoid connection message on instace's "rectangles" on the top and the bottom
   */
  static MsgConnectedEndpoints glueMsgToInstancesPair(Visio::IVShapePtr msgShape, Visio::IVShapePtr leftInstance, Visio::IVShapePtr rightInstance, 
                                     double yPos, MsgSnapType snapType, bool dontGlueIfConnected = true);
  /**
   *  Glue given message to instance, but only to "line" of instance, rectangles at the bottom and top are ignored
   *  @param msgCell            message cell which should be glued (e.g. BeginX, EndX)
   *  @param yPos               vertical position where to glue message to instance (in page units - visPageUnits)
   */
  static bool glueMsgToInstance(Visio::IVShapePtr msgShape, Visio::IVShapePtr instanceShape, const _bstr_t & msgCell, double yPos, 
                                VisUnitCodes units = visPageUnits);
  static bool glueMsgToInstance(Visio::IVShapePtr msgShape, Visio::IVShapePtr instanceShape, const _bstr_t & msgCell, double yPos, 
                                MsgDirection msgDirection, bool dontGlueIfConnected = true, VisUnitCodes units = visPageUnits);
  /**
   *
   */
  static MscPoint* getIntersectionWithInstance(Visio::IVShapePtr msgShape, Visio::IVShapePtr instanceShape, VisUnitCodes units = visPageUnits);
  static MscPoint* getIntersectionWithInstance(const MscPoint& beginPoint,const MscPoint& endPoint, 
                                               Visio::IVShapePtr instanceShape, VisUnitCodes units = visPageUnits);
  /**
   *
   */
  static bool isPointOnInstancesLine(const MscPoint& point, Visio::IVShapePtr instShape, VisUnitCodes units = visPageUnits);
  /**
   * Get Directional vector of given message shape
   */
  static bool getMsgVector(Visio::IVShapePtr msgShape, MscPoint* resultVector, VisUnitCodes units = visMillimeters);

  /**
   *  automatically snaps current message to nearest instances
   *  @param msgShape a shape to snap
   *  @param posX     MouseX position for closest instances computing
   *  @param posY     MouseY position for closest instances computing
   */
  static void snap(Visio::IVShapePtr msgShape, double posX, double posY, MsgSnapType snapType);
  /**
   * Snap message to instaces whether its enpoints are on the instance's line
   */
  static bool resnap(Visio::IVShapePtr msgShape, Visio::IVShapesPtr shapes, double precision = 0.0000001);  
  /**
   * Get all instances the given message is connected to
   */
  static std::vector<Visio::IVShapePtr> getConnectedInstances(Visio::IVShapePtr msgShape);
  /**
   *
   */
  static bool pointsEqual(const MscPoint& point1, const MscPoint& point2, double precision = 0.0000001);
  /**
   *
   */
  static Visio::IVShapePtr getCoregionAt(Visio::IVShapePtr instShape, double yPos, VisUnitCodes units = visPageUnits);
  /**
   *
   */
  static Visio::IVShapePtr snapEndPointToClosestInstance(Visio::IVShapePtr msgShape, _bstr_t endPointX, MsgDirection direction);
  /**
   *  Set length of message given by direction and length of msgVector
   *  @param msgVector     Vector must be in milimeters!!!
   *  @param fixedColumnX  Fixed column (vis1DBeginX, vis1DEndX) stays fixed and other side of message will be prolonged
   */
  static void setMsgLength(Visio::IVShapePtr msgShape, MscPoint msgVector, VisCellIndices fixedColumnX);
  /**
   *
   */
  static MscPoint* getIntersectionWithCoregion( MscPoint beginPoint, MscPoint endPoint, 
                                          Visio::IVShapePtr coregionShape, VisUnitCodes units = visPageUnits);

private:
  /**
   *  Function used to determine, whether message should be connected to coregion at given yPos,
   *  because oblique coregions have some "dead zones", where message can't be glued
   *  @param  instanceShape instance shape, which can contains coregion(s)
   *  @param  xPos          sets xPos on the coregion to snap to (zero (left side) or one (right side))
   *  @param  yPos          gets yPos where messsage should be snapped
   */
  static bool coregionTreatment(Visio::IVShapePtr& instanceShape, MscPoint& instBegin, MscPoint& instEnd, 
                                MsgDirection posToCoregion, bool& xPos, const double& yPos, VisUnitCodes units = visPageUnits);
};

// $Id: messageSnapping.h 952 2010-09-16 21:53:03Z mbezdeka $