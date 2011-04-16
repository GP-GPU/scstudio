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
* $Id: messageJump.cpp 973 2010-09-18 17:43:26Z mbezdeka $
*/

#include "stdafx.h"
#include "dllmodule.h"
#include "messageJump.h"
#include "messageSnapping.h"

#define SNAP_REG_PATH _T("Software\\Sequence Chart Studio\\MessageSnapping")

bool CMessageJump::isEnabled()
{
  return GetRegistry<bool>(SNAP_REG_PATH,  NULL, _T("EdgeTreatmentEnabled"), false);
}
int CMessageJump::getEdgeInstanceTreatment()
{
  return GetRegistry<int>(SNAP_REG_PATH,  NULL, _T("EdgeTreatmentType"), 0);
}

bool CMessageJump::jump(Visio::IVShapePtr& msgShape, std::vector<Visio::IVShapePtr> connectedInstances, MsgJumpDirection direction, bool asCopy)
{
  if(!msgShape || !connectedInstances.size())
    return false;

  std::vector<Visio::IVShapePtr> instances = connectedInstances;
  bool left = (CShapeUtils::getMsgDirection(msgShape) == MSG_LEFT);
  
  Visio::IVShapePtr origIntsLeft = NULL;
  Visio::IVShapePtr origIntsRight = NULL;

  switch(instances.size())
  {
  case 1:
    {
    MscPoint* intPoint = CMessageSnapping::getIntersectionWithInstance(msgShape, instances.at(0));
    if(intPoint && (CShapeUtils::getShapeCell(msgShape, "PinX") < intPoint->get_x()))
      origIntsRight = instances.at(0);
    else
      origIntsLeft = instances.at(0);
    delete intPoint;
    }
    break;

  case 2:
    origIntsLeft = instances.at(0);
    origIntsRight = instances.at(1);
    if(left)
      CShapeUtils::swapShape(origIntsLeft, origIntsRight);
    break;

  default:
    return false;
  }

  //UNDONE: Copy message when jumping isn't implemented yet
  //Duplicate message (creates a new instance of shape)
  /*if(asCopy)
  {
    msgShape->Application->EventsEnabled = false;
    msgShape = CShapeUtils::duplicateShape(msgShape, get_shape_type(msgShape));
    msgShape->Application->EventsEnabled = true;
  */

  //Erase information about re-snaping
  if(origIntsLeft && origIntsRight && getMsgNeedsResnap(msgShape))
    setMsgNeedsResnap(msgShape, 0);

  std::vector<Visio::IVShapePtr> intersIntsBegin, intersIntsEnd;
  intersIntsBegin = CMessageSnapping::getIntersectInstances(msgShape, CShapeUtils::getShapeCell(msgShape,left ? "EndY" : "BeginY"), MSSNAP_STRAIGHTEN);
  intersIntsEnd = CMessageSnapping::getIntersectInstances(msgShape, CShapeUtils::getShapeCell(msgShape,left ? "BeginY" : "EndY"), MSSNAP_STRAIGHTEN);

#define _s(x) CShapeUtils::getShapeCell(msgShape,x)
  MscPoint begin(_s("BeginX"),_s("BeginY")), end(_s("EndX"),_s("EndY"));
  Visio::IVShapePtr closestLeft = getClosestInstanceFromShape(left ? end : begin, origIntsLeft, intersIntsBegin, direction);
  Visio::IVShapePtr closestRight = getClosestInstanceFromShape(left ? begin : end, origIntsRight, intersIntsEnd, direction);
#undef _s

  MscPoint msgVec;
  CMessageSnapping::getMsgVector(msgShape, &msgVec);
  if(left)
    CShapeUtils::swapShape(closestLeft, closestRight);

  //Edge treatment
  double PinY = CShapeUtils::getShapeCell(msgShape, "PinY");
  TShapeType shapeType = closestLeft ? ST_BMSC_MESSAGE_LOST : ST_BMSC_MESSAGE_FOUND;
  if((closestLeft ^ closestRight) && origIntsLeft && origIntsRight) //NOTE: new instance is only one, previous instances were two
  {
    if(!isEnabled()) // If Edge treatment isn't enabled we are done and Message won't be snapped to any new instance
      return false; 
    changeMsgType(msgShape, (getEdgeInstanceTreatment() == 1) ? shapeType : ST_BMSC_MESSAGE);
  }

  //Glue messages to instances
  MsgConnectedEndpoints conn = CMessageSnapping::glueMsgToInstancesPair(msgShape, closestLeft, closestRight, PinY, MSSNAP_PRESERVE_VERTICAL, false);

  //Preserve its length (If message is snapped only to one instance, preserve its length)
  switch(conn)
  {
  case MSCE_BEGIN: //LEFT point is snapped, adjust right point
  case MSCE_END: //RIGHT point is snapped, adjust left point
    if(origIntsLeft && origIntsRight)
      changeMsgType(msgShape, (getEdgeInstanceTreatment() == 1) ? shapeType : ST_BMSC_MESSAGE);
    CMessageSnapping::setMsgLength(msgShape, msgVec, (conn == MSCE_BEGIN) ? vis1DBeginX : vis1DEndX);
    if(origIntsLeft && origIntsRight)
      CMessageSnapping::resnap(msgShape, msgShape->Application->ActivePage->Shapes, 0.0001);

    if(getMsgNeedsResnap(msgShape) == 0)
      setMsgNeedsResnap(msgShape, true);
    break;
  case MSCE_NONE:
    return false;
  }

  return true;
}

