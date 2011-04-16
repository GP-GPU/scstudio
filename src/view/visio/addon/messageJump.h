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
* $Id: messageJump.h 973 2010-09-18 17:43:26Z mbezdeka $
*/

#pragma once

#include "enums.h"
#include "shapeUtils.h"
#include "visualize.h"

//! Class enables messages to jump between instances (coregions)
class CMessageJump
{
public:
  static bool isEnabled();
  static int getEdgeInstanceTreatment();
  /**
   * Jumps with given message to the left or to the right
   * @param msgShape            a message which we wnat to jump with
   * @param connectedInstances  instances the message is currently connected to
   * @param direction           jump direction (MSJUMP_LEFT, MSJUMP_RIGHT)
   * @param asCopy              creates message duplicate and then jump with duplicate
   * @return Returns true if jump succeeded, false otherwise
   */  
  static bool jump(Visio::IVShapePtr& msgShape, std::vector<Visio::IVShapePtr> connectedInstances, MsgJumpDirection direction, bool asCopy);
  /**
   * Finds closest instance from shape in horizontal direction
   */
  static Visio::IVShapePtr getClosestInstanceFromShape(const MscPoint& pointOnShape, Visio::IVShapePtr shape, 
    std::vector<Visio::IVShapePtr> instances, MsgJumpDirection direction);
  /**
   * 
   */
  static bool getMsgNeedsResnap(Visio::IVShapePtr msgShape);
  /**
   * 
   */
  static void setMsgNeedsResnap(Visio::IVShapePtr msgShape, bool bTrueIfNeeds);
  /**
   * Changes message's type and deletes previous one
   * @param msgShape  a message which type we want to change
   * @param shapeType type of a new message (ST_BMSC_MESSAGE, ST_BMSC_MESSAGE_FOUND, ST_BMSC_MESSAGE_LOST)
   * @return Returns true if change succeeded, false otherwise
   */
  static bool changeMsgType(Visio::IVShapePtr& msgShape, TShapeType shapeType);
};

// $Id: messageJump.h 973 2010-09-18 17:43:26Z mbezdeka $