Visio::IVShapePtr CMessageJump::getClosestInstanceFromShape(const MscPoint& pointOnShape, Visio::IVShapePtr shape, std::vector<Visio::IVShapePtr> instances, MsgJumpDirection direction)
{
  if(!shape)
    return NULL;

  if(get_shape_type(shape) == ST_BMSC_COREGION)
    shape = shape->Connects->ToSheet;
    
  Visio::IVShapePtr closestInstance = NULL;
  double closestDistance = 0;
  for(std::vector<Visio::IVShapePtr>::iterator it = instances.begin(); it != instances.end(); it++)
  {
    if(*it == shape)
      continue;
  
    boost::shared_ptr<MscPoint> intPoint (CMessageSnapping::getIntersectionWithInstance(MscPoint(pointOnShape.get_x()-1, pointOnShape.get_y()), 
                                                                                MscPoint(pointOnShape.get_x(), pointOnShape.get_y()), *it));
    if(!intPoint || !CMessageSnapping::isPointOnInstancesLine(*intPoint, *it))
      continue;
    if(CMessageSnapping::pointsEqual(*intPoint, pointOnShape, 0.001))
      return *it;

    double xVector = pointOnShape.get_x() - intPoint->get_x();
    if((xVector * ((direction == MSJUMP_LEFT) ? 1 : -1)) < 0)
      continue;

    double distance = sqrt(pow(xVector,2) + pow((pointOnShape.get_y() - intPoint->get_y()),2));    

    if(!closestInstance || distance <= closestDistance)
    {
      closestDistance = distance;
      closestInstance = *it;
    }
  }
  return closestInstance;
}

bool CMessageJump::getMsgNeedsResnap(Visio::IVShapePtr msgShape)
{
  if(!msgShape->GetCellExists(_T("User.Resnap"),0))
    return false;
  if(msgShape->Cells["User.Resnap"]->ResultInt[visNone][visTruncate] != 1)
    return false;

  return true;
}

void CMessageJump::setMsgNeedsResnap(Visio::IVShapePtr msgShape, bool bTrueIfNeeds)
{
  bool bCellExists = (msgShape->GetCellExists(_T("User.Resnap"),0) == -1);

  if(bTrueIfNeeds && !bCellExists)
  {
    msgShape->AddNamedRow(visSectionUser, _T("Resnap"), 0);
    msgShape->Cells["User.Resnap"]->FormulaU = _T("1");
  }
  else if(!bTrueIfNeeds && bCellExists)
    msgShape->DeleteRow(visSectionUser,msgShape->CellsRowIndex["User.Resnap"]);
}

bool CMessageJump::changeMsgType(Visio::IVShapePtr& msgShape, TShapeType shapeType)
{
  if(!msgShape || !isMessageShape(msgShape))
    return false;

  msgShape->Application->EventsEnabled = false;
  Visio::IVShapePtr newShape = CShapeUtils::duplicateShape(msgShape, shapeType);
  msgShape->Delete();
  msgShape = newShape;
  msgShape->Application->EventsEnabled = true;

  return true;
}

// $Id: messageJump.cpp 973 2010-09-18 17:43:26Z mbezdeka